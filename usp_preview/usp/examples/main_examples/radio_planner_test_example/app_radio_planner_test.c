/**
 * @file      app_radio_planner_test.c
 *
 * @brief     Radio Planner stress test application implementation
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2025. All rights reserved.
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

#include "app_radio_planner_test.h"
#include <stdint.h>
#include "smtc_rac_api.h"
#include "smtc_modem_hal.h"
#include "radio_planner.h"
#include "radio_planner_types.h"
#include "smtc_rac_api.h"

// Use unified logging system
#define RAC_LOG_APP_PREFIX "APP-RP-TEST"
#include "smtc_rac_log.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS ----------------------------------------------------------
 */

typedef struct radio_planner_test_s
{
    uint8_t             index;            // index of the radio access
    uint8_t             radio_access_id;  // store result of `smtc_rac_open_radio`
    smtc_rac_context_t* transaction;      // associated transaction
    uint8_t             payload_tx[RP_TEST_PAYLOAD_LENGTH];
    uint8_t             payload_rx[RP_TEST_PAYLOAD_LENGTH];
    statistics_t        statistics;
    bool                launch_this_task;

} radio_planner_test_t;
static const char* const priority_names[NUMBER_OF_RADIO_ACCESS] = { "VERY_HIGH_PRIORITY", "HIGH_PRIORITY",
                                                                    "MEDIUM_PRIORITY", "LOW_PRIORITY",
                                                                    "VERY_LOW_PRIORITY" };

static void common_callback_post_radio_transaction( rp_status_t status, uint8_t index );
static void callback_pre_radio_transaction_task_very_high_priority( void );
static void callback_pre_radio_transaction_task_high_priority( void );
static void callback_pre_radio_transaction_task_medium_priority( void );
static void callback_pre_radio_transaction_task_low_priority( void );
static void callback_pre_radio_transaction_task_very_low_priority( void );
static void callback_post_radio_transaction_task_very_high_priority( rp_status_t status );
static void callback_post_radio_transaction_task_high_priority( rp_status_t status );
static void callback_post_radio_transaction_task_medium_priority( rp_status_t status );
static void callback_post_radio_transaction_task_low_priority( rp_status_t status );
static void callback_post_radio_transaction_task_very_low_priority( rp_status_t status );
void ( *callback_pre_radio_transaction[NUMBER_OF_RADIO_ACCESS] )( void ) = {
    callback_pre_radio_transaction_task_very_high_priority, callback_pre_radio_transaction_task_high_priority,
    callback_pre_radio_transaction_task_medium_priority, callback_pre_radio_transaction_task_low_priority,
    callback_pre_radio_transaction_task_very_low_priority
};
void ( *callback_post_radio_transaction[NUMBER_OF_RADIO_ACCESS] )( rp_status_t status ) = {
    callback_post_radio_transaction_task_very_high_priority, callback_post_radio_transaction_task_high_priority,
    callback_post_radio_transaction_task_medium_priority, callback_post_radio_transaction_task_low_priority,
    callback_post_radio_transaction_task_very_low_priority
};
static void     launch_task_callbacks_task_very_high_priority( void );
static void     launch_task_callbacks_task_high_priority( void );
static void     launch_task_callbacks_task_medium_priority( void );
static void     launch_task_callbacks_task_low_priority( void );
static void     launch_task_callbacks_task_very_low_priority( void );
static void     print_full_statistics( void );
static uint32_t test_launch_time_ms;
void ( *launch_task_callbacks[NUMBER_OF_RADIO_ACCESS] )( void ) = { launch_task_callbacks_task_very_high_priority,
                                                                    launch_task_callbacks_task_high_priority,
                                                                    launch_task_callbacks_task_medium_priority,
                                                                    launch_task_callbacks_task_low_priority,
                                                                    launch_task_callbacks_task_very_low_priority };
static radio_planner_test_t radio_planner_test[NUMBER_OF_RADIO_ACCESS];
#define GET_NOW_MS( ) smtc_modem_hal_get_time_in_ms( )
#define SET_START_DELAY_MS( target_time_ms, index )                         \
    radio_planner_test[index].transaction->scheduler_config.start_time_ms = \
        smtc_modem_hal_get_time_in_ms( ) + target_time_ms

#define SET_SCHEDULING_ASAP( index ) \
    radio_planner_test[index].transaction->scheduler_config.scheduling = SMTC_RAC_ASAP_TRANSACTION
#define SET_SCHEDULING_SCHEDULED( index ) \
    radio_planner_test[index].transaction->scheduler_config.scheduling = SMTC_RAC_SCHEDULED_TRANSACTION
#define START_TRANSACTION( index )                                                                              \
    while( smtc_rac_submit_radio_transaction( radio_planner_test[index].radio_access_id ) != SMTC_RAC_SUCCESS ) \
    {                                                                                                           \
        radio_planner_test[index].transaction->scheduler_config.start_time_ms =                                 \
            smtc_modem_hal_get_time_in_ms( ) + 1000;                                                            \
    }
#define ACTIVE_TASK( index ) radio_planner_test[index].launch_this_task = true
#define DISABLE_TASK( index ) radio_planner_test[index].launch_this_task = false
/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

// Test payload buffers

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void radio_planner_test_init( void )
{
    SMTC_MODEM_HAL_TRACE_PRINTF( "Initializing Radio Planner Test Suite...\n" );

    // Initialize test metrics

    radio_planner_test[0].radio_access_id = smtc_rac_open_radio( RAC_VERY_HIGH_PRIORITY );
    radio_planner_test[1].radio_access_id = smtc_rac_open_radio( RAC_HIGH_PRIORITY );
    radio_planner_test[2].radio_access_id = smtc_rac_open_radio( RAC_MEDIUM_PRIORITY );
    radio_planner_test[3].radio_access_id = smtc_rac_open_radio( RAC_LOW_PRIORITY );
    radio_planner_test[4].radio_access_id = smtc_rac_open_radio( RAC_VERY_LOW_PRIORITY );
    for( int i = 0; i < NUMBER_OF_RADIO_ACCESS; i++ )
    {
        DISABLE_TASK( i );
    }
    ACTIVE_TASK( VERY_HIGH_PRIORITY );
    ACTIVE_TASK( HIGH_PRIORITY );
    ACTIVE_TASK( MEDIUM_PRIORITY );
    // ACTIVE_TASK( LOW_PRIORITY );
    ACTIVE_TASK( VERY_LOW_PRIORITY );

    for( int i = 0; i < NUMBER_OF_RADIO_ACCESS; i++ )
    {
        memset( &radio_planner_test[i].statistics, 0, sizeof( statistics_t ) );
        radio_planner_test[i].index       = i;
        radio_planner_test[i].transaction = smtc_rac_get_context( radio_planner_test[i].radio_access_id );
        radio_planner_test[i].transaction->smtc_rac_data_buffer_setup.tx_payload_buffer =
            &radio_planner_test[i].payload_tx[0];
        radio_planner_test[i].transaction->smtc_rac_data_buffer_setup.rx_payload_buffer =
            &radio_planner_test[i].payload_rx[0];
        radio_planner_test[i].transaction->smtc_rac_data_buffer_setup.size_of_tx_payload_buffer =
            sizeof( radio_planner_test[i].payload_tx );
        radio_planner_test[i].transaction->smtc_rac_data_buffer_setup.size_of_rx_payload_buffer =
            sizeof( radio_planner_test[i].payload_rx );
        radio_planner_test[i].transaction->modulation_type                        = SMTC_RAC_MODULATION_LORA;
        radio_planner_test[i].transaction->radio_params.lora.is_tx                = true;
        radio_planner_test[i].transaction->radio_params.lora.is_ranging_exchange  = false;
        radio_planner_test[i].transaction->radio_params.lora.frequency_in_hz      = RF_FREQ_IN_HZ + i * 1000000;
        radio_planner_test[i].transaction->radio_params.lora.tx_power_in_dbm      = TX_OUTPUT_POWER_DBM - ( i * 2 );
        radio_planner_test[i].transaction->radio_params.lora.sf                   = LORA_SPREADING_FACTOR;
        radio_planner_test[i].transaction->radio_params.lora.bw                   = LORA_BANDWIDTH;
        radio_planner_test[i].transaction->radio_params.lora.cr                   = LORA_CODING_RATE;
        radio_planner_test[i].transaction->radio_params.lora.preamble_len_in_symb = LORA_PREAMBLE_LENGTH;
        radio_planner_test[i].transaction->radio_params.lora.header_type          = LORA_PKT_LEN_MODE;
        radio_planner_test[i].transaction->radio_params.lora.invert_iq_is_on      = LORA_IQ;
        radio_planner_test[i].transaction->radio_params.lora.crc_is_on            = LORA_CRC;
        radio_planner_test[i].transaction->radio_params.lora.sync_word            = 0x34;
        radio_planner_test[i].transaction->radio_params.lora.tx_size              = RP_TEST_PAYLOAD_LENGTH;
        radio_planner_test[i].transaction->radio_params.lora.max_rx_size = sizeof( radio_planner_test[i].payload_rx );

        radio_planner_test[i].transaction->scheduler_config.start_time_ms    = 0;    // set at each transaction
        radio_planner_test[i].transaction->scheduler_config.duration_time_ms = 200;  // set at each transaction
        radio_planner_test[i].transaction->scheduler_config.scheduling =
            SMTC_RAC_ASAP_TRANSACTION;  // set at each transaction
        radio_planner_test[i].transaction->scheduler_config.callback_post_radio_transaction =
            callback_post_radio_transaction[i];
        radio_planner_test[i].transaction->scheduler_config.callback_pre_radio_transaction =
            callback_pre_radio_transaction[i];
        if( radio_planner_test[i].launch_this_task )
        {
            launch_task_callbacks[i]( );
        }
    }
    test_launch_time_ms = smtc_modem_hal_get_time_in_ms( );
}

static void callback_pre_radio_transaction_task_very_high_priority( void )
{
    radio_planner_test[VERY_HIGH_PRIORITY].transaction->smtc_rac_data_result.radio_start_timestamp_ms =
        smtc_modem_hal_get_time_in_ms( );
    print_full_statistics( );
    smtc_rac_unlock_radio_access( radio_planner_test[VERY_HIGH_PRIORITY].radio_access_id );
}
static void callback_pre_radio_transaction_task_high_priority( void )
{
}

static void callback_pre_radio_transaction_task_medium_priority( void )
{
}

static void callback_pre_radio_transaction_task_low_priority( void )
{
}

static void callback_pre_radio_transaction_task_very_low_priority( void )
{
}

static void callback_post_radio_transaction_task_very_high_priority( rp_status_t status )
{
    common_callback_post_radio_transaction( status, VERY_HIGH_PRIORITY );
}
static void callback_post_radio_transaction_task_high_priority( rp_status_t status )
{
    common_callback_post_radio_transaction( status, HIGH_PRIORITY );
}

static void callback_post_radio_transaction_task_medium_priority( rp_status_t status )
{
    common_callback_post_radio_transaction( status, MEDIUM_PRIORITY );
}

static void callback_post_radio_transaction_task_low_priority( rp_status_t status )
{
    common_callback_post_radio_transaction( status, LOW_PRIORITY );
}

static void callback_post_radio_transaction_task_very_low_priority( rp_status_t status )
{
    common_callback_post_radio_transaction( status, VERY_LOW_PRIORITY );
}

static void launch_task_callbacks_task_very_high_priority( void )
{
    uint8_t index = VERY_HIGH_PRIORITY;

    SET_START_DELAY_MS( smtc_modem_hal_get_random_nb_in_range( 30000, 30000 ), index );
    SET_SCHEDULING_SCHEDULED( index );
    // START_TRANSACTION( index );
    smtc_rac_lock_radio_access( radio_planner_test[index].radio_access_id,
                                radio_planner_test[index].transaction->scheduler_config );
    radio_planner_test[index].statistics.total_transactions_launched++;
}

static void launch_task_callbacks_task_high_priority( void )
{
    uint8_t index = HIGH_PRIORITY;

    SET_START_DELAY_MS( smtc_modem_hal_get_random_nb_in_range( 100, 10000 ), index );
    SET_SCHEDULING_SCHEDULED( index );
    START_TRANSACTION( index );
    radio_planner_test[index].statistics.total_transactions_launched++;
}
static void launch_task_callbacks_task_medium_priority( void )
{
    uint8_t index                                                        = MEDIUM_PRIORITY;
    radio_planner_test[index].transaction->radio_params.lora.is_tx       = true;
    radio_planner_test[index].transaction->cad_context.cad_enabled       = true;
    radio_planner_test[index].transaction->cad_context.cad_symb_nb       = RAL_LORA_CAD_04_SYMB;
    radio_planner_test[index].transaction->cad_context.cad_exit_mode     = RAL_LORA_CAD_LBT;
    radio_planner_test[index].transaction->cad_context.cad_timeout_in_ms = 10000;
    radio_planner_test[index].transaction->radio_params.lora.sf          = RAL_LORA_SF9;
    radio_planner_test[index].transaction->cad_context.sf                = RAL_LORA_SF9;
    radio_planner_test[index].transaction->cad_context.bw                = LORA_BANDWIDTH;
    radio_planner_test[index].transaction->cad_context.rf_freq_in_hz     = RF_FREQ_IN_HZ + index * 1000000;
    radio_planner_test[index].transaction->cad_context.invert_iq_is_on   = LORA_IQ;

    SET_START_DELAY_MS( smtc_modem_hal_get_random_nb_in_range( 100, 10000 ), index );
    SET_SCHEDULING_SCHEDULED( index );
    START_TRANSACTION( index );
    radio_planner_test[index].statistics.total_transactions_launched++;
}
static void launch_task_callbacks_task_low_priority( void )
{
    uint8_t index                                                          = LOW_PRIORITY;
    radio_planner_test[index].transaction->radio_params.lora.is_tx         = false;
    radio_planner_test[index].transaction->cad_context.cad_enabled         = false;
    radio_planner_test[index].transaction->cad_context.cad_symb_nb         = RAL_LORA_CAD_04_SYMB;
    radio_planner_test[index].transaction->cad_context.cad_exit_mode       = RAL_LORA_CAD_RX;
    radio_planner_test[index].transaction->cad_context.cad_timeout_in_ms   = 1000;
    radio_planner_test[index].transaction->cad_context.sf                  = LORA_SPREADING_FACTOR;
    radio_planner_test[index].transaction->cad_context.bw                  = LORA_BANDWIDTH;
    radio_planner_test[index].transaction->cad_context.rf_freq_in_hz       = RF_FREQ_IN_HZ + index * 1000000;
    radio_planner_test[index].transaction->cad_context.invert_iq_is_on     = LORA_IQ;
    radio_planner_test[index].transaction->radio_params.lora.rx_timeout_ms = 100;
    // SET_START_DELAY_MS( smtc_modem_hal_get_random_nb_in_range( 100, 10000 ), index );
    radio_planner_test[index].transaction->scheduler_config.start_time_ms = smtc_modem_hal_get_time_in_ms( ) + 1000;
    SET_SCHEDULING_ASAP( index );
    START_TRANSACTION( index );
    radio_planner_test[index].statistics.total_transactions_launched++;
}
static void launch_task_callbacks_task_very_low_priority( void )
{
    // rx continouous mode with 1 second timeout
    uint8_t index = VERY_LOW_PRIORITY;

    radio_planner_test[index].transaction->radio_params.lora.is_tx         = false;
    radio_planner_test[index].transaction->radio_params.lora.rx_timeout_ms = 10000;
    smtc_rac_set_active_time_out( radio_planner_test[index].radio_access_id,
                                  smtc_modem_hal_get_random_nb_in_range( 1000, 4000 ) );
    SET_START_DELAY_MS( smtc_modem_hal_get_random_nb_in_range( 10, 100 ), index );
    SET_SCHEDULING_ASAP( index );
    START_TRANSACTION( index );
    radio_planner_test[index].statistics.total_transactions_launched++;
}
static void print_full_statistics( void )
{
    SMTC_MODEM_HAL_TRACE_PRINTF( "Full statistics:\n" );
    uint32_t total_transactions_radio_time_ms = 0;
    for( int i = 0; i < NUMBER_OF_RADIO_ACCESS; i++ )
    {
        print_statistics( i );
        total_transactions_radio_time_ms += radio_planner_test[i].statistics.total_transactions_radio_time_ms;
    }
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions radio time: %d ms , test duration = %d ms\n",
                                 total_transactions_radio_time_ms,
                                 smtc_modem_hal_get_time_in_ms( ) - test_launch_time_ms );
    //                            uint32_t tmp = radio_planner_test[0].statistics.total_transactions_radio_time_ms ;
    // memset( &radio_planner_test[0].statistics, 0, sizeof( statistics_t ) * NUMBER_OF_RADIO_ACCESS );
    // radio_planner_test[0].statistics.total_transactions_radio_time_ms += tmp;
}
void print_statistics( uint8_t index )
{
    SMTC_MODEM_HAL_TRACE_PRINTF( "Statistics:\n" );

    SMTC_MODEM_HAL_TRACE_PRINTF( "**************************************************\n" );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Statistics for priority %s:\n", priority_names[index] );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions launched: %d\n",
                                 radio_planner_test[index].statistics.total_transactions_launched );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions completed: %d\n",
                                 radio_planner_test[index].statistics.total_transactions_completed );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions tx: %d\n",
                                 radio_planner_test[index].statistics.total_transactions_tx );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions rx: %d\n",
                                 radio_planner_test[index].statistics.total_transactions_rx );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions cad positive: %d\n",
                                 radio_planner_test[index].statistics.total_transactions_cad_positive );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions cad negative: %d\n",
                                 radio_planner_test[index].statistics.total_transactions_cad_negative );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions aborted: %d\n",
                                 radio_planner_test[index].statistics.total_transactions_aborted );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions errors: %d\n",
                                 radio_planner_test[index].statistics.total_transactions_errors );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Last transaction time: %d ms\n",
                                 radio_planner_test[index].statistics.last_transaction_time_ms );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions radio locked: %d\n",
                                 radio_planner_test[index].statistics.total_transactions_radio_locked );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions radio unlocked: %d\n",
                                 radio_planner_test[index].statistics.total_transactions_radio_unlocked );
    SMTC_MODEM_HAL_TRACE_PRINTF( "Total transactions radio time: %d ms\n",
                                 radio_planner_test[index].statistics.total_transactions_radio_time_ms );
    SMTC_MODEM_HAL_TRACE_PRINTF( "**************************************************\n" );
}

static void common_callback_post_radio_transaction( rp_status_t status, uint8_t index )
{
    radio_planner_test[index].statistics.last_transaction_time_ms = GET_NOW_MS( );
    if( radio_planner_test[index].transaction->smtc_rac_data_result.radio_start_timestamp_ms !=
        0 )  // don't count the aborted task never launched
    {
        radio_planner_test[0].statistics.total_transactions_radio_time_ms +=
            radio_planner_test[index].transaction->smtc_rac_data_result.radio_end_timestamp_ms -
            radio_planner_test[index].transaction->smtc_rac_data_result.radio_start_timestamp_ms;
    }
    radio_planner_test[index].statistics.total_transactions_completed++;

    switch( status )
    {
    case RP_STATUS_TX_DONE:
        radio_planner_test[index].statistics.total_transactions_tx++;
        break;
    case RP_STATUS_CAD_POSITIVE:
        radio_planner_test[index].statistics.total_transactions_cad_positive++;
        break;
    case RP_STATUS_CAD_NEGATIVE:
        radio_planner_test[index].statistics.total_transactions_cad_negative++;
        break;
    case RP_STATUS_RX_PACKET:
    case RP_STATUS_RX_TIMEOUT:
        radio_planner_test[index].statistics.total_transactions_rx++;
        break;

    case RP_STATUS_TASK_ABORTED:

        radio_planner_test[index].statistics.total_transactions_aborted++;

        break;
    case RP_STATUS_RADIO_LOCKED:

        radio_planner_test[index].statistics.total_transactions_radio_locked++;  // should never append
        break;

    case RP_STATUS_RADIO_UNLOCKED:
        radio_planner_test[index].statistics.total_transactions_radio_unlocked++;
        break;

    default:
        radio_planner_test[index].statistics.total_transactions_errors++;
        SMTC_MODEM_HAL_TRACE_ERROR( "Unexpected transaction status: %d for priority %s\n", status,
                                    priority_names[index] );
        while( 1 )
        {
        }  // wait for watchdog to reset the system
        break;
    }

    launch_task_callbacks[index]( );
}