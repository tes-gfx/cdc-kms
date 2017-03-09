/*
 * cdc_hw.c  --  CDC Display Controller Hardware Interface
 *
 * Copyright (C) 2016 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/slab.h>
#include "cdc_drv.h"
#include "cdc_crtc.h"
#include <cdc_base.h>


u32 cdc_read_reg(struct cdc_device *cdc, u32 reg)
{
  return ioread32(cdc->mmio + (reg*4));
}


void cdc_write_reg(struct cdc_device *cdc, u32 reg, u32 val)
{
  iowrite32(val, cdc->mmio + (reg*4));
}


static int layer_offset(int layer)
{
  return (layer + 1) * CDC_OFFSET_LAYER;
}


u32 cdc_read_layer_reg(struct cdc_device *cdc, int layer, u32 reg)
{
  return cdc_read_reg(cdc, layer_offset(layer) + reg);
}


void cdc_write_layer_reg(struct cdc_device *cdc, int layer, u32 reg, u32 val)
{
	cdc_write_reg(cdc, layer_offset(layer) + reg, val);
}


static u32 read_reg(struct cdc_device *cdc, u32 reg)
{
  return ioread32(cdc->mmio + (reg*4));
}


static void write_reg(struct cdc_device *cdc, u32 reg, u32 val)
{
  iowrite32(val, cdc->mmio + (reg*4));
}


cdc_bool cdc_arch_init(cdc_context *context, cdc_platform_settings a_platform)
{
  struct cdc_device *cdc = (struct cdc_device *)(a_platform);
  context->m_platform = cdc;
  return CDC_TRUE;
}


void cdc_arch_exit(cdc_context* a_base) {
}


cdc_uint32 cdc_arch_readReg(cdc_context *a_context, cdc_uint32 a_regAddress)
{
  struct cdc_device *cdc = (struct cdc_device *)(a_context->m_platform);
  cdc_uint32 val;

  val = read_reg(cdc, a_regAddress);

  return val;
}


void cdc_arch_writeReg(cdc_context *a_context, cdc_uint32 a_regAddress, cdc_uint32 a_value)
{
  struct cdc_device *cdc = (struct cdc_device *)(a_context->m_platform);

  write_reg(cdc, a_regAddress, a_value);
}


void cdc_irq_set(struct cdc_device *cdc, cdc_irq_type irq, bool enable)
{
  u32 status;

  status = read_reg(cdc, CDC_REG_GLOBAL_IRQ_ENABLE);

  if(enable)
    status |= irq;
  else
    status &= ~(irq);

  write_reg(cdc, CDC_REG_GLOBAL_IRQ_ENABLE, status);
}


static irqreturn_t cdc_irq(int irq, void *arg)
{
  struct cdc_device *cdc = (struct cdc_device *)arg;
  u32 status;

  status = read_reg(cdc, CDC_REG_GLOBAL_IRQ_STATUS);
  write_reg(cdc, CDC_REG_GLOBAL_IRQ_CLEAR, status);

  if(status & CDC_IRQ_LINE)
  {
    cdc_crtc_irq(&cdc->crtc);
  }
  if(status & CDC_IRQ_BUS_ERROR)
  {
    dev_err(cdc->dev, "BUS error IRQ triggered\n");
  }
  if(status & CDC_IRQ_FIFO_UNDERRUN)
  {
    // disable underrun IRQ to prevent message flooding
    cdc_irq_set(cdc, CDC_IRQ_FIFO_UNDERRUN, false);

    dev_err(cdc->dev, "FIFO underrun\n");
  }
  if(status & CDC_IRQ_SLAVE_TIMING_NO_SIGNAL)
  {
    dev_err(cdc->dev, "SLAVE no signal\n");
  }
  if(status & CDC_IRQ_SLAVE_TIMING_NO_SYNC)
  {
    dev_err(cdc->dev, "SLAVE no sync\n");
  }

  return IRQ_HANDLED;
}


cdc_bool cdc_arch_initIRQ(cdc_context *a_context)
{
  struct cdc_device *cdc = (struct cdc_device *)(a_context->m_platform);
  struct platform_device *pdev = to_platform_device(cdc->dev);

  int irq;
  int ret;
  irq = platform_get_irq(pdev, 0);

  if(irq < 0) {
    dev_err(cdc->dev, "Could not get platform IRQ number\n");
    return CDC_FALSE;
  }

  ret = devm_request_irq(cdc->dev, irq, cdc_irq, 0, dev_name(cdc->dev), cdc);
  if(ret < 0) {
    dev_err(cdc->dev, "Failed to register IRQ\n");
    return CDC_FALSE;
  }
  return CDC_TRUE;
}


void cdc_arch_deinitIRQ(cdc_context *a_context)
{

}


cdc_ptr cdc_arch_malloc(cdc_uint32 a_size)
{
  return kzalloc(a_size, GFP_KERNEL);
}


void cdc_arch_free(cdc_ptr a_ptr)
{
  kfree(a_ptr);
}


cdc_uint32 cdc_arch_readLayerReg(cdc_context *a_context, cdc_uint8 a_layer,
    cdc_uint32 a_regAddress)
{
  return cdc_arch_readReg(a_context, layer_offset(a_layer) + a_regAddress);
}


void cdc_arch_writeLayerReg(cdc_context *a_context, cdc_uint8 a_layer,
    cdc_uint32 a_regAddress, cdc_uint32 a_value)
{
  return cdc_arch_writeReg(a_context, layer_offset(a_layer) + a_regAddress, a_value);
}


cdc_bool cdc_arch_setPixelClk(cdc_uint32 a_clk)
{
  return CDC_FALSE;
}

