/**
 * @file xiaosync.h
 * @date 14.05.2026
 */

#include <stdint.h>

/**
 * @brief Peripheral configuration
 *
 * Configure the GRTC for the capture task.
 * Configure the GPIOTE for event generation triggered by the external signal.
 * Configure the DPPI to connect the GPIOTE even to the GRTC capture task.
 *
 * @retval 0 Configuration success
 * @retval -ENODEV Missing nrfx driver
 * @return nrfx error code on failure
 */
int syncInit(void);

/**
 * @brief Enable edge detection on the external trigger signal
 */
void syncEnable(void);

/**
 * @brief Disable edge detection on the external trigger signal
 *
 * It waits for pending synchronization events to be processed.
 *
 * @retval 0 No events were pending
 * @retval k_work_busy_get() status
 */
int syncDisable(void);

/**
 * @brief Get the elapsed time since the synchronization event
 *
 * The GRTC 52-bit SYSCOUNTER runs at 1 MHz.
 * The function computes the elapsed time in microseconds since the tick
 * captured on the active edge of the external synchronization signal.
 *
 * @retval -1 No synchronization
 * @return Elapsed microseconds since epoch
 */
int64_t getDeltaMicroSeconds(void);
