/*!
 * @file      apps_configuration.h
 *
 * @brief     Common configuration for application-level radio and FLRC parameters.
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

#ifndef APPS_CONFIGURATION_H
#define APPS_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include <stdint.h>

#include "ral.h"
#include "smtc_rac.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

/*!
 * @brief Default TX output power in dBm.
 * @note Range: [-9, +22] for sub-GHz, [-19, 12] for 2.4GHz (HF_PA).
 */
#ifndef TX_OUTPUT_POWER_DBM
#define TX_OUTPUT_POWER_DBM 0
#endif

#ifndef RF_FREQ_IN_HZ
#define RF_FREQ_IN_HZ 902150000  // 902150000 2401150000 868700000
#endif

/*********************************
 * WOR LORA parameters
 *********************************/

#ifndef WOR_LORA_SPREADING_FACTOR
#define WOR_LORA_SPREADING_FACTOR RAL_LORA_SF8
#endif

#ifndef WOR_LORA_BANDWIDTH
#define WOR_LORA_BANDWIDTH RAL_LORA_BW_125_KHZ
#endif

#ifndef WOR_LORA_CODING_RATE
#define WOR_LORA_CODING_RATE RAL_LORA_CR_4_5
#endif

#ifndef WOR_LORA_PREAMBLE_LENGTH
#define WOR_LORA_PREAMBLE_LENGTH 12
#endif

#ifndef WOR_LORA_IQ
#define WOR_LORA_IQ true
#endif

#ifndef WOR_ACK_LORA_IQ
#define WOR_ACK_LORA_IQ !WOR_LORA_IQ
#endif

#ifndef WOR_LORA_CRC
#define WOR_LORA_CRC true
#endif

#ifndef WOR_RX_TIMEOUT
#define WOR_RX_TIMEOUT 1000
#endif

#ifndef WOR_LORA_PKT_LEN_MODE
#define WOR_LORA_PKT_LEN_MODE RAL_LORA_PKT_EXPLICIT
#endif

#ifndef WOR_LORA_SYNCWORD
#define WOR_LORA_SYNCWORD LORA_PRIVATE_NETWORK_SYNCWORD
#endif

#ifndef WOR_LORA_PAYLOAD_LENGTH
#define WOR_LORA_PAYLOAD_LENGTH 8
#endif

/*!
 * @brief The cycle period in the next transaction. (Unit: ms)
 */
#ifndef SLEEP_DELAY_MS
#define SLEEP_DELAY_MS 3000
#endif

/*!
 * @brief Time margin (Unit: ms)
 */
#ifndef MARGIN_RADIO_MS
#define MARGIN_RADIO_MS 50
#endif

/*********************************
 * DATA FLRC parameters
 *********************************/

static const uint8_t default_syncword_1[4] = { 0x90, 0x56, 0x34, 0x12 };

#ifndef DATA_FLRC_BR_BPS
#define DATA_FLRC_BR_BPS RAL_FLRC_RAW_BIT_RATE_2_600_MBPS
#endif

#ifndef DATA_FLRC_CR
#define DATA_FLRC_CR RAL_FLRC_CR_3_4
#endif

#ifndef DATA_FLRC_PULSE_SHAPE
#define DATA_FLRC_PULSE_SHAPE RAL_FLRC_PULSE_SHAPE_BT_05
#endif

#ifndef DATA_FLRC_PREAMBLE_BITS
#define DATA_FLRC_PREAMBLE_BITS RAL_FLRC_PREAMBLE_LENGTH_32_BITS
#endif

#ifndef DATA_FLRC_SYNCWORD_LEN
#define DATA_FLRC_SYNCWORD_LEN RAL_FLRC_SYNCWORD_LENGTH_4_BYTES
#endif

#ifndef DATA_FLRC_SYNCWORD
#define DATA_FLRC_SYNCWORD RAL_FLRC_TX_SYNCWORD_1
#endif

#ifndef DATA_FLRC_MATCH_SYNCWORD
#define DATA_FLRC_MATCH_SYNCWORD RAL_FLRC_RX_MATCH_SYNCWORD_1
#endif

#ifndef DATA_FLRC_PLD_IS_FIX
#define DATA_FLRC_PLD_IS_FIX false
#endif

#ifndef DATA_FLRC_CRC
#define DATA_FLRC_CRC RAL_FLRC_CRC_2_BYTES
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*!
 * @brief The packet length for every payload. Unit: Bytes
 */
#define FLRC_RADIO_PAYLOAD_MAX_LENGTH RAC_FLRC_BURST_RADIO_PAYLOAD_MAX_LENGTH

/*!
 * @brief The length for every packet index. Unit: Bytes
 */
#define FLRC_PACKET_INDEX_LEN 2

/*!
 * @brief Every packet includes "image data + 2 bytes packet index". Unit: Bytes
 */
#define FLRC_DATA_LENGTH ( FLRC_RADIO_PAYLOAD_MAX_LENGTH - FLRC_PACKET_INDEX_LEN )

/*!
 * @brief Change the display mode.
 * @note 1: Display the chick looking images
 * @note 0: Display the text messages
 */
#define IS_DISPLAY_IMAGE_MODE 1

#if ( IS_DISPLAY_IMAGE_MODE )
/*!
 * @brief The number of images to transmit.
 * @note  Fixed value. Changes are not allowed.
 */
#define IMAGE_NUMS 20
/*!
 * @brief The single image size. Unit: Bytes
 * @note  Fixed value. Changes are not allowed.
 */
#define SINGLE_IMAGE_SIZE 1024
/*!
 * @brief All the images size. Unit: Bytes
 * @note  Fixed value. Changes are not allowed.
 */
#define TOTAL_IMAGES_SIZE ( IMAGE_NUMS * SINGLE_IMAGE_SIZE )
/*!
 * @brief The packet number that is transmitted. Unit: Bytes
 * @note  Fixed value. Changes are not allowed.
 */
#define FLRC_PACKET_NUMS ( ( TOTAL_IMAGES_SIZE + FLRC_DATA_LENGTH - 1 ) / FLRC_DATA_LENGTH )

#else

/*!
 * @brief Define the data length that wants to test.
 *
 * @note The maximum allowable data length is 60 KBytes.
 * @note The real testing data will be set as @ref TRANSMIT_DATA_SIZES.
 */
#define SET_TRANSMIT_DATA_SIZES 20 * 1024  // Maximum value: 60*1024. Unit: bytes
/*!
 * @brief Make sure the minimum length for each packet is greater than or equal to 6 on the FLRC type. Refer to
 * datasheet.
 */
#define TRANSMIT_DATA_SIZES                                                                                   \
    ( ( SET_TRANSMIT_DATA_SIZES % FLRC_RADIO_PAYLOAD_MAX_LENGTH <= 6 )                                        \
          ? ( SET_TRANSMIT_DATA_SIZES + ( 6 - ( SET_TRANSMIT_DATA_SIZES % FLRC_RADIO_PAYLOAD_MAX_LENGTH ) ) ) \
          : SET_TRANSMIT_DATA_SIZES )
#define FLRC_PACKET_NUMS ( ( TRANSMIT_DATA_SIZES + FLRC_DATA_LENGTH - 1 ) / FLRC_DATA_LENGTH )
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/*!
 * @brief  The entry of main logic for this demo
 */
void image_transfer_entry( void );

static inline const uint16_t ral_flrc_raw_bit_rate_to_value( const ral_flrc_raw_bit_rate_t value )
{
    switch( value )
    {
    case RAL_FLRC_RAW_BIT_RATE_2_600_MBPS:
    {
        return 2600;
    }

    case RAL_FLRC_RAW_BIT_RATE_2_080_MBPS:
    {
        return 2080;
    }

    case RAL_FLRC_RAW_BIT_RATE_1_300_MBPS:
    {
        return 1300;
    }

    case RAL_FLRC_RAW_BIT_RATE_1_040_MBPS:
    {
        return 1040;
    }

    case RAL_FLRC_RAW_BIT_RATE_0_650_MBPS:
    {
        return 650;
    }

    case RAL_FLRC_RAW_BIT_RATE_0_520_MBPS:
    {
        return 520;
    }

    case RAL_FLRC_RAW_BIT_RATE_0_325_MBPS:
    {
        return 325;
    }

    case RAL_FLRC_RAW_BIT_RATE_0_260_MBPS:
    {
        return 260;
    }

    default:
    {
        return 0;
    }
    }
}

static inline const char* ral_flrc_cr_to_short_str( const ral_flrc_cr_t value )
{
    switch( value )
    {
    case RAL_FLRC_CR_1_2:
    {
        return ( const char* ) "1_2";
    }

    case RAL_FLRC_CR_3_4:
    {
        return ( const char* ) "3_4";
    }

    case RAL_FLRC_CR_1_1:
    {
        return ( const char* ) "1";
    }

    case RAL_FLRC_CR_2_3:
    {
        return ( const char* ) "2_3";
    }

    default:
    {
        return ( const char* ) "Unknown";
    }
    }
}

#ifdef __cplusplus
}
#endif

#endif  // APPS_CONFIGURATION_H

/* --- EOF ------------------------------------------------------------------ */