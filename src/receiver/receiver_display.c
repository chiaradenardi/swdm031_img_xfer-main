/*!
 * \file    receiver_display.c
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

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER( display, LOG_LEVEL_INF );

#include "receiver_display.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

#define DISPLAY_THREAD_STACKSIZE 4096
#define DISPLAY_THREAD_PRIORITY 3

/*!
 * @brief The maximum number of texts on a single line
 */
#define DISPLAY_TEXT_MAX_LEN 25

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

typedef struct
{
    uint8_t row;
    char    text[DISPLAY_TEXT_MAX_LEN];
} display_msg_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*!
 * @brief Define a font
 */
LV_FONT_DECLARE( font_syke_regular_size_12 );

static struct k_thread display_thread;
K_THREAD_STACK_DEFINE( display_stack, DISPLAY_THREAD_STACKSIZE );

#if ( IS_DISPLAY_IMAGE_MODE )
/*!
 * @brief The current frame on the display
 */
static uint8_t current_frame = 0;

/*!
 * @brief The maximum packet index that has been received
 */
static uint32_t rev_pkt_max_idx = 0;

/*!
 * @brief The buffer to save the received data
 */
static uint8_t rev_image_buf[IMAGE_NUMS][SINGLE_IMAGE_SIZE] = { 0 };
#endif

/*!
 * @brief Text labels for different rows
 */
static lv_obj_t* text_labels[DISPLAY_MAX_ROW];

/*!
 * @brief Screen object
 */
static lv_obj_t* screen;

/*!
 * @brief Message queue for display updates
 */
K_MSGQ_DEFINE( display_msgq, sizeof( display_msg_t ), 20, 4 );

/*!
 * @brief Flag to indicate if LVGL objects are still active
 */
static bool lvgl_active = true;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

static void display_thread_entry( void* pkf_buf, void* p2, void* p3 );

/*!
 * @brief Process display messages from the queue
 */
static void process_display_messages( void );

#if ( IS_DISPLAY_IMAGE_MODE )
/*!
 * @brief Clean up LVGL objects and switch to image mode
 */
static void cleanup_lvgl_and_switch_to_image( void );

/*!
 * @brief Display part of data for a frame image
 *
 * @param [in] data Pointer to the date to display
 * @param [in] data_len The data length
 * @param [in] start_addr The starting address in the frame
 */
static void display_refresh( uint8_t* data, uint16_t data_len, uint16_t start_addr );
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void display_thread_init( pkt_buffer_t* pkf_buf )
{
    k_thread_create( &display_thread, display_stack, K_THREAD_STACK_SIZEOF( display_stack ), display_thread_entry,
                     pkf_buf, NULL, NULL, DISPLAY_THREAD_PRIORITY, K_INHERIT_PERMS, K_FOREVER );
    k_thread_start( &display_thread );
}

void display_row_texts( uint8_t row, const char* fmt, ... )
{
    if( row >= DISPLAY_MAX_ROW )
    {
        LOG_ERR( "Row number is out of range" );
        return;
    }

    display_msg_t msg;
    msg.row = row;

    va_list args;
    va_start( args, fmt );
    vsnprintf( msg.text, sizeof( msg.text ), fmt, args );
    va_end( args );

    /* Try to send message to display thread */
    if( k_msgq_put( &display_msgq, &msg, K_NO_WAIT ) != 0 )
    {
        LOG_WRN( "Display message queue full, dropping message for row %u", row );
    }
}

void update_percentage_display( int value )
{
    static int last_interval    = -1;
    int        current_interval = value / 10;

    /* Decrease the times of changing text on the display */
    if( current_interval == last_interval )
        return;

    last_interval = current_interval;

    display_msg_t msg;
    msg.row = DISPLAY_ROW5;
    if( value == 0 )
    {
        char tmp[] = { "Waiting......" };
        strcpy( msg.text, tmp );
    }
    else
    {
        lv_snprintf( msg.text, sizeof( msg.text ), "Progress: %d%%", current_interval * 10 );
    }
    if( k_msgq_put( &display_msgq, &msg, K_NO_WAIT ) != 0 )
    {
        LOG_WRN( "Display message queue full, dropping row 5" );
    }
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void display_thread_entry( void* pkf_buf, void* p2, void* p3 )
{
#if ( IS_DISPLAY_IMAGE_MODE )
    pkt_buffer_t* dis_pkt_buf = ( pkt_buffer_t* ) pkf_buf;
#else
    ARG_UNUSED( pkf_buf );
#endif
    ARG_UNUSED( p2 );
    ARG_UNUSED( p3 );

    screen = lv_scr_act( );
    if( screen == NULL )
    {
        LOG_ERR( "Failed to get screen object" );
        return;
    }

    /* At receiver's side, a maximum of two lines are supported on the display. */
    for( uint8_t i = 0; i < DISPLAY_MAX_ROW; i++ )
    {
        text_labels[i] = lv_label_create( screen );
        if( text_labels[i] == NULL )
        {
            LOG_ERR( "Failed to create text label %d", i );
            return;
        }
        lv_obj_align( text_labels[i], LV_ALIGN_TOP_LEFT, 0, i * 12 );
        lv_obj_set_style_text_font( text_labels[i], &font_syke_regular_size_12, LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_text_color( text_labels[i], lv_color_hex( 0x000000 ), LV_PART_MAIN );
        lv_label_set_text( text_labels[i], " " );
    }

    while( 1 )
    {
        if( lvgl_active )
        {
            process_display_messages( );
            lv_timer_handler( );
            k_msleep( 50 );
        }
#if ( IS_DISPLAY_IMAGE_MODE )
        else
        {
            k_msleep( 10 );
        }

        display_pkt_data_t* pkt_data = k_fifo_get( dis_pkt_buf->filled_buf, K_NO_WAIT );  // K_NO_WAIT K_MSEC(750)
        if( pkt_data == NULL )
        {
            continue;
        }

        /* To show the image data when receiving first packet */
        if( lvgl_active )
        {
            cleanup_lvgl_and_switch_to_image( );
        }

        if( k_sem_take( dis_pkt_buf->clr_buf_sem, K_NO_WAIT ) == 0 )
        {
            LOG_DBG( "Reset all the related variables." );
            current_frame   = 0;
            rev_pkt_max_idx = 0;
            memset( rev_image_buf, 0, sizeof( rev_image_buf ) );
        }

        // LOG_INF( "pkt idx: %d", pkt_data->pkt_idx );
        const uint16_t pkt_idx       = pkt_data->pkt_idx;
        const uint32_t offset_in_buf = pkt_idx * FLRC_DATA_LENGTH;
        const uint8_t  frame_idx     = offset_in_buf / SINGLE_IMAGE_SIZE;
        if( frame_idx >= IMAGE_NUMS )
        {
            LOG_ERR( "Frame idx is out of range!" );
            k_fifo_put( dis_pkt_buf->free_buf, pkt_data );
            continue;
        }
        memcpy( ( uint8_t* ) rev_image_buf + offset_in_buf, pkt_data->data, pkt_data->data_len );

        /* Retransmission flush */
        if( rev_pkt_max_idx > pkt_idx )
        {
            /* Refresh a whole frame data */
            display_refresh( rev_image_buf[frame_idx], SINGLE_IMAGE_SIZE, 0 );
            k_fifo_put( dis_pkt_buf->free_buf, pkt_data );
            continue;
        }
        else
        {
            rev_pkt_max_idx = pkt_idx;
        }

        if( offset_in_buf % SINGLE_IMAGE_SIZE + pkt_data->data_len > SINGLE_IMAGE_SIZE )
        {
            const uint16_t frame_last_pkt_len = SINGLE_IMAGE_SIZE - offset_in_buf % SINGLE_IMAGE_SIZE;
            /* Display the last data for the current frame */
            display_refresh( ( uint8_t* ) rev_image_buf + offset_in_buf, frame_last_pkt_len,
                             offset_in_buf % SINGLE_IMAGE_SIZE );
            /* Display the next frame's data */
            display_refresh( ( uint8_t* ) rev_image_buf + ( frame_idx + 1 ) * SINGLE_IMAGE_SIZE,
                             pkt_data->data_len - frame_last_pkt_len, 0 );
            current_frame = frame_idx + 1;
        }
        else
        {
            display_refresh( ( uint8_t* ) rev_image_buf + offset_in_buf, pkt_data->data_len,
                             offset_in_buf % SINGLE_IMAGE_SIZE );
        }
        k_fifo_put( dis_pkt_buf->free_buf, pkt_data );
#endif
    }
}

#if ( IS_DISPLAY_IMAGE_MODE )
static void display_refresh( uint8_t* data, uint16_t data_len, uint16_t start_addr )
{
    const struct device* display_device = ( struct device* ) DEVICE_DT_GET( DT_CHOSEN( zephyr_display ) );

    /* Show a full frame at one time */
    if( start_addr == 0 && data_len == SINGLE_IMAGE_SIZE )
    {
        struct display_buffer_descriptor full_frame_desc = {
            .width = 128, .height = 64, .buf_size = SINGLE_IMAGE_SIZE, .pitch = 128
        };
        display_write( display_device, 0, 0, &full_frame_desc, data );
        return;
    }

    /* The staring column and row */
    uint8_t start_page = start_addr / 128;
    uint8_t start_col  = start_addr % 128;

    /* The end column and row */
    uint16_t end_addr = start_addr + data_len - 1;
    uint8_t  end_page = end_addr / 128;
    uint8_t  end_col  = end_addr % 128;

    uint16_t data_index = 0;
    for( uint8_t page = start_page; page <= end_page; page++ )
    {
        /* The starting column and end column */
        uint8_t col_start = ( page == start_page ) ? start_col : 0;
        uint8_t col_end   = ( page == end_page ) ? end_col : 127;
        uint8_t cols      = col_end - col_start + 1;
        /* Show the data by page */
        struct display_buffer_descriptor buf_desc = { .width = cols, .height = 8, .buf_size = cols, .pitch = cols };
        display_write( display_device, col_start, page * 8, &buf_desc, data + data_index );
        data_index += cols;
    }
}

static void cleanup_lvgl_and_switch_to_image( void )
{
    if( screen != NULL )
    {
        /* Clear all LVGL objects */
        lv_obj_clean( screen );
        lv_timer_handler( );

        /* Clear text labels array */
        for( int i = 0; i < DISPLAY_MAX_ROW; i++ )
        {
            text_labels[i] = NULL;
        }

        lvgl_active = false;
        LOG_INF( "Switched to image display mode, LVGL objects cleaned up" );
    }
}
#endif

static void process_display_messages( void )
{
    display_msg_t msg;

    while( k_msgq_get( &display_msgq, &msg, K_NO_WAIT ) == 0 )
    {
        if( msg.row < DISPLAY_MAX_ROW && text_labels[msg.row] != NULL )
        {
            lv_label_set_text( text_labels[msg.row], msg.text );
        }
    }
}

/* --- EOF ------------------------------------------------------------------ */