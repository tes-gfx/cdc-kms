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

#endif // CDC_CONFIG_H_INCLUDED
