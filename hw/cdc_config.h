/*
 * cdc_config.h  --  CDC Display Controller video mode configs
 *
 * Copyright (C) 2007 - 2016 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

 /*--------------------------------------------------------------------------
 *
 * Title: Configuration
 *  The values in cdc_config.h can change depending on the used CDC.
 *  In this file e.g. the pixel formats are defined.
 *
 *  As the values are subject to change the documentation below is just an example.
 *  Please have a look into the file to be sure of the settings of your CDC.
 *-------------------------------------------------------------------------- */
 
#ifndef CDC_CONFIG_H_INCLUDED
#define CDC_CONFIG_H_INCLUDED

/*---------------------------------------------------------------------------
 * Constants: Framebuffer Formats
 *  Possible framebuffer formats (see <cdc_layer_setPixelFormat>)
 *
 *  CDC_FBMODE_ARGB8888 - 32 bit pixel format with alpha(8), red(8), green(8), blue(8) from MSB to LSB
 *  CDC_FBMODE_RGB888   - 24 bit pixel format with red(8), green(8), blue(8) from MSB to LSB
 *  CDC_FBMODE_RGB565   - 16 bit pixel format with red(5), green(6), blue(5) from MSB to LSB
 *  CDC_FBMODE_ARGB4444 - 16 bit pixel format with alpha(4), red(4), green(4), blue(4) from MSB to LSB
 *  CDC_FBMODE_ARGB1555 - 16 bit pixel format with alpha(1), red(5), green(5), blue(5) from MSB to LSB
 *  CDC_FBMODE_AL88     - 16 bit pixel format with alpha(8), luminance(8) from MSB to LSB 
 *  CDC_FBMODE_AL44     - 8 bit pixel format with alpha(4), luminance(4) from MSB to LSB
 *  CDC_FBMODE_L8       - 8 bit pixel format with greyscale only (expanded onto all 4 channels)
 */
#define CDC_FBMODE_ARGB8888 0
#define CDC_FBMODE_RGB888 1
#define CDC_FBMODE_RGB565 2
#define CDC_FBMODE_ARGB4444 3
#define CDC_FBMODE_ARGB1555 4
#define CDC_FBMODE_AL88 5
#define CDC_FBMODE_AL44 6
#define CDC_FBMODE_L8 7


static const cdc_uint8 cdc_formats_bpp[] = {4, 3, 2, 2, 2, 2, 1, 1};


// Video modes

// this type can be initialized using the below defines, such that the fields get names
typedef struct {
  cdc_uint16 h_sync;
  cdc_uint16 h_bporch;
  cdc_uint16 h_width;
  cdc_uint16 h_fporch;
  cdc_uint16 v_sync;
  cdc_uint16 v_bporch;
  cdc_uint16 v_height;
  cdc_uint16 v_fporch;
  cdc_float  clk;
  cdc_bool   neg_hsync;
  cdc_bool   neg_vsync;
  cdc_bool   neg_blank;
  cdc_bool   inv_clk;
} cdc_video_mode;

// Note: the last parameter 'inv_clk' has only been used recently. It has only been checked for the VEEK-MT that it is set correctly so far!
#define CDC_VIDEO_MODE_1280x1024_60          112,   248,   1280,    48,   3,   38, 1024,   1, 108.00, CDC_FALSE, CDC_FALSE, CDC_FALSE, CDC_TRUE
#define CDC_VIDEO_MODE_1024x768_60           136,   160,   1024,    24,   6,   29,  768,   3,  65.00, CDC_FALSE, CDC_FALSE, CDC_FALSE, CDC_TRUE
#define CDC_VIDEO_MODE_800x600_60            128,    88,    800,    40,   4,   23,  600,   1,  40.00, CDC_FALSE, CDC_FALSE, CDC_FALSE, CDC_TRUE
#define CDC_VIDEO_MODE_800x600_60_DUAL    128<<1, 88<<1, 800<<1, 40<<1,   4,   23,  600,   1,  80.00, CDC_FALSE, CDC_FALSE, CDC_FALSE, CDC_TRUE
#define CDC_VIDEO_MODE_800x480_60_VEEK_MT     30,    16,    800,   210,  13,   10,  480,  22,  33.00, CDC_FALSE, CDC_FALSE, CDC_FALSE, CDC_FALSE
#define CDC_VIDEO_MODE_800x480_60_NEEK       112,    23,    800,    32,   5,   10,  480,  10,  29.59, CDC_FALSE, CDC_FALSE, CDC_FALSE, CDC_TRUE
#define CDC_VIDEO_MODE_640x480_60             96,    48,    640,    16,   2,   33,  480,  10,  25.18, CDC_FALSE, CDC_FALSE, CDC_FALSE, CDC_TRUE



#endif // CDC_CONFIG_H_INCLUDED
