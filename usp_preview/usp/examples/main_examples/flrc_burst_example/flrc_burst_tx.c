/*!
 * @file      flrc_burst_tx.c
 *
 * @brief     Simple FLRC burst example
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2024. All rights reserved.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "smtc_rac.h"
#include "smtc_rac_api.h"
#include "smtc_sw_platform_helper.h"
#include "smtc_modem_hal.h"

#include "flrc_burst_tx.h"
#include "flrc_burst_utils.h"

#include "apps_configuration.h"

#define RAC_LOG_APP_PREFIX "FLRC-BURST"
#include "smtc_rac_log.h"
#include "smtc_hal_dbg_trace.h"

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

#define TX_RX_BUFFER_MAX_SIZE ( 255 )

#define DELAY_BEFORE_SEND_DATA_MS 18

#define FLRC_RADIO_PAYLOAD_MAX_LENGTH ( RAC_FLRC_BURST_RADIO_PAYLOAD_MAX_LENGTH )

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
} flrc_burst_tx_state_t;

typedef struct flrc_burst_tx_s
{
    flrc_burst_tx_state_t   state;
    uint8_t                 radio_access_id;
    smtc_rac_context_t*     transaction;
    uint8_t*                tx_data;
    uint32_t                tx_data_size;
    uint16_t                tx_payload_buffer_size[2]; /*!< Size (in bytes) of the payload data in the buffer. */
    uint16_t                next_packet_buffer_index;
    uint16_t                nb_packets_to_send;
    uint16_t                bytes_sent;
    ral_flrc_raw_bit_rate_t raw_bit_rate;  // FLRC bit rate and bandwidth
    void ( *user_callback )( bool is_successful );
} flrc_burst_tx_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */
static flrc_burst_tx_t flrc_burst_tx = { 0 };
static uint8_t         wor_ack_rx_payload[TX_RX_BUFFER_MAX_SIZE];
static uint8_t         wor_payload[TX_RX_BUFFER_MAX_SIZE];

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

static void post_transaction_callback( rp_status_t status );

static void pre_transaction_callback( void );

static void prepare_tx_data_packet( void );
static void send_tx_data_packet( uint32_t timestamp_ms );
static void prepare_next_packet_buffer_tx( void );

// WOR
static void prepare_wor_packet( uint32_t tx_data_size, ral_flrc_raw_bit_rate_t raw_bit_rate, uint32_t crc );
static void send_wor_packet( void );
static void prepare_wor_ack_rx_packet( void );
static void listen_wor_ack_rx_packet( uint32_t timestamp_ms );
/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void flrc_burst_protocol_tx_init( void ( *user_callback )( bool is_successful ) )
{
    SMTC_HAL_TRACE_INFO( "FLRC burst TX init\n" );
    memset( &flrc_burst_tx, 0, sizeof( flrc_burst_tx ) );

    flrc_burst_tx.user_callback = user_callback;

    flrc_burst_tx.radio_access_id = SMTC_SW_PLATFORM( smtc_rac_open_radio( RAC_HIGH_PRIORITY ) );
    flrc_burst_tx.transaction     = smtc_rac_get_context( flrc_burst_tx.radio_access_id );

    flrc_burst_tx.transaction->scheduler_config.callback_pre_radio_transaction  = pre_transaction_callback;
    flrc_burst_tx.transaction->scheduler_config.callback_post_radio_transaction = post_transaction_callback;
    flrc_burst_tx.raw_bit_rate                                                  = DATA_FLRC_RAW_BIT_RATE;
}

void flrc_burst_protocol_tx_start_new_transaction( uint8_t* tx_data, uint32_t tx_data_size )
{
    if( flrc_burst_tx.state != FLRC_BURST_TX_STATE_IDLE )
    {
        return;
    }

    flrc_burst_tx.tx_data_size = tx_data_size;
    flrc_burst_tx.tx_data      = tx_data;
    flrc_burst_tx.state        = FLRC_BURST_TX_STATE_WOR_TX;

    uint32_t crc = generate_crc( flrc_burst_tx.tx_data, flrc_burst_tx.tx_data_size );

    prepare_wor_packet( flrc_burst_tx.tx_data_size, flrc_burst_tx.raw_bit_rate, crc );
    send_wor_packet( );
}

bool flrc_burst_protocol_tx_is_ready( void )
{
    return ( flrc_burst_tx.state == FLRC_BURST_TX_STATE_IDLE );
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
        if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_WOR_TX )
        {
            // SMTC_HAL_TRACE_INFO( "FLRC BURST PROTOCOL WOR successfully sent\n" );
            flrc_burst_tx.state = FLRC_BURST_TX_STATE_WOR_ACK_RX;
            prepare_wor_ack_rx_packet( );
            listen_wor_ack_rx_packet( flrc_burst_tx.transaction->smtc_rac_data_result.radio_end_timestamp_ms +
                                      WOR_ACK_DELAY_MS );
        }
        break;
    case RP_STATUS_RX_PACKET:
        if( flrc_burst_tx.state == FLRC_BURST_TX_STATE_WOR_ACK_RX )
        {
            if( ( wor_ack_rx_payload[0] == 'W' ) && ( wor_ack_rx_payload[1] == 'O' ) &&
                ( wor_ack_rx_payload[2] == 'R' ) && ( wor_ack_rx_payload[3] == ' ' ) &&
                ( wor_ack_rx_payload[4] == 'O' ) && ( wor_ack_rx_payload[5] == 'K' ) )
            {
                // SMTC_HAL_TRACE_INFO( "FLRC BURST PROTOCOL WOR ACK received\n" );

                flrc_burst_tx.state = FLRC_BURST_TX_STATE_TX_DATA;
                prepare_tx_data_packet( );
                send_tx_data_packet( flrc_burst_tx.transaction->smtc_rac_data_result.radio_end_timestamp_ms +
                                     DELAY_BEFORE_SEND_DATA_MS );
            }
            else
            {
                SMTC_HAL_TRACE_INFO(
                    "TX:"
                    "WOR ACK not received\n" );
                if( flrc_burst_tx.user_callback != NULL )
                {
                    flrc_burst_tx.user_callback( false );
                }
                flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
            }
        }
        break;
    case RP_STATUS_REQUEST_NEXT_TX_PAYLOAD:
        // Packet n is sent, prepare the next packet (n+2) if exists
        // Packet (n+1) should be already prepared, to be able to send it at the same time
        if( flrc_burst_tx.nb_packets_to_send > flrc_burst_tx.next_packet_buffer_index )
        {
            prepare_next_packet_buffer_tx( );
        }
        break;
    case RP_STATUS_RX_TIMEOUT:
        SMTC_HAL_TRACE_WARNING( "FLRC BURST PROTOCOL Rx timeout\n" );
        if( flrc_burst_tx.user_callback != NULL )
        {
            flrc_burst_tx.user_callback( false );
        }
        flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
        break;
    case RP_STATUS_RX_CRC_ERROR:
        SMTC_HAL_TRACE_WARNING( "FLRC BURST PROTOCOL CRC error\n" );
        if( flrc_burst_tx.user_callback != NULL )
        {
            flrc_burst_tx.user_callback( false );
        }
        flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
        break;
    case RP_STATUS_TASK_ABORTED:
    {
        SMTC_HAL_TRACE_WARNING( "FLRC BURST PROTOCOL task aborted\n" );
        if( flrc_burst_tx.user_callback != NULL )
        {
            flrc_burst_tx.user_callback( false );
        }
        flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
        break;
    }
    case RP_STATUS_RADIO_LOCKED:
    {
        SMTC_HAL_TRACE_WARNING( "FLRC BURST PROTOCOL radio locked\n" );
        break;
    }
    case RP_STATUS_RADIO_UNLOCKED:
    {
        // Status that indicate the end of TX
        if( flrc_burst_tx.user_callback != NULL )
        {
            uint32_t duration_ms = flrc_burst_tx.transaction->smtc_rac_data_result.radio_end_timestamp_ms -
                                   flrc_burst_tx.transaction->smtc_rac_data_result.radio_start_timestamp_ms;

            if( duration_ms > 0 )
            {
                // Calculate bits per second first to avoid overflow
                uint32_t bit_rate_bps        = ( flrc_burst_tx.tx_data_size * 8U * 1000U ) / duration_ms;
                uint32_t bit_rate_integer    = bit_rate_bps / 1000000U;
                uint32_t bit_rate_fractional = bit_rate_bps % 1000000U;
                SMTC_HAL_TRACE_INFO( "FLRC BURST Tx transaction %lu bytes in %lu ms\n", flrc_burst_tx.tx_data_size,
                                     duration_ms );
                SMTC_HAL_TRACE_INFO( "freq_hz: %lu, FLRC BURST raw bit rate: %s\n",
                                     flrc_burst_tx.transaction->radio_params.flrc_burst.frequency_in_hz,
                                     ral_flrc_raw_bit_rate_to_str( flrc_burst_tx.raw_bit_rate ) );
                SMTC_HAL_TRACE_INFO( "FLRC BURST effective bit rate: %lu.%03lu Mbit/s\n", bit_rate_integer,
                                     bit_rate_fractional );
            }
            else
            {
                SMTC_HAL_TRACE_INFO( "FLRC BURST transaction duration: %lu ms (invalid)\n", duration_ms );
            }
            flrc_burst_tx.user_callback( true );
        }
        flrc_burst_tx.state = FLRC_BURST_TX_STATE_IDLE;
        break;
    }
    default:
    {
        SMTC_HAL_TRACE_ERROR( "Unknown transaction status: %d\n", status );
    }
    }
}

static void pre_transaction_callback( void )
{
}
static void prepare_tx_data_packet( void )
{
    flrc_burst_tx.next_packet_buffer_index = 0;
    flrc_burst_tx.bytes_sent               = 0;

    flrc_burst_tx.transaction->modulation_type                              = SMTC_RAC_MODULATION_FLRC_BURST;
    flrc_burst_tx.transaction->radio_params.flrc_burst.is_tx                = true;
    flrc_burst_tx.transaction->radio_params.flrc_burst.frequency_in_hz      = DATA_RF_FREQ_IN_HZ;
    flrc_burst_tx.transaction->radio_params.flrc_burst.tx_power_in_dbm      = TX_OUTPUT_POWER_DBM;
    flrc_burst_tx.transaction->radio_params.flrc_burst.raw_bit_rate         = flrc_burst_tx.raw_bit_rate;
    flrc_burst_tx.transaction->radio_params.flrc_burst.cr                   = DATA_FLRC_CR;
    flrc_burst_tx.transaction->radio_params.flrc_burst.pulse_shape          = DATA_FLRC_PULSE_SHAPE;
    flrc_burst_tx.transaction->radio_params.flrc_burst.preamble_len         = DATA_FLRC_PREAMBLE_BITS;
    flrc_burst_tx.transaction->radio_params.flrc_burst.sync_word_len        = DATA_FLRC_SYNCWORD_LEN;
    flrc_burst_tx.transaction->radio_params.flrc_burst.tx_syncword_index    = DATA_FLRC_SYNCWORD;
    flrc_burst_tx.transaction->radio_params.flrc_burst.match_sync_word      = DATA_FLRC_MATCH_SYNCWORD;
    flrc_burst_tx.transaction->radio_params.flrc_burst.pld_is_fix           = DATA_FLRC_PLD_IS_FIX;
    flrc_burst_tx.transaction->radio_params.flrc_burst.crc_type             = DATA_FLRC_CRC;

    flrc_burst_tx.transaction->radio_params.flrc_burst.sync_word[DATA_FLRC_SYNCWORD - 1] = &default_syncword_1[0];

    flrc_burst_tx.transaction->radio_params.flrc_burst.crc_seed       = 0xffffffff;
    flrc_burst_tx.transaction->radio_params.flrc_burst.crc_polynomial = 0x755B;

    // poly flrc compatible sx1280:
    // #define FLRC_SX1280_CRC_POLY_16 0x755B
    // #define FLRC_SX1280_CRC_POLY_24 0x5D6DCB
    // #define FLRC_SX1280_CRC_POLY_32 0xF4ACFB13
    // seed=0xffffffff

    // Indicate the total size of the data to sent
    flrc_burst_tx.transaction->radio_params.flrc_burst.burst_tx_size = flrc_burst_tx.tx_data_size;

    // Packets of FLRC_RADIO_PAYLOAD_MAX_LENGTH size (except possibly the last packet)
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
}

static void send_tx_data_packet( uint32_t timestamp_ms )
{
    flrc_burst_tx.transaction->scheduler_config.start_time_ms = timestamp_ms;
    flrc_burst_tx.transaction->scheduler_config.scheduling    = SMTC_RAC_ASAP_TRANSACTION;

    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_tx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
}

static void prepare_next_packet_buffer_tx( void )
{
    uint32_t size_payload_left_to_send = flrc_burst_tx.tx_data_size - flrc_burst_tx.bytes_sent;
    uint16_t buffer_index              = ( flrc_burst_tx.next_packet_buffer_index % 2 );

    flrc_burst_tx.tx_payload_buffer_size[buffer_index] =
        MIN( size_payload_left_to_send, FLRC_RADIO_PAYLOAD_MAX_LENGTH );

    flrc_burst_tx.transaction->radio_params.flrc_burst.tx_fifo_payload_buffer[buffer_index] =
        &flrc_burst_tx.tx_data[flrc_burst_tx.bytes_sent];
    flrc_burst_tx.transaction->radio_params.flrc_burst.tx_fifo_payload_length[buffer_index] =
        &flrc_burst_tx.tx_payload_buffer_size[buffer_index];

    flrc_burst_tx.next_packet_buffer_index++;
    flrc_burst_tx.bytes_sent += flrc_burst_tx.tx_payload_buffer_size[buffer_index];
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE WOR FUNCTIONS DEFINITION --------------------------------------------
 */

static void prepare_wor_packet( uint32_t tx_data_size, ral_flrc_raw_bit_rate_t raw_bit_rate, uint32_t crc )
{
    // configure transaction context

    flrc_burst_tx.transaction->modulation_type = SMTC_RAC_MODULATION_LORA;

    flrc_burst_tx.transaction->radio_params.lora.is_tx                = true;
    flrc_burst_tx.transaction->radio_params.lora.is_ranging_exchange  = false;
    flrc_burst_tx.transaction->radio_params.lora.frequency_in_hz      = WOR_RF_FREQ_IN_HZ;
    flrc_burst_tx.transaction->radio_params.lora.tx_power_in_dbm      = TX_OUTPUT_POWER_DBM;
    flrc_burst_tx.transaction->radio_params.lora.sf                   = WOR_LORA_SPREADING_FACTOR;
    flrc_burst_tx.transaction->radio_params.lora.bw                   = WOR_LORA_BANDWIDTH;
    flrc_burst_tx.transaction->radio_params.lora.cr                   = WOR_LORA_CODING_RATE;
    flrc_burst_tx.transaction->radio_params.lora.preamble_len_in_symb = WOR_LORA_LONG_PREAMBLE_LENGTH;
    flrc_burst_tx.transaction->radio_params.lora.header_type          = LORA_PKT_LEN_MODE;
    flrc_burst_tx.transaction->radio_params.lora.invert_iq_is_on      = WOR_LORA_IQ;
    flrc_burst_tx.transaction->radio_params.lora.crc_is_on            = WOR_LORA_CRC;
    flrc_burst_tx.transaction->radio_params.lora.sync_word            = LORA_SYNCWORD;
    flrc_burst_tx.transaction->radio_params.lora.tx_size              = sizeof( wor_payload );

    flrc_burst_tx.transaction->lbt_context.lbt_enabled = false;
    // flrc_burst_tx.transaction->lbt_context.listen_duration_ms                  = LBT_SNIFF_DURATION_MS_DEFAULT;
    // flrc_burst_tx.transaction->lbt_context.threshold_dbm                       = LBT_THRESHOLD_DBM_DEFAULT;
    // flrc_burst_tx.transaction->lbt_context.bandwidth_hz                        = LBT_BW_HZ__DEFAULT;

    wor_payload[0] = 'W';
    wor_payload[1] = 'O';
    wor_payload[2] = 'R';
    wor_payload[3] = tx_data_size;
    wor_payload[4] = tx_data_size >> 8;
    wor_payload[5] = tx_data_size >> 16;
    wor_payload[6] = tx_data_size >> 24;
    wor_payload[7] = raw_bit_rate;

    // Add CRC to the data
    wor_payload[8]  = ( uint8_t ) ( crc & 0xFF );
    wor_payload[9]  = ( uint8_t ) ( ( crc >> 8 ) & 0xFF );
    wor_payload[10] = ( uint8_t ) ( ( crc >> 16 ) & 0xFF );
    wor_payload[11] = ( uint8_t ) ( ( crc >> 24 ) & 0xFF );

    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.tx_payload_buffer         = wor_payload;
    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.size_of_tx_payload_buffer = sizeof( wor_payload );
}

static void send_wor_packet( void )
{
    flrc_burst_tx.transaction->scheduler_config.scheduling    = SMTC_RAC_ASAP_TRANSACTION;
    flrc_burst_tx.transaction->scheduler_config.start_time_ms = smtc_modem_hal_get_time_in_ms( );
    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_tx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
    // SMTC_HAL_TRACE_INFO( "FLRC BURST PROTOCOL WOR packet request scheduled\n" );
}

static void prepare_wor_ack_rx_packet( void )
{
    flrc_burst_tx.transaction->modulation_type = SMTC_RAC_MODULATION_LORA;

    flrc_burst_tx.transaction->radio_params.lora.is_tx                = false;
    flrc_burst_tx.transaction->radio_params.lora.is_ranging_exchange  = false;
    flrc_burst_tx.transaction->radio_params.lora.frequency_in_hz      = WOR_RF_FREQ_IN_HZ;
    flrc_burst_tx.transaction->radio_params.lora.sf                   = WOR_LORA_SPREADING_FACTOR;
    flrc_burst_tx.transaction->radio_params.lora.bw                   = WOR_LORA_BANDWIDTH;
    flrc_burst_tx.transaction->radio_params.lora.cr                   = WOR_LORA_CODING_RATE;
    flrc_burst_tx.transaction->radio_params.lora.preamble_len_in_symb = WOR_ACK_LORA_PREAMBLE_LENGTH;
    flrc_burst_tx.transaction->radio_params.lora.header_type          = LORA_PKT_LEN_MODE;
    flrc_burst_tx.transaction->radio_params.lora.invert_iq_is_on      = WOR_ACK_LORA_IQ;
    flrc_burst_tx.transaction->radio_params.lora.crc_is_on            = WOR_LORA_CRC;
    flrc_burst_tx.transaction->radio_params.lora.sync_word            = LORA_SYNCWORD;
    flrc_burst_tx.transaction->radio_params.lora.rx_timeout_ms        = WOR_RX_TIMEOUT;
    flrc_burst_tx.transaction->radio_params.lora.symb_nb_timeout      = 16;
    flrc_burst_tx.transaction->radio_params.lora.max_rx_size          = TX_RX_BUFFER_MAX_SIZE;

    flrc_burst_tx.transaction->scheduler_config.scheduling = SMTC_RAC_SCHEDULED_TRANSACTION;

    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.rx_payload_buffer         = wor_ack_rx_payload;
    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.size_of_tx_payload_buffer = sizeof( wor_ack_rx_payload );
    flrc_burst_tx.transaction->smtc_rac_data_buffer_setup.size_of_rx_payload_buffer = TX_RX_BUFFER_MAX_SIZE;
}

static void listen_wor_ack_rx_packet( uint32_t timestamp_ms )
{
    flrc_burst_tx.transaction->scheduler_config.start_time_ms = timestamp_ms - 1;
    SMTC_MODEM_HAL_PANIC_ON_FAILURE( smtc_rac_submit_radio_transaction( flrc_burst_tx.radio_access_id ) ==
                                     SMTC_RAC_SUCCESS );
    // SMTC_HAL_TRACE_INFO( "listen WOR ACK packet\n" );
}
