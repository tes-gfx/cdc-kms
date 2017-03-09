/*
 * cdc_hw.h  --  CDC Display Controller Hardware Interface
 *
 * Copyright (C) 2016 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CDC_HW_HELPERS_H_
#define CDC_HW_HELPERS_H_


#include <linux/types.h>

#include <cdc.h>
#include "cdc_drv.h"


void cdc_hw_setPixelFormat(struct cdc_device *cdc, int layer, u8 format);
void cdc_hw_setBlendMode(struct cdc_device *cdc, u8 layer, cdc_blend_factor a_factor1, cdc_blend_factor a_factor2);
void cdc_hw_setWindow(struct cdc_device *cdc, u8 layer, u16 startX, u16 startY, u16 width, u16 height, s16 pitch);
void cdc_hw_setCBAddress(struct cdc_device *cdc, u8 layer, dma_addr_t address);
void cdc_hw_layer_setEnabled(struct cdc_device *cdc, u8 layer, bool enable);
void cdc_hw_resetRegisters(struct cdc_device *cdc);
bool cdc_hw_triggerShadowReload(struct cdc_device *cdc, bool in_vblank);
void cdc_hw_setEnabled(struct cdc_device *cdc, bool enable);
void cdc_hw_setBackgroundColor(struct cdc_device *cdc, u32 color);

#endif /* CDC_HW_HELPERS_H_ */
