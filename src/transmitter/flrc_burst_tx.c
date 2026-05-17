/*!
 * \file flrc_burst_tx.c
 *
 * \brief Transmit all the image data
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2026. All rights reserved.
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "smtc_rac.h"
#include "smtc_rac_api.h"
#include "smtc_sw_platform_helper.h"
#include "smtc_modem_hal.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER( tx, LOG_LEVEL_INF );

#include "flrc_burst_tx.h"
#include "transmitter_display.h"
#include "apps_configuration.h"
#include "../xiaosync/xiaosync.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/**
 * @brief Returns the minimum value between a and b
 *
 * @param [in] a 1st value
 * @param [in] b 2nd value
 * @retval Minimum value
 */
#ifndef MIN
#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

#if ( IS_DISPLAY_IMAGE_MODE )
/*!
 * @brief The array of all the image data
 */
extern const uint8_t image_chick_looking_buf[IMAGE_NUMS][1024];
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

typedef enum flrc_burst_tx_state_s
{
    FLRC_BURST_TX_STATE_IDLE,
    FLRC_BURST_TX_STATE_WOR_TX,
    FLRC_BURST_TX_STATE_WOR_ACK_RX,
    FLRC_BURST_TX_STATE_TX_DATA,
    FLRC_BURST_TX_STATE_RX_RESULT,
} flrc_burst_tx_state_t;

typedef struct flrc_burst_tx_s
{
    flrc_burst_tx_state_t   state;
    uint8_t                 radio_access_id;
    smtc_rac_context_t*     transaction;
    uint8_t*                tx_data;
    uint32_t                tx_data_size;
    uint16_t                tx_payload_buffer_size[2];
    uint8_t                 tx_flrc_payload_buffer[2][FLRC_RADIO_PAYLOAD_MAX_LENGTH];
    uint16_t                next_packet_buffer_index;
    uint16_t                nb_packets_to_send;
    uint32_t                bytes_sent;
    ral_flrc_raw_bit_rate_t raw_bit_rate;
} flrc_burst_tx_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

static flrc_burst_tx_t flrc_burst_tx = { 0 };
static uint8_t         lora_payload[WOR_LORA_PAYLOAD_LENGTH];

/*!
 * @brief Note the timestamp while all the image data are transmitted.
 */
static uint32_t tx_end_timestamp;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

static void start_new_tx_transaction( uint32_t timestamp_ms );
static void post_transaction_callback( rp_status_t status );
static void send_wor_packet( uint32_t tx_data_size, ral_flrc_raw_bit_rate_t raw_bit_rate, uint32_t timestamp_ms );
static void listen_wor_ack_packet( uint32_t timestamp_ms );

static void send_tx_data_packet( uint32_t timestamp_ms );
static void prepare_next_packet_buffer_tx( void );

static void listen_rx_result_packet( uint32_t timestamp_ms );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void image_transfer_entry( void )
{
    display_thread_init( );
    k_msleep( 10 );  // Delay for waiting the display finished

#if ( IS_DISPLAY_IMAGE_MODE )
    /* Add a 2-byte packet index to each packet. */
    flrc_burst_tx.tx_data_size = TOTAL_IMAGES_SIZE + FLRC_PACKET_NUMS * FLRC_PACKET_INDEX_LEN;
    /* Image data pointer */
    flrc_burst_tx.tx_data = ( uint8_t* ) image_chick_looking_buf;
#else
    flrc_burst_tx.tx_data_size = TRANSMIT_DATA_SIZES;
#endif

    uint32_t integer_part = RF_FREQ_IN_HZ / 1000000;
    uint32_t decimal_part = ( RF_FREQ_IN_HZ % 1000000 ) / 10000;
    display_row_texts( DISPLAY_ROW1, "Transmitter  %u.%uM", integer_part, decimal_part );
    display_row_texts( DISPLAY_ROW2, "%dKb/s  CR: %s", ral_flrc_raw_bit_rate_to_value( DATA_FLRC_BR_BPS ),
                       ral_flrc_cr_to_short_str( DATA_FLRC_CR ) );

    flrc_burst_tx.radio_access_id = SMTC_SW_PLATFORM( smtc_rac_open_radio( RAC_HIGH_PRIORITY ) );
    flrc_burst_tx.transaction     = smtc_rac_get_context( flrc_burst_tx.radio_access_id );

    flrc_burst_tx.transaction->scheduler_config.callback_pre_radio_transaction  = NULL;
    flrc_burst_tx.transaction->scheduler_config.callback_post_radio_transaction = post_transaction_callback;
    flrc_burst_tx.raw_bit_rate                                                  = DATA_FLRC_BR_BPS;

    LOG_INF( "\n" );
    LOG_INF( "===== TRANSMITTER RADIO CONFIGURATION =====" );
    LOG_INF( "Modulation: FLRC" );
    LOG_INF( "Frequency: %u.%u MHz", integer_part, decimal_part );
    LOG_INF( "TX Power: %d dBm", TX_OUTPUT_POWER_DBM );
    LOG_INF( "Bitrate: %s ", ral_flrc_raw_bit_rate_to_str( DATA_FLRC_BR_BPS ) );
    LOG_INF( "Coding Rate: %s", ral_flrc_cr_to_str( DATA_FLRC_CR ) );
    LOG_INF( "Preamble Length: %d bits", 4 * ( DATA_FLRC_PREAMBLE_BITS + 1 ) );
    LOG_INF( "The %d bytes will be transmitted. The number of transmitted packets: %d", flrc_burst_tx.tx_data_size,
             FLRC_PACKET_NUMS );

    flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
    start_new_tx_transaction( smtc_modem_hal_get_time_in_ms( ) + MARGIN_RADIO_MS );
}

void flrc_raw_bit_rate_changed( void )
{
    if( ( flrc_burst_tx.raw_bit_rate == RAL_FLRC_RAW_BIT_RATE_0_260_MBPS ) ||
        ( flrc_burst_tx.raw_bit_rate > RAL_FLRC_RAW_BIT_RATE_2_600_MBPS ) )
    {
        flrc_burst_tx.raw_bit_rate = RAL_FLRC_RAW_BIT_RATE_2_600_MBPS;
    }
    else
    {
        flrc_burst_tx.raw_bit_rate--;
    }
    /* Change the data rate message on the row 2 of screen */
    display_row_texts( DISPLAY_ROW2, "%dKb/s  CR: %s", ral_flrc_raw_bit_rate_to_value( flrc_burst_tx.raw_bit_rate ),
                       ral_flrc_cr_to_short_str( DATA_FLRC_CR ) );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void start_new_tx_transaction( uint32_t timestamp_ms )
{
    LOG_INF( "\n" );
    LOG_INF( "Send a WOR packet in the %u ms.", timestamp_ms - smtc_modem_hal_get_time_in_ms( ) );
    /* Show the "Waiting......" message on the row 5 of screen. */
    update_percentage_display( 0 );

    flrc_burst_tx.state = FLRC_BURST_TX_STATE_WOR_TX;
    send_wor_packet( flrc_burst_tx.tx_data_size, flrc_burst_tx.raw_bit_rate, timestamp_ms );
    tx_end_timestamp = 0;
}

static void post_transaction_callback( rp_status_t status )
{
    switch( status )
    {
    case RP_STATUS_TX_DONE:
        if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_WOR_TX )
        {
            flrc_burst_tx.state = FLRC_BURST_TX_STATE_WOR_ACK_RX;
            listen_wor_ack_packet( flrc_burst_tx.transaction->smtc_rac_data_result.radio_end_timestamp_ms +
                                   MARGIN_RADIO_MS / 2 );
        }
        break;
    case RP_STATUS_RX_PACKET:
        if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_WOR_ACK_RX )
        {
            if( ( lora_payload[0] == 'W' ) && ( lora_payload[1] == 'O' ) && ( lora_payload[2] == 'R' ) &&
                ( lora_payload[3] == ' ' ) && ( lora_payload[4] == 'O' ) && ( lora_payload[5] == 'K' ) )
            {
                LOG_INF( "WOR ACK received" );
                flrc_burst_tx.state = FLRC_BURST_TX_STATE_TX_DATA;
                send_tx_data_packet( flrc_burst_tx.transaction->smtc_rac_data_result.radio_end_timestamp_ms +
                                     MARGIN_RADIO_MS );
            }
            else
            {
                LOG_WRN( "WOR ACK not received." );
                flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
            }
        }
        else if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_RX_RESULT )
        {
            if( ( lora_payload[0] == 'E' ) && ( lora_payload[1] == 'N' ) )  // End of transmission
            {
                uint32_t rx_data_size = 0;
                int16_t  mean_rssi;

                rx_data_size = ( uint32_t ) lora_payload[2];
                rx_data_size |= ( uint32_t ) lora_payload[3] << 8;
                rx_data_size |= ( uint32_t ) lora_payload[4] << 16;
                rx_data_size |= ( uint32_t ) lora_payload[5] << 24;
                mean_rssi = ( int16_t ) ( ( ( int16_t ) lora_payload[7] << 8 ) | ( uint8_t ) lora_payload[6] );

                uint8_t per_result = ( uint8_t ) ( ( ( flrc_burst_tx.tx_data_size - rx_data_size ) * 100 ) /
                                                   flrc_burst_tx.tx_data_size );
                LOG_INF( "%u bytes were received by receiver.", rx_data_size );
                LOG_INF( "PER: %u%%  RSSI: %d dBm", per_result, mean_rssi );
                display_row_texts( DISPLAY_ROW4, "PER: %u%% RSSI: %d", per_result, mean_rssi );
            }
            else
            {
                LOG_WRN( "A wrong 'END' packet was received." );
                display_row_texts( DISPLAY_ROW4, "PER: %u%% RSSI: %d", 100, 0 );
            }
            flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
        }
        break;
    case RP_STATUS_REQUEST_NEXT_TX_PAYLOAD:
        // Packet n is sent, prepare the next packet (n+2) if exists
        // Packet (n+1) should be already prepared, to be able to send it at the same time
        if( flrc_burst_tx.nb_packets_to_send > flrc_burst_tx.next_packet_buffer_index )
        {
            prepare_next_packet_buffer_tx( );
        }
        /* Show the percentage of transmission on the row 5 of screen. */
        update_percentage_display( flrc_burst_tx.next_packet_buffer_index * 100 / flrc_burst_tx.nb_packets_to_send );
        break;
    case RP_STATUS_RX_TIMEOUT:
        if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_WOR_ACK_RX )
        {
            LOG_WRN( "Rx timeout(listen WOR ACK)" );
        }
        else if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_RX_RESULT )
        {
            display_row_texts( DISPLAY_ROW4, "PER: %u%% RSSI: %d", 100, 0 );
            LOG_WRN( "Rx timeout(listen RX result)" );
        }
        else
        {
            LOG_WRN( "Rx timeout(state: %u)", flrc_burst_tx.state );
        }
        flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
        break;
    case RP_STATUS_RX_CRC_ERROR:
        if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_WOR_ACK_RX )
        {
            LOG_WRN( "CRC error(listen WOR ACK)" );
        }
        else if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_RX_RESULT )
        {
            display_row_texts( DISPLAY_ROW4, "PER: %u%% RSSI: %d", 100, 0 );
            LOG_WRN( "CRC error(listen RX result)" );
        }
        else
        {
            LOG_WRN( "CRC error(state: %u)", flrc_burst_tx.state );
        }
        flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
        break;
    case RP_STATUS_TASK_ABORTED:
    {
        LOG_WRN( "Task aborted(state: %u)", flrc_burst_tx.state );
        flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
        break;
    }
    case RP_STATUS_RADIO_LOCKED:
    {
        LOG_WRN( "Radio locked(state: %u)", flrc_burst_tx.state );
        break;
    }
    case RP_STATUS_RADIO_UNLOCKED:
    {
        // Status that indicate the end of TX
        uint32_t duration_ms = flrc_burst_tx.transaction->smtc_rac_data_result.radio_end_timestamp_ms -
                               flrc_burst_tx.transaction->smtc_rac_data_result.radio_start_timestamp_ms;
        tx_end_timestamp = flrc_burst_tx.transaction->smtc_rac_data_result.radio_end_timestamp_ms;


        /* --- INIZIO AGGIUNTA SINCRONIZZAZIONE --- */
        int64_t sync_time_end = getDeltaMicroSeconds();
        if (sync_time_end >= 0) {
            printk("TX: Fine invio immagine (Burst) a %lld us dal Big Bang\n", sync_time_end);
        }
        /* --- FINE AGGIUNTA SINCRONIZZAZIONE --- */

        if( duration_ms > 0 )
        {
            // Calculate bits per second first to avoid overflow
            uint32_t bit_rate_bps        = ( flrc_burst_tx.tx_data_size * 8U * 1000U ) / duration_ms;
            uint32_t bit_rate_integer    = bit_rate_bps / 1000000U;
            uint32_t bit_rate_fractional = bit_rate_bps % 1000000U;
            LOG_INF( "TX transaction %u bytes in %u ms", flrc_burst_tx.tx_data_size, duration_ms );
            LOG_INF( "Calculate the effective bit rate: %u.%03u Mbit/s", bit_rate_integer, bit_rate_fractional );
            display_row_texts( DISPLAY_ROW3, "Elapse: %u ms", duration_ms );
        }
        else
        {
            LOG_ERR( "Transaction duration: %u ms (invalid)", duration_ms );
        }
        flrc_burst_tx.state = FLRC_BURST_TX_STATE_RX_RESULT;
        break;
    }
    default:
    {
        LOG_ERR( "Unknown transaction status: %d", status );
    }
    }

    if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_IDLE )
    {
        if( tx_end_timestamp > 0 )
        {
            start_new_tx_transaction( tx_end_timestamp + SLEEP_DELAY_MS + MARGIN_RADIO_MS );
        }
        else
        {
            start_new_tx_transaction( smtc_modem_hal_get_time_in_ms( ) + MARGIN_RADIO_MS );
        }
    }
    else if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_RX_RESULT )
    {
        listen_rx_result_packet( smtc_modem_hal_get_time_in_ms( ) + MARGIN_RADIO_MS / 2 );
    }
}

static void send_wor_packet( uint32_t tx_data_size, ral_flrc_raw_bit_rate_t raw_bit_rate, uint32_t timestamp_ms )
{
    flrc_burst_tx.transaction->modulation_type = SMTC_RAC_MODULATION_LORA;

    flrc_burst_tx.transaction->radio_params.lora.is_tx                = true;
    flrc_burst_tx.transaction->radio_params.lora.is_ranging_exchange  = false;
    flrc_burst_tx.transaction->radio_params.lora.frequency_in_hz      = RF_FREQ_IN_HZ;
    flrc_burst_tx.transaction->radio_params.lora.tx_power_in_dbm      = TX_OUTPUT_POWER_DBM;
    flrc_burst_tx.transaction->radio_params.lora.sf                   = WOR_LORA_SPREADING_FACTOR;
    flrc_burst_tx.transaction->radio_params.lora.bw                   = WOR_LORA_BANDWIDTH;
    flrc_burst_tx.transaction->radio_params.lora.cr                   = WOR_LORA_CODING_RATE;
    flrc_burst_tx.transaction->radio_params.lora.preamble_len_in_symb = WOR_LORA_PREAMBLE_LENGTH;
    flrc_burst_tx.transaction->radio_params.lora.header_type          = WOR_LORA_PKT_LEN_MODE;
    flrc_burst_tx.transaction->radio_params.lora.invert_iq_is_on      = WOR_LORA_IQ;
    flrc_burst_tx.transaction->radio_params.lora.crc_is_on            = WOR_LORA_CRC;
    flrc_burst_tx.transaction->radio_params.lora.sync_word            = WOR_LORA_SYNCWORD;
    flrc_burst_tx.transaction->radio_params.lora.tx_size              = sizeof( lora_payload );
    flrc_burst_tx.transaction->lbt_context.lbt_enabled                = false;

    memset( lora_payload, 0, sizeof( lora_payload ) );
    lora_payload[0] = 'W';
    lora_payload[1] = 'O';
    lora_payload[2] = 'R';
    lora_payload[3] = ( uint8_t ) ( tx_data_size & 0xFF );
    lora_payload[4] = ( uint8_t ) ( ( tx_data_size >> 8 ) & 0xFF );
    lora_payload[5] = ( uint8_t ) ( ( tx_data_size >> 16 ) & 0xFF );
    lora_payload[6] = ( uint8_t ) ( ( tx_data_size >> 24 ) & 0xFF );
    lora_payload[7] = raw_bit_rate;

    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.tx_payload_buffer         = lora_payload;
    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.size_of_tx_payload_buffer = sizeof( lora_payload );

    flrc_burst_tx.transaction->scheduler_config.scheduling    = SMTC_RAC_ASAP_TRANSACTION;
    flrc_burst_tx.transaction->scheduler_config.start_time_ms = timestamp_ms;
    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_tx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
}

static void listen_wor_ack_packet( uint32_t timestamp_ms )
{
    flrc_burst_tx.transaction->radio_params.lora.is_tx           = false;
    flrc_burst_tx.transaction->radio_params.lora.invert_iq_is_on = WOR_ACK_LORA_IQ;
    flrc_burst_tx.transaction->radio_params.lora.max_rx_size     = sizeof( lora_payload );
    flrc_burst_tx.transaction->radio_params.lora.rx_timeout_ms   = WOR_RX_TIMEOUT;

    memset( lora_payload, 0, sizeof( lora_payload ) );
    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.rx_payload_buffer         = lora_payload;
    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.size_of_rx_payload_buffer = sizeof( lora_payload );

    flrc_burst_tx.transaction->scheduler_config.scheduling    = SMTC_RAC_SCHEDULED_TRANSACTION;
    flrc_burst_tx.transaction->scheduler_config.start_time_ms = timestamp_ms;
    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_tx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
}

static void send_tx_data_packet( uint32_t timestamp_ms )
{
    flrc_burst_tx.next_packet_buffer_index = 0;
    flrc_burst_tx.bytes_sent               = 0;

    flrc_burst_tx.transaction->modulation_type = SMTC_RAC_MODULATION_FLRC_BURST;

    flrc_burst_tx.transaction->radio_params.flrc_burst.is_tx             = true;
    flrc_burst_tx.transaction->radio_params.flrc_burst.frequency_in_hz   = RF_FREQ_IN_HZ;
    flrc_burst_tx.transaction->radio_params.flrc_burst.tx_power_in_dbm   = TX_OUTPUT_POWER_DBM;
    flrc_burst_tx.transaction->radio_params.flrc_burst.raw_bit_rate      = flrc_burst_tx.raw_bit_rate;
    flrc_burst_tx.transaction->radio_params.flrc_burst.cr                = DATA_FLRC_CR;
    flrc_burst_tx.transaction->radio_params.flrc_burst.pulse_shape       = DATA_FLRC_PULSE_SHAPE;
    flrc_burst_tx.transaction->radio_params.flrc_burst.preamble_len      = DATA_FLRC_PREAMBLE_BITS;
    flrc_burst_tx.transaction->radio_params.flrc_burst.sync_word_len     = DATA_FLRC_SYNCWORD_LEN;
    flrc_burst_tx.transaction->radio_params.flrc_burst.tx_syncword_index = DATA_FLRC_SYNCWORD;
    flrc_burst_tx.transaction->radio_params.flrc_burst.match_sync_word   = DATA_FLRC_MATCH_SYNCWORD;
    flrc_burst_tx.transaction->radio_params.flrc_burst.pld_is_fix        = DATA_FLRC_PLD_IS_FIX;
    flrc_burst_tx.transaction->radio_params.flrc_burst.crc_type          = DATA_FLRC_CRC;
    flrc_burst_tx.transaction->radio_params.flrc_burst.sync_word[0]      = &default_syncword_1[0];
    flrc_burst_tx.transaction->radio_params.flrc_burst.crc_seed          = 0xffffffff;
    flrc_burst_tx.transaction->radio_params.flrc_burst.crc_polynomial    = 0x755B;

    /* poly flrc compatible sx1280:
      #define FLRC_SX1280_CRC_POLY_16 0x755B
      #define FLRC_SX1280_CRC_POLY_24 0x5D6DCB
      #define FLRC_SX1280_CRC_POLY_32 0xF4ACFB13
      seed=0xffffffff */

    /* Indicate the total size of the data to sent */
    flrc_burst_tx.transaction->radio_params.flrc_burst.burst_tx_size = flrc_burst_tx.tx_data_size;
    /* Packets of FLRC_RADIO_PAYLOAD_MAX_LENGTH size (except possibly the last packet) */
    flrc_burst_tx.nb_packets_to_send =
        flrc_burst_tx.tx_data_size / ( FLRC_RADIO_PAYLOAD_MAX_LENGTH ) +
        ( ( flrc_burst_tx.tx_data_size % ( FLRC_RADIO_PAYLOAD_MAX_LENGTH ) != 0 ) ? 1 : 0 );

    // Prepare first and 2nd buffer to send
    prepare_next_packet_buffer_tx( );
    if( flrc_burst_tx.nb_packets_to_send > 1 )
    {
        prepare_next_packet_buffer_tx( );
    }

    flrc_burst_tx.transaction->radio_params.flrc_burst.size_of_tx_fifo_payload_buffer[0] =
        FLRC_RADIO_PAYLOAD_MAX_LENGTH;
    flrc_burst_tx.transaction->radio_params.flrc_burst.size_of_tx_fifo_payload_buffer[1] =
        FLRC_RADIO_PAYLOAD_MAX_LENGTH;

    flrc_burst_tx.transaction->scheduler_config.start_time_ms = timestamp_ms;
    flrc_burst_tx.transaction->scheduler_config.scheduling    = SMTC_RAC_ASAP_TRANSACTION;

    /* --- INIZIO AGGIUNTA SINCRONIZZAZIONE --- */
    int64_t sync_time = getDeltaMicroSeconds();
    if (sync_time >= 0) {
        printk("TX: Inizio invio immagine (Burst) a %lld us dal Big Bang\n", sync_time);
    }
    /* --- FINE AGGIUNTA SINCRONIZZAZIONE --- */

    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_tx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
}

static void prepare_next_packet_buffer_tx( void )
{
    uint32_t size_payload_left_to_send;
    uint16_t buffer_index = ( flrc_burst_tx.next_packet_buffer_index % 2 );
    /* Add two bytes packet index at the beginning of each packet */
    flrc_burst_tx.tx_flrc_payload_buffer[buffer_index][0] = flrc_burst_tx.next_packet_buffer_index;
    flrc_burst_tx.tx_flrc_payload_buffer[buffer_index][1] = flrc_burst_tx.next_packet_buffer_index >> 8;

#if ( IS_DISPLAY_IMAGE_MODE )
    size_payload_left_to_send                          = TOTAL_IMAGES_SIZE - flrc_burst_tx.bytes_sent;
    flrc_burst_tx.tx_payload_buffer_size[buffer_index] = MIN( size_payload_left_to_send, FLRC_DATA_LENGTH );
    /* Copy the image data into TX buffer */
    memcpy( &flrc_burst_tx.tx_flrc_payload_buffer[buffer_index][2], &flrc_burst_tx.tx_data[flrc_burst_tx.bytes_sent],
            flrc_burst_tx.tx_payload_buffer_size[buffer_index] );

    /* Accumulate the length of iamge data */
    flrc_burst_tx.bytes_sent += ( flrc_burst_tx.tx_payload_buffer_size[buffer_index] );
    /* Add the packet index length for each packet. It is the real transmission data length of radio. */
    flrc_burst_tx.tx_payload_buffer_size[buffer_index] += FLRC_PACKET_INDEX_LEN;
#else
    size_payload_left_to_send = flrc_burst_tx.tx_data_size - flrc_burst_tx.bytes_sent;
    flrc_burst_tx.tx_payload_buffer_size[buffer_index] =
        MIN( size_payload_left_to_send, FLRC_RADIO_PAYLOAD_MAX_LENGTH );
    /* Every packet's length is greater than or equal to 6 according to TRANSMIT_DATA_SIZES. */
    uint32_t data_length = flrc_burst_tx.tx_payload_buffer_size[buffer_index] - FLRC_PACKET_INDEX_LEN;
    if( buffer_index == 0 )
    {
        memset( &flrc_burst_tx.tx_flrc_payload_buffer[buffer_index][2], 0x11, data_length );
    }
    else
    {
        memset( &flrc_burst_tx.tx_flrc_payload_buffer[buffer_index][2], 0x22, data_length );
    }

    /* Add the packet index length for each packet. It is the real transmission data length of radio. */
    flrc_burst_tx.bytes_sent += ( flrc_burst_tx.tx_payload_buffer_size[buffer_index] );
#endif

    /* TX buffer pointer */
    flrc_burst_tx.transaction->radio_params.flrc_burst.tx_fifo_payload_buffer[buffer_index] =
        flrc_burst_tx.tx_flrc_payload_buffer[buffer_index];
    flrc_burst_tx.transaction->radio_params.flrc_burst.tx_fifo_payload_length[buffer_index] =
        &flrc_burst_tx.tx_payload_buffer_size[buffer_index];

    flrc_burst_tx.next_packet_buffer_index++;
}

static void listen_rx_result_packet( uint32_t timestamp_ms )
{
    flrc_burst_tx.transaction->modulation_type = SMTC_RAC_MODULATION_LORA;

    flrc_burst_tx.transaction->radio_params.lora.is_tx                = false;
    flrc_burst_tx.transaction->radio_params.lora.is_ranging_exchange  = false;
    flrc_burst_tx.transaction->radio_params.lora.frequency_in_hz      = RF_FREQ_IN_HZ;
    flrc_burst_tx.transaction->radio_params.lora.sf                   = WOR_LORA_SPREADING_FACTOR;
    flrc_burst_tx.transaction->radio_params.lora.bw                   = WOR_LORA_BANDWIDTH;
    flrc_burst_tx.transaction->radio_params.lora.cr                   = WOR_LORA_CODING_RATE;
    flrc_burst_tx.transaction->radio_params.lora.preamble_len_in_symb = WOR_LORA_PREAMBLE_LENGTH;
    flrc_burst_tx.transaction->radio_params.lora.header_type          = WOR_LORA_PKT_LEN_MODE;
    flrc_burst_tx.transaction->radio_params.lora.invert_iq_is_on      = WOR_ACK_LORA_IQ;
    flrc_burst_tx.transaction->radio_params.lora.crc_is_on            = WOR_LORA_CRC;
    flrc_burst_tx.transaction->radio_params.lora.sync_word            = WOR_LORA_SYNCWORD;
    flrc_burst_tx.transaction->radio_params.lora.rx_timeout_ms        = WOR_RX_TIMEOUT;
    flrc_burst_tx.transaction->radio_params.lora.max_rx_size          = sizeof( lora_payload );

    memset( lora_payload, 0, sizeof( lora_payload ) );
    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.rx_payload_buffer         = lora_payload;
    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.size_of_rx_payload_buffer = sizeof( lora_payload );

    flrc_burst_tx.transaction->scheduler_config.scheduling    = SMTC_RAC_SCHEDULED_TRANSACTION;
    flrc_burst_tx.transaction->scheduler_config.start_time_ms = timestamp_ms;
    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_tx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
}