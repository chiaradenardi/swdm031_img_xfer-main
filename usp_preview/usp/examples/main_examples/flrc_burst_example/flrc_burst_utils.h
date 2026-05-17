/*!
 * @file      flrc_burst_utils.h
 *
 * @brief     Utils for the FLRC burst example
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2024. All rights reserved.
 */

#ifndef __FLRC_BURST_UTILS_H__
#define __FLRC_BURST_UTILS_H__

#include <stdint.h>

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */
uint32_t generate_crc( const uint8_t* buf, int len );

#endif  // __FLRC_BURST_UTILS_H__
