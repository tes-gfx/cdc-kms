/*
 * cdc_encoder.c  --  CDC Display Controller Encoder
 *
 * Copyright (C) 2017 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_atomic_helper.h>

#include "cdc_regs.h"
#include "cdc_drv.h"
#include "cdc_kms.h"
#include "cdc_lvdscon.h"
#include "cdc_encoder.h"
#include "cdc_hdmienc.h"
#include "cdc_hdmicon.h"


struct drm_encoder * cdc_connector_best_encoder(struct drm_connector *connector)
{
  struct cdc_connector *con = to_cdc_connector(connector);
  struct cdc_device *cdc = con->cdc;

  dev_dbg(cdc->dev, "best encoder is at 0x%p\n", con->encoder);

  return cdc_encoder_to_drm_encoder(con->encoder);
}


static void cdc_encoder_enable(struct drm_encoder *encoder)
{
  struct cdc_device *cdc = encoder->dev->dev_private;
  dev_dbg(cdc->dev, "%s\n", __func__);
}


static void cdc_encoder_disable(struct drm_encoder *encoder)
{
  struct cdc_device *cdc = encoder->dev->dev_private;
  dev_dbg(cdc->dev, "%s\n", __func__);
}


static int cdc_encoder_atomic_check(struct drm_encoder *encoder,
                                    struct drm_crtc_state *crtc_state,
                                    struct drm_connector_state *conn_state)
{
  return 0;
}


static const struct drm_encoder_helper_funcs encoder_helper_funcs = {
    .enable       = cdc_encoder_enable,
    .disable      = cdc_encoder_disable,
    .atomic_check = cdc_encoder_atomic_check,
};


static const struct drm_encoder_funcs encoder_funcs = {
  .destroy = drm_encoder_cleanup,
};


int cdc_encoder_init(struct cdc_device *cdc, u32 enc_type, struct device_node * enc_node, struct device_node *con_node)
{
  struct cdc_encoder *enc;
  struct drm_encoder *encoder;
  int ret = 0;

  enc = devm_kzalloc(cdc->dev, sizeof(*enc), GFP_KERNEL);
  if(enc == NULL)
          return -ENOMEM;

  encoder = cdc_encoder_to_drm_encoder(enc);

  if(enc_type == DRM_MODE_ENCODER_TMDS)
  {
    ret = cdc_hdmienc_init(cdc, enc, enc_node);
    if(ret < 0)
    {
      dev_err(cdc->dev, "Error initializing HDMI drm_encoder: %d\n", ret);
      goto done;
    }
  }
  else
  {
    ret = drm_encoder_init(cdc->ddev, encoder, &encoder_funcs, enc_type);
    if(ret < 0)
    {
      dev_err(cdc->dev, "Error initializing LVDS drm_encoder: %d\n", ret);
      goto done;
    }

    drm_encoder_helper_add(encoder, &encoder_helper_funcs);
  }

  switch(enc_type)
  {
  case DRM_MODE_ENCODER_LVDS:
    dev_dbg(cdc->dev, "Initializing LVDS connector...\n");
    ret = cdc_lvds_connector_init(cdc, enc, enc_node);
    break;
  case DRM_MODE_ENCODER_TMDS:
      dev_dbg(cdc->dev, "Initializing HDMI connector...\n");
      ret = cdc_hdmi_connector_init(cdc, enc);
      break;
  default:
      ret = -EINVAL;
      break;
  }

done:
  if(ret < 0) {
    if(encoder->name)
      encoder->funcs->destroy(encoder);
    devm_kfree(cdc->dev, encoder);
  }
  return ret;
}
