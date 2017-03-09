/*
 * cdc_base.h  --  CDC Display Controller base definitions
 *
 * Copyright (C) 2007 - 2016 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CDC_BASE_H_INCLUDED
#define CDC_BASE_H_INCLUDED

#include <cdc.h>

#ifndef NULL
  #define NULL ((void *) 0)
#endif

// global registers
#define CDC_REG_GLOBAL_HW_REVISION          0x00
#define CDC_REG_GLOBAL_LAYER_COUNT          0x01
#define CDC_REG_GLOBAL_SYNC_SIZE            0x02
#define CDC_REG_GLOBAL_BACK_PORCH           0x03
#define CDC_REG_GLOBAL_ACTIVE_WIDTH         0x04
#define CDC_REG_GLOBAL_TOTAL_WIDTH          0x05
#define CDC_REG_GLOBAL_CONTROL              0x06
#define CDC_REG_GLOBAL_CONFIG1              0x07
#define CDC_REG_GLOBAL_CONFIG2              0x08
#define CDC_REG_GLOBAL_SHADOW_RELOAD        0x09
#define CDC_REG_GLOBAL_GAMMA                0x0a
#define CDC_REG_GLOBAL_BG_COLOR             0x0b
#define CDC_REG_GLOBAL_IRQ_POLARITY         0x0c
#define CDC_REG_GLOBAL_IRQ_ENABLE           0x0d
#define CDC_REG_GLOBAL_IRQ_STATUS           0x0e
#define CDC_REG_GLOBAL_IRQ_CLEAR            0x0f
#define CDC_REG_GLOBAL_LINE_IRQ_POSITION    0x10
#define CDC_REG_GLOBAL_POSITION             0x11
#define CDC_REG_GLOBAL_SYNC_STATUS          0x12
#define CDC_REG_GLOBAL_BG_LAYER_BASE        0x13
#define CDC_REG_GLOBAL_BG_LAYER_INC         0x14
#define CDC_REG_GLOBAL_BG_LAYER_ADDR        0x15
#define CDC_REG_GLOBAL_BG_LAYER_DATA        0x16
#define CDC_REG_GLOBAL_SLAVE_TIMING_STATUS  0x17
#define CDC_REG_GLOBAL_EXT_DISPLAY          0x18

//control bits
#define CDC_REG_GLOBAL_CONTROL_HSYNC            0x80000000u
#define CDC_REG_GLOBAL_CONTROL_VSYNC            0x40000000u
#define CDC_REG_GLOBAL_CONTROL_BLANK            0x20000000u
#define CDC_REG_GLOBAL_CONTROL_CLK_POL          0x10000000u
#define CDC_REG_GLOBAL_CONTROL_SLAVE_TIMING     0x00040000u
#define CDC_REG_GLOBAL_CONTROL_BACKGROUND_LAYER 0x00020000u
#define CDC_REG_GLOBAL_CONTROL_DITHERING        0x00010000u
#define CDC_REG_GLOBAL_CONTROL_ENABLE           0x00000001u

// per layer registers
#define CDC_REG_LAYER_CONFIG_1                     0x00
#define CDC_REG_LAYER_CONFIG_2                     0x01
#define CDC_REG_LAYER_CONTROL                      0x02
#define CDC_REG_LAYER_WINDOW_H                     0x03
#define CDC_REG_LAYER_WINDOW_V                     0x04
#define CDC_REG_LAYER_COLOR_KEY                    0x05
#define CDC_REG_LAYER_PIXEL_FORMAT                 0x06
#define CDC_REG_LAYER_ALPHA                        0x07
#define CDC_REG_LAYER_COLOR                        0x08
#define CDC_REG_LAYER_BLENDING                     0x09
#define CDC_REG_LAYER_FB_BUS_CONTROL               0x0a
#define CDC_REG_LAYER_AUX_FB_CONTROL               0x0b
#define CDC_REG_LAYER_CB_START                     0x0c
#define CDC_REG_LAYER_CB_LENGTH                    0x0d
#define CDC_REG_LAYER_FB_LINES                     0x0e
#define CDC_REG_LAYER_AUX_FB_START                 0x0f
#define CDC_REG_LAYER_AUX_FB_LENGTH                0x10
#define CDC_REG_LAYER_AUX_FB_LINES                 0x11
#define CDC_REG_LAYER_CLUT                         0x12

#define CDC_REG_LAYER_SCALER_INPUT_SIZE            0x13
#define CDC_REG_LAYER_SCALER_OUTPUT_SIZE           0x14
#define CDC_REG_LAYER_SCALER_V_SCALING_FACTOR      0x15
#define CDC_REG_LAYER_SCALER_V_SCALING_PHASE       0x16
#define CDC_REG_LAYER_SCALER_H_SCALING_FACTOR      0x17
#define CDC_REG_LAYER_SCALER_H_SCALING_PHASE       0x18
#define CDC_REG_LAYER_YCBCR_SCALE_1                0x19
#define CDC_REG_LAYER_YCBCR_SCALE_2                0x1a

#define SCALER_FRACTION        (13)
//layer config bits
#define CDC_REG_LAYER_CONFIG_ALPHA_PLANE  0x00000008u
//layer config 2 bits                      
#define CDC_REG_LAYER_CONFIG_SCALER_ENABLED 0x80000000u
#define CDC_REG_LAYER_CONFIG_YCBCR_ENABLED  0x40000000u

//layer control bits
#define CDC_REG_LAYER_CONTROL_DEFAULT_COLOR_BLENDING  0x00000200u
#define CDC_REG_LAYER_CONTROL_MIRRORING_ENABLE        0x00000100u
#define CDC_REG_LAYER_CONTROL_INSERTION_MODE          0x000000c0u
#define CDC_REG_LAYER_CONTROL_COLOR_KEY_REPLACE       0x00000020u
#define CDC_REG_LAYER_CONTROL_CLUT_ENABLE             0x00000010u
#define CDC_REG_LAYER_CONTROL_H_DUPLICATION           0x00000008u
#define CDC_REG_LAYER_CONTROL_V_DUPLICATION           0x00000004u
#define CDC_REG_LAYER_CONTROL_COLOR_KEY_ENABLE        0x00000002u
#define CDC_REG_LAYER_CONTROL_ENABLE                  0x00000001u


#endif // CDC_BASE_H_INCLUDED
