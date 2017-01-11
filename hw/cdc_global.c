/*
 * cdc_global.c  --  CDC Display Controller Global + Initialization functions
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


extern volatile cdc_uint32 cdc_error_state;
extern volatile cdc_ptr cdc_error_context;
 
/*--------------------------------------------------------------------------
 * Group: Device Management
 *-------------------------------------------------------------------------- */
 
 
/*--------------------------------------------------------------------------
 * function: cdc_init
 * Initialize the display controller.
 *
 * This resets all global and layer registers (expect the background layer and CLUT RAMs).
 * The internal interrupt handling is also activated.
 *
 * Parameters:
 *  a_platform - Pointer to a platform dependent device settings struct
 *
 * Returns:
 *  - a context pointer on success
 *  - NULL in case of an error
 *
 * See also: 
 *   <cdc_exit>
 */
cdc_ptr cdc_init(cdc_platform_settings a_platform)
{
  cdc_context *context;
  cdc_global_config config;
  cdc_uint32 i;
  
  context = (cdc_context*)cdc_arch_malloc(sizeof(cdc_context));
  
  // in case of a malloc error return
  if (!context)
  {
    return NULL;
  }
  
  // call platform init function
  if (!cdc_arch_init(context, a_platform))
  {
    cdc_arch_free(context);
    return NULL;
  }

  context->m_hash = 0x0CDC0000 + sizeof(cdc_context);
  context->m_enabled = CDC_FALSE;
  
  context->m_hw_revision    = cdc_arch_readReg(context, CDC_REG_GLOBAL_HW_REVISION);
  context->m_shadow_regs    = CDC_FALSE;  // assume by default that we have no shadow registers
  
  context->m_global_config1 = cdc_arch_readReg(context, CDC_REG_GLOBAL_CONFIG1);
  context->m_global_config2 = cdc_arch_readReg(context, CDC_REG_GLOBAL_CONFIG2);
  
  // consistency checks
  config = cdc_getGlobalConfig(context);
  if (config.m_configuration_reading)
  {
    if (config.m_shadow_registers)
      context->m_shadow_regs = CDC_TRUE;
    context->m_layer_count = cdc_arch_readReg(context, CDC_REG_GLOBAL_LAYER_COUNT);
  }
  else
  {
    // in case configuration reading is not available, we assume a minimum config
    context->m_layer_count = 1;
  }

  //TODO: check more hw compatibility parameters  
  
  context->m_layers = (cdc_layer*)cdc_arch_malloc(sizeof(cdc_layer)*context->m_layer_count);
  // in case of a malloc error free context and return
  if(!context->m_layers)
  {
    context->m_hash = 0;
    cdc_arch_free(context);
    return NULL;  
  }
  for(i=0; i<context->m_layer_count; i++)
  {
    context->m_layers[i].m_config_1 = cdc_arch_readLayerReg(context, i, CDC_REG_LAYER_CONFIG_1);
    context->m_layers[i].m_config_2 = cdc_arch_readLayerReg(context, i, CDC_REG_LAYER_CONFIG_2);
  }
  
  //intialize irq callbacks
  context->m_irq_line = NULL;
  context->m_irq_line_data = 0;
  context->m_irq_fifo_underrun = NULL;
  context->m_irq_fifo_underrun_data = 0;
  context->m_irq_bus_error = NULL;
  context->m_irq_bus_error_data = 0;
  context->m_irq_reload = NULL;
  context->m_irq_reload_data = 0;
  context->m_irq_slave_timing_no_signal = NULL;
  context->m_irq_slave_timing_no_signal_data = 0;
  context->m_irq_slave_timing_no_sync = NULL;
  context->m_irq_slave_timing_no_sync_data = 0;
  
  cdc_int_resetRegisters(context);
  
  // disable IRQs
  context->m_irq_enabled = 0x0;
  cdc_arch_writeReg(context, CDC_REG_GLOBAL_IRQ_ENABLE, context->m_irq_enabled);

  // clear all IRQs
  cdc_arch_writeReg(context, CDC_REG_GLOBAL_IRQ_CLEAR, 0x1f);

  // call platform IRQ init function
  if (!cdc_arch_initIRQ(context))
  {
    context->m_hash = 0;
    cdc_arch_free(context->m_layers);
    cdc_arch_free(context);    
    return NULL;
  }
  
  return (cdc_handle)context;
}

/*--------------------------------------------------------------------------
 * function: cdc_exit
 * Shut down the display controller and driver.
 *
 * - Disable the display controller
 * - Unregister all ISRs
 * - Free all structures
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *
 * See also: 
 *  <cdc_init>
 */
void cdc_exit(cdc_handle a_handle)
{
  cdc_context *context;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    // disable IRQs
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_IRQ_ENABLE, 0x0);
    
    // call platform de-init functions
    cdc_arch_deinitIRQ(context);
    cdc_arch_exit(context);
  
    cdc_int_setEnabled(context, CDC_FALSE);
  
    cdc_arch_free(context->m_layers);
    context->m_hash = 0;
    cdc_arch_free(context);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_getError
 * Return the latest error code and clear the error state.
 *
 * After an error occurred the error needs to be queried before 
 * any further driver call.
 *
 * See also:
 *  <cdc_error_code>
 */
cdc_uint32 cdc_getError()
{
  cdc_uint32 error = cdc_error_state;
  cdc_error_state = CDC_ERROR_NO_ERROR;
  return error;
}


/*--------------------------------------------------------------------------
 * function: cdc_getGlobalConfig
 * Retrieve the CDC's configuration / capabilities.
 *
 * Parameters:
 *  a_handle   - CDC Context pointer
 *
 * Returns:
 *  <cdc_global_config>, containing the capabilities of the CDC.
 */
cdc_global_config cdc_getGlobalConfig(cdc_handle a_handle)
{
  cdc_context *context;
  cdc_global_config config;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    config.m_revision_major                = context->m_hw_revision >> 8;
    config.m_revision_minor                = context->m_hw_revision & 0xff;
    config.m_layer_count                   = context->m_layer_count;
    config.m_blind_mode                    = (context->m_global_config1 >> 31) & 0x1;
    config.m_configuration_reading         = (context->m_global_config1 >> 30) & 0x1;
    config.m_status_registers              = (context->m_global_config1 >> 29) & 0x1;
    config.m_dither_width_programmable     = (context->m_global_config1 >> 28) & 0x1;
    config.m_sync_polarity_programmable    = (context->m_global_config1 >> 27) & 0x1;
    config.m_irq_polarity_programmable     = (context->m_global_config1 >> 26) & 0x1;
    config.m_timing_programmable           = (context->m_global_config1 >> 25) & 0x1;
    config.m_line_irq_programmable         = (context->m_global_config1 >> 24) & 0x1;
    config.m_background_blending           = (context->m_global_config1 >> 23) & 0x1;
    config.m_background_color_programmable = (context->m_global_config1 >> 22) & 0x1;
    config.m_shadow_registers              = (context->m_global_config1 >> 21) & 0x1;
    config.m_gamma_correction_technique    = (context->m_global_config1 >> 17) & 0x7;
    config.m_dithering_technique           = (context->m_global_config1 >> 14) & 0x3;
    config.m_precise_blending              = (context->m_global_config1 >> 12) & 0x1;
    config.m_red_width                     = (context->m_global_config1 >> 8) & 0xf;
    config.m_green_width                   = (context->m_global_config1 >> 4) & 0xf;
    config.m_blue_width                    = (context->m_global_config1 >> 0) & 0xf;
    config.m_slave_timing_mode_available   = (context->m_global_config2 >> 1) & 0x1;
    config.m_bg_layer_available            = (context->m_global_config2 >> 0) & 0x1;
  }
  return config;
}

/*--------------------------------------------------------------------------
 * function: cdc_getLayerCount
 * Get the number of layers.
 *
 * Parameters:
 *  a_handle   - CDC Context pointer
 *
 * Returns:
 *  The number of layers
 */
cdc_uint8 cdc_getLayerCount(cdc_handle a_handle)
{
  cdc_context *context;
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    return context->m_layer_count;
  }
  return 0;
}

/*--------------------------------------------------------------------------
 * function: cdc_getStatus
 * Get the current CDC status.
 *
 * The CDC status is its timing status (current x- and y-position and the blank/sync status)
 * as well as the slave timing status.
 *
 * Parameters:
 *  a_handle   - CDC Context pointer
 *
 * Returns:
 *  <cdc_global_status>, containing the current CDC status
 */
cdc_global_status cdc_getStatus(cdc_handle a_handle)
{
  cdc_context *context;
  cdc_global_status status;
  cdc_uint32 position;
  cdc_uint32 sync_status;
  cdc_uint32 st_status;
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    position    = cdc_arch_readReg(context, CDC_REG_GLOBAL_POSITION);
    sync_status = cdc_arch_readReg(context, CDC_REG_GLOBAL_SYNC_STATUS);
    st_status   = cdc_arch_readReg(context, CDC_REG_GLOBAL_SLAVE_TIMING_STATUS);
    
    status.m_x = (position >> 16) &0xffff;
    status.m_y = (position >> 0) &0xffff;
    status.m_hsync  = (sync_status >> 3) &0x1;
    status.m_vsync  = (sync_status >> 2) &0x1;
    status.m_hblank = (sync_status >> 1) &0x1;
    status.m_vblank = (sync_status >> 0) &0x1;
    status.m_low_frequency_mode = (st_status >> 16) &0x1;
    status.m_external_sync_line = (st_status >> 0) &0xffff;
  }
  return status;
}

/*--------------------------------------------------------------------------
 * function: cdc_triggerShadowReload
 * Trigger a reload of the shadow registers.
 *
 * Parameters:
 *  a_handle    - CDC Context pointer
 *  a_in_vblank - Reload in next vblank if CDC_TRUE, immediate reload otherwise
 *
 * Returns:
 *  CDC_TRUE if the shadow registers have not yet been updated 
 *   after the last request, CDC_FALSE else
 */
cdc_bool cdc_triggerShadowReload(cdc_handle a_handle, cdc_bool a_in_vblank)
{
  cdc_context *context;
  context = cdc_int_validateContext(a_handle);
  if (context && context->m_shadow_regs)
  {
    if(a_in_vblank==CDC_TRUE)
    {
      cdc_arch_writeReg(context, CDC_REG_GLOBAL_SHADOW_RELOAD, 2);
    }
    else
    {
      cdc_arch_writeReg(context, CDC_REG_GLOBAL_SHADOW_RELOAD, 1);
    }
    return CDC_TRUE;
  }
  return CDC_FALSE;
}

/*--------------------------------------------------------------------------
 * function: cdc_updatePending
 * Query if the CDC has processed all register changes (shadow status)
 *
 * Parameters:
 *  a_handle   - CDC Context pointer
 *
 * Returns:
 *  CDC_TRUE if the shadow registers have not yet been updated 
 *   after the last request, CDC_FALSE else
 */
cdc_bool cdc_updatePending(cdc_handle a_handle)
{
  cdc_context *context;
  context = cdc_int_validateContext(a_handle);
  if (context)
  {
    // when shadow registers are not used, update is never pending
    if (context->m_shadow_regs && cdc_arch_readReg(context, CDC_REG_GLOBAL_SHADOW_RELOAD))
    {
      return CDC_TRUE;
    }
  }
  return CDC_FALSE;
}

/*--------------------------------------------------------------------------
 * function: cdc_registerISR
 * Register interrupt callback for specified interrupt.
 *
 * Parameters:
 *  a_handle   - CDC Context pointer
 *  a_type     - The interrupt type
 *  a_callback - Pointer to a "void isr(cdc_uint32)" function
 *  a_data     - Data that will be passed to the called function
 *
 * See also:
 *  <cdc_irq_type>
 */
void cdc_registerISR(cdc_handle a_handle, cdc_irq_type a_type, cdc_isr_callback a_callback, cdc_uint32 a_data)
{
  cdc_context *context;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    switch(a_type)
    {
      case CDC_IRQ_LINE:
        context->m_irq_line = a_callback;
        context->m_irq_line_data = a_data;
        break;
      case CDC_IRQ_FIFO_UNDERRUN:
        context->m_irq_fifo_underrun = a_callback;
        context->m_irq_fifo_underrun_data = a_data;
        break;
      case CDC_IRQ_BUS_ERROR:
        context->m_irq_bus_error = a_callback;
        context->m_irq_bus_error_data = a_data;
        break;
      case CDC_IRQ_RELOAD:
        context->m_irq_reload = a_callback;
        context->m_irq_reload_data = a_data;
        break;
      case CDC_IRQ_SLAVE_TIMING_NO_SIGNAL:
        context->m_irq_slave_timing_no_signal = a_callback;
        context->m_irq_slave_timing_no_signal_data = a_data;
        break;
      case CDC_IRQ_SLAVE_TIMING_NO_SYNC:
        context->m_irq_slave_timing_no_sync = a_callback;
        context->m_irq_slave_timing_no_sync_data = a_data;
        break;
    }
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_setScanlineIRQPosition
 * Set the line number of the scanline IRQ, relative to the first visible line.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_line   - Line number in which the scanline IRQ 
 */
void cdc_setScanlineIRQPosition(cdc_handle a_handle, cdc_sint16 a_line)
{
  cdc_context *context;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    a_line += cdc_arch_readReg(context, CDC_REG_GLOBAL_BACK_PORCH) & 0xFFFF;
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_LINE_IRQ_POSITION, a_line);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_setEnabled
 * Enable/disable output of display controller.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_enable - CDC_TRUE to enable output, CDC_FALSE to disable output
 */
void cdc_setEnabled(cdc_handle a_handle, cdc_bool a_enable)
{
  cdc_context *context;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    context->m_enabled = a_enable;
    cdc_int_setEnabled(context, a_enable);
  }
}

/*--------------------------------------------------------------------------
 * Group: Global Settings
 *-------------------------------------------------------------------------- */


/*--------------------------------------------------------------------------
 * function: cdc_setTiming
 * Set the display timing (resolution).
 * Setting the display timing is done immediately and not synchronized 
 * with the vblank.
 * After a timing change all layers are disabled and the layer windows are set to the active area.
 *
 * Parameters:
 *  a_handle    - CDC Context pointer
 *  a_h_sync    - horizontal sync length in pixels
 *  a_h_bPorch  - horizontal backporch in pixels
 *  a_h_width   - display width in pixels
 *  a_h_fPorch  - horizontal front porch in pixels
 *  a_v_sync    - vertical sync length in pixels
 *  a_v_bPorch  - vertical backporch in pixels
 *  a_v_width   - display height in pixels
 *  a_v_fPorch  - vertical front porch in pixels
 *  a_clk       - pixel clk in Mhz (must be supported by platform dependent backend/pll)
 *  a_neg_hsync - hsync polarity, CDC_TRUE for negative hsync
 *  a_neg_vsync - vsync polarity, CDC_TRUE for negative vsync
 *  a_neg_blank - blank polarity, CDC_TRUE for negative blank
 *  a_inv_clk   - pixel clock polarity, CDC_TRUE for inverted pixel clock
 */
void cdc_setTiming(cdc_handle a_handle, cdc_uint16 a_h_sync, cdc_uint16 a_h_bPorch, cdc_uint16 a_h_width, cdc_uint16 a_h_fPorch, 
                                        cdc_uint16 a_v_sync, cdc_uint16 a_v_bPorch, cdc_uint16 a_v_width, cdc_uint16 a_v_fPorch,
                                        cdc_float a_clk, cdc_bool a_neg_hsync, cdc_bool a_neg_vsync, cdc_bool a_neg_blank, cdc_bool a_inv_clk)
{
  cdc_context *context;
  cdc_uint32 sync_size;
  cdc_uint32 back_porch;
  cdc_uint32 active_width;
  cdc_uint32 total_width;
  cdc_uint32 polarity_mask = 0;
  cdc_uint32 control;
  cdc_uint32 i;

  context = cdc_int_validateContext(a_handle);
  if(context)
  {
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
    cdc_int_setEnabled(context, CDC_FALSE);

    // set pll
    //TODO: check for errors
    cdc_arch_setPixelClk(a_clk);
    

    // set timing registers
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_SYNC_SIZE, sync_size);
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_BACK_PORCH, back_porch);
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_ACTIVE_WIDTH, active_width);
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_TOTAL_WIDTH, total_width);
    
    // set scanline irq line
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_LINE_IRQ_POSITION, (active_width&0x0000ffff)+1);
    
    // apply the sync polarity mask
    control = cdc_arch_readReg(context, CDC_REG_GLOBAL_CONTROL);
    control  = (control & ~(CDC_REG_GLOBAL_CONTROL_HSYNC|CDC_REG_GLOBAL_CONTROL_VSYNC|CDC_REG_GLOBAL_CONTROL_BLANK|CDC_REG_GLOBAL_CONTROL_CLK_POL)) | polarity_mask;
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_CONTROL, control);
    
    // disable all layers and reset windows
    for(i=0; i<context->m_layer_count;i++)
    {
      // disable layer
      context->m_layers[i].m_control &= ~CDC_REG_LAYER_CONTROL_ENABLE;
      cdc_arch_writeLayerReg(context, i, CDC_REG_LAYER_CONTROL, context->m_layers[i].m_control);
      
      // reset window
      cdc_arch_writeLayerReg(context, i, CDC_REG_LAYER_WINDOW_H, (active_width&0xffff0000u) | ((back_porch>>16)+1));
      cdc_arch_writeLayerReg(context, i, CDC_REG_LAYER_WINDOW_V, ((active_width&0xffffu)<<16) | ((back_porch&0xffffu)+1));
      context->m_layers[i].m_window_width = a_h_width;
      context->m_layers[i].m_window_height = a_v_width;
      cdc_arch_writeLayerReg(context, i, CDC_REG_LAYER_FB_LINES, a_v_width);
      context->m_layers[i].m_CB_pitch = 0;
      // TODO: reset alpha layer pitch if applicable
      cdc_int_updateBufferLength(context, i);
      // force reload of all shadowed registers
      cdc_arch_writeReg(context, CDC_REG_GLOBAL_SHADOW_RELOAD, 1);
    }
    
    // restore cdc enabled status
    cdc_int_setEnabled(context, context->m_enabled);
  }
}            

/*--------------------------------------------------------------------------
 * function: cdc_setBackgroundColor
 * Set the display background color.
 * The background color is only used, if layers are deactivated or
 * (partially) transparent and the background layer feature is disabled.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_color  - Background color in RGB888 format.
 *
 * See also:
 *  <cdc_setEnableBackgroundLayer>
 */
void cdc_setBackgroundColor(cdc_handle a_handle, cdc_uint32 a_color)
{
  cdc_context *context;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_BG_COLOR, a_color);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_uploadBackgroundLayer
 * Upload data for the background layer.
 *
 * The background layer contains up to 512 RGB24 values.
 *
 * Parameters:
 *  a_handle - CDC Context pointer
 *  a_start  - Start address in background layer RAM (0..511)
 *  a_length - Length of data (max. 512)
 *  a_data   - Pointer to background layer data.
 *
 * See also:
 *  <cdc_configureBackgroundLayer>
 *  <cdc_setEnableBackgroundLayer>
 */
void cdc_uploadBackgroundLayer(cdc_handle a_handle, cdc_uint32 a_start, cdc_uint32 a_length, cdc_uint32 *a_data)
{
  cdc_context *context;
  cdc_uint32 i;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    //TODO range checks
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_BG_LAYER_ADDR, a_start);
    for(i=0; i<a_length; i++)
    {
      cdc_arch_writeReg(context, CDC_REG_GLOBAL_BG_LAYER_DATA, a_data[i]);
    }
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_configureBackgroundLayer
 * Configure the background layer mode.
 *
 * Parameters:
 *  a_handle   - CDC Context pointer
 *  a_maskMode - Mask mode for the background layer (see cdc_bg_mode for possible modes)
 *  a_base     - Base address inside the background RAM (unsigned fixed point with 4 bit fractional)
 *  a_incX     - X-increment for the background RAM (signed fixed point with 4 bit fractional)
 *  a_incY     - Y-increment for the background RAM (signed fixed point with 4 bit fractional)
 *
 * See also:
 *  <cdc_uploadBackgroundLayer>
 *  <cdc_setEnableBackgroundLayer>
 *  <cdc_bg_mode>
 */
void cdc_configureBackgroundLayer(cdc_handle a_handle, cdc_bg_mode a_maskMode, cdc_uint16 a_base, cdc_sint16 a_incX, cdc_sint16 a_incY)
{
  cdc_context *context;
  // TODO range checks
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_BG_LAYER_BASE, (a_maskMode<<24)|a_base);
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_BG_LAYER_INC, ((((cdc_uint32)a_incX)&0xffff)<<16)|(((cdc_uint32)a_incY)&0xffff));
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_setEnableBackgroundLayer
 * Enable/disable the background layer
 *
 * Parameters:
 *  a_handle   - CDC Context pointer
 *  a_enable   - enable background layer if CDC_TRUE, else use background color
 *
 * See also:
 *  <cdc_setBackgroundColor>
 *  <cdc_uploadBackgroundLayer>
 *  <cdc_configureBackgroundLayer>
 */
void cdc_setEnableBackgroundLayer(cdc_handle a_handle, cdc_bool a_enable)
{
  cdc_context *context;
  cdc_uint32 control;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    control = cdc_arch_readReg(context, CDC_REG_GLOBAL_CONTROL);
    if(a_enable == CDC_TRUE)
    {
      control |= CDC_REG_GLOBAL_CONTROL_BACKGROUND_LAYER;
    }
    else
    {
      control &= ~CDC_REG_GLOBAL_CONTROL_BACKGROUND_LAYER;
    }
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_CONTROL, control);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_setDitherEnabled
 * Enable/disable dithering.
 * 
 * Parameters:
 *  a_handle   - CDC Context pointer
 *  a_enable   - Enable dithering if CDC_TRUE
 */
void cdc_setDitherEnabled(cdc_handle a_handle, cdc_bool a_enable)
{
  cdc_context *context;
  cdc_uint32 control;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    control = cdc_arch_readReg(context, CDC_REG_GLOBAL_CONTROL);
    if(a_enable == CDC_TRUE)
    {
      control |= CDC_REG_GLOBAL_CONTROL_DITHERING;
    }
    else
    {
      control &= ~CDC_REG_GLOBAL_CONTROL_DITHERING;
    }
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_CONTROL, control);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_setSlaveTimingModeEnabled
 * Enable/disable slave timing mode.
 * 
 * Parameters:
 *  a_handle   - CDC Context pointer
 *  a_enable   - enable slave timing mode if CDC_TRUE
 */
void cdc_setSlaveTimingModeEnabled(cdc_handle a_handle, cdc_bool a_enable)
{
  cdc_context *context;
  cdc_uint32 control;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    control = cdc_arch_readReg(context, CDC_REG_GLOBAL_CONTROL);
    if(a_enable == CDC_TRUE)
    {
      control |= CDC_REG_GLOBAL_CONTROL_SLAVE_TIMING;
    }
    else
    {
      control &= ~CDC_REG_GLOBAL_CONTROL_SLAVE_TIMING;
    }
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_CONTROL, control);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_setDualView
 * Enable dualview and specify clock pixel clock settings.
 * 
 * Parameters:
 *  a_handle            - CDC Context pointer
 *  a_enable            - Enable dual view
 *  a_subpixel_mixing   - Swap green channel for both views
 *  a_half_clock_even   - Enable clock for even pixels
 *  a_half_clock_odd    - Enable clock for odd pixels
 *  a_half_clock_shift  - Shift even and odd clocks to have their rising edge on the falling edge of the common clock
 */
void cdc_setDualView(cdc_handle a_handle, cdc_bool a_enable, cdc_bool a_subpixel_mixing, cdc_bool a_half_clock_even, cdc_bool a_half_clock_odd, cdc_bool a_half_clock_shift)
{
  cdc_context *context;
  cdc_uint32 control;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    control = cdc_arch_readReg(context, CDC_REG_GLOBAL_EXT_DISPLAY);
    if(a_enable == CDC_TRUE)
    {
      control |= 0x00100000u;
    }
    else
    {
      control &= ~0x00100000u;
    }
    if(a_subpixel_mixing == CDC_TRUE)
    {
      control |= 0x00200000u;
    }
    else
    {
      control &= ~0x00200000u;
    }
    if(a_half_clock_even == CDC_TRUE)
    {
      control |= 0x00400000u;
    }
    else
    {
      control &= ~0x00400000u;
    }
    if(a_half_clock_odd == CDC_TRUE)
    {
      control |= 0x00800000u;
    }
    else
    {
      control &= ~0x00800000u;
    }
    if(a_half_clock_shift == CDC_TRUE)
    {
      control |= 0x01000000u;
    }
    else
    {
      control &= ~0x01000000u;
    }
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_EXT_DISPLAY, control);
  }
}

/*--------------------------------------------------------------------------
 * function: cdc_setDualPort
 * Set the dual port mode.
 * 
 * Parameters:
 *  a_handle   - CDC Context pointer
 *  a_mode     - The mode of operation (see <cdc_dual_port_mode>)
 */
void cdc_setDualPort(cdc_handle a_handle, cdc_dual_port_mode a_mode)
{
  cdc_context *context;
  cdc_uint32 control;
  
  context = cdc_int_validateContext(a_handle);
  if(context)
  {
    control = cdc_arch_readReg(context, CDC_REG_GLOBAL_EXT_DISPLAY);
    control &= ~0x30000000u;
    control |= (a_mode & 3) << 28;
    cdc_arch_writeReg(context, CDC_REG_GLOBAL_EXT_DISPLAY, control);
  }
}

