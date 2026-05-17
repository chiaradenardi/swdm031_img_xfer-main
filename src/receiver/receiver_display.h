/*!
 * \file    receiver_display.h
 *
 * \brief   Handle the display on the receiver's side
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
#ifndef RECEIVER_DISPLAY_H
#define RECEIVER_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include "apps_configuration.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/ring_buffer.h>

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

/*!
 * @brief The row number of display
 */
enum display_row_e
{
    DISPLAY_ROW1 = 0,
    DISPLAY_ROW2,
    DISPLAY_ROW3,
    DISPLAY_ROW4,
    DISPLAY_ROW5,
    DISPLAY_MAX_ROW,
};

/*!
 * @brief The message for a packet through a mailbaox
 */
typedef struct display_pkt_data_s
{
    void*    fifo_reserved;  // Save for FIFO kernel's use
    uint8_t  data[FLRC_DATA_LENGTH];
    uint16_t data_len;
    uint32_t pkt_idx;
} __attribute__( ( aligned( 4 ) ) ) display_pkt_data_t;

typedef struct pkt_buffer_s
{
    struct k_fifo* free_buf;
    struct k_fifo* filled_buf;
    struct k_sem*  clr_buf_sem;
} pkt_buffer_t;

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/*!
 * @brief The display initiazation
 *
 * @param [in] pkf_buf Pointer to the packet data
 */
void display_thread_init( pkt_buffer_t* pkf_buf );

/*!
 * @brief  Display texts on a specific row
 *
 * @param [in] row The row number
 * @param [in] fmt The format string
 * @param [in] ... Variable arguments
 */
void display_row_texts( uint8_t row, const char* fmt, ... );

/*!
 * @brief  Update the percentage on the display
 *
 * @param [in] value The progress value
 */
void update_percentage_display( int value );

#ifdef __cplusplus
}
#endif

#endif  // RECEIVER_DISPLAY_H