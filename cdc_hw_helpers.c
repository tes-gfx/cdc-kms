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

#include "cdc_hw_helpers.h"

#include "cdc_drv.h"
#include "cdc_hw.h"
#include <cdc_base.h>


static void updateBufferLength(struct cdc_device *cdc, int layer)
{
	u32 length;
	s32 pitch;
	u8 format_bpp;

	format_bpp = cdc_formats_bpp[cdc->planes[layer].pixel_format];
	length = cdc->planes[layer].window_width * format_bpp;

	pitch = cdc->planes[layer].fb_pitch;
	if (pitch == 0) pitch = length;
	length += cdc->hw.bus_width - 1; // add bus_width_in_bits - 1
	cdc_write_layer_reg(cdc, layer, CDC_REG_LAYER_CB_LENGTH, (((u32)pitch) << 16) | length );

	dev_info(cdc->dev, "[CDC] Updating buffer length: length=%u pitch=%d bpp=%u win_width=%u\n",
			length, pitch, format_bpp, cdc->planes[layer].window_width);
}


static void setEnabled(struct cdc_device *cdc, bool enable)
{
	u32 control;

	control = cdc_read_reg(cdc, CDC_REG_GLOBAL_CONTROL);
	if(enable)
	{
		cdc_write_reg(cdc, CDC_REG_GLOBAL_CONTROL, control|CDC_REG_GLOBAL_CONTROL_ENABLE);
	}
	else
	{
		cdc_write_reg(cdc, CDC_REG_GLOBAL_CONTROL, control&~CDC_REG_GLOBAL_CONTROL_ENABLE);
	}
}


void cdc_hw_setPixelFormat(struct cdc_device *cdc, int layer, u8 format)
{
	cdc->planes[layer].pixel_format = format;
	cdc_write_layer_reg(cdc, layer, CDC_REG_LAYER_PIXEL_FORMAT, format);
	updateBufferLength(cdc, layer);
}


void cdc_hw_setBlendMode(struct cdc_device *cdc, u8 layer, cdc_blend_factor a_factor1, cdc_blend_factor a_factor2)
{
	cdc_write_layer_reg(cdc, layer, CDC_REG_LAYER_BLENDING, (a_factor1<<8) | a_factor2);
}


void cdc_hw_setWindow(struct cdc_device *cdc, u8 layer, u16 startX, u16 startY, u16 width, u16 height, s16 pitch)
{
	u32 activeStartX;
	u32 activeStartY;

	dev_info(cdc->dev, "%s x=%u y=%u w=%u h=%u p=%d\n", __func__, startX, startY, width, height, pitch);

	//TODO range checks and truncation
	//TODO: check windowing ability
	activeStartY = cdc_read_reg(cdc, CDC_REG_GLOBAL_BACK_PORCH);
	activeStartX = activeStartY >> 16;
	activeStartY &= 0xffff;
	cdc->planes[layer].window_width = width;
	cdc_write_layer_reg(cdc, layer, CDC_REG_LAYER_WINDOW_H, ((startX + activeStartX + width)<<16) | (startX + activeStartX + 1));
	cdc->planes[layer].window_height = height;
	cdc_write_layer_reg(cdc, layer, CDC_REG_LAYER_WINDOW_V, ((startY + activeStartY + height)<<16) | (startY + activeStartY + 1));

	cdc->planes[layer].fb_pitch = pitch;
	cdc_write_layer_reg(cdc, layer, CDC_REG_LAYER_FB_LINES, height);

	updateBufferLength(cdc, layer);
}


void cdc_hw_setCBAddress(struct cdc_device *cdc, u8 layer, dma_addr_t address)
{
	//TODO: check address alignment
	cdc_write_layer_reg(cdc, layer, CDC_REG_LAYER_CB_START, address);
}


void cdc_hw_layer_setEnabled(struct cdc_device *cdc, u8 layer, bool enable)
{
	if(enable)
		cdc->planes[layer].control |= CDC_REG_LAYER_CONTROL_ENABLE;
	else
		cdc->planes[layer].control &= ~CDC_REG_LAYER_CONTROL_ENABLE;

	cdc_write_layer_reg(cdc, layer, CDC_REG_LAYER_CONTROL, cdc->planes[layer].control);
}


void cdc_hw_resetRegisters(struct cdc_device *cdc)
{
  u32 i;
  u32 control;
  u32 h_b_porch_accum;
  u32 v_b_porch_accum;
  u32 h_width_accum;
  u32 v_width_accum;

  // initialize cdc registers with default values, but keep timings
  // initialize global registers
  control = cdc_read_reg(cdc, CDC_REG_GLOBAL_CONTROL);
  control &= CDC_REG_GLOBAL_CONTROL_HSYNC|CDC_REG_GLOBAL_CONTROL_VSYNC|CDC_REG_GLOBAL_CONTROL_BLANK|CDC_REG_GLOBAL_CONTROL_CLK_POL;
  cdc_write_reg(cdc, CDC_REG_GLOBAL_CONTROL,             control);
  cdc_write_reg(cdc, CDC_REG_GLOBAL_BG_COLOR,            0);
  cdc_write_reg(cdc, CDC_REG_GLOBAL_IRQ_ENABLE,          0);
  h_b_porch_accum = cdc_read_reg(cdc, CDC_REG_GLOBAL_BACK_PORCH);
  v_b_porch_accum = h_b_porch_accum & 0xffff;
  h_b_porch_accum >>= 16;
  h_width_accum = cdc_read_reg(cdc, CDC_REG_GLOBAL_ACTIVE_WIDTH);
  v_width_accum = h_width_accum & 0xffff;
  h_width_accum >>= 16;

  cdc_write_reg(cdc, CDC_REG_GLOBAL_LINE_IRQ_POSITION, v_width_accum+1);
  cdc_write_reg(cdc, CDC_REG_GLOBAL_EXT_DISPLAY, 0);
  // note background layer registers are not initialized here as background layer is disabled by default

  // initialize per layer registers
  for(i=0; i<cdc->hw.layer_count;i++)
  {
    cdc_write_layer_reg(cdc, i,  CDC_REG_LAYER_CONTROL,      0);
    cdc->planes[i].control = 0;

    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_WINDOW_H, (h_width_accum<<16)|(h_b_porch_accum+1));
    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_WINDOW_V, (v_width_accum<<16)|(v_b_porch_accum+1));
    cdc->planes[i].window_width = h_width_accum - h_b_porch_accum; // active width of window
    cdc->planes[i].window_height = v_width_accum - v_b_porch_accum; // active height of window

    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_COLOR_KEY,    0);
    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_PIXEL_FORMAT, 0);
    cdc->planes[i].pixel_format = 0;
    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_ALPHA,        0xff);
    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_COLOR,        0);
    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_BLENDING,     ((CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA)<<8)|CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA_INV);
    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_AUX_FB_CONTROL,   0);
//    cdc->planes[i].AuxFB_control = 0;
    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_CB_START,     0);
    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_FB_LINES,     v_width_accum - v_b_porch_accum);
    cdc->planes[i].fb_pitch = 0;
    cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_AUX_FB_START,     0);
//    cdc->planes[i].AuxFB_pitch = 0;

    cdc->planes[i].fb_width  = cdc->planes[i].window_width;
    cdc->planes[i].fb_height = cdc->planes[i].window_height;

    // update color buffer and alpha buffer length settings
    updateBufferLength(cdc, i);

    // note CLUT registers are not initialized here as CLUT is disabled by default
  }

  if (cdc->hw.shadow_regs)
  {
    // force immediate reload of all shadowed registers
	  cdc_write_reg(cdc, CDC_REG_GLOBAL_SHADOW_RELOAD, 1);
  }
}


bool cdc_hw_triggerShadowReload(struct cdc_device *cdc, bool in_vblank)
{
	if (cdc->hw.shadow_regs)
	{
		if(in_vblank)
		{
			cdc_write_reg(cdc, CDC_REG_GLOBAL_SHADOW_RELOAD, 2);
		}
		else
		{
			cdc_write_reg(cdc, CDC_REG_GLOBAL_SHADOW_RELOAD, 1);
		}
		return CDC_TRUE;
	}
	return CDC_FALSE;
}


void cdc_hw_setTiming(struct cdc_device *cdc, cdc_uint16 a_h_sync, cdc_uint16 a_h_bPorch, cdc_uint16 a_h_width, cdc_uint16 a_h_fPorch,
		cdc_uint16 a_v_sync, cdc_uint16 a_v_bPorch, cdc_uint16 a_v_width, cdc_uint16 a_v_fPorch,
		cdc_float a_clk, cdc_bool a_neg_hsync, cdc_bool a_neg_vsync, cdc_bool a_neg_blank, cdc_bool a_inv_clk)
{
	u32 sync_size;
	u32 back_porch;
	u32 active_width;
	u32 total_width;
	u32 polarity_mask = 0;
	u32 control;
	u32 i;

	// calculate accumulated timing settings
	sync_size    = ((a_h_sync-1) << 16) + a_v_sync -1;
	back_porch   = (a_h_bPorch << 16) + a_v_bPorch + sync_size;
	active_width = (a_h_width << 16) + a_v_width + back_porch;
	total_width  = (a_h_fPorch << 16) + a_v_fPorch + active_width;

	// build up the sync polarity flags
	if(a_neg_hsync == CDC_TRUE)
	{
	  polarity_mask |= CDC_REG_GLOBAL_CONTROL_HSYNC;
	}
	if(a_neg_vsync == CDC_TRUE)
	{
	  polarity_mask |= CDC_REG_GLOBAL_CONTROL_VSYNC;
	}
	if(a_neg_blank == CDC_TRUE)
	{
	  polarity_mask |= CDC_REG_GLOBAL_CONTROL_BLANK;
	}
	if(a_inv_clk == CDC_TRUE)
	{
	  polarity_mask |= CDC_REG_GLOBAL_CONTROL_CLK_POL;
	}

	// disable the CDC
	setEnabled(cdc, false);

	// set pll
	//TODO: check for errors
	cdc_arch_setPixelClk(a_clk);


	// set timing registers
	cdc_write_reg(cdc, CDC_REG_GLOBAL_SYNC_SIZE, sync_size);
	cdc_write_reg(cdc, CDC_REG_GLOBAL_BACK_PORCH, back_porch);
	cdc_write_reg(cdc, CDC_REG_GLOBAL_ACTIVE_WIDTH, active_width);
	cdc_write_reg(cdc, CDC_REG_GLOBAL_TOTAL_WIDTH, total_width);

	// set scanline irq line
	cdc_write_reg(cdc, CDC_REG_GLOBAL_LINE_IRQ_POSITION, (active_width&0x0000ffff)+1);

	// apply the sync polarity mask
	control = cdc_read_reg(cdc, CDC_REG_GLOBAL_CONTROL);
	control  = (control & ~(CDC_REG_GLOBAL_CONTROL_HSYNC|CDC_REG_GLOBAL_CONTROL_VSYNC|CDC_REG_GLOBAL_CONTROL_BLANK|CDC_REG_GLOBAL_CONTROL_CLK_POL)) | polarity_mask;
	cdc_write_reg(cdc, CDC_REG_GLOBAL_CONTROL, control);

	// disable all layers and reset windows
	for(i=0; i<cdc->hw.layer_count;i++)
	{
	  // disable layer
	  cdc->planes[i].control &= ~CDC_REG_LAYER_CONTROL_ENABLE;
	  cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_CONTROL, cdc->planes[i].control);

	  // reset window
	  cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_WINDOW_H, (active_width&0xffff0000u) | ((back_porch>>16)+1));
	  cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_WINDOW_V, ((active_width&0xffffu)<<16) | ((back_porch&0xffffu)+1));
	  cdc->planes[i].window_width = a_h_width;
	  cdc->planes[i].window_height = a_v_width;
	  cdc_write_layer_reg(cdc, i, CDC_REG_LAYER_FB_LINES, a_v_width);
	  cdc->planes[i].fb_pitch = 0;
	  // TODO: reset alpha layer pitch if applicable
	  updateBufferLength(cdc, i);
	  // force reload of all shadowed registers
	  cdc_write_reg(cdc, CDC_REG_GLOBAL_SHADOW_RELOAD, 1);
	}

	// restore cdc enabled status
	setEnabled(cdc, cdc->hw.enabled);
}


void cdc_hw_setEnabled(struct cdc_device *cdc, bool enable)
{
	cdc->hw.enabled = enable;
	setEnabled(cdc, enable);
}


void cdc_hw_setBackgroundColor(struct cdc_device *cdc, u32 color)
{
	cdc_write_reg(cdc, CDC_REG_GLOBAL_BG_COLOR, color);
}
