/*!
 * \file    flrc_burst_rx.c
 *
 * \brief   Receive the image data and put them into FIFO for display thread
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
LOG_MODULE_REGISTER( rx, LOG_LEVEL_INF );

#include "receiver_display.h"
#include "apps_configuration.h"
#include "../xiaosync/xiaosync.h"
/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

typedef enum flrc_burst_rx_state_s
{
    FLRC_BURST_RX_STATE_IDLE,
    FLRC_BURST_RX_STATE_CAD,
    FLRC_BURST_RX_STATE_WOR_ACK_TX,
    FLRC_BURST_RX_STATE_RX_DATA,
    FLRC_BURST_RX_STATE_TX_RESULT,
} flrc_burst_rx_state_t;

typedef struct flrc_burst_rx_protocol_rx_s
{
    flrc_burst_rx_state_t state;
    uint8_t               radio_access_id;
    smtc_rac_context_t*   transaction;
    uint8_t               rx_flrc_payload_buffer[FLRC_RADIO_PAYLOAD_MAX_LENGTH];
    uint32_t              rx_bytes_received;
    uint32_t              rx_data_size_from_wor;  // Size of the total payload to receive indicated in the WOR received
    ral_flrc_raw_bit_rate_t br_bw_from_wor;       // Received in the WOR
    int32_t                 lora_freq_offset_hz;
    uint16_t                nb_packets_received;  /*!< Number of packets of the burst received successfully. */
    uint16_t                nb_packets_crc_error; /*!< Number of packets of the burst received with a CRC error. */
    int32_t                 rssi_mean;
} flrc_burst_rx_protocol_rx_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

#if ( IS_DISPLAY_IMAGE_MODE )
/*!
 * @brief Define two FIFO buffers and a semaphore for showing image data on the screen.
 */
K_FIFO_DEFINE( free_buffers );
K_FIFO_DEFINE( filled_buffers );
K_SEM_DEFINE( clear_buf_sem, 0, 1 );

/*!
 * @brief Define a variable between the current thread and display thread
 */
static display_pkt_data_t pkt_data[FLRC_PACKET_NUMS];

static pkt_buffer_t pkt_buf = { .free_buf    = &free_buffers,
                                .filled_buf  = &filled_buffers,
                                .clr_buf_sem = &clear_buf_sem };
#endif

static flrc_burst_rx_protocol_rx_t flrc_burst_rx = { 0 };
static uint8_t                     lora_payload[WOR_LORA_PAYLOAD_LENGTH];

/*!
 * @brief Note the timestamp while all the image data are received.
 */
static uint32_t rx_end_timestamp;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

static void post_transaction_callback( rp_status_t status );
static void listen_wor_packet( uint32_t timestamp_ms );
static void send_wor_ack_tx_packet( uint32_t timestamp_ms );
static void listen_rx_data_packet( uint32_t timestamp_ms );

static void send_result_tx_packet( uint32_t timestamp_ms, uint32_t received_size, int16_t mean_rssi );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void image_transfer_entry( void )
{
#if ( IS_DISPLAY_IMAGE_MODE )
    /* Initialize the image data FIFO */
    for( uint32_t i = 0; i < FLRC_PACKET_NUMS; i++ )
    {
        k_fifo_put( &free_buffers, &pkt_data[i] );
    }
    /* Initialize the display */
    display_thread_init( &pkt_buf );
#else
    display_thread_init( NULL );
#endif

    k_msleep( 10 );  // Delay for waiting the display finished

    uint32_t integer_part = RF_FREQ_IN_HZ / 1000000;
    uint32_t decimal_part = ( RF_FREQ_IN_HZ % 1000000 ) / 10000;
    display_row_texts( DISPLAY_ROW1, "Receiver  %u.%uM", integer_part, decimal_part );
    display_row_texts( DISPLAY_ROW2, "%dKb/s CR: %s", ral_flrc_raw_bit_rate_to_value( DATA_FLRC_BR_BPS ),
                       ral_flrc_cr_to_short_str( DATA_FLRC_CR ) );

    memset( &flrc_burst_rx, 0, sizeof( flrc_burst_rx ) );

    flrc_burst_rx.radio_access_id = SMTC_SW_PLATFORM( smtc_rac_open_radio( RAC_HIGH_PRIORITY ) );
    flrc_burst_rx.transaction     = smtc_rac_get_context( flrc_burst_rx.radio_access_id );
    flrc_burst_rx.transaction->scheduler_config.callback_post_radio_transaction = post_transaction_callback;
    flrc_burst_rx.transaction->scheduler_config.callback_pre_radio_transaction  = NULL;

    LOG_INF( "\n" );
    LOG_INF( "===== RECEIVER RADIO CONFIGURATION =====" );
    LOG_INF( "Modulation: FLRC" );
    LOG_INF( "Frequency: %u.%u MHz", integer_part, decimal_part );
    LOG_INF( "TX Power: %d dBm", TX_OUTPUT_POWER_DBM );
    LOG_INF( "Bitrate: %s ", ral_flrc_raw_bit_rate_to_str( DATA_FLRC_BR_BPS ) );
    LOG_INF( "Coding Rate: %s", ral_flrc_cr_to_str( DATA_FLRC_CR ) );
    LOG_INF( "Preamble Length: %d bits", 4 * ( DATA_FLRC_PREAMBLE_BITS + 1 ) );

    /* Start to listen the WOR packet */
    flrc_burst_rx.state = FLRC_BURST_RX_STATE_CAD;
    rx_end_timestamp    = 0;
    listen_wor_packet( smtc_modem_hal_get_time_in_ms( ) + MARGIN_RADIO_MS);
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void post_transaction_callback( rp_status_t status )
{
    switch( status )
    {
    case RP_STATUS_TX_DONE:
        LOG_INF("TX callback status: %d", status);

        if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_WOR_ACK_TX )
        {
            flrc_burst_rx.state = FLRC_BURST_RX_STATE_RX_DATA;
            listen_rx_data_packet( flrc_burst_rx.transaction->smtc_rac_data_result.radio_end_timestamp_ms +
                                   MARGIN_RADIO_MS / 2 );
        }
        else if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_TX_RESULT )
        {
            flrc_burst_rx.state = FLRC_BURST_RX_STATE_CAD;
        }
        break;

    case RP_STATUS_RX_PACKET:
        LOG_INF("WOR packet RICEVUTO! Status: %d", status);
        if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_CAD )
        {
            if( ( lora_payload[0] == 'W' ) && ( lora_payload[1] == 'O' ) && ( lora_payload[2] == 'R' ) )
            {
                uint32_t rx_data_size_from_wor_tmp = 0;

                rx_data_size_from_wor_tmp = ( uint32_t ) lora_payload[3];
                rx_data_size_from_wor_tmp |= ( uint32_t ) lora_payload[4] << 8;
                rx_data_size_from_wor_tmp |= ( uint32_t ) lora_payload[5] << 16;
                rx_data_size_from_wor_tmp |= ( uint32_t ) lora_payload[6] << 24;
                /* Get data size from transmitter side. */
                flrc_burst_rx.rx_data_size_from_wor = rx_data_size_from_wor_tmp;
                /* Get raw bit rate from transmitter side. */
                flrc_burst_rx.br_bw_from_wor      = lora_payload[7];
                flrc_burst_rx.lora_freq_offset_hz = flrc_burst_rx.transaction->smtc_rac_data_result.lora_freq_offset_hz;
                flrc_burst_rx.state               = FLRC_BURST_RX_STATE_WOR_ACK_TX;
                send_wor_ack_tx_packet( flrc_burst_rx.transaction->smtc_rac_data_result.radio_end_timestamp_ms +
                                        MARGIN_RADIO_MS );
#if ( IS_DISPLAY_IMAGE_MODE )
                /* CLear FIFO and give a semaphore to display thread. Get ready for the next reception. */
                k_sem_give( pkt_buf.clr_buf_sem );
                while( !k_fifo_is_empty( pkt_buf.filled_buf ) )
                {
                    void* data = k_fifo_get( pkt_buf.filled_buf, K_NO_WAIT );
                    k_fifo_put( pkt_buf.free_buf, data );
                }
#else
                /* Change the raw bit rate message on the row 2 of screen */
                display_row_texts( DISPLAY_ROW2, "%dKb/s  CR: %s",
                                   ral_flrc_raw_bit_rate_to_value( flrc_burst_rx.br_bw_from_wor ),
                                   ral_flrc_cr_to_short_str( DATA_FLRC_CR ) );
#endif
            }
            else
            {
                LOG_ERR( "A wrong WOR packet." );
            }
        }
        else if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_RX_DATA )
        {
            uint16_t packet_idx = 0;

            packet_idx = flrc_burst_rx.rx_flrc_payload_buffer[0];
            packet_idx |= flrc_burst_rx.rx_flrc_payload_buffer[1] << 8;
            flrc_burst_rx.rx_bytes_received += flrc_burst_rx.transaction->smtc_rac_data_result.rx_size;

            flrc_burst_rx.rssi_mean = flrc_burst_rx.rssi_mean * flrc_burst_rx.nb_packets_received;
            flrc_burst_rx.rssi_mean += flrc_burst_rx.transaction->smtc_rac_data_result.rssi_result;
            flrc_burst_rx.nb_packets_received++;
            flrc_burst_rx.rssi_mean /= flrc_burst_rx.nb_packets_received;

            if( flrc_burst_rx.rx_bytes_received == flrc_burst_rx.transaction->radio_params.flrc_burst.burst_rx_size )
            {
                smtc_rac_flrc_burst_rx_done( flrc_burst_rx.radio_access_id );
            }

#if ( IS_DISPLAY_IMAGE_MODE )
            display_pkt_data_t* pkt_data_fifo = k_fifo_get( pkt_buf.free_buf, K_NO_WAIT );
            if( pkt_data_fifo == NULL )
            {
                LOG_ERR( "No space in the free buffer" );
            }
            else
            {
                /* Image data length = packet length - packet index length */
                uint32_t image_data_len =
                    flrc_burst_rx.transaction->smtc_rac_data_result.rx_size - FLRC_PACKET_INDEX_LEN;
                memcpy( pkt_data_fifo->data, &flrc_burst_rx.rx_flrc_payload_buffer[2], image_data_len );
                pkt_data_fifo->data_len = image_data_len;
                pkt_data_fifo->pkt_idx  = packet_idx;
                k_fifo_put( pkt_buf.filled_buf, pkt_data_fifo );
            }
#else
            /* Update the percentage on the row 5 of screen */
            int value = ( int ) ( ( uint64_t ) flrc_burst_rx.rx_bytes_received * 100 /
                                  flrc_burst_rx.transaction->radio_params.flrc_burst.burst_rx_size );
            update_percentage_display( value );
#endif
        }
        break;

    case RP_STATUS_RX_TIMEOUT:
        if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_CAD )
        {
            LOG_WRN( "Rx timeout(CAD)" );
        }
        else
        {
            LOG_WRN( "Rx timeout(state: %u)", flrc_burst_rx.state );
        }
        break;
    case RP_STATUS_RX_CRC_ERROR:
        if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_CAD )
        {
            LOG_WRN( "CRC error(CAD)" );
        }
        else if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_RX_DATA )
        {
            flrc_burst_rx.nb_packets_crc_error++;
            LOG_WRN( " CRC error (%u) data packets", flrc_burst_rx.nb_packets_crc_error );
        }
        else
        {
            LOG_WRN( "CRC error(state: %u)", flrc_burst_rx.state );
        }
        break;
    case RP_STATUS_TASK_ABORTED:
    {
        LOG_WRN( "Task aborted(state: %u)", flrc_burst_rx.state );
        if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_RX_DATA )
        {
            LOG_INF( "RX data ( burst stats : packet rx %u, crc error %u, rssi mean %d dBm)",
                     flrc_burst_rx.nb_packets_received, flrc_burst_rx.nb_packets_crc_error, flrc_burst_rx.rssi_mean );
        }
        flrc_burst_rx.state = FLRC_BURST_RX_STATE_CAD;
        break;
    }
    case RP_STATUS_RADIO_LOCKED:
    {
        LOG_WRN( "Radio locked(state: %u)", flrc_burst_rx.state );
        break;
    }
    case RP_STATUS_RADIO_UNLOCKED:
    {
        if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_RX_DATA )
        {
            LOG_INF( "Data received (%u bytes). freq_offset_hz: %d Hz", flrc_burst_rx.rx_bytes_received,
                     flrc_burst_rx.transaction->radio_params.flrc_burst.rx_frequency_offset_in_hz );
            LOG_INF( "Radio ( burst stats : packet rx %u, crc error %u, rssi mean %d dBm)",
                     flrc_burst_rx.nb_packets_received, flrc_burst_rx.nb_packets_crc_error, flrc_burst_rx.rssi_mean );

            /* --- INIZIO AGGIUNTA SINCRONIZZAZIONE --- */
            int64_t sync_time_end = getDeltaMicroSeconds();
            if (sync_time_end >= 0) {
                printk("RX: Fine ricezione immagine (Burst) a %lld us dal Big Bang\n", sync_time_end);
            }
            /* --- FINE AGGIUNTA SINCRONIZZAZIONE --- */

            uint8_t per_result = ( uint8_t ) ( ( ( flrc_burst_rx.transaction->radio_params.flrc_burst.burst_rx_size -
                                                   flrc_burst_rx.rx_bytes_received ) *
                                                 100 ) /
                                               flrc_burst_rx.transaction->radio_params.flrc_burst.burst_rx_size );
            LOG_INF( "PER: %u%%", per_result );
#if ( !IS_DISPLAY_IMAGE_MODE )
            display_row_texts( DISPLAY_ROW3, "RCV: %d B", flrc_burst_rx.rx_bytes_received );
            display_row_texts( DISPLAY_ROW4, "PER: %u%% RSSI: %d", per_result, flrc_burst_rx.rssi_mean );
#endif
            flrc_burst_rx.state = FLRC_BURST_RX_STATE_TX_RESULT;
            rx_end_timestamp    = flrc_burst_rx.transaction->smtc_rac_data_result.radio_end_timestamp_ms;
        }
        break;
    }
    default:
    {
        LOG_ERR( "Unknown transaction status: %d", status );
    }
    }

    // Restart RX
    if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_CAD )
    {
        if( rx_end_timestamp > 0 )
        {
            listen_wor_packet( rx_end_timestamp + SLEEP_DELAY_MS + MARGIN_RADIO_MS / 2 );
            rx_end_timestamp = 0;
        }
        else
        {
            listen_wor_packet( smtc_modem_hal_get_time_in_ms( ) + MARGIN_RADIO_MS / 2 );
        }
    }
    else if( flrc_burst_rx.state == FLRC_BURST_RX_STATE_TX_RESULT )
    {
        send_result_tx_packet( smtc_modem_hal_get_time_in_ms( ) + MARGIN_RADIO_MS, flrc_burst_rx.rx_bytes_received,
                               flrc_burst_rx.rssi_mean );
    }
}

static void listen_wor_packet( uint32_t timestamp_ms )
{
    uint32_t now = smtc_modem_hal_get_time_in_ms();
    int32_t delay = (int32_t)(timestamp_ms - now);
    
    LOG_INF("RX CONFIG: now=%u ms, scheduled=%u ms, delay=%d ms", 
            now, timestamp_ms, delay);
    
    if (delay < 0) {
        LOG_ERR("ERROR: RX timestamp nel passato!");
    }
    LOG_INF( "\n" );
    LOG_INF( "Open RX in the %u ms", timestamp_ms - smtc_modem_hal_get_time_in_ms( ) );
#if ( !IS_DISPLAY_IMAGE_MODE )
    update_percentage_display( 0 );
#endif

    flrc_burst_rx.transaction->modulation_type = SMTC_RAC_MODULATION_LORA;

    flrc_burst_rx.transaction->radio_params.lora.is_tx                = false;
    flrc_burst_rx.transaction->radio_params.lora.is_ranging_exchange  = false;
    flrc_burst_rx.transaction->radio_params.lora.frequency_in_hz      = RF_FREQ_IN_HZ;
    flrc_burst_rx.transaction->radio_params.lora.sf                   = WOR_LORA_SPREADING_FACTOR;
    flrc_burst_rx.transaction->radio_params.lora.bw                   = WOR_LORA_BANDWIDTH;
    flrc_burst_rx.transaction->radio_params.lora.cr                   = WOR_LORA_CODING_RATE;
    flrc_burst_rx.transaction->radio_params.lora.preamble_len_in_symb = WOR_LORA_PREAMBLE_LENGTH;
    flrc_burst_rx.transaction->radio_params.lora.header_type          = WOR_LORA_PKT_LEN_MODE;
    flrc_burst_rx.transaction->radio_params.lora.invert_iq_is_on      = WOR_LORA_IQ;
    flrc_burst_rx.transaction->radio_params.lora.crc_is_on            = WOR_LORA_CRC;
    flrc_burst_rx.transaction->radio_params.lora.sync_word            = WOR_LORA_SYNCWORD;
    flrc_burst_rx.transaction->radio_params.lora.rx_timeout_ms        = WOR_RX_TIMEOUT;
    flrc_burst_rx.transaction->radio_params.lora.max_rx_size          = sizeof( lora_payload );

    flrc_burst_rx.transaction->smtc_rac_data_buffer_setup.rx_payload_buffer         = lora_payload;
    flrc_burst_rx.transaction->smtc_rac_data_buffer_setup.size_of_rx_payload_buffer = sizeof( lora_payload );

    flrc_burst_rx.transaction->scheduler_config.scheduling    = SMTC_RAC_ASAP_TRANSACTION;
    flrc_burst_rx.transaction->scheduler_config.start_time_ms = timestamp_ms;
    
    /* --- INIZIO AGGIUNTA SINCRONIZZAZIONE --- */
    int64_t sync_time = getDeltaMicroSeconds();
    if (sync_time >= 0) {
        printk("RX: Inizio ascolto immagine (Burst) a %lld us dal Big Bang\n", sync_time);
    }
    /* --- FINE AGGIUNTA SINCRONIZZAZIONE --- */

    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_rx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
}

static void send_wor_ack_tx_packet( uint32_t timestamp_ms )
{
    flrc_burst_rx.transaction->radio_params.lora.is_tx           = true;
    flrc_burst_rx.transaction->radio_params.lora.invert_iq_is_on = WOR_ACK_LORA_IQ;
    flrc_burst_rx.transaction->radio_params.lora.tx_size         = sizeof( lora_payload );
    flrc_burst_rx.transaction->lbt_context.lbt_enabled           = false;

    memset( lora_payload, 0, sizeof( lora_payload ) );
    lora_payload[0] = 'W';
    lora_payload[1] = 'O';
    lora_payload[2] = 'R';
    lora_payload[3] = ' ';
    lora_payload[4] = 'O';
    lora_payload[5] = 'K';

    flrc_burst_rx.transaction->smtc_rac_data_buffer_setup.tx_payload_buffer         = lora_payload;
    flrc_burst_rx.transaction->smtc_rac_data_buffer_setup.size_of_tx_payload_buffer = sizeof( lora_payload );

    flrc_burst_rx.transaction->scheduler_config.scheduling    = SMTC_RAC_SCHEDULED_TRANSACTION;
    flrc_burst_rx.transaction->scheduler_config.start_time_ms = timestamp_ms;
    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_rx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
}

static void listen_rx_data_packet( uint32_t timestamp_ms )
{
    flrc_burst_rx.nb_packets_crc_error = 0;
    flrc_burst_rx.nb_packets_received  = 0;
    flrc_burst_rx.rssi_mean            = 0;
    flrc_burst_rx.rx_bytes_received    = 0;

    flrc_burst_rx.transaction->modulation_type = SMTC_RAC_MODULATION_FLRC_BURST;

    flrc_burst_rx.transaction->radio_params.flrc_burst.is_tx                     = false;
    flrc_burst_rx.transaction->radio_params.flrc_burst.frequency_in_hz           = RF_FREQ_IN_HZ;
    flrc_burst_rx.transaction->radio_params.flrc_burst.rx_frequency_offset_in_hz = flrc_burst_rx.lora_freq_offset_hz;
    flrc_burst_rx.transaction->radio_params.flrc_burst.raw_bit_rate              = flrc_burst_rx.br_bw_from_wor;
    flrc_burst_rx.transaction->radio_params.flrc_burst.cr                        = DATA_FLRC_CR;
    flrc_burst_rx.transaction->radio_params.flrc_burst.pulse_shape               = DATA_FLRC_PULSE_SHAPE;
    flrc_burst_rx.transaction->radio_params.flrc_burst.preamble_len              = DATA_FLRC_PREAMBLE_BITS;
    flrc_burst_rx.transaction->radio_params.flrc_burst.sync_word_len             = DATA_FLRC_SYNCWORD_LEN;
    flrc_burst_rx.transaction->radio_params.flrc_burst.tx_syncword_index         = DATA_FLRC_SYNCWORD;
    flrc_burst_rx.transaction->radio_params.flrc_burst.match_sync_word           = DATA_FLRC_MATCH_SYNCWORD;
    flrc_burst_rx.transaction->radio_params.flrc_burst.pld_is_fix                = DATA_FLRC_PLD_IS_FIX;
    flrc_burst_rx.transaction->radio_params.flrc_burst.crc_type                  = DATA_FLRC_CRC;
    flrc_burst_rx.transaction->radio_params.flrc_burst.max_rx_size               = FLRC_RADIO_PAYLOAD_MAX_LENGTH;
    flrc_burst_rx.transaction->radio_params.flrc_burst.sync_word[0]              = &default_syncword_1[0];
    flrc_burst_rx.transaction->radio_params.flrc_burst.crc_seed                  = 0xffffffff;
    flrc_burst_rx.transaction->radio_params.flrc_burst.crc_polynomial            = 0x755B;

    // Indicate the total size of the data to receive
    flrc_burst_rx.transaction->radio_params.flrc_burst.burst_rx_size = flrc_burst_rx.rx_data_size_from_wor;
    // Indicate the buffer to receive packets
    flrc_burst_rx.transaction->smtc_rac_data_buffer_setup.rx_payload_buffer = flrc_burst_rx.rx_flrc_payload_buffer;
    flrc_burst_rx.transaction->smtc_rac_data_buffer_setup.size_of_rx_payload_buffer =
        sizeof( flrc_burst_rx.rx_flrc_payload_buffer );

    flrc_burst_rx.transaction->scheduler_config.scheduling    = SMTC_RAC_ASAP_TRANSACTION;
    flrc_burst_rx.transaction->scheduler_config.start_time_ms = timestamp_ms;
    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_rx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
}

static void send_result_tx_packet( uint32_t timestamp_ms, uint32_t received_size, int16_t mean_rssi )
{
    flrc_burst_rx.transaction->modulation_type = SMTC_RAC_MODULATION_LORA;

    flrc_burst_rx.transaction->radio_params.lora.is_tx                = true;
    flrc_burst_rx.transaction->radio_params.lora.is_ranging_exchange  = false;
    flrc_burst_rx.transaction->radio_params.lora.frequency_in_hz      = RF_FREQ_IN_HZ;
    flrc_burst_rx.transaction->radio_params.lora.tx_power_in_dbm      = TX_OUTPUT_POWER_DBM;
    flrc_burst_rx.transaction->radio_params.lora.sf                   = WOR_LORA_SPREADING_FACTOR;
    flrc_burst_rx.transaction->radio_params.lora.bw                   = WOR_LORA_BANDWIDTH;
    flrc_burst_rx.transaction->radio_params.lora.cr                   = WOR_LORA_CODING_RATE;
    flrc_burst_rx.transaction->radio_params.lora.preamble_len_in_symb = WOR_LORA_PREAMBLE_LENGTH;
    flrc_burst_rx.transaction->radio_params.lora.header_type          = WOR_LORA_PKT_LEN_MODE;
    flrc_burst_rx.transaction->radio_params.lora.invert_iq_is_on      = WOR_ACK_LORA_IQ;
    flrc_burst_rx.transaction->radio_params.lora.crc_is_on            = WOR_LORA_CRC;
    flrc_burst_rx.transaction->radio_params.lora.sync_word            = WOR_LORA_SYNCWORD;
    flrc_burst_rx.transaction->radio_params.lora.tx_size              = sizeof( lora_payload );
    flrc_burst_rx.transaction->lbt_context.lbt_enabled                = false;

    memset( lora_payload, 0, sizeof( lora_payload ) );
    /* End of transmission*/
    lora_payload[0] = 'E';
    lora_payload[1] = 'N';
    lora_payload[2] = ( uint8_t ) ( received_size & 0xFF );
    lora_payload[3] = ( uint8_t ) ( ( received_size >> 8 ) & 0xFF );
    lora_payload[4] = ( uint8_t ) ( ( received_size >> 16 ) & 0xFF );
    lora_payload[5] = ( uint8_t ) ( ( received_size >> 24 ) & 0xFF );
    lora_payload[6] = ( uint8_t ) ( mean_rssi & 0xFF );
    lora_payload[7] = ( uint8_t ) ( ( mean_rssi >> 8 ) & 0xFF );

    flrc_burst_rx.transaction->smtc_rac_data_buffer_setup.tx_payload_buffer         = lora_payload;
    flrc_burst_rx.transaction->smtc_rac_data_buffer_setup.size_of_tx_payload_buffer = sizeof( lora_payload );

    flrc_burst_rx.transaction->scheduler_config.scheduling    = SMTC_RAC_SCHEDULED_TRANSACTION;
    flrc_burst_rx.transaction->scheduler_config.start_time_ms = timestamp_ms;
    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_rx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
}
