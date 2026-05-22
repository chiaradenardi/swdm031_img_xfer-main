/**
 * @file      main.c
 *
 * @brief     Application main
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

#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type

#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>

#include <smtc_modem_hal.h>
#include <smtc_zephyr_usp_api.h>
#include <smtc_sw_platform_helper.h>
#include "apps_configuration.h"
#include "lr20xx_system.h"
#include "../xiaosync/xiaosync.h"

#if CONFIG_TRANSMITTER
#include "flrc_burst_tx.h"
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

LOG_MODULE_REGISTER( img_xfer, LOG_LEVEL_INF );

/**
 * @brief Led on the XIAO board.
 */
#define LED0_NODE DT_ALIAS( led0 )
#if !DT_NODE_HAS_STATUS( LED0_NODE, okay )
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

/**
 * @brief User button on the XIAO board.
 */
#define USER_BUTTON_NODE DT_ALIAS( smtc_user_button )
#if !DT_NODE_HAS_STATUS( USER_BUTTON_NODE, okay )
#error "Unsupported board: smtc-user-button devicetree alias is not defined"
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

static const struct gpio_dt_spec usr_led = GPIO_DT_SPEC_GET( LED0_NODE, gpios );
static const struct gpio_dt_spec button  = GPIO_DT_SPEC_GET_OR( USER_BUTTON_NODE, gpios, { 0 } );

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

static struct gpio_callback button_cb_data;
static volatile bool        user_button_is_press = false;  // Flag for button status

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/**
 * @brief Startup animation
 */
static uint8_t anim_completed_count = 0;
static bool    anim_end_flag        = false;

K_SEM_DEFINE( periodical_event_sem, 0, 1 );
/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

static void boot_animation( void );
static void animation_completed_cb( lv_anim_t* anim );

static int  configure_user_button( void );
static void user_button_callback( const struct device* dev, struct gpio_callback* cb, uint32_t pins );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

int main( void )
{
    LOG_INF("=== BOOT SEQUENCE START ===");
    uint32_t boot_start = smtc_modem_hal_get_time_in_ms();
    //Aggiunta per sync
    int err = syncInit();
    if (err == 0) {
        syncEnable();
        printk("Sistema di Sincronizzazione HW pronto su D0\n");
    } else {
        printk("Errore inizializzazione Sync: %d\n", err);
    }

    LOG_INF( "===== LR20xx Image Transfer Demo =====" );

    /* Startup logo */
    boot_animation( );
    while( 1 )
    {
        lv_timer_handler( );
        if( anim_end_flag == true )
        {
            // Clear the whole display
            lv_obj_clean( lv_scr_act( ) );
            break;
        }
        k_msleep( 20 );
    }
    /* Boot animation has ended. Display for a while.*/
    k_msleep( 1000 );

    if( !gpio_is_ready_dt( &usr_led ) )
    {
        LOG_ERR( "user led is not ready" );
    }
    else
    {
        gpio_pin_configure_dt( &usr_led, GPIO_OUTPUT_ACTIVE );
    }

    if( configure_user_button( ) != 0 )
    {
        LOG_ERR( "Issue when configuring user button, aborting" );
    }

    SMTC_SW_PLATFORM_INIT( );
    SMTC_SW_PLATFORM_VOID( smtc_rac_init( ) );

    lr20xx_system_version_t version;
    lr20xx_status_t         status;

    void* radio_driver_context = smtc_rac_get_radio_driver_context( );
    /* Get the firmware version */
    status = lr20xx_system_get_version( radio_driver_context, &version );
    if( status == LR20XX_STATUS_OK )
    {
        LOG_INF( "Major firmware version: 0x%02X", version.major );
        LOG_INF( "Minor firmware version: 0x%02X", version.minor );
    }

    //AGGIUNTA PER SYNCH 
    LOG_INF("\n=== SYNCHRONIZATION PHASE ===");
    LOG_INF("Waiting for HW sync edge on P1.04...");

    while (getDeltaMicroSeconds() == -1) {
        k_sleep(K_MSEC(10)); 
    }

    syncDisable();
    int64_t delta_us = getDeltaMicroSeconds();
    LOG_INF("✓ Sync detected! Epoch offset: %" PRId64 " us", delta_us);

    uint32_t sync_detected = smtc_modem_hal_get_time_in_ms();
    LOG_INF("Time to detect sync: %u ms", sync_detected - boot_start);

    // IMPORTANTE: Aspettare 50ms prima di iniziare la radio
    // Questo sincronizza TX e RX nello stesso momento
    LOG_INF("Waiting 50ms before radio startup...");
    k_msleep(50);

    LOG_INF("✓ Starting image transfer\n");
    LOG_INF("Time at radio start: %u ms", smtc_modem_hal_get_time_in_ms());
    image_transfer_entry( );

    while( true )
    {
        if( user_button_is_press == true )
        {
            user_button_is_press = false;

#if CONFIG_TRANSMITTER
            flrc_raw_bit_rate_changed( );
#endif
            LOG_INF( "User button pushed" );
        }

        if( user_button_is_press == false )
        {
            k_sem_take( &periodical_event_sem, K_MSEC( 1000 ) );
        }
        gpio_pin_toggle_dt( &usr_led );
    }
    return 0;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */
static void animation_completed_cb( lv_anim_t* anim )
{
    anim_completed_count++;
    if( anim_completed_count >= 2 )  // Both width and height animations completed
    {
        anim_end_flag = true;
    }
}

static void boot_animation( void )
{
    // Reset animation completion counter
    anim_completed_count = 0;
    anim_end_flag        = false;

    LV_IMG_DECLARE( lora_logo );
    lv_obj_t* img = lv_img_create( lv_scr_act( ) );
    lv_img_set_src( img, &lora_logo );
    lv_obj_align( img, LV_ALIGN_CENTER, 0, 0 );

    // Set the initial size - 10%
    uint16_t target_w = lora_logo.header.w;
    uint16_t target_h = lora_logo.header.h;
    lv_obj_set_size( img, target_w * 0.1, target_h * 0.1 );

    // Define the animation - width and height
    lv_anim_t anim_w, anim_h;
    // width
    lv_anim_init( &anim_w );
    lv_anim_set_var( &anim_w, img );
    lv_anim_set_exec_cb( &anim_w, ( lv_anim_exec_xcb_t ) lv_obj_set_width );
    lv_anim_set_time( &anim_w, 1000 );                        // animation duration 1s
    lv_anim_set_values( &anim_w, target_w * 0.1, target_w );  // from 10% to 100%
    lv_anim_set_ready_cb( &anim_w, animation_completed_cb );  // End callback
    // height
    lv_anim_init( &anim_h );
    lv_anim_set_var( &anim_h, img );
    lv_anim_set_exec_cb( &anim_h, ( lv_anim_exec_xcb_t ) lv_obj_set_height );
    lv_anim_set_time( &anim_h, 1000 );                        // animation duration 1s
    lv_anim_set_values( &anim_h, target_h * 0.1, target_h );  // from 10% to 100%
    lv_anim_set_ready_cb( &anim_h, animation_completed_cb );  // End callback
    lv_anim_start( &anim_w );
    lv_anim_start( &anim_h );
}

static void user_button_callback( const struct device* dev, struct gpio_callback* cb, uint32_t pins )
{
    static uint32_t last_press_timestamp_ms = 0;

    // Debounce the button press, avoid multiple triggers
    if( ( int32_t ) ( smtc_modem_hal_get_time_in_ms( ) - last_press_timestamp_ms ) > 500 )
    {
        last_press_timestamp_ms = smtc_modem_hal_get_time_in_ms( );
        user_button_is_press    = true;
    }
    // Wake up the main thread
    k_sem_give( &periodical_event_sem );
}

static int configure_user_button( void )
{
    int ret = 0;
    if( !gpio_is_ready_dt( &button ) )
    {
        printk( "Error: button device %s is not ready\n", button.port->name );
        return 1;
    }

    ret = gpio_pin_configure_dt( &button, GPIO_INPUT );
    if( ret != 0 )
    {
        printk( "Error %d: failed to configure %s pin %d\n", ret, button.port->name, button.pin );
        return 1;
    }

    ret = gpio_pin_interrupt_configure_dt( &button, GPIO_INT_EDGE_TO_ACTIVE );
    if( ret != 0 )
    {
        printk( "Error %d: failed to configure interrupt on %s pin %d\n", ret, button.port->name, button.pin );
        return 1;
    }

    gpio_init_callback( &button_cb_data, user_button_callback, BIT( button.pin ) );
    gpio_add_callback( button.port, &button_cb_data );

    return 0;
}

/* --- EOF ------------------------------------------------------------------ */
