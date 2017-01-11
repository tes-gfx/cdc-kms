/*
 * cdc_layer.c  --  CDC Display Controller layer functions
 *
 * Copyright (C) 2007 - 2016 TES Electronic Solutions GmbH
 * Author: Christian Thaler <christian.thaler@tes-dst.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <cdc.h>
#include <cdc_base.h>


/*--------------------------------------------------------------------------
 * function: cdc_getLayerConfig
 * Retrieve the layers configuration / capabilities.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *
 * Returns:
 *  <cdc_layer_config>, containing the capabilities of the specified layer.
 */
cdc_layer_config cdc_getLayerConfig(cdc_handle a_handle, cdc_uint8 a_layer)
{
  cdc_context *context;
  cdc_layer_config config;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    config.m_supported_pixel_formats    = (context->m_layers[a_layer].m_config_1 >> 24) & 0xff;
    config.m_supported_blend_factors_f1 = (context->m_layers[a_layer].m_config_1 >> 16) & 0xff;
    config.m_supported_blend_factors_f2 = (context->m_layers[a_layer].m_config_1 >> 8) & 0xff;
    config.m_alpha_mode_available       = (context->m_layers[a_layer].m_config_1 >> 7) & 0x1;
    config.m_clut_available             = (context->m_layers[a_layer].m_config_1 >> 6) & 0x1;
    config.m_windowing_avialable        = (context->m_layers[a_layer].m_config_1 >> 5) & 0x1;
    config.m_default_color_programmable = (context->m_layers[a_layer].m_config_1 >> 4) & 0x1;
    config.m_ab_availabe                = (context->m_layers[a_layer].m_config_1 >> 3) & 0x1;
    config.m_cb_pitch_available         = (context->m_layers[a_layer].m_config_1 >> 2) & 0x1;
    config.m_duplication_available      = (context->m_layers[a_layer].m_config_1 >> 1) & 0x1;
    config.m_color_key_available        = (context->m_layers[a_layer].m_config_1 >> 0) & 0x1;
  }
  return config;
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setEnabled
 * Enable/disable layer.
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 * 
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_enable - Enable layer if set to CDC_TRUE
 *  
 * See also:
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setEnabled(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    if(a_enable==CDC_TRUE)
    {
      context->m_layers[a_layer].m_control |= CDC_REG_LAYER_CONTROL_ENABLE;
    }
    else
    {
      context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_ENABLE;
    }
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_CONTROL, context->m_layers[a_layer].m_control);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setDuplication
 * Enable/disable horizontal or vertical pixel duplication.
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle     - CDC Context pointer
 *  a_layer      - Layer number (starting at 0)
 *  a_horizontal - Enable horizontal duplication if set to CDC_TRUE
 *  a_vertical   - Enable vertical duplication if set to CDC_TRUE
 *
 * See also:
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setDuplication(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_horizontal, cdc_bool a_vertical)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    if(a_horizontal==CDC_TRUE)
    {
      context->m_layers[a_layer].m_control |= CDC_REG_LAYER_CONTROL_H_DUPLICATION;
    }
    else
    {
      context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_H_DUPLICATION;
    }
    if(a_vertical==CDC_TRUE)
    {
      context->m_layers[a_layer].m_control |= CDC_REG_LAYER_CONTROL_V_DUPLICATION;
    }
    else
    {
      context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_V_DUPLICATION;
    }
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_CONTROL, context->m_layers[a_layer].m_control);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setCLUTEnabled
 * Enable/disable color lookup table.
 *
 * Color lookup takes the blue channel of the framebuffer and replaces the 
 * pixel color with the corresponding color in the CLUT-RAM.
 *
 * In case of a blue channel with less than 8 bit the value is bit-replicated to 8 bit before the lookup.
 * 
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_enable - Enable CLUT if set to CDC_TRUE 
 *
 * See also:
 *  <cdc_layer_uploadCLUT>
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setCLUTEnabled(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    if(a_enable==CDC_TRUE)
    {
      context->m_layers[a_layer].m_control |= CDC_REG_LAYER_CONTROL_CLUT_ENABLE;
    }
    else
    {
      context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_CLUT_ENABLE;
    }
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_CONTROL, context->m_layers[a_layer].m_control);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setColorKeyEnabled
 * Enable/disable color key feature.
 *
 * Cannot be used with alpha mode keying. If this function is called, alpha mode is disabled.
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_enable - Enable color key if set to CDC_TRUE
 *
 * See also:
 *  <cdc_layer_setColorKey>
 *  <cdc_layer_setAlphaModeEnabled>
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setColorKeyEnabled(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_COLOR_KEY_REPLACE;
    if(a_enable==CDC_TRUE)
    {
      context->m_layers[a_layer].m_control |= CDC_REG_LAYER_CONTROL_COLOR_KEY_ENABLE;
    }
    else
    {
      context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_COLOR_KEY_ENABLE;
    }
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_CONTROL, context->m_layers[a_layer].m_control);
  }
}


/*--------------------------------------------------------------------------
 * function: cdc_layer_setMirroringEnabled
 * Enable/disable mirroring feature.
 *
 * Attention: The frame buffer address must be set to the last byte of the
 * last pixel in the first line!
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_enable - Enable mirroring if set to CDC_TRUE
 *
 * See also:
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setMirroringEnabled(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    if(a_enable==CDC_TRUE)
    {
      context->m_layers[a_layer].m_control |= CDC_REG_LAYER_CONTROL_MIRRORING_ENABLE;
    }
    else
    {
      context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_MIRRORING_ENABLE;
    }
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_CONTROL, context->m_layers[a_layer].m_control);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setAlphaModeEnabled
 * Enable/disable Alpha layer mode.
 *
 * Cannot be used with color keying. If this function is called, color keying is disabled.
 * The default color for the alpha mode is set with <cdc_layer_setColorKey>.
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect. 
 *
 * Parameters:
 *  a_handle   - CDC Context pointer
 *  a_layer    - Layer number (starting at 0)
 *  a_enable - Enable color key if set to CDC_TRUE
 *
 * See also:
 *  <cdc_layer_setColorKey>
 *  <cdc_layer_setColorKeyEnabled>
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setAlphaModeEnabled(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_COLOR_KEY_ENABLE;
    if(a_enable==CDC_TRUE)
    {
      context->m_layers[a_layer].m_control |= CDC_REG_LAYER_CONTROL_COLOR_KEY_REPLACE;
    }
    else
    {
      context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_COLOR_KEY_REPLACE;
    }
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_CONTROL, context->m_layers[a_layer].m_control);
  }
}
 
/*--------------------------------------------------------------------------
 * function: cdc_layer_setWindow
 * Set the layer window position and size.
 * 
 * The window position is given relative to the active area of the screen, starting at 0,0 in the upper left corner.
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 * 
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_startX - Horizontal start of window on screen
 *  a_startY - Vertical start of windows on screen
 *  a_width  - Window width
 *  a_height - Window height
 *  a_pitch  - Number of bytes between the start of one line and the start of the next line (may be negative for vertical flip)
 *
 * See also: 
 *  <cdc_layer_moveWindow>
 *  <cdc_layer_setCBPitch>
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setWindow(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint16 a_startX, cdc_uint16 a_startY, cdc_uint16 a_width, cdc_uint16 a_height, cdc_sint16 a_pitch)
{
  cdc_context *context;
  cdc_uint32 activeStartX;
  cdc_uint32 activeStartY;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    //TODO range checks and truncation
    //TODO: check windowing ability
    activeStartY = cdc_arch_readReg(context, CDC_REG_GLOBAL_BACK_PORCH);
    activeStartX = activeStartY >> 16;
    activeStartY &= 0xffff;
    context->m_layers[a_layer].m_window_width = a_width;
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_WINDOW_H, ((a_startX + activeStartX + a_width)<<16) | (a_startX + activeStartX + 1));
    context->m_layers[a_layer].m_window_height = a_height;
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_WINDOW_V, ((a_startY + activeStartY + a_height)<<16) | (a_startY + activeStartY + 1));
    if (context->m_layers[a_layer].m_config_2 & CDC_REG_LAYER_CONFIG_SCALER_ENABLED)
    {
      cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_SCALER_OUTPUT_SIZE, (a_height << 16) | a_width);
      cdc_int_updateScalingFactors(context, a_layer);
    }
    else 
    {
      
      cdc_bool ycbcr_semiplanar; 
      cdc_reg_aux_fb_control aux_fb_control;
      aux_fb_control.m_value = 0;
      aux_fb_control.m_value = context->m_layers[a_layer].m_AuxFB_control;      
      
      ycbcr_semiplanar = ((cdc_bool) (context->m_layers[a_layer].m_config_2 & CDC_REG_LAYER_CONFIG_YCBCR_ENABLED))
                            && (aux_fb_control.m_fields.m_ycbcr_convert_on == 1)
                            && (aux_fb_control.m_fields.m_ycbcr_mode == CDC_YCBCR_MODE_SEMI_PLANAR) ;
      
      context->m_layers[a_layer].m_CB_pitch = a_pitch;
      cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_FB_LINES, a_height);
      if (ycbcr_semiplanar) {
        cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_AUX_FB_LINES, a_height/2);
      }        
      cdc_int_updateBufferLength(context, a_layer);
    }
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setBufferLines
 * Set the number of framebuffer lines.
 * 
 * Setting the number of framebuffer lines is not necessary for normal applications. 
 * It is provided to do some tricks with the windowing, like sprites.
 * 
 * Attention: The number of framebuffer lines will be updated internally after each call to <cdc_layer_setWindow>, 
 * <cdc_layer_setPixelFormat>, <cdc_layer_setCBPitch>, and <cdc_setTiming>, so a custom number of lines needs 
 * updating after a call to one of the functions.
 * 
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 * 
 * Parameters:
 *  a_handle    - CDC Context pointer
 *  a_layer     - Layer number (starting at 0)
 *  a_lines     - Number of lines in the framebuffer
 *
 * See also: 
 *  <cdc_layer_setWindow>
 *  <cdc_layer_setPixelFormat>
 *  <cdc_layer_setCBPitch>
 *  <cdc_setTiming>
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setBufferLines(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint32 a_lines)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_FB_LINES, a_lines);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setInsertionMode
 * Select, on which pixel positions (odd/even) of the layer the pixels shall be inserted and blended.
 * The first active pixel in a line has index 0, i.e. it is even. 
 * 
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle    - CDC Context pointer
 *  a_layer     - Layer number (starting at 0)
 *  a_mode      - Insertion mode (see <cdc_insertion_mode>)
 *
 * See also:
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setInsertionMode(cdc_handle a_handle, cdc_uint8 a_layer, cdc_insertion_mode a_mode)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_INSERTION_MODE;
    context->m_layers[a_layer].m_control |= (a_mode & 3)<< 6;
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_CONTROL, context->m_layers[a_layer].m_control);
    cdc_int_updateBufferLength(context, a_layer);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setDefaultColor
 * Set the layer's default color.
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_enable - Enable blending with default color for disabled layers/out-of-window pixels
 *  a_color  - Layer's default color in ARGB8888 format
 *
 * See also:
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setDefaultColor(cdc_handle a_handle, cdc_uint8 a_layer, cdc_bool a_enable, cdc_uint32 a_color)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    if(a_enable==CDC_TRUE)
    {
      context->m_layers[a_layer].m_control |= CDC_REG_LAYER_CONTROL_DEFAULT_COLOR_BLENDING;
    }
    else
    {
      context->m_layers[a_layer].m_control &= ~CDC_REG_LAYER_CONTROL_DEFAULT_COLOR_BLENDING;
    }
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_CONTROL, context->m_layers[a_layer].m_control);    
    
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_COLOR, a_color);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setConstantAlpha
 * Set alpha value for layer blending
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 * 
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_alpha  - Alpha value for layer
 *
 * See also: 
 *  <cdc_layer_setBlendMode>
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setConstantAlpha(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint8 a_alpha)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_ALPHA, a_alpha);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setColorKey
 * Set the layer color key or the layer default color for alpha mode.
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_color  - Color key/Alpha mode color
 *
 * See also:
 *  <cdc_layer_setColorKeyEnabled>
 *  <cdc_layer_setAlphaModeEnabled>
 *  <cdc_triggerShadowReload>
*/
void cdc_layer_setColorKey(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint32 a_color)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_COLOR_KEY, a_color);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setPixelFormat
 * Set the layers pixel format.
 *
 * For valid pixel formats see <Framebuffer Formats>
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle   - CDC Context pointer
 *  a_layer    - Layer number (starting at 0)
 *  a_format   - pixel format
 *
 * See also:
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setPixelFormat(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint8 a_format)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    context->m_layers[a_layer].m_pixel_format = a_format;
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_PIXEL_FORMAT, a_format);
    cdc_int_updateBufferLength(context, a_layer);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setCBAddress
 * Set the color frame buffer start address.
 *
 * Depending on the configuration the start address has to be bus aligned or 
 * aligned to a fixed number of bits.
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle  - CDC Context pointer
 *  a_layer   - Layer number (starting at 0)
 *  a_address - Address of the color frame buffer
 *
 * See also:
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setCBAddress(cdc_handle a_handle, cdc_uint8 a_layer, cdc_frame_ptr a_address)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    //TODO: check address alignment
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_CB_START, a_address);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setCBPitch
 * Set the color frame buffer pitch.
 *
 * The color buffer pitch is the number of bytes from one line to the next.
 * 
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_pitch  - Number of bytes between the start of one line and the start of the next line (may be negative for vertical flip)
 *
 * See also: 
 *  <cdc_layer_setWindow>
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setCBPitch(cdc_handle a_handle, cdc_uint8 a_layer, cdc_sint16 a_pitch)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    context->m_layers[a_layer].m_CB_pitch = a_pitch;
    cdc_int_updateBufferLength(context, a_layer);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setCBSize
 * Set the layer color buffer size.
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_width  - Width of color buffer in pixels
 *  a_height - Height of color buffer in lines
 *  a_pitch  - Number of bytes between the start of one line and the start of the next line (may be negative for vertical flip)
 *
 * See also:
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setCBSize(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint16 a_width, cdc_uint16 a_height, cdc_sint16 a_pitch)
{
  cdc_context *context;
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    context->m_layers[a_layer].m_CB_width = a_width;
    context->m_layers[a_layer].m_CB_height = a_height;
    context->m_layers[a_layer].m_CB_pitch = a_pitch;
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_FB_LINES, a_height);
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_SCALER_INPUT_SIZE, (a_height << 16) | a_width);
    cdc_int_updateScalingFactors(context, a_layer);
    cdc_int_updateBufferLength(context, a_layer);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setAuxFbAddress
 * Set the auxiliary famebuffer start address.
 * 
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 *
 * Parameters:
 *  a_handle   - CDC Context pointer
 *  a_layer    - Layer number (starting at 0)
 *  a_address  - Address of the auxiliary buffer
 *
 * See also:
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setAuxFbAddress(cdc_handle a_handle, cdc_uint8 a_layer, cdc_frame_ptr a_address)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_AUX_FB_START, a_address);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setAuxFbPitch
 * Set the auxiliary frame buffer pitch.
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect.
 * 
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_pitch  - Number of bytes between the start of one line and the start of the next line (may be negative for vertical flip)
 *
 * See also:
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setAuxFbPitch(cdc_handle a_handle, cdc_uint8 a_layer, cdc_sint16 a_pitch)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    context->m_layers[a_layer].m_AuxFB_pitch = a_pitch;
    cdc_int_updateBufferLength(context, a_layer);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_setBlendMode
 * Set the layer blend factors
 *
 * The layer is blended according to the following formula:
 *
 * c' = f1 * c + f2 * cs (c': blended color, c: current layer color, cs: subjacent layers blended color)
 *
 * If shadow registers are used <cdc_triggerShadowReload> must be called to 
 * put the register change in effect. 
 *
 * Parameters:
 *  a_handle  - CDC Context pointer
 *  a_layer   - Layer number (starting at 0)
 *  a_factor1 - Blend factor for layer color
 *  a_factor2 - Blend factor for layer background
 *
 * See also: 
 *  <cdc_layer_setConstantAlpha>
 *  <cdc_blend_factor>
 *  <cdc_triggerShadowReload>
 */
void cdc_layer_setBlendMode(cdc_handle a_handle, cdc_uint8 a_layer, cdc_blend_factor a_factor1, cdc_blend_factor a_factor2)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context)
  {
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_BLENDING, (a_factor1<<8) | a_factor2);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_layer_uploadCLUT
 * Upload the color lookup table
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 *  a_start  - Start address inside layers CLUT RAM (offset)
 *  a_length - Number of CLUT entries to upload
 *  a_data   - CLUT data (RGB888, 32 bit aligned, the upper 8 bit are ignored)
 *
 * See also: 
 *  <cdc_layer_setCLUTEnabled>
 */
void cdc_layer_uploadCLUT(cdc_handle a_handle, cdc_uint8 a_layer, cdc_uint8 a_start, cdc_uint16 a_length, cdc_uint32 *a_data)
{
  cdc_context *context;
  cdc_uint32 i;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);  
  
  // CLUT depth is fixed to 256
  if (a_start>255)
    a_start = 255;
  if (a_start+a_length>256)
    a_length = 256-a_start;
  
  if(context)
  {
    for(i=0; i<a_length; i++)
    {
      cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_CLUT, ((i+a_start)<<24)|(a_data[i]&0xffffff));
    }
  }
}


/*--------------------------------------------------------------------------
 * function: cdc_layer_YCbCrEnabled
 * Retuns wheathere
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_layer  - Layer number (starting at 0)
 * Returns:
 *  - CDC_TRUE if YCbCr is enabled for current layer
 *  - CDC_FALSE if YCbCr is dissabled for current layer
 *
 */
cdc_bool cdc_layer_YCbCrEnabled(cdc_handle a_handle, cdc_uint8 a_layer) {
  
  cdc_context *context;

  context = cdc_int_validateLayerContext(a_handle, a_layer);  
  
  if(context && (context->m_layers[a_layer].m_config_2 & CDC_REG_LAYER_CONFIG_YCBCR_ENABLED))
  {
    return CDC_TRUE;
  }
  return CDC_FALSE;
}



/*--------------------------------------------------------------------------
 * function: cdc_layer_setYCbCrScale1
 * Set the YCbCr scale1 factors (red_cr_scale/blue_cb_scale)
 *
 *
 * Parameters:
 *  a_handle  - CDC Context pointer
 *  a_layer   - Layer number (starting at 0)
 *  a_scale1  - Scale factor for YCbCr conversion (red_cr_scale/blue_cb_scale)
 *
 * See also: 
 *  <cdc_layer_setYCbCrScale2>
 */
void cdc_layer_setYCbCrScale1(cdc_handle a_handle, cdc_uint8 a_layer, cdc_reg_ycbcr_scale_1 a_scale1)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context);
  {
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_YCBCR_SCALE_1, a_scale1.m_value);
  }
}



/*--------------------------------------------------------------------------
 * function: cdc_layer_setYCbCrScale2
 * Set the YCbCr scale2 factors (green_cr_scale/green_cb_scale)
 *
 *
 * Parameters:
 *  a_handle  - CDC Context pointer
 *  a_layer   - Layer number (starting at 0)
 *  a_scale2  - Scale factor for YCbCr conversion (green_cr_scale/green_cb_scale)
 *
 * See also: 
 *  <cdc_layer_setYCbCrScale1>
 */
void cdc_layer_setYCbCrScale2(cdc_handle a_handle, cdc_uint8 a_layer, cdc_reg_ycbcr_scale_2 a_scale2)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  if(context);
  {
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_YCBCR_SCALE_2, a_scale2.m_value);
  }
}


/*--------------------------------------------------------------------------
 * function: cdc_layer_setAuxFbControl
 * Set the contol fields of auxiliary framebuffer
 *
 *
 * Parameters:
 *  a_handle  - CDC Context pointer
 *  a_layer   - Layer number (starting at 0)
 *  a_aux_fb_contol  - Contol fields of auxiliary framebuffer
 *
 * See also: 
 *  <cdc_layer_setYCbCrScale1>
 *  <cdc_layer_setYCbCrScale2>
 */
void cdc_layer_setAuxFbControl(cdc_handle a_handle, cdc_uint8 a_layer, cdc_reg_aux_fb_control a_aux_fb_contol)
{
  cdc_context *context;
  
  context = cdc_int_validateLayerContext(a_handle, a_layer);
  
  if(context);
  {
    context->m_layers[a_layer].m_AuxFB_control = a_aux_fb_contol.m_value;
    cdc_arch_writeLayerReg(context, a_layer, CDC_REG_LAYER_AUX_FB_CONTROL, a_aux_fb_contol.m_value);
    cdc_int_updateBufferLength(context, a_layer);
  }
}

