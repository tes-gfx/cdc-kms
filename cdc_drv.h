/*
 * cdc_drv.h  --  CDC Display Controller DRM driver
 *
 * Copyright (C) 2016 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __CDC_DRV_H__
#define __CDC_DRV_H__
#include <linux/kernel.h>
#include <video/videomode.h>
#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <cdc.h>

struct cdc_device;
struct cdc_format;
struct drm_pending_vblank_event;
struct drm_fbdev_cma;
struct altera_pll;

struct cdc_plane {
  struct drm_plane plane;
  struct cdc_device *cdc;
  int hw_idx;
  bool enabled;
  bool used;

  cdc_uint8 alpha;
};


struct cdc_device {
  struct drm_crtc crtc;
  struct device *dev;
  struct drm_device *ddev;

  void __iomem *mmio;

  cdc_handle drv;
  cdc_global_config config;
  int num_layer;
  unsigned int max_pitch;
  unsigned int max_width;
  unsigned int max_height;

  struct clk *pclk;
  struct drm_pending_vblank_event *event;
  wait_queue_head_t flip_wait;
  struct drm_fbdev_cma *fbdev;
  struct cdc_plane *planes;

  int dpms;
  bool wait_for_vblank;
  bool early_poll; // did a poll occur before FBDEV was setup?
  bool enabled;
  bool started;
  bool neg_blank;
  bool neg_pixclk;

  bool fifo_underrun;

  // plane properties
  struct drm_property *alpha;

  struct {
    wait_queue_head_t wait;
    u32 pending;
  } commit;

  /* FIXME HACK for MesseDemo */
  unsigned int irq_stat;
  spinlock_t irq_slck;
  wait_queue_head_t irq_waitq;
};

#endif
