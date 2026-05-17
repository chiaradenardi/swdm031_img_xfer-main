/**
 * @file      app_radio_planner_test.h
 *
 * @brief     Radio Planner stress test application
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

#ifndef APP_RADIO_PLANNER_TEST_H
#define APP_RADIO_PLANNER_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include <stdbool.h>

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */
#define NUMBER_OF_RADIO_ACCESS 5
#define RP_TEST_PAYLOAD_LENGTH 30
// Test configuration
#ifndef RF_FREQ_IN_HZ
#define RF_FREQ_IN_HZ 868100000
#endif

#ifndef TX_OUTPUT_POWER_DBM
#define TX_OUTPUT_POWER_DBM 14
#endif

#ifndef LORA_SPREADING_FACTOR
#define LORA_SPREADING_FACTOR RAL_LORA_SF7
#endif

#ifndef LORA_BANDWIDTH
#define LORA_BANDWIDTH RAL_LORA_BW_125_KHZ
#endif

#ifndef LORA_CODING_RATE
#define LORA_CODING_RATE RAL_LORA_CR_4_5
#endif

#ifndef LORA_PREAMBLE_LENGTH
#define LORA_PREAMBLE_LENGTH 1000
#endif

#ifndef LORA_PKT_LEN_MODE
#define LORA_PKT_LEN_MODE RAL_LORA_PKT_EXPLICIT
#endif

#ifndef LORA_IQ
#define LORA_IQ false
#endif

#ifndef LORA_CRC
#define LORA_CRC true
#endif

typedef enum
{
    VERY_HIGH_PRIORITY = 0,
    HIGH_PRIORITY      = 1,
    MEDIUM_PRIORITY    = 2,
    LOW_PRIORITY       = 3,
    VERY_LOW_PRIORITY  = 4
} priority_t;
typedef struct statistics_s
{
    uint32_t total_transactions_launched;
    uint32_t total_transactions_completed;
    uint32_t total_transactions_tx;
    uint32_t total_transactions_rx;
    uint32_t total_transactions_aborted;
    uint32_t total_transactions_errors;
    uint32_t last_transaction_time_ms;
    uint32_t total_transactions_cad_positive;
    uint32_t total_transactions_cad_negative;
    uint32_t total_transactions_radio_locked;
    uint32_t total_transactions_radio_unlocked;
    uint32_t total_transactions_radio_time_ms;
} statistics_t;
/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */
/**
 * @brief Initialize radio planner test application
 *
 * This function:
 * - Initializes test infrastructure
 * - Registers test hooks with radio planner
 * - Prepares test metrics
 */
void radio_planner_test_init( void );

/**
 * @brief Trigger next test on button press
 *
 * This cycles through different test scenarios:
 * - Priority conflicts and preemption tests
 * - Critical timing tests
 * - State transition tests
 * - Immediate radio access tests
 * - Multi-hook stress tests
 */
void radio_planner_test_on_button_press( void );

void print_statistics( uint8_t index );
#ifdef __cplusplus
}
#endif

#endif  // APP_RADIO_PLANNER_TEST_H
