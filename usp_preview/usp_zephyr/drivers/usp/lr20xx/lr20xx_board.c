/**
 * @file      lr20xx_board.c
 *
 * @brief     lr20xx_board implementation
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

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>

#include <zephyr/usp/lora_lbm_transceiver.h>

#include "lr20xx_hal_context.h"
#include "lr20xx_system_types.h"

LOG_MODULE_REGISTER( lora_lr20xx, CONFIG_LORA_BASICS_MODEM_DRIVERS_LOG_LEVEL );

#define LR20XX_SPI_OPERATION ( SPI_WORD_SET( 8 ) | SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB )

/*
 * -----------------------------------------------------------------------------
 * --- IRQ HANDLING ------------------------------------------------------------
 * -----------------------------------------------------------------------------
 */

#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER

/**
 * @brief Helper macro to handle IRQ for a specific type
 */
#define LR20XX_HANDLE_IRQ( data, irq_type )                                                         \
    do                                                                                              \
    {                                                                                               \
        IF_ENABLED( CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_OWN_THREAD,                      \
                    ( data->pending_irq_mask |= BIT( irq_type ); k_sem_give( &data->gpio_sem ); ) ) \
        IF_ENABLED( CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_GLOBAL_THREAD,                   \
                    ( k_work_submit( &data->work[irq_type] ); ) )                                   \
        IF_ENABLED( CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_NO_THREAD,                       \
                    ( if( data->irq_handlers[irq_type].user_callback ) {                            \
                        data->irq_handlers[irq_type].user_callback( data->lr20xx_dev );             \
                    } ) )                                                                           \
    } while( 0 )

/**
 * @brief GPIO callback for main IRQ
 */
static void lr20xx_gpio_cb_main( const struct device* dev, struct gpio_callback* cb, uint32_t pins )
{
    struct lr20xx_hal_context_data_t* data =
        CONTAINER_OF( cb, struct lr20xx_hal_context_data_t, irq_handlers[LR20XX_IRQ_INDEX_MAIN].gpio_cb );

    LR20XX_HANDLE_IRQ( data, LR20XX_IRQ_INDEX_MAIN );
}

/**
 * @brief GPIO callback for FIFO IRQ
 */
static void lr20xx_gpio_cb_fifo( const struct device* dev, struct gpio_callback* cb, uint32_t pins )
{
    struct lr20xx_hal_context_data_t* data =
        CONTAINER_OF( cb, struct lr20xx_hal_context_data_t, irq_handlers[LR20XX_IRQ_INDEX_FIFO].gpio_cb );

    LR20XX_HANDLE_IRQ( data, LR20XX_IRQ_INDEX_FIFO );
}

/**
 * @brief Table of GPIO callbacks indexed by IRQ type
 */
static const gpio_callback_handler_t lr20xx_gpio_callbacks[LR20XX_MAX_IRQ_DIOS] = {
    [LR20XX_IRQ_INDEX_MAIN] = lr20xx_gpio_cb_main,
    [LR20XX_IRQ_INDEX_FIFO] = lr20xx_gpio_cb_fifo,
};

#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_GLOBAL_THREAD
/**
 * @brief Work handler for main IRQ
 */
static void lr20xx_work_cb_main( struct k_work* work )
{
    struct lr20xx_hal_context_data_t* data =
        CONTAINER_OF( work, struct lr20xx_hal_context_data_t, work[LR20XX_IRQ_INDEX_MAIN] );

    if( data->irq_handlers[LR20XX_IRQ_INDEX_MAIN].user_callback )
    {
        data->irq_handlers[LR20XX_IRQ_INDEX_MAIN].user_callback( data->lr20xx_dev );
    }
}

/**
 * @brief Work handler for FIFO IRQ
 */
static void lr20xx_work_cb_fifo( struct k_work* work )
{
    struct lr20xx_hal_context_data_t* data =
        CONTAINER_OF( work, struct lr20xx_hal_context_data_t, work[LR20XX_IRQ_INDEX_FIFO] );

    if( data->irq_handlers[LR20XX_IRQ_INDEX_FIFO].user_callback )
    {
        data->irq_handlers[LR20XX_IRQ_INDEX_FIFO].user_callback( data->lr20xx_dev );
    }
}

static const k_work_handler_t lr20xx_work_handlers[LR20XX_MAX_IRQ_DIOS] = {
    [LR20XX_IRQ_INDEX_MAIN] = lr20xx_work_cb_main,
    [LR20XX_IRQ_INDEX_FIFO] = lr20xx_work_cb_fifo,
};
#endif /* CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_GLOBAL_THREAD */

#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_OWN_THREAD
/**
 * @brief Thread handler for IRQ processing
 */
static void lr20xx_thread( struct lr20xx_hal_context_data_t* data )
{
    while( 1 )
    {
        k_sem_take( &data->gpio_sem, K_FOREVER );

        /* Process all pending IRQs */
        for( int i = 0; i < LR20XX_MAX_IRQ_DIOS; i++ )
        {
            if( data->pending_irq_mask & BIT( i ) )
            {
                data->pending_irq_mask &= ~BIT( i );
                if( data->irq_handlers[i].user_callback )
                {
                    data->irq_handlers[i].user_callback( data->lr20xx_dev );
                }
            }
        }
    }
}
#endif /* CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_OWN_THREAD */

#endif /* CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC API --------------------------------------------------------------
 * -----------------------------------------------------------------------------
 */

bool lora_transceiver_board_fifo_line_level( const struct device* dev )
{
    struct lr20xx_hal_context_cfg_t* config = dev->config;
    for( int i = 0; i < config->dios_config_num; i++ )
    {
        lr20xx_dio_cfg_t dio_config = config->dios_config[i];

        // Catch 1st DIO line with FIFO enabled
        if( ( dio_config.irq_mask & ( LR20XX_SYSTEM_IRQ_FIFO_RX | LR20XX_SYSTEM_IRQ_FIFO_TX ) ) )
        {
            return ( bool ) gpio_pin_get_dt( &dio_config.gpio );
        }
    }

    return false;
}

void lora_transceiver_board_attach_interrupt( const struct device* dev, event_cb_t cb )
{
    lora_transceiver_board_attach_interrupt_indexed( dev, cb, LORA_TRANSCEIVER_IRQ_MAIN );
}

void lora_transceiver_board_attach_interrupt_indexed( const struct device* dev, event_cb_t cb,
                                                      lora_transceiver_irq_type_t irq_type )
{
#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER
    struct lr20xx_hal_context_data_t* data = dev->data;

    if( irq_type < data->irq_handlers_count )
    {
        data->irq_handlers[irq_type].user_callback = cb;
    }
    else
    {
        LOG_WRN( "IRQ type %d not configured (only %d IRQs available)", irq_type, data->irq_handlers_count );
    }
#else
    LOG_ERR( "Event trigger not supported!" );
#endif /* CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER */
}

void lora_transceiver_board_enable_interrupt( const struct device* dev )
{
#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER
    const struct lr20xx_hal_context_cfg_t* config = dev->config;

    for( int i = 0; i < config->dios_config_num; i++ )
    {
        lr20xx_dio_cfg_t dio_config = config->dios_config[i];

        if( dio_config.function == LR20XX_SYSTEM_DIO_FUNC_IRQ )
        {
            gpio_pin_interrupt_configure_dt( &config->dios_config[i].gpio, GPIO_INT_EDGE_TO_ACTIVE );
        }
    }
#else
    LOG_ERR( "Event trigger not supported!" );
#endif /* CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER */
}

void lora_transceiver_board_disable_interrupt( const struct device* dev )
{
#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER
    const struct lr20xx_hal_context_cfg_t* config = dev->config;

    for( int i = 0; i < config->dios_config_num; i++ )
    {
        lr20xx_dio_cfg_t dio_config = config->dios_config[i];

        if( dio_config.function == LR20XX_SYSTEM_DIO_FUNC_IRQ )
        {
            gpio_pin_interrupt_configure_dt( &config->dios_config[i].gpio, GPIO_INT_DISABLE );
        }
    }
#else
    LOG_ERR( "Event trigger not supported!" );
#endif /* CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER */
}

uint32_t lora_transceiver_get_tcxo_startup_delay_ms( const struct device* dev )
{
    const struct lr20xx_hal_context_cfg_t* config = dev->config;

    return config->tcxo_cfg.wakeup_time_ms;
}

int32_t lora_transceiver_get_model( const struct device* dev )
{
    // FIXME:
    // const struct lr20xx_hal_context_cfg_t *config = dev->config;
    // return (config->chip_type.major << 8) + config->chip_type.minor;
    return 0;
}

/**
 * @brief Initialise lr20xx.
 * Initialise all GPIOs and configure interrupt on event pin.
 *
 * @param dev
 * @return int
 */
static int lr20xx_init( const struct device* dev )
{
    const struct lr20xx_hal_context_cfg_t* config = dev->config;
    struct lr20xx_hal_context_data_t*      data   = dev->data;
    int                                    ret;
    int                                    irq_count = 0;

    /* Check the SPI device */
    if( !device_is_ready( config->spi.bus ) )
    {
        LOG_ERR( "Could not find SPI device" );
        return -EINVAL;
    }

    /* Busy pin */
    ret = gpio_pin_configure_dt( &config->busy, GPIO_INPUT );
    if( ret < 0 )
    {
        LOG_ERR( "Could not configure busy gpio" );
        return ret;
    }

    /* Reset pin */
    ret = gpio_pin_configure_dt( &config->reset, GPIO_OUTPUT_INACTIVE );
    if( ret < 0 )
    {
        LOG_ERR( "Could not configure reset gpio" );
        return ret;
    }

    data->radio_status               = RADIO_AWAKE;
    data->tx_power_offset_db_current = config->tx_power_offset_db;  // Has to be copied in user-modified 'data' struct

    /* IRQ pins initialization */
#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER
    data->lr20xx_dev         = dev;
    data->irq_handlers_count = 0;

#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_OWN_THREAD
    data->pending_irq_mask = 0;
    k_sem_init( &data->gpio_sem, 0, K_SEM_MAX_LIMIT );

    k_thread_create( &data->thread, data->thread_stack,
                     CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_THREAD_STACK_SIZE,
                     ( k_thread_entry_t ) lr20xx_thread, data, NULL, NULL,
                     K_PRIO_COOP( CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_THREAD_PRIORITY ), 0, K_NO_WAIT );
#endif /* CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_OWN_THREAD */
#endif /* CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER */

    /* Configure each DIO */
    for( int i = 0; i < config->dios_config_num; i++ )
    {
        lr20xx_dio_cfg_t dio_config = config->dios_config[i];

        if( dio_config.function == LR20XX_SYSTEM_DIO_FUNC_IRQ )
        {
            lr20xx_irq_index_t irq_type = dio_config.irq_type;

            if( irq_type >= LR20XX_MAX_IRQ_DIOS )
            {
                LOG_ERR( "Invalid irq-type %d for DIO %d (max %d)", irq_type, dio_config.dio, LR20XX_MAX_IRQ_DIOS );
                return -EINVAL;
            }

            ret = gpio_pin_configure_dt( &dio_config.gpio, GPIO_INPUT );
            if( ret < 0 )
            {
                LOG_ERR( "Could not configure DIO %d gpio", dio_config.dio );
                return ret;
            }

#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER
            if( data->irq_handlers[irq_type].configured )
            {
                LOG_ERR( "IRQ type %d already configured (duplicate irq-type in DT?)", irq_type );
                return -EINVAL;
            }

            /* Initialize this IRQ handler using irq_type from DT */
            data->irq_handlers[irq_type].gpio          = &config->dios_config[i].gpio;
            data->irq_handlers[irq_type].user_callback = NULL;
            data->irq_handlers[irq_type].configured    = true;

            /* Initialize GPIO callback for this handler using the specific callback for this IRQ type */
            gpio_init_callback( &data->irq_handlers[irq_type].gpio_cb, lr20xx_gpio_callbacks[irq_type],
                                BIT( dio_config.gpio.pin ) );

            if( gpio_add_callback( dio_config.gpio.port, &data->irq_handlers[irq_type].gpio_cb ) )
            {
                LOG_ERR( "Could not set event pin callback for DIO %d", dio_config.dio );
                return -EIO;
            }

#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_GLOBAL_THREAD
            /* Initialize work handler for this IRQ */
            data->work[irq_type].handler = lr20xx_work_handlers[irq_type];
#endif /* CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER_GLOBAL_THREAD */

            LOG_DBG( "Configured IRQ handler %d (%s) for DIO %d on pin %d", irq_type,
                     irq_type == LR20XX_IRQ_INDEX_MAIN ? "MAIN" : "FIFO", dio_config.dio, dio_config.gpio.pin );

            irq_count++;
#endif /* CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER */
        }
    }

#ifdef CONFIG_LORA_BASICS_MODEM_DRIVERS_EVENT_TRIGGER
    data->irq_handlers_count = irq_count;
    LOG_INF( "Initialized %d IRQ handlers", irq_count );
#endif

    return 0;
}

#if IS_ENABLED( CONFIG_PM_DEVICE )
/**
 * @brief Power management action define.
 * Not implemented yet.
 *
 * @param dev
 * @param action
 * @return int
 */
static int lr20xx_pm_action( const struct device* dev, enum pm_device_action action )
{
    int ret = 0;

    switch( action )
    {
    case PM_DEVICE_ACTION_RESUME:
        /* Put the lr20xx into normal operation mode */
        break;
    case PM_DEVICE_ACTION_SUSPEND:
        /* Put the lr20xx into sleep mode */
        break;
    default:
        return -ENOTSUP;
    }

    return ret;
}
#endif /* IS_ENABLED(CONFIG_PM_DEVICE) */

/*
 * Device creation macro.
 */

#define DT_PROP_BY_IDX_U8( node_id, prop, idx ) ( ( uint8_t ) DT_PROP_BY_IDX( node_id, prop, idx ) )

#define DT_TABLE_U8( node_id, prop ) { DT_FOREACH_PROP_ELEM_SEP( node_id, prop, DT_PROP_BY_IDX_U8, (, ) ) }

#define CONFIGURE_GPIO_IF_IN_DT( node_id, name, dt_prop ) \
    COND_CODE_1( DT_NODE_HAS_PROP( node_id, dt_prop ), (.name = GPIO_DT_SPEC_GET( node_id, dt_prop ), ), ( ) )

#define LR20XX_CFG_TCXO( node_id )                                                                       \
    {                                                                                                    \
        .xosc_cfg       = COND_CODE_1( DT_PROP( node_id, tcxo_wakeup_time ) == 0, ( RAL_XOSC_CFG_XTAL ), \
                                       ( RAL_XOSC_CFG_TCXO_RADIO_CTRL ) ),                               \
        .voltage        = DT_PROP( node_id, tcxo_voltage ),                                              \
        .wakeup_time_ms = DT_PROP( node_id, tcxo_wakeup_time ),                                          \
    }

#define LR20XX_CFG_LF_CLK( node_id )                  \
    {                                                 \
        .lf_clk_cfg     = DT_PROP( node_id, lf_clk ), \
        .wait_32k_ready = true,                       \
    }

#define LR20XX_RSSI_CFG( node_id, range )                                                  \
    {                                                                                      \
        .gain_offset = DT_PROP( node_id, DT_CAT3( rssi_calibration_, range, _offset ) ),   \
        .gain_tune   = DT_TABLE_U8( node_id, DT_CAT3( rssi_calibration_, range, _tune ) ), \
    }

#define LR20XX_DIOS_CONFIG( node_id ) { DT_FOREACH_CHILD_SEP( DT_CHILD( node_id, dios ), LR20XX_DIO_CONFIG, (, ) ) }

#define LR20XX_DIO_CONFIG( node_id )                                              \
    {                                                                             \
        .dio           = DT_REG_ADDR( node_id ),                                  \
        .function      = DT_PROP( node_id, function ),                            \
        .sleep_drive   = DT_PROP_OR( node_id, sleep_drive, ( 0 ) ),               \
        .irq_mask      = DT_PROP_OR( node_id, irq_mask, LR20XX_SYSTEM_IRQ_NONE ), \
        .irq_type      = DT_PROP_OR( node_id, irq_type, LR20XX_IRQ_INDEX_MAIN ),  \
        .gpio          = GPIO_DT_SPEC_GET_OR( node_id, dio_gpios, { 0 } ),        \
        .rf_switch_cfg = DT_PROP_OR( node_id, rf_sw, ( 0 ) ),                     \
    }

#define LR20XX_CONFIG( node_id )                                                                                    \
    {                                                                                                               \
        .spi                = SPI_DT_SPEC_GET( node_id, LR20XX_SPI_OPERATION, 0 ),                                  \
        .reset              = GPIO_DT_SPEC_GET( node_id, reset_gpios ),                                             \
        .busy               = GPIO_DT_SPEC_GET( node_id, busy_gpios ),                                              \
        .dios_config_num    = ARRAY_SIZE( lr20xx_dios_config_##node_id ),                                           \
        .dios_config        = lr20xx_dios_config_##node_id,                                                         \
        .hf_clk_out_scaling = DT_PROP_OR( node_id, hf_clk_out_scaling, ( 0 ) ),                                     \
        .tcxo_cfg           = LR20XX_CFG_TCXO( node_id ),                                                           \
        .lf_clck_cfg        = LR20XX_CFG_LF_CLK( node_id ),                                                         \
        .reg_mode           = DT_PROP( node_id, reg_mode ),                                                         \
        .tx_power_offset_db = DT_PROP_OR( node_id, tx_power_offset, 0 ),                                            \
        .rx_boosted_cfg     = ( lr20xx_radio_common_rx_path_boost_mode_t ) DT_PROP_OR( node_id, rx_boost_cfg, 0 ),  \
        .pa_ramp_time       = ( lr20xx_radio_common_ramp_time_t ) DT_PROP_OR( node_id, pa_ramp_time, 5 ),           \
        .pa_lf_cfg_table    = ( lr20xx_pa_pwr_cfg_t* ) DT_CAT( pa_lf_cfg_table_, node_id ),                         \
        .pa_hf_cfg_table    = ( lr20xx_pa_pwr_cfg_t* ) DT_CAT( pa_hf_cfg_table_, node_id ),                         \
        .tx_dbm_to_ua_reg_mode_dcdc_lf_vreg = ( uint32_t* ) DT_CAT( tx_dbm_to_ua_reg_mode_dcdc_lf_vreg_, node_id ), \
        .tx_dbm_to_ua_reg_mode_ldo_lf_vreg  = ( uint32_t* ) DT_CAT( tx_dbm_to_ua_reg_mode_ldo_lf_vreg_, node_id ),  \
        .tx_dbm_to_ua_reg_mode_dcdc_hf_vreg = ( uint32_t* ) DT_CAT( tx_dbm_to_ua_reg_mode_dcdc_hf_vreg_, node_id ), \
        .rx_bw_to_ua_reg_mode_dcdc_lf_vreg  = ( uint32_t* ) DT_CAT( rx_bw_to_ua_reg_mode_dcdc_lf_vreg_, node_id ),  \
        .rx_bw_to_ua_reg_mode_dcdc_hf_vreg  = ( uint32_t* ) DT_CAT( rx_bw_to_ua_reg_mode_dcdc_hf_vreg_, node_id ),  \
        .rx_bw_to_ua_reg_mode_dcdc_lf_vreg_boosted =                                                                \
            ( uint32_t* ) DT_CAT( rx_bw_to_ua_reg_mode_dcdc_lf_vreg_boosted_, node_id ),                            \
        .rx_bw_to_ua_reg_mode_dcdc_hf_vreg_boosted =                                                                \
            ( uint32_t* ) DT_CAT( rx_bw_to_ua_reg_mode_dcdc_hf_vreg_boosted_, node_id ),                            \
        .rx_bw_to_ua_reg_mode_ldo_lf_vreg = ( uint32_t* ) DT_CAT( rx_bw_to_ua_reg_mode_ldo_lf_vreg_, node_id ),     \
        .rx_bw_to_ua_reg_mode_ldo_hf_vreg = ( uint32_t* ) DT_CAT( rx_bw_to_ua_reg_mode_ldo_hf_vreg_, node_id ),     \
        .rx_bw_to_ua_reg_mode_ldo_lf_vreg_boosted =                                                                 \
            ( uint32_t* ) DT_CAT( rx_bw_to_ua_reg_mode_ldo_lf_vreg_boosted_, node_id ),                             \
        .rx_bw_to_ua_reg_mode_ldo_hf_vreg_boosted =                                                                 \
            ( uint32_t* ) DT_CAT( rx_bw_to_ua_reg_mode_ldo_hf_vreg_boosted_, node_id ),                             \
        .calibration_freqs = ( uint32_t* ) DT_CAT( calibration_freqs_, node_id ),                                   \
    }

#define LR20XX_DEVICE_INIT( node_id )                                                            \
    DEVICE_DT_DEFINE( node_id, lr20xx_init, PM_DEVICE_DT_GET( node_id ), &lr20xx_data_##node_id, \
                      &lr20xx_config_##node_id, POST_KERNEL, CONFIG_LORA_BASICS_MODEM_DRIVERS_INIT_PRIORITY, NULL );

#define LR20XX_DEFINE( node_id )                                                                                         \
    static struct lr20xx_hal_context_data_t lr20xx_data_##node_id;                                                       \
    static uint8_t                          pa_lf_cfg_table_##node_id[] = DT_TABLE_U8( node_id, tx_power_cfg_lf );       \
    static uint8_t                          pa_hf_cfg_table_##node_id[] = DT_TABLE_U8( node_id, tx_power_cfg_hf );       \
    static uint32_t                         tx_dbm_to_ua_reg_mode_dcdc_lf_vreg_##node_id[] =                             \
        DT_PROP( node_id, tx_dbm_to_ua_reg_mode_dcdc_lf_vreg );                                                          \
    static uint32_t tx_dbm_to_ua_reg_mode_ldo_lf_vreg_##node_id[] =                                                      \
        DT_PROP( node_id, tx_dbm_to_ua_reg_mode_ldo_lf_vreg );                                                           \
    static uint32_t tx_dbm_to_ua_reg_mode_dcdc_hf_vreg_##node_id[] =                                                     \
        DT_PROP( node_id, tx_dbm_to_ua_reg_mode_dcdc_hf_vreg );                                                          \
    static uint32_t rx_bw_to_ua_reg_mode_dcdc_lf_vreg_##node_id[] =                                                      \
        DT_PROP( node_id, rx_bw_to_ua_reg_mode_dcdc_lf_vreg );                                                           \
    static uint32_t rx_bw_to_ua_reg_mode_dcdc_hf_vreg_##node_id[] =                                                      \
        DT_PROP( node_id, rx_bw_to_ua_reg_mode_dcdc_hf_vreg );                                                           \
    static uint32_t rx_bw_to_ua_reg_mode_dcdc_lf_vreg_boosted_##node_id[] =                                              \
        DT_PROP( node_id, rx_bw_to_ua_reg_mode_dcdc_lf_vreg_boosted );                                                   \
    static uint32_t rx_bw_to_ua_reg_mode_dcdc_hf_vreg_boosted_##node_id[] =                                              \
        DT_PROP( node_id, rx_bw_to_ua_reg_mode_dcdc_hf_vreg_boosted );                                                   \
    static uint32_t rx_bw_to_ua_reg_mode_ldo_lf_vreg_##node_id[] =                                                       \
        DT_PROP( node_id, rx_bw_to_ua_reg_mode_ldo_lf_vreg );                                                            \
    static uint32_t rx_bw_to_ua_reg_mode_ldo_hf_vreg_##node_id[] =                                                       \
        DT_PROP( node_id, rx_bw_to_ua_reg_mode_ldo_hf_vreg );                                                            \
    static uint32_t rx_bw_to_ua_reg_mode_ldo_lf_vreg_boosted_##node_id[] =                                               \
        DT_PROP( node_id, rx_bw_to_ua_reg_mode_ldo_lf_vreg_boosted );                                                    \
    static uint32_t rx_bw_to_ua_reg_mode_ldo_hf_vreg_boosted_##node_id[] =                                               \
        DT_PROP( node_id, rx_bw_to_ua_reg_mode_ldo_hf_vreg_boosted );                                                    \
    static uint32_t                              calibration_freqs_##node_id[]  = DT_PROP( node_id, calibration_freqs ); \
    static lr20xx_dio_cfg_t                      lr20xx_dios_config_##node_id[] = LR20XX_DIOS_CONFIG( node_id );         \
    static const struct lr20xx_hal_context_cfg_t lr20xx_config_##node_id        = LR20XX_CONFIG( node_id );              \
    PM_DEVICE_DT_DEFINE( node_id, lr20xx_pm_action );                                                                    \
    LR20XX_DEVICE_INIT( node_id )

DT_FOREACH_STATUS_OKAY( semtech_lr2021, LR20XX_DEFINE )
DT_FOREACH_STATUS_OKAY( semtech_lr2022, LR20XX_DEFINE )
