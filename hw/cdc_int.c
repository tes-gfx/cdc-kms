/*
 * cdc_int.c  --  CDC Display Controller internal helper functions
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
#include <linux/kernel.h>


//store error state outside of context in case a context-error occurred
volatile cdc_error_code cdc_error_state=0;
volatile cdc_ptr cdc_error_context=NULL;


/*--------------------------------------------------------------------------
 * function: cdc_int_setError
 * Set an error code.
 *
 * After setting an error code the CDC driver is in an error state and the 
 * error needs to be queried before further operation.
 *
 * Parameters:
 *  a_context - The context the error occurred in (or NULL)
 *  a_error   - The error code
 */
void cdc_int_setError(cdc_context *a_context, cdc_error_code a_error)
{
  cdc_error_state = a_error;
  cdc_error_context = a_context;
}

/*--------------------------------------------------------------------------
 * function: cdc_int_validateContext
 * Return the given pointer as a CDC context pointer.
 *
 * The pointer is checked to be a valid context.
 * In case of an error state operation is stopped by returning NULL.
 *
 * Parameters:
 *  a_context - pointer to a CDC context
 *
 * Returns:
 *  - a context Pointer on success
 *  - NULL if a_context is no CDC context or driver is in error state
 */
cdc_context *cdc_int_validateContext(cdc_ptr a_context)
{
  cdc_context *context=a_context;
  if(context->m_hash != 0x0CDC0000 + sizeof(cdc_context))
  {
    // not a valid context -> return NULL
    cdc_int_setError(NULL,CDC_ERROR_CONTEXT);
    return NULL;
  }
  if(cdc_error_state!=CDC_ERROR_NO_ERROR)
  {
    // driver is in error state -> return NULL
    return NULL;
  }
  return context;
}

/*--------------------------------------------------------------------------
 * function: cdc_int_validateLayerContext
 * Return the given pointer as a CDC context pointer.
 *
 * The pointer is checked to be a valid context.
 * The layer is checked to be a valid layer number.
 * In case of an error state operation is stopped by returning NULL.
 *
 * Parameters:
 *  a_context - Pointer to a CDC context
 *  a_layer   - Layer number
 *
 * Returns:
 *  - a context pointer on success
 *  - NULL if a_context is no CDC context or driver is in error state
 */
cdc_context *cdc_int_validateLayerContext(cdc_ptr a_context, cdc_uint8 a_layer)
{
  cdc_context *context;
  context = cdc_int_validateContext(a_context);
  if(context)
  {
    if(a_layer>=context->m_layer_count)
    {
      cdc_int_setError(context, CDC_ERROR_LAYER_COUNT);
      return NULL;
    }
  }
  return context;  
}

/*--------------------------------------------------------------------------
 * function: cdc_int_resetRegisters
 * Put all registers into a predefined state.
 *
 * All CDC registers but the CLUT and BG-RAM are set to their reset state.
 *
 * Parameters:
 *  a_context - Pointer to a CDC context
 */
void cdc_int_resetRegisters(cdc_context *a_context)
{
  cdc_uint32 i;
  cdc_uint32 control;
  cdc_uint32 h_b_porch_accum;
  cdc_uint32 v_b_porch_accum;
  cdc_uint32 h_width_accum;
  cdc_uint32 v_width_accum;

  // initialize cdc registers with default values, but keep timings
  // initialize global registers
  control = cdc_arch_readReg(a_context, CDC_REG_GLOBAL_CONTROL);
  control &= CDC_REG_GLOBAL_CONTROL_HSYNC|CDC_REG_GLOBAL_CONTROL_VSYNC|CDC_REG_GLOBAL_CONTROL_BLANK|CDC_REG_GLOBAL_CONTROL_CLK_POL;
  cdc_arch_writeReg(a_context, CDC_REG_GLOBAL_CONTROL,             control);
  cdc_arch_writeReg(a_context, CDC_REG_GLOBAL_BG_COLOR,            0);
  cdc_arch_writeReg(a_context, CDC_REG_GLOBAL_IRQ_ENABLE,          0);
  h_b_porch_accum = cdc_arch_readReg(a_context, CDC_REG_GLOBAL_BACK_PORCH);
  v_b_porch_accum = h_b_porch_accum & 0xffff;
  h_b_porch_accum >>= 16;
  h_width_accum = cdc_arch_readReg(a_context, CDC_REG_GLOBAL_ACTIVE_WIDTH);
  v_width_accum = h_width_accum & 0xffff;
  h_width_accum >>= 16;
  
  cdc_arch_writeReg(a_context, CDC_REG_GLOBAL_LINE_IRQ_POSITION, v_width_accum+1);
  cdc_arch_writeReg(a_context, CDC_REG_GLOBAL_EXT_DISPLAY, 0);
  // note background layer registers are not initialized here as background layer is disabled by default

  // initialize per layer registers
  for(i=0; i<a_context->m_layer_count;i++)
  {
    cdc_arch_writeLayerReg(a_context, i,  CDC_REG_LAYER_CONTROL,      0);
    a_context->m_layers[i].m_control = 0;
    
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_WINDOW_H, (h_width_accum<<16)|(h_b_porch_accum+1));
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_WINDOW_V, (v_width_accum<<16)|(v_b_porch_accum+1));
    a_context->m_layers[i].m_window_width = h_width_accum - h_b_porch_accum; // active width of window
    a_context->m_layers[i].m_window_height = v_width_accum - v_b_porch_accum; // active height of window
    
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_COLOR_KEY,    0);
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_PIXEL_FORMAT, 0);
    a_context->m_layers[i].m_pixel_format = 0;
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_ALPHA,        0xff);
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_COLOR,        0);
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_BLENDING,     ((CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA)<<8)|CDC_BLEND_PIXEL_ALPHA_X_CONST_ALPHA_INV);
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_AUX_FB_CONTROL,   0);
    a_context->m_layers[i].m_AuxFB_control = 0;
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_CB_START,     0);
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_FB_LINES,     v_width_accum - v_b_porch_accum);
    a_context->m_layers[i].m_CB_pitch = 0;
    cdc_arch_writeLayerReg(a_context, i, CDC_REG_LAYER_AUX_FB_START,     0);
    a_context->m_layers[i].m_AuxFB_pitch = 0;
    
    a_context->m_layers[i].m_CB_width  = a_context->m_layers[i].m_window_width;
    a_context->m_layers[i].m_CB_height = a_context->m_layers[i].m_window_height;
    
    // update color buffer and alpha buffer length settings
    cdc_int_updateBufferLength(a_context, i);
    
    // note CLUT registers are not initialized here as CLUT is disabled by default
  }
  
  if (a_context->m_shadow_regs)
  {
    // force immediate reload of all shadowed registers
    cdc_arch_writeReg(a_context, CDC_REG_GLOBAL_SHADOW_RELOAD, 1);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_int_updateBufferLength
 * Calculate the new buffer length after an update to pixel fromat, window size or pitch.
 *
 * Parameters:
 *  a_context - Pointer to a CDC context
 *  a_layer   - Layer number
 */
void cdc_int_updateBufferLength(cdc_context *a_context, cdc_uint8 a_layer)
{
  cdc_uint32 length;
  cdc_sint32 pitch;
  cdc_uint8 format_bpp;
  cdc_uint8 alpha_plane_bpp;
  cdc_bool  ycbcr_ena;
  cdc_reg_aux_fb_control aux_fb_control;
  
  
  aux_fb_control.m_value = 0;
  aux_fb_control.m_value = a_context->m_layers[a_layer].m_AuxFB_control;
  ycbcr_ena = ((cdc_bool) (a_context->m_layers[a_layer].m_config_2 & CDC_REG_LAYER_CONFIG_YCBCR_ENABLED)) && (aux_fb_control.m_fields.m_ycbcr_convert_on == 1);
  
  alpha_plane_bpp = 1; //default 1 byte per pixel
  
  if (ycbcr_ena) {  
    switch (aux_fb_control.m_fields.m_ycbcr_mode) {
      case CDC_YCBCR_MODE_INTERLEAVED: 
        format_bpp = 2;
        break;
      case CDC_YCBCR_MODE_SEMI_PLANAR: 
        format_bpp = 1;
        alpha_plane_bpp = 2;  //2 bytes per pixel (Cb, Cr)
        break;
      default: //CDC_YCBCR_MODE_PLANAR
        //not supported
        return;
    }
  } else {
    format_bpp = cdc_formats_bpp[a_context->m_layers[a_layer].m_pixel_format];
  }

  // in case of insertion mode != 0, the layer length is only half the true length 
  if (a_context->m_layers[a_layer].m_config_2 & CDC_REG_LAYER_CONFIG_SCALER_ENABLED)
  {
    length = a_context->m_layers[a_layer].m_CB_width * format_bpp;
  }
  else 
  {
    length = a_context->m_layers[a_layer].m_window_width * format_bpp;

    if (a_context->m_layers[a_layer].m_control & CDC_REG_LAYER_CONTROL_INSERTION_MODE)
    {
      // window width must be even
#ifdef __linux
//      BUG_ON( (a_context->m_layers[a_layer].m_window_width & 1) != 0 );
#else
      assert( (a_context->m_layers[a_layer].m_window_width & 1) == 0 );
#endif
      length >>= 1;
    }
  }
   
  pitch = a_context->m_layers[a_layer].m_CB_pitch;
  if (pitch == 0) pitch = length;
  length += (1<<((a_context->m_global_config2 >> 4) & 7)) - 1; // add bus_width - 1
  cdc_arch_writeLayerReg(a_context, a_layer, CDC_REG_LAYER_CB_LENGTH, (((cdc_uint32)pitch) << 16) | length );
  
  if ((a_context->m_layers[a_layer].m_config_1 & CDC_REG_LAYER_CONFIG_ALPHA_PLANE) || ycbcr_ena)
  {
    if (a_context->m_layers[a_layer].m_config_2 & CDC_REG_LAYER_CONFIG_SCALER_ENABLED)
    {
      length = a_context->m_layers[a_layer].m_CB_width; //need changes
    } 
    else
    {
      length = a_context->m_layers[a_layer].m_window_width * alpha_plane_bpp;      

      if ((a_context->m_layers[a_layer].m_control & CDC_REG_LAYER_CONTROL_INSERTION_MODE) || (aux_fb_control.m_fields.m_ycbcr_mode == CDC_YCBCR_MODE_SEMI_PLANAR))        
      { 
        // window width must be even
#ifdef __linux
        //BUG_ON( (a_context->m_layers[a_layer].m_window_width & 1) != 0 );
#else
        assert( (a_context->m_layers[a_layer].m_window_width & 1) == 0 );
#endif
        length >>= 1;
      }
    }
    
    pitch = a_context->m_layers[a_layer].m_AuxFB_pitch;
    if (pitch == 0) {     
        pitch = length;      
    }
    length += (1<<((a_context->m_global_config2 >> 4) & 7)) - 1; // add bus_width - 1
    cdc_arch_writeLayerReg(a_context, a_layer, CDC_REG_LAYER_AUX_FB_LENGTH, (((cdc_uint32)pitch) << 16) | length);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_int_setEnabled
 * Enable/disable CDC.
 *
 * Parameters:
 *  a_context - Pointer to a CDC context
 *  a_enable  - CDC_TRUE to enable CDC, CDC_FALSE to disable CDC
 *
 */
void cdc_int_setEnabled(cdc_context *a_context, cdc_bool a_enable)
{
  cdc_uint32 control;
  control = cdc_arch_readReg(a_context, CDC_REG_GLOBAL_CONTROL);
  if(a_enable==CDC_TRUE)
  {
    cdc_arch_writeReg(a_context, CDC_REG_GLOBAL_CONTROL, control|CDC_REG_GLOBAL_CONTROL_ENABLE);
  }
  else
  {
    cdc_arch_writeReg(a_context, CDC_REG_GLOBAL_CONTROL, control&~CDC_REG_GLOBAL_CONTROL_ENABLE);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_int_calculateScalingFactor
 * Calculate scaling factor for scaler.
 *
 * Parameters:
 *  a_in  - Input size in pixels
 *  a_out - Output size in pixels
 *
 */
cdc_uint16 cdc_int_calculateScalingFactor(cdc_uint16 a_in, cdc_uint16 a_out)
{
  cdc_uint32 factor = ((a_in-1) << SCALER_FRACTION) / (a_out-1);
  return (cdc_uint16)(factor & 0xFFFF);
}
/*--------------------------------------------------------------------------
 * function: cdc_int_updateScalingFactors
 * Calculate the scaling factor after an update to window size or color buffer size.
 *
 * Parameters:
 *  a_context - Pointer to a CDC context
 *  a_layer   - Layer number
 */ 
void cdc_int_updateScalingFactors(cdc_context *a_context, cdc_uint8 a_layer)
{
  cdc_uint16 in_size;
  cdc_uint16 out_size;
  cdc_uint16 h_scaling_factor;
  cdc_uint16 h_scaling_phase;
  cdc_uint16 v_scaling_factor;
  in_size = a_context->m_layers[a_layer].m_CB_width;
  out_size = a_context->m_layers[a_layer].m_window_width;
  h_scaling_factor = cdc_int_calculateScalingFactor(in_size, out_size);
  h_scaling_phase = h_scaling_factor + ( 1 << SCALER_FRACTION );
  in_size = a_context->m_layers[a_layer].m_CB_height;
  out_size = a_context->m_layers[a_layer].m_window_height;
  v_scaling_factor = cdc_int_calculateScalingFactor(in_size, out_size);
  cdc_arch_writeLayerReg(a_context, a_layer, CDC_REG_LAYER_SCALER_H_SCALING_FACTOR, h_scaling_factor);
  cdc_arch_writeLayerReg(a_context, a_layer, CDC_REG_LAYER_SCALER_H_SCALING_PHASE, h_scaling_phase);
  cdc_arch_writeLayerReg(a_context, a_layer, CDC_REG_LAYER_SCALER_V_SCALING_FACTOR, v_scaling_factor);
  cdc_arch_writeLayerReg(a_context, a_layer, CDC_REG_LAYER_SCALER_V_SCALING_PHASE, v_scaling_factor);
}
