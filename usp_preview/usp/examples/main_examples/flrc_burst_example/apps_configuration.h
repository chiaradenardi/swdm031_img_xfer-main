/*!
 * @file      apps_configuration.h
 *
 * @brief     Common configuration header for application-level radio and FLRC parameters.
 *
 * This file centralizes all the key configuration macros and parameters used by the application,
 * especially for FLRC and ranging operations. It provides default values for frequency, power,
 * payload length, modulation parameters, and other radio settings. These macros ensure that all
 * parts of the application use consistent radio settings and make it easy to adapt the configuration
 * for different regions, hardware, or use cases.
 *
 * Key features:
 *   - Defines the default RF frequency and TX output power.
 *   - Sets the payload length for ranging initialization.
 *   - Specifies FLRC modulation parameters: spreading factor, bandwidth, coding rate, preamble length, packet mode, IQ,
 * CRC, and sync word.
 *   - Uses include guards to prevent multiple inclusion.
 *   - Provides C++ compatibility.
 *
 * Usage:
 *   - Include this header in any application file that needs access to radio or FLRC configuration.
 *   - Override the macros before including this file if custom values are needed.
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2022. All rights reserved.
 */

#ifndef APPS_CONFIGURATION_H
#define APPS_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 * Includes standard integer types and the radio abstraction layer definitions.
 */
#include <stdint.h>
#include "ral.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 * These macros define the default configuration for the radio and FLRC modulation.
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPE -----------------------------------------------------------
 * These macros define the default configuration for the radio and FLRC modulation.
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 * (None defined in this file, but section reserved for future use.)
 */

/*!
 * @brief Default TX output power in dBm.
 *        Range: [-17, +22] for sub-GHz, [-18, 13] for 2.4GHz (HF_PA).
 */
#ifndef TX_OUTPUT_POWER_DBM
#define TX_OUTPUT_POWER_DBM 14
#endif

/*********************************
 * WOR LORA parameters
 *********************************/
#ifndef WOR_RF_FREQ_IN_HZ
#define WOR_RF_FREQ_IN_HZ 867100000
#endif

#ifndef WOR_LORA_SPREADING_FACTOR
#define WOR_LORA_SPREADING_FACTOR RAL_LORA_SF5
#endif

#ifndef WOR_LORA_BANDWIDTH
#define WOR_LORA_BANDWIDTH RAL_LORA_BW_125_KHZ
#endif

#ifndef WOR_LORA_CODING_RATE
#define WOR_LORA_CODING_RATE RAL_LORA_CR_4_5
#endif

#ifndef WOR_LORA_PREAMBLE_LENGTH_TX
#define WOR_LORA_PREAMBLE_LENGTH_TX 8
#endif

#ifndef WOR_LORA_LONG_PREAMBLE_LENGTH
#define WOR_LORA_LONG_PREAMBLE_LENGTH 4000
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
#define WOR_RX_TIMEOUT 3000
#endif

#ifndef WOR_SYM_NB_TIMEOUT
#define WOR_SYM_NB_TIMEOUT 50
#endif

#ifndef WOR_ACK_DELAY_MS
#define WOR_ACK_DELAY_MS 50
#endif

#ifndef WOR_ACK_LORA_PREAMBLE_LENGTH
#define WOR_ACK_LORA_PREAMBLE_LENGTH 12
#endif

#ifndef LORA_PKT_LEN_MODE
#define LORA_PKT_LEN_MODE RAL_LORA_PKT_EXPLICIT
#endif

/*!
 * @brief LoRa sync word.
 */
#ifndef LORA_SYNCWORD
#define LORA_SYNCWORD LORA_PRIVATE_NETWORK_SYNCWORD
#endif

/*********************************
 * DATA FLRC parameters
 *********************************/

static const uint8_t default_syncword_1[4] = { 0x90, 0x56, 0x34, 0x12 };
static const uint8_t default_syncword_2[4] = { 0x00, 0x00, 0x00, 0x00 };
static const uint8_t default_syncword_3[4] = { 0x00, 0x00, 0x00, 0x00 };

#ifndef DATA_RF_FREQ_IN_HZ
#define DATA_RF_FREQ_IN_HZ WOR_RF_FREQ_IN_HZ
#endif

#ifndef DATA_FLRC_RAW_BIT_RATE
#define DATA_FLRC_RAW_BIT_RATE RAL_FLRC_RAW_BIT_RATE_2_600_MBPS
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

#ifndef DATA_FLRC_MAX_TIMEOUT_MS
#define DATA_FLRC_MAX_TIMEOUT_MS 1
#endif

#ifndef DATA_FLRC_MIN_TIMEOUT_MS
#define DATA_FLRC_MIN_TIMEOUT_MS 1
#endif

#ifndef FLRC_DATA_DELAY_MS
#define FLRC_DATA_DELAY_MS 8
#endif

#ifdef __cplusplus
}
#endif

#endif  // APPS_CONFIGURATION_H

/* --- EOF ------------------------------------------------------------------ */