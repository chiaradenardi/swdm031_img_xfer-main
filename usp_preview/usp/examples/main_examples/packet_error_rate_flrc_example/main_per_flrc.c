/**
 * @file      main_per_flrc.c
 *
 * @brief     Simple PER (Packet Error Rate) example with FLRC modulation
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2025. All rights reserved.
 */

#include "app_per_flrc.h"

#include "smtc_hal_button.h"
#include "smtc_hal_led.h"
#include "smtc_hal_mcu.h"
#include "smtc_hal_watchdog.h"
#include "smtc_rac_api.h"

#define RAC_LOG_APP_PREFIX "MAIN-PER-FLRC"
#include "smtc_rac_log.h"

static const uint32_t SLEEP_DELAY = 1000;  // ms

void main_per_flrc( void )
{
    hal_mcu_init( );

    // Initialize LEDs and button
    hal_led_init( );
    hal_button_init( NULL, NULL );

    smtc_rac_init( );

    per_flrc_init( );

    while( true )
    {
        hal_watchdog_reload( );
        smtc_rac_run_engine( );

        if( hal_button_is_pressed( ) )
        {
            hal_button_clear( );
            RAC_LOG_INFO( "button pressed\n" );
            per_flrc_on_button_press( );
        }

        hal_mcu_disable_irq( );
        if( ( !hal_button_is_pressed( ) ) && ( smtc_rac_is_irq_flag_pending( ) == false ) )
        {
            hal_mcu_set_sleep_for_ms( SLEEP_DELAY );
            hal_watchdog_reload( );
        }
        hal_mcu_enable_irq( );
    }
}
