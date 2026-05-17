/*!
 * @file      flrc_burst_rx.h
 *
 * @brief     Simple FLRC burst example with LORA and FLRC Burst modulation
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2024. All rights reserved.
 */

#ifndef __FLRC_BURST_RX_H__
#define __FLRC_BURST_RX_H__

#include <stdint.h>

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */
void flrc_burst_protocol_rx_init( uint8_t* payload, uint32_t payload_size, void ( *user_callback )( void ) );
#endif  // __FLRC_BURST_RX_H__
