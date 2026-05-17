/**
 * @file      ral_udp_pf_bsp.c
 *
 * @brief     Board Support Package for UDP Packet Forwarder (stub - no physical radio)
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2025. All rights reserved.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ral_udp_pf_bsp.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void ral_udp_pf_bsp_get_rf_switch_cfg( const void* context, ral_udp_pf_bsp_rf_switch_cfg_t* rf_switch_cfg )
{
    // UDP virtual radio has no RF switch configuration
    ( void ) context;
    ( void ) rf_switch_cfg;
}

void ral_udp_pf_bsp_get_tx_cfg( const void* context, const ral_udp_pf_bsp_tx_cfg_input_params_t* input_params,
                                ral_udp_pf_bsp_tx_cfg_output_params_t* output_params )
{
    // UDP virtual radio - just pass through power settings
    ( void ) context;
    if( output_params != NULL )
    {
        output_params->chip_output_pwr_in_dbm_configured = input_params->system_output_pwr_in_dbm;
        output_params->chip_output_pwr_in_dbm_expected   = input_params->system_output_pwr_in_dbm;
    }
}

void ral_udp_pf_bsp_get_reg_mode( const void* context, ral_udp_pf_bsp_reg_mode_t* reg_mode )
{
    // UDP virtual radio has no regulator mode
    ( void ) context;
    if( reg_mode != NULL )
    {
        *reg_mode = RAL_UDP_PF_BSP_REG_MODE_DCDC;
    }
}

void ral_udp_pf_bsp_get_xosc_cfg( const void* context, ral_xosc_cfg_t* xosc_cfg,
                                  ral_udp_pf_bsp_xosc_cfg_t* tcxo_is_radio_controlled, uint32_t* supply_voltage,
                                  uint32_t* startup_time_in_tick )
{
    // UDP virtual radio has no crystal oscillator
    ( void ) context;
    if( xosc_cfg != NULL )
    {
        *xosc_cfg = RAL_XOSC_CFG_XTAL;
    }
    if( tcxo_is_radio_controlled != NULL )
    {
        *tcxo_is_radio_controlled = RAL_UDP_PF_BSP_XOSC_CFG_XTAL;
    }
    if( supply_voltage != NULL )
    {
        *supply_voltage = 0;
    }
    if( startup_time_in_tick != NULL )
    {
        *startup_time_in_tick = 0;
    }
}

void ral_udp_pf_bsp_get_trim_cap( const void* context, uint8_t* trimming_cap_xta, uint8_t* trimming_cap_xtb )
{
    // UDP virtual radio has no trimming capacitors
    ( void ) context;
    if( trimming_cap_xta != NULL )
    {
        *trimming_cap_xta = 0;
    }
    if( trimming_cap_xtb != NULL )
    {
        *trimming_cap_xtb = 0;
    }
}

void ral_udp_pf_bsp_get_rx_boost_cfg( const void* context, bool* rx_boost_is_activated )
{
    // UDP virtual radio has no RX boost
    ( void ) context;
    if( rx_boost_is_activated != NULL )
    {
        *rx_boost_is_activated = false;
    }
}

void ral_udp_pf_bsp_get_ocp_value( const void* context, uint8_t* ocp_in_step_of_2_5_ma )
{
    // UDP virtual radio has no over-current protection
    ( void ) context;
    if( ocp_in_step_of_2_5_ma != NULL )
    {
        *ocp_in_step_of_2_5_ma = 0;
    }
}

void ral_udp_pf_bsp_get_lora_cad_det_peak( ral_lora_sf_t sf, ral_lora_bw_t bw, ral_lora_cad_symbs_t nb_symbol,
                                           uint8_t* in_out_cad_det_peak )
{
    // UDP virtual radio - use default CAD detection peak
    ( void ) sf;
    ( void ) bw;
    ( void ) nb_symbol;
    if( in_out_cad_det_peak != NULL )
    {
        *in_out_cad_det_peak = 22;  // Default value
    }
}

/* --- EOF ------------------------------------------------------------------ */
