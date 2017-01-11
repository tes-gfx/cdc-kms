/*
 * cdc.h  --  CDC Display Controller main function and type definitions
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
 * Title: Types
 *
 *-------------------------------------------------------------------------- */
 
#ifndef CDC_H_INCLUDED
#define CDC_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif
/*--------------------------------------------------------------------------- */


#define CDC_FALSE 0
#define CDC_TRUE 1


/*--------------------------------------------------------------------------
 * Enum: cdc_error_code
 *  Error code
 *
 *  CDC_ERROR_NO_ERROR     - No error
 *  CDC_ERROR_CONTEXT      - Context is not valid
 *  CDC_ERROR_LAYER_COUNT  - Layer number was too high
 *  CDC_ERROR_PIXEL_FORMAT - Pixel format not supported by layer
 */
typedef enum {
  CDC_ERROR_NO_ERROR = 0,
  CDC_ERROR_CONTEXT,
  CDC_ERROR_LAYER_COUNT,
  CDC_ERROR_PIXEL_FORMAT,
} cdc_error_code;

/*--------------------------------------------------------------------------
 * Enum: cdc_irq_type
 *  IRQ type (see <cdc_registerISR>)
 *
 *  CDC_IRQ_LINE                   - Programmable Scanline Interrupt 
 *  CDC_IRQ_FIFO_UNDERRUN          - Indicates a fifo underrun 
 *  CDC_IRQ_BUS_ERROR              - Indicates a bus error
 *  CDC_IRQ_RELOAD                 - Issued on every shadow reload
 *  CDC_IRQ_SLAVE_TIMING_NO_SIGNAL - Issued if slave timing mode is enabled, but 
 *                                   no signal is detected
 *  CDC_IRQ_SLAVE_TIMING_NO_SYNC   - Issued if slave timing mode is enabled, but 
 *                                   CDC is currently not in sync with external sync source
 */
typedef enum {
  CDC_IRQ_LINE                   = 0x01,
  CDC_IRQ_FIFO_UNDERRUN          = 0x02,
  CDC_IRQ_BUS_ERROR              = 0x04,
  CDC_IRQ_RELOAD                 = 0x08,
  CDC_IRQ_SLAVE_TIMING_NO_SIGNAL = 0x10,
  CDC_IRQ_SLAVE_TIMING_NO_SYNC   = 0x20,
} cdc_irq_type;

/*--------------------------------------------------------------------------
 * Enum: cdc_blend_factor
 *  Blend factor used for blending between layers (see <cdc_layer_setBlendMode>)
 *
 *  CDC_BLEND_ONE                           - Blend factor of 1.0
 *  CDC_BLEND_ZERO                          - Blend factor of 0.0
 *  CDC_BLEND_PIXEL_ALPHA                   - Blend factor is taken from pixel's alpha value
 *  CDC_BLEND_PIXEL_ALPHA_INV               - Blend factor of 1.0 - pixel's alpha value
 *  CDC_BLEND_CONST_ALPHA                   - Constant alpha as blend factor
 *  CDC_BLEND_CONST_ALPHA_INV               - 1.0 - Constant alpha as blend factor
 *  CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA     - Pixel alpha * constant alpha as blend factor
 *  CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA_INV - 1.0 - (Pixel alpha * constant alpha as blend factor)
 */
typedef enum {
  CDC_BLEND_ONE                           = 0,
  CDC_BLEND_ZERO                          = 1,
  CDC_BLEND_PIXEL_ALPHA                   = 2,
  CDC_BLEND_PIXEL_ALPHA_INV               = 3,
  CDC_BLEND_CONST_ALPHA                   = 4,
  CDC_BLEND_CONST_ALPHA_INV               = 5,
  CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA     = 6,
  CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA_INV = 7,
} cdc_blend_factor;

/*--------------------------------------------------------------------------
 * Enum: cdc_bg_mode
 *  Background layer modes (optional feature: special 512 pixel RAM based background layer)
 *
 *  CDC_BG_TILED_32x16 - Background layer uses 32x16 pixel size repeated tiles
 *  CDC_BG_TILED_16x32 - Background layer uses 16x32 pixel size repeated tiles
 *  CDC_BG_LINEAR      - Background layer uses a linear pattern (like 1x512 pixel tiles)
 *
 *  See also:
 *   <cdc_configureBackgroundLayer>
 *   <cdc_uploadBackgroundLayer>
 *   <cdc_setEnableBackgroundLayer>
 */
typedef enum {
  CDC_BG_TILED_32x16 = 0,
  CDC_BG_TILED_16x32,
  CDC_BG_LINEAR,
} cdc_bg_mode;

/*--------------------------------------------------------------------------
 * Enum: cdc_insertion_mode
 *  Layer insertion mode (see <cdc_layer_setInsertionMode>)
 *
 *  CDC_INSERTION_MODE_DEFAULT   - Insert on all pixels 
 *  CDC_INSERTION_MODE_ODD       - Only insert on odd pixels 
 *  CDC_INSERTION_MODE_EVEN      - Only insert on even pixels 
 *  CDC_INSERTION_MODE_DUPLICATE - Duplicate on even and odd pixels 
 */
typedef enum {
  CDC_INSERTION_MODE_DEFAULT = 0,
  CDC_INSERTION_MODE_ODD,
  CDC_INSERTION_MODE_EVEN,
  CDC_INSERTION_MODE_DUPLICATE,
} cdc_insertion_mode;

/*--------------------------------------------------------------------------
 * Enum: cdc_dual_port_mode
 *  Dual-port mode (see <cdc_setDualPort>)
 *
 *  CDC_DUAL_PORT_MODE_OFF   - Secondary port is off (default)
 *  CDC_DUAL_PORT_MODE_CLONE - Both ports show the same
 *  CDC_DUAL_PORT_MODE_DUAL  - Different images on both ports (see <cdc_setDualView>)
 */
typedef enum {
  CDC_DUAL_PORT_MODE_OFF = 0,
  CDC_DUAL_PORT_MODE_CLONE,
  CDC_DUAL_PORT_MODE_DUAL,
} cdc_dual_port_mode;


/*--------------------------------------------------------------------------
 * Enum: cdc_insertion_mode
 *  YCbCr insertion mode
 *
 *  CDC_YCBCR_MODE_INTERLEAVED   - YCbCr interleaved in one framebuffer
 *  CDC_YCBCR_MODE_SEMI_PLANAR   - Y in one framebuffer, CbCr in separate framebuffer 
 *  CDC_YCBCR_MODE_PLANAR        - not supported
 */
typedef enum {
  CDC_YCBCR_MODE_INTERLEAVED = 0,
  CDC_YCBCR_MODE_SEMI_PLANAR,
  CDC_YCBCR_MODE_PLANAR,
} cdc_ycbcr_mode;


/* 
 * Type: cdc_bool 
 *  Boolean type, can hold CDC_TRUE or CDC_FALSE
 */
typedef unsigned cdc_bool;

/* 
 * Type: cdc_uint8 
 *  8 bit unsigned integer type
 */
typedef unsigned char cdc_uint8;

/* 
 * Type: cdc_uint16 
 *  16 bit unsigned integer type
 */
typedef unsigned short cdc_uint16;

/* 
 * Type: cdc_sint16 
 *  16 bit signed integer type
 */
typedef short cdc_sint16;

/* 
 * Type: cdc_uint32 
 *  32 bit unsigned integer type
 */
typedef unsigned cdc_uint32;

/* 
 * Type: cdc_sint32 
 *  32 bit signed integer type
 */
typedef unsigned cdc_sint32;

/* 
 * Type: cdc_ptr 
 *  Multipurpose pointer
 *
 *  Used e.g. for malloc or free.
 */
typedef void     *cdc_ptr;

/* 
 * Type: cdc_frame_ptr
 *  Pointer for a framebuffer (color or alpha) address (in CDC-address space)
 */
typedef cdc_uint32 cdc_frame_ptr;

/* 
 * Type: cdc_handle 
 *  External context handle of a cdc context
 */
typedef void     *cdc_handle;

/* Type: cdc_platform_settings 
 *  Platform dependent settings for cdc_init
 */
typedef void     *cdc_platform_settings;


/* 
 * Type: cdc_float 
 *  32 bit IEEE 754 floating point number
 */
typedef int      cdc_float;

/* 
 * Type: cdc_isr_callback(cdc_uint32)
 *  Signature of an interrupt callback
 *
 *  The argument ist the a_data parameter of cdc_registerISR.
 *
 *  See also:
 *   <cdc_registerISR>
 */
typedef void (*cdc_isr_callback)(cdc_uint32);


/* Type: cdc_global_status
 *  This struct holds the current status of the CDC (retrieved with <cdc_getStatus>)
 *  
 *  m_x                  - Current x-position
 *  m_y                  - Current y-position
 *  m_hsync              - Current h-sync status
 *  m_vsync              - Current v-sync status
 *  m_hblank             - Current h-blank status
 *  m_vblank             - Current v-blank status
 *  m_low_frequency_mode - Slave timing mode is in low frequency mode
 *  m_external_sync_line - Slave timing mode, line in which the external sync signal 
 *   was detected (or 0xffff if slave timing mode is disabled)
 */
typedef struct cdc_global_status_tag
{
  cdc_uint32 m_x:16;
  cdc_uint32 m_y:16;
  cdc_uint32 m_hsync:1;
  cdc_uint32 m_vsync:1;
  cdc_uint32 m_hblank:1;
  cdc_uint32 m_vblank:1;
  cdc_uint32 m_low_frequency_mode:1;
  cdc_uint32 m_external_sync_line:16;
} cdc_global_status;

/* Type: cdc_global_config
 *  This struct holds the global CDC configuration options (retrieved with <cdc_getGlobalConfig>)
 *  
 *  m_revision_major                - Major version of the CDC core
 *  m_revision_minor                - Minor version of the CDC core
 *  m_layer_count                   - Number of layers in this CDC
 *  m_blind_mode                    - Blind mode enabled (if this is set the other status information might be inaccurate)
 *  m_configuration_reading         - If set, the CDC HW configuration (number of layers etc.) can be read from read-only registers
 *  m_status_registers              - If set, the CDC status can be queried with <cdc_getStatus>
 *  m_dither_width_programmable     - If set the dither width can be changed
 *  m_sync_polarity_programmable    - If set, the sync/blank polarity is programmable
 *  m_irq_polarity_programmable     - If set, the IRQ polarity is programmable
 *  m_timing_programmable           - If set, the display timing is programmable (see <cdc_setTiming>)
 *  m_line_irq_programmable         - If set, the line for the line IRQ is programmable (see <cdc_setScanlineIRQPosition>)
 *  m_background_blending           - If set, blending with the background color/-layer ist useable
 *  m_background_color_programmable - If set, the background color can be changed (see <cdc_setBackgroundColor>)
 *  m_shadow_registers              - If set, the layer resgisters are shadowed (not the CLUT tough)
 *  m_gamma_correction_technique    - Reserved for gamma correction type
 *  m_dithering_technique           - Reserved for the dither type
 *  m_precise_blending              - If set, precise blending is enabled.
 *  m_red_width                     - The number of bits for the red channel output
 *  m_green_width                   - The number of bits for the green channel output
 *  m_blue_width                    - The number of bits for the blue channel output
 *  m_slave_timing_mode_available   - If set, the CDC can be synchronized with another video input
 *  m_bg_layer_available            - If set, the background layer feature can be used (see <cdc_configureBackgroundLayer>)
 */
typedef struct cdc_global_config_tag
{
  cdc_uint32 m_revision_major:8;
  cdc_uint32 m_revision_minor:8;
  cdc_uint32 m_layer_count:8;
  cdc_uint32 m_blind_mode:1;
  cdc_uint32 m_configuration_reading:1;
  cdc_uint32 m_status_registers:1;
  cdc_uint32 m_dither_width_programmable:1;
  cdc_uint32 m_sync_polarity_programmable:1;
  cdc_uint32 m_irq_polarity_programmable:1;
  cdc_uint32 m_timing_programmable:1;
  cdc_uint32 m_line_irq_programmable:1;
  cdc_uint32 m_background_blending:1;
  cdc_uint32 m_background_color_programmable:1;
  cdc_uint32 m_shadow_registers:1;
  cdc_uint32 m_gamma_correction_technique:3;
  cdc_uint32 m_dithering_technique:2;
  cdc_uint32 m_precise_blending:1;
  cdc_uint32 m_red_width:4;
  cdc_uint32 m_green_width:4;
  cdc_uint32 m_blue_width:4;
  cdc_uint32 m_slave_timing_mode_available:1;
  cdc_uint32 m_bg_layer_available:1;
} cdc_global_config;

/* Type: cdc_layer_config
 *  This struct holds the per layer CDC configuration options (retrieved with <cdc_getLayerConfig>)
 * 
 *  m_supported_pixel_formats    - 8 bit mask indicating which pixel formats are valid for this layer
 *  m_supported_blend_factors_f1 - 8 bit mask indicating which blend factors can be used as f1 (see <cdc_layer_setBlendMode>)
 *  m_supported_blend_factors_f2 - 8 bit mask indicating which blend factors can be used as f2 (see <cdc_layer_setBlendMode>)
 *  m_alpha_mode_available       - If set, this layer can be used as a special alpha layer (see <cdc_layer_setAlphaModeEnabled>)
 *  m_clut_available             - If set, an up to 256-entry color lookup table can be used (see <cdc_layer_setCLUTEnabled>)
 *  m_windowing_avialable        - If set, this layer can be reduced to a smaller area (see <cdc_layer_setWindow>)
 *  m_default_color_programmable - If set, the default color can be changed
 *  m_ab_availabe                - If set, an alpha buffer (alpha-layer) can be enabled for this layer)
 *  m_cb_pitch_available         - If set, a pitch!=length between two lines in the framebuffer can be specified (see <cdc_layer_setCBPitch>)
 *  m_duplication_available      - If set, a horizontal and vertical pixel duplication can be enabled
 *  m_color_key_available        - If set, color keying can be used (see <cdc_layer_setColorKeyEnabled>)
 */
typedef struct cdc_layer_config_tag
{
  cdc_uint32 m_supported_pixel_formats:8;
  cdc_uint32 m_supported_blend_factors_f1:8;
  cdc_uint32 m_supported_blend_factors_f2:8;
  cdc_uint32 m_alpha_mode_available:1;
  cdc_uint32 m_clut_available:1;
  cdc_uint32 m_windowing_avialable:1;
  cdc_uint32 m_default_color_programmable:1;
  cdc_uint32 m_ab_availabe:1;
  cdc_uint32 m_cb_pitch_available:1;
  cdc_uint32 m_duplication_available:1;
  cdc_uint32 m_color_key_available:1;

} cdc_layer_config;


typedef union cdc_reg_aux_fb_control_tag
{
  struct cdc_reg_aux_fb_control_fields
  {
    cdc_uint32 m_alpha_plane_on            : 1;    // (0)
    cdc_uint32 m_vertical_duplication_on   : 1;    // (1)
    cdc_uint32 m_horizontal_duplication_on : 1;    // (2)
    cdc_uint32 m_ycbcr_convert_on          : 1;    // (3)
    cdc_uint32 m_ycbcr_mode                : 2;    // (5..4)
    cdc_uint32 m_y_first                   : 1;    // (6)
    cdc_uint32 m_cb_first                  : 1;    // (7)
    cdc_uint32 m_odd_first                 : 1;    // (8)
    cdc_uint32 m_y_headroom_on             : 1;    // (9)
    cdc_uint32 m_reserved1                 : 22;   // not used (31..10)
  } m_fields;
  cdc_uint32 m_value;
} cdc_reg_aux_fb_control;

typedef union cdc_reg_ycbcr_scale_1_tag
{
  struct cdc_reg_ycbcr_scale_1_fields
  {
    cdc_uint32 m_red_cr_scale             : 10;
    cdc_uint32 m_reserved1                : 6;
    cdc_uint32 m_blue_cb_scale            : 10;
    cdc_uint32 m_reserved2                : 6;
  } m_fields;
  cdc_uint32 m_value;
} cdc_reg_ycbcr_scale_1;

typedef union cdc_reg_ycbcr_scale_2_tag
{
  struct cdc_reg_ycbcr_scale_2_fields
  {
    cdc_uint32 m_green_cr_scale             : 10;
    cdc_uint32 m_reserved1                  : 6;
    cdc_uint32 m_green_cb_scale             : 10;
    cdc_uint32 m_reserved2                  : 6;
  }        m_fields;
  cdc_uint32 m_value;
} cdc_reg_ycbcr_scale_2;

#include <cdc_config.h>

/******************************************************************************
 *  global functions                                                          *
 ******************************************************************************/
cdc_handle cdc_init(cdc_platform_settings a_platform);
void cdc_exit(cdc_handle a_handle);
cdc_error_code cdc_getError(void);

cdc_global_config cdc_getGlobalConfig(cdc_handle a_handle);
cdc_uint8 cdc_getLayerCount(cdc_handle a_handle);
cdc_global_status cdc_getStatus(cdc_handle a_handle);
cdc_bool cdc_triggerShadowReload(cdc_handle a_handle, cdc_bool a_in_vblank);
cdc_bool cdc_updatePending(cdc_handle a_handle);

void cdc_registerISR(cdc_handle a_handle, cdc_irq_type a_type, cdc_isr_callback a_callback, cdc_uint32 a_data);
void cdc_setScanlineIRQPosition(cdc_handle a_handle, cdc_sint16 a_line);

void cdc_setEnabled(cdc_handle a_handle, cdc_bool a_enable);

void cdc_setModeline(cdc_handle a_handle, char *modeline);
void cdc_setTiming(cdc_handle a_handle, cdc_uint16 a_h_sync, cdc_uint16 a_h_bPorch, cdc_uint16 a_h_width, cdc_uint16 a_h_fPorch, 
                                     cdc_uint16 a_v_sync, cdc_uint16 a_v_bPorch, cdc_uint16 a_v_width, cdc_uint16 a_v_fPorch, 
									 cdc_float a_clk, cdc_bool a_neg_hsync, cdc_bool a_neg_vsync, cdc_bool a_neg_blank, cdc_bool a_inv_clk);

void cdc_setBackgroundColor(cdc_handle a_handle, cdc_uint32 a_color);
void cdc_uploadBackgroundLayer(cdc_handle a_handle, cdc_uint32 a_start, cdc_uint32 a_length, cdc_uint32 *a_data);
void cdc_configureBackgroundLayer(cdc_handle a_handle, cdc_bg_mode a_maskMode, cdc_uint16 a_base, cdc_sint16 a_incX, cdc_sint16 a_incY);
void cdc_setEnableBackgroundLayer(cdc_handle a_handle, cdc_bool a_enable);
void cdc_setDitherEnabled(cdc_handle a_handle, cdc_bool a_enable);
void cdc_setSlaveTimingModeEnabled(cdc_handle a_handle, cdc_bool a_enable);
void cdc_setDualView(cdc_handle a_handle, cdc_bool a_enable, cdc_bool a_subpixel_mixing, cdc_bool a_half_clock_even, cdc_bool a_half_clock_odd, cdc_bool a_half_clock_shift);
void cdc_setDualPort(cdc_handle a_handle, cdc_dual_port_mode a_mode);

/******************************************************************************
 *  layer functions                                                           *
 ******************************************************************************/
cdc_layer_config cdc_getLayerConfig(cdc_handle a_handle, cdc_uint8 a_layer);
void cdc_layer_setEnabled(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable);
void cdc_layer_setDuplication(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_horizontal, cdc_bool a_vertical);
void cdc_layer_setCLUTEnabled(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable);
void cdc_layer_setColorKeyEnabled(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable);
void cdc_layer_setMirroringEnabled(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable);
void cdc_layer_setAlphaModeEnabled(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable);

void cdc_layer_setWindow(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint16 a_startX, cdc_uint16 a_startY, cdc_uint16 a_width, cdc_uint16 a_height, cdc_sint16 a_pitch);
void cdc_layer_setBufferLines(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint32 a_lines);
void cdc_layer_moveWindow(cdc_handle a_handle, cdc_uint8 a_layer, cdc_sint16 a_incX, cdc_sint16 a_incY);
void cdc_layer_setInsertionMode(cdc_handle a_handle, cdc_uint8 a_layer, cdc_insertion_mode a_mode);
void cdc_layer_setDefaultColor(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable, cdc_uint32 a_color);
void cdc_layer_setConstantAlpha(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint8 a_alpha);
void cdc_layer_setColorKey(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint32 a_color);
void cdc_layer_setPixelFormat(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint8 a_format);
void cdc_layer_setCBAddress(cdc_handle a_handle, cdc_uint8 a_layer, cdc_frame_ptr a_address);
void cdc_layer_setCBPitch(cdc_handle a_handle, cdc_uint8 a_layer, cdc_sint16 a_pitch);
void cdc_layer_setAuxFbAddress(cdc_handle a_handle, cdc_uint8 a_layer, cdc_frame_ptr a_address);
void cdc_layer_setAuxFbPitch(cdc_handle a_handle, cdc_uint8 a_layer, cdc_sint16 a_pitch);
void cdc_layer_setBlendMode(cdc_handle a_handle, cdc_uint8 a_layer, cdc_blend_factor a_factor1, cdc_blend_factor a_factor2);
void cdc_layer_uploadCLUT(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint8 a_start, cdc_uint16 a_length, cdc_uint32 *a_data);
void cdc_layer_setCBSize(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint16 a_width, cdc_uint16 a_height, cdc_sint16 a_pitch);
cdc_bool cdc_layer_YCbCrEnabled(cdc_handle a_handle, cdc_uint8 a_layer);
void cdc_layer_setYCbCrScale1(cdc_handle a_handle, cdc_uint8 a_layer, cdc_reg_ycbcr_scale_1 a_scale1);
void cdc_layer_setYCbCrScale2(cdc_handle a_handle, cdc_uint8 a_layer, cdc_reg_ycbcr_scale_2 a_scale2);
void cdc_layer_setAuxFbControl(cdc_handle a_handle, cdc_uint8 a_layer, cdc_reg_aux_fb_control a_aux_fb_contol);

/*--------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif // CDC_H_INCLUDED
