/*
 * cdc_drv.c  --  CDC Display Controller DRM driver
 *
 * Copyright (C) 2016 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/io.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>

#include <linux/seq_file.h>

#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_gem_cma_helper.h>


#include <cdc.h>
#include <cdc_base.h>

#include "cdc_drv.h"
#include "cdc_kms.h"
#include "cdc_crtc.h"
#include "altera_pll.h"

static const struct platform_device_id cdc_id_table[] = {
  { "cdc", 0 },
  { }
};

MODULE_DEVICE_TABLE(platform, cdc_id_table);

static const struct of_device_id cdc_of_table[] = {
  { .compatible = "tes,cdc-2.1", .data = NULL },
  { }
};

MODULE_DEVICE_TABLE(of, cdc_of_table);

/* configuration dependent */
static void cdc_init_dev(struct cdc_device *cdc) {
  cdc->max_width = 2047;
  cdc->max_height = 2047;
  cdc->max_pitch = 8192;

  /* FIXME HACK for MesseDemo   */
  spin_lock_init(&cdc->irq_slck);
  init_waitqueue_head(&cdc->irq_waitq);
}

static int cdc_unload(struct drm_device *dev) {
  struct cdc_device *cdc = dev->dev_private;

  dev_dbg(cdc->dev, "%s\n", __func__);

  // Disable VBLANK in DRM subsystem and hardware
  drm_crtc_vblank_off(&cdc->crtc);
  cdc_crtc_set_vblank(cdc, false);

  if(cdc->fbdev) {
    drm_fbdev_cma_fini(cdc->fbdev);
  }

  drm_kms_helper_poll_fini(dev);
  drm_mode_config_cleanup(dev);
  drm_vblank_cleanup(dev);

  dev->irq_enabled = 0;
  dev->dev_private = NULL;

  cdc_exit(cdc->drv);

  return 0;
}


static void cdc_clock_init(struct cdc_device *cdc)
{
  struct device_node *np = cdc->dev->of_node;
  struct device_node *clk_node;

  clk_node = of_parse_phandle(np, "pixel-clock", 0);
  if(!clk_node)
  {
    dev_err(cdc->dev, "Could not find pixel clock PLL\n");
  }

  cdc->pclk = altera_pll_clk_create(cdc->dev, clk_node);
}


static int cdc_load(struct drm_device *dev, unsigned long flags)
{
  struct platform_device *pdev = dev->platformdev;
  struct device_node *np = pdev->dev.of_node;
  struct cdc_device *cdc;
  struct resource *mem;
  int ret = 0;

  if(np == NULL) {
    dev_err(dev->dev, "no platform data\n");
    return -ENODEV;
  }

  cdc = devm_kzalloc(&pdev->dev, sizeof(*cdc), GFP_KERNEL);
  if(cdc == NULL)
  {
    dev_err(dev->dev, "failed to allocate driver data\n");
    return -ENOMEM;
  }

  init_waitqueue_head(&cdc->commit.wait);

  cdc->dev = &pdev->dev;
  cdc->ddev = dev;
  dev->dev_private = cdc;

  cdc_init_dev(cdc);

  mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  cdc->mmio = devm_ioremap_resource(&pdev->dev, mem);
  dev_dbg(&pdev->dev, "Mapped IO from 0x%x to 0x%p\n", mem->start, cdc->mmio);
  if(IS_ERR(cdc->mmio))
  {
    return PTR_ERR(cdc->mmio);
  }

  cdc_crtc_set_vblank(cdc, false);
  cdc->drv = cdc_init(cdc);
  cdc->config = cdc_getGlobalConfig(cdc->drv);
  cdc->num_layer = cdc_getLayerCount(cdc->drv);

  dev_dbg(&pdev->dev, "Found %d layer(s)\n", cdc->num_layer);

  ret = drm_vblank_init(dev, 1);
  if(ret < 0)
  {
    dev_err(&pdev->dev, "failed to initialize vblank\n");
    goto done;
  }

  cdc_clock_init(cdc);

  ret = cdc_modeset_init(cdc);
  if(ret < 0)
  {
    dev_err(&pdev->dev, "failed to initialize CDC Modeset\n");
    goto done;
  }

  dev->irq_enabled = 1;

  platform_set_drvdata(pdev, cdc);

done:
  if(ret)
    cdc_unload(dev);

  return ret;
}


static void cdc_preclose(struct drm_device *dev, struct drm_file *file)
{
  struct cdc_device *cdc = dev->dev_private;

  cdc_crtc_cancel_page_flip(&cdc->crtc, file);
}


static void cdc_lastclose(struct drm_device *dev)
{
  struct cdc_device *cdc = dev->dev_private;

  drm_fbdev_cma_restore_mode(cdc->fbdev);
}


static int cdc_enable_vblank(struct drm_device *dev, int crtc)
{
  struct cdc_device *cdc = dev->dev_private;
  cdc_crtc_set_vblank(cdc, true);

  return 0;
}

static void cdc_disable_vblank(struct drm_device *dev, int crtc)
{
  struct cdc_device *cdc = dev->dev_private;
  cdc_crtc_set_vblank(cdc, false);
}


#ifdef CONFIG_DEBUG_FS
static int cdc_regs_show(struct seq_file *m, void *arg)
{
        struct drm_info_node *node = (struct drm_info_node *) m->private;
        struct drm_device *dev = node->minor->dev;
        struct cdc_device *cdc = dev->dev_private;
        cdc_context *context;
        unsigned i;

        context = (cdc_context*) cdc->drv;

        pm_runtime_get_sync(dev->dev);

        /* Show the first few registers */
        // todo: make this dependent on layer count or device memory range
        for(i=0; i<(0x20 + 0x20*2); i += 4)
        {
          u32 reg = cdc_arch_readReg(context, i);
          seq_printf(m, "%03x: %08x", i * 4, reg);
          reg = cdc_arch_readReg(context, i+1);
          seq_printf(m, " %08x",   reg);
          reg = cdc_arch_readReg(context, i+2);
          seq_printf(m, " %08x",   reg);
          reg = cdc_arch_readReg(context, i+3);
          seq_printf(m, " %08x\n", reg);
        }

        pm_runtime_put_sync(dev->dev);

        return 0;
}

static int cdc_dump_fb(struct seq_file *m, void *arg)
{
  struct drm_info_node *node = (struct drm_info_node *) m->private;
  struct drm_device *dev = node->minor->dev;
  struct drm_framebuffer *fb;
  int ret;
  int i = 0;

  ret = mutex_lock_interruptible(&dev->mode_config.mutex);
  if(ret)
          return ret;

  ret = mutex_lock_interruptible(&dev->struct_mutex);
  if(ret) {
          mutex_unlock(&dev->mode_config.mutex);
          return ret;
  }

  list_for_each_entry(fb, &dev->mode_config.fb_list, head)
  {
    struct drm_gem_cma_object *obj = drm_fb_cma_get_gem_obj(fb, 0);

    if(i==1)
    {
      seq_write(m, obj->vaddr, 800*600*4); /* FIXME: static size, bad hack! */
    }
    ++i;
  }

  mutex_unlock(&dev->struct_mutex);
  mutex_unlock(&dev->mode_config.mutex);

  return 0;
}

static int cdc_mm_show(struct seq_file *m, void *arg)
{
  struct drm_info_node *node = (struct drm_info_node *) m->private;
  struct drm_device *dev = node->minor->dev;
  return drm_mm_dump_table(m, &dev->vma_offset_manager->vm_addr_space_mm);
}

static struct drm_info_list cdc_debugfs_list[] = {
  { "regs",   cdc_regs_show, 0 },
  { "mm",     cdc_mm_show,   0 },
  { "fb",     drm_fb_cma_debugfs_show, 0 },
  { "fbdump", cdc_dump_fb, 0 },
};

static int cdc_debugfs_init(struct drm_minor *minor)
{
  struct drm_device *dev = minor->dev;
  int ret;

  ret = drm_debugfs_create_files(cdc_debugfs_list,
                  ARRAY_SIZE(cdc_debugfs_list),
                  minor->debugfs_root, minor);

  if(ret) {
    dev_err(dev->dev, "could not install cdc_debugfs_list\n");
    return ret;
  }

  return ret;
}

static void cdc_debugfs_cleanup(struct drm_minor *minor)
{
  drm_debugfs_remove_files(cdc_debugfs_list,
                  ARRAY_SIZE(cdc_debugfs_list), minor);
}
#endif


#include "cdc_ioctl.h"

long cdc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  struct drm_file *file_priv = filp->private_data;
  struct drm_device *dev;
  struct cdc_device *cdc;
  char stack_data[128];
  unsigned int nr = HACK_IOCTL_NR(cmd);
  unsigned int size;

  dev = file_priv->minor->dev;
  cdc = dev->dev_private;

  if(nr >= 0xe0)
  {
    size = _IOC_SIZE(cmd);

    if(cmd & IOC_IN)
    {
      if(copy_from_user(stack_data, (void __user *) arg, size) != 0)
      {
        return -EFAULT;
      }
    }

    /* TODO: Dispatching */
    switch(nr)
    {
    case 0xe0:
    {
      struct hack_set_cb *set_cb = (struct hack_set_cb*) stack_data;

      cdc_layer_setCBAddress(cdc->drv, 0, (unsigned int) set_cb->phy_addr);
      cdc_layer_setCBSize(cdc->drv, 0, set_cb->width, set_cb->height, set_cb->pitch);
      cdc_triggerShadowReload(cdc->drv, CDC_TRUE);
      break;
    }

    case 0xe1:
    {
      struct hack_set_winpos *winpos = (struct hack_set_winpos*) stack_data;

      cdc_layer_setWindow(cdc->drv, 0, winpos->x, winpos->y, winpos->width, winpos->height, winpos->width*4);
      cdc_layer_setEnabled(cdc->drv, 0, CDC_TRUE);
      cdc_triggerShadowReload(cdc->drv, CDC_TRUE);
      break;
    }

    case 0xe2:
    {
      struct hack_set_alpha *alpha = (struct hack_set_alpha*) stack_data;

      cdc_layer_setBlendMode(cdc->drv, 0, CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA, CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA_INV);
      cdc_layer_setConstantAlpha(cdc->drv, 0, alpha->alpha);
      cdc_triggerShadowReload(cdc->drv, CDC_TRUE);
      break;
    }

    case 0xe3:
    {
      unsigned long flags;

      spin_lock_irqsave(&cdc->irq_slck, flags);
      cdc->irq_stat = 0;
      spin_unlock_irqrestore(&cdc->irq_slck, flags);

      wait_event_interruptible(cdc->irq_waitq, cdc->irq_stat);

      break;
    }

    default:
      printk(KERN_ERR "Unknown IOCTL (nr = %u)!\n", nr);
    }

    return 0;
  }

  return drm_ioctl(filp, cmd, arg);
}


static const struct file_operations cdc_fops = {
  .owner          = THIS_MODULE,
  .open           = drm_open,
  .release        = drm_release,
  .unlocked_ioctl = cdc_ioctl,
#ifdef CONFIG_COMPAT
  .compat_ioctl   = drm_compat_ioctl,
#endif
  .poll           = drm_poll,
  .read           = drm_read,
  .llseek         = no_llseek,
  .mmap           = drm_gem_cma_mmap,
};

static struct drm_driver cdc_driver = {
  .driver_features           = DRIVER_GEM | DRIVER_MODESET | DRIVER_PRIME | DRIVER_ATOMIC,
  .load                      = cdc_load,
  .unload                    = cdc_unload,
  .preclose                  = cdc_preclose,
  .lastclose                 = cdc_lastclose,
  .set_busid                 = drm_platform_set_busid,
  .get_vblank_counter        = drm_vblank_count,
  .enable_vblank             = cdc_enable_vblank,
  .disable_vblank            = cdc_disable_vblank,
  .gem_free_object           = drm_gem_cma_free_object,
  .prime_handle_to_fd        = drm_gem_prime_handle_to_fd,
  .prime_fd_to_handle        = drm_gem_prime_fd_to_handle,
  .gem_prime_import          = drm_gem_prime_import,
  .gem_prime_export          = drm_gem_prime_export,
  .gem_prime_get_sg_table    = drm_gem_cma_prime_get_sg_table,
  .gem_prime_import_sg_table = drm_gem_cma_prime_import_sg_table,
  .gem_prime_vmap            = drm_gem_cma_prime_vmap,
  .gem_prime_vunmap          = drm_gem_cma_prime_vunmap,
  .gem_prime_mmap            = drm_gem_cma_prime_mmap,
  .gem_vm_ops                = &drm_gem_cma_vm_ops,
  .dumb_create               = drm_gem_cma_dumb_create,
  .dumb_map_offset           = drm_gem_cma_dumb_map_offset,
  .dumb_destroy              = drm_gem_dumb_destroy,
#ifdef CONFIG_DEBUG_FS
  .debugfs_init              = cdc_debugfs_init,
  .debugfs_cleanup           = cdc_debugfs_cleanup,
#endif
  .fops  = &cdc_fops,
  .name  = "tes-cdc",
  .desc  = "TES CDC Display Controller",
  .date  = "20160303",
  .major = 1,
  .minor = 0,
};

static int cdc_pm_suspend(struct device *dev) {
  struct cdc_device *cdc = dev_get_drvdata(dev);

  dev_dbg(dev, "%s\n", __func__);

  /* TODO: Suspend */
  drm_kms_helper_poll_disable(cdc->ddev);

  return 0;
}

static int cdc_pm_resume(struct device *dev) {
  struct cdc_device *cdc = dev_get_drvdata(dev);

  dev_dbg(dev, "%s\n", __func__);

  /* TODO: Resume */
  drm_kms_helper_poll_enable(cdc->ddev);

  return 0;
}

static const struct dev_pm_ops cdc_pm_ops = {
  SET_SYSTEM_SLEEP_PM_OPS(cdc_pm_suspend, cdc_pm_resume)
};

static int cdc_probe(struct platform_device *pdev) {
  return drm_platform_init(&cdc_driver, pdev);
}

static int cdc_remove(struct platform_device *pdev) {
  struct cdc_device *cdc = platform_get_drvdata(pdev);

  drm_put_dev(cdc->ddev);
  return 0;
}

static struct platform_driver cdc_platform_driver = {
  .probe = cdc_probe,
  .remove = cdc_remove,
  .driver = {
    .name = "tes-cdc",
    .pm = &cdc_pm_ops,
    .of_match_table = cdc_of_table,
  },
  .id_table = cdc_id_table,
};

module_platform_driver(cdc_platform_driver);

MODULE_AUTHOR("Christian Thaler <christian.thaler@tes-dst.com>");
MODULE_DESCRIPTION("TES CDC Display Controller DRM Driver");
MODULE_LICENSE("GPL");

