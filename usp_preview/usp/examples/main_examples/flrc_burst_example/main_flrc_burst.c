/*!
 * \file      main_flrc_burst.c
 *
 * \brief     main program for FLRC burst example
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include <stdio.h>
#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type
#include <string.h>

#include "main.h"

#include "smtc_modem_hal.h"

#include "example_options.h"

// Use unified logging system
#define RAC_LOG_APP_PREFIX "MAIN-BURST-FLRC"

#include "smtc_hal_mcu.h"
#include "smtc_hal_gpio.h"
#include "smtc_hal_watchdog.h"
#include "modem_pinout.h"
#include "smtc_rac_log.h"

#include "apps_configuration.h"

#include "smtc_rac_api.h"

#include "smtc_sw_platform_helper.h"

#include "flrc_burst_rx.h"
#include "flrc_burst_tx.h"
/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

#define RECEIVER 1
#define TRANSMITTER 2

#if( ROLE == RECEIVER )

#elif( ROLE == TRANSMITTER )

#else
#error "Please define ROLE as either RECEIVER or TRANSMITTER"
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

static const uint32_t SLEEP_DELAY = 4000;  // ms

/**
 * @brief Watchdog counter reload value during sleep (The period must be lower than MCU watchdog period (here 32s))
 */
#define WATCHDOG_RELOAD_PERIOD_MS 20000

#if( ROLE == TRANSMITTER )
#define DATA_SIZE ( 20 * 1024 )
#else
#define DATA_SIZE ( 20 * 1024 )
#endif
/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

typedef struct user_button_s
{
    bool     is_pressed;
    uint32_t last_press_timestamp;
} user_button_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */
static const uint32_t BUTTON_TRIGGER_DELAY = 500;  // ms

static user_button_t user_button = {
    .is_pressed           = false,
    .last_press_timestamp = 0,
};

#if( ROLE == TRANSMITTER )
static uint8_t        data_flrc_count = 0;
static uint32_t       tx_timestamp_ms = 0;
#endif

static uint8_t data[DATA_SIZE];  // TX (data to send) or RX (data received)

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

#if( ROLE == RECEIVER )
static void data_received_cb( void );
#endif

#if( ROLE == TRANSMITTER )
static void data_sent_cb( bool is_successful );
#endif

static void user_button_callback( void* context );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

/**
 * @brief Example to send data using the Burst FLRC protocol
 *
 */
void main_flrc_burst( void )
{
    hal_mcu_init( );

    smtc_rac_init( );

    hal_gpio_irq_t nucleo_blue_button = {
        .pin      = EXTI_BUTTON,
        .context  = &user_button,
        .callback = user_button_callback,
    };
    hal_gpio_init_in( EXTI_BUTTON, BSP_GPIO_PULL_MODE_NONE, BSP_GPIO_IRQ_MODE_FALLING, &nucleo_blue_button );

    hal_gpio_init_out( SMTC_LED_TX, false );
    hal_gpio_init_out( SMTC_LED_RX, false );

#if( ROLE == RECEIVER )
    flrc_burst_protocol_rx_init( data, sizeof( data ), data_received_cb );
#elif( ROLE == TRANSMITTER )
    flrc_burst_protocol_tx_init( data_sent_cb );
#endif

    while( true )
    {
        hal_watchdog_reload( );
        smtc_rac_run_engine( );

#if( ROLE == TRANSMITTER )
        if( ( user_button.is_pressed == true ) ||
            ( ( smtc_modem_hal_get_time_in_ms( ) - tx_timestamp_ms ) > SLEEP_DELAY ) )
        {
            tx_timestamp_ms = smtc_modem_hal_get_time_in_ms( );

            // RAC_LOG_INFO( "button pressed or timer\n" );

            if( flrc_burst_protocol_tx_is_ready( ) )
            {
                // Prepare new data to send
                for( uint32_t i = 0; i < DATA_SIZE; i++ )
                {
                    if( i % 128 == 0 )
                    {
                        data_flrc_count++;
                    }
                    data[i] = data_flrc_count;
                }
                user_button.is_pressed = false;

                flrc_burst_protocol_tx_start_new_transaction( data, sizeof( data ) );
            }
        }
#endif

        hal_mcu_disable_irq( );
        if( ( user_button.is_pressed == false ) && ( smtc_rac_is_irq_flag_pending( ) == false ) )
        {
            hal_mcu_set_sleep_for_ms( SLEEP_DELAY );
            hal_watchdog_reload( );
        }
        hal_mcu_enable_irq( );
    }
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void user_button_callback( void* context )
{
    UNUSED( context );
    uint32_t timestamp = smtc_modem_hal_get_time_in_ms( );
    if( ( timestamp - user_button.last_press_timestamp ) > BUTTON_TRIGGER_DELAY )
    {
        user_button.last_press_timestamp = timestamp;
        user_button.is_pressed           = true;
    }
}

#if( ROLE == RECEIVER )
static void data_received_cb( void )
{
    RAC_LOG_INFO( "Data OK" );
}
#endif

#if( ROLE == TRANSMITTER )
static void data_sent_cb( bool is_successful )
{
    if( is_successful )
    {
        RAC_LOG_INFO( "Data sent" );
    }
    else
    {
        RAC_LOG_INFO( "Failed to sent data" );
    }
}
#endif
/* --- EOF ------------------------------------------------------------------ */
