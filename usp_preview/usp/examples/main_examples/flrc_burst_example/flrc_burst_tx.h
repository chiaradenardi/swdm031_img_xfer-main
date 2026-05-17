/*!
 * @file      flrc_burst_tx.h
 *
 * @brief     Simple FLRC burst example with LORA and FLRC Burst modulation
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2024. All rights reserved.
 */

#ifndef __FLRC_BURST_TX_H__
#define __FLRC_BURST_TX_H__

#include <stdint.h>

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */
void flrc_burst_protocol_tx_init( void ( *user_callback )( bool is_successful ) );

void flrc_burst_protocol_tx_start_new_transaction( uint8_t* tx_data, uint32_t tx_data_size );

bool flrc_burst_protocol_tx_is_ready( void );

#endif  // __FLRC_BURST_TX_H__
