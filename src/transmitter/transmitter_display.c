/*!
 * \file    transmitter_display.c
 *
 * \brief   Handle the display on the transmitter's side
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
#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER( display, LOG_LEVEL_INF );

#include "transmitter_display.h"

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

static lv_obj_t* text_labels[DISPLAY_MAX_ROW];
K_MSGQ_DEFINE( display_msgq, sizeof( display_msg_t ), 30, 4 );

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

static void display_thread_entry( void* p1, void* p2, void* p3 );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void display_thread_init( void )
{
    k_thread_create( &display_thread, display_stack, K_THREAD_STACK_SIZEOF( display_stack ), display_thread_entry, NULL,
                     NULL, NULL, DISPLAY_THREAD_PRIORITY, K_INHERIT_PERMS, K_FOREVER );
    k_thread_start( &display_thread );
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

    if( k_msgq_put( &display_msgq, &msg, K_NO_WAIT ) != 0 )
    {
        LOG_WRN( "Display message queue full, dropping row %u", row );
    }
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

static void display_thread_entry( void* p1, void* p2, void* p3 )
{
    ARG_UNUSED( p1 );
    ARG_UNUSED( p2 );
    ARG_UNUSED( p3 );

    display_msg_t rev_msg;

    // Initialize display objects
    lv_obj_t* screen = lv_scr_act( );
    if( !screen )
    {
        LOG_ERR( "Failed to get screen object" );
        return;
    }

    for( uint8_t i = 0; i < DISPLAY_MAX_ROW; i++ )
    {
        if( text_labels[i] == NULL )
        {
            text_labels[i] = lv_label_create( screen );
            lv_obj_align( text_labels[i], LV_ALIGN_TOP_LEFT, 0, i * 12 );
            lv_obj_set_style_text_font( text_labels[i], &font_syke_regular_size_12, LV_PART_MAIN | LV_STATE_DEFAULT );
            lv_obj_set_style_text_color( text_labels[i], lv_color_hex( 0x000000 ), LV_PART_MAIN );
            lv_label_set_text( text_labels[i], " " );
        }
    }

    while( 1 )
    {
        k_msgq_get( &display_msgq, &rev_msg, K_NO_WAIT );

        if( rev_msg.row < DISPLAY_MAX_ROW && text_labels[rev_msg.row] != NULL )
        {
            lv_label_set_text( text_labels[rev_msg.row], rev_msg.text );
        }

        lv_timer_handler( );
        k_msleep( 50 );
    }
}

/* --- EOF ------------------------------------------------------------------ */