/**
 * @file xiaosync.c
 * @date 14.05.2026
 */

#include "xiaosync.h"

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/spinlock.h>

#include <zephyr/drivers/gpio.h>
#include <nrfx_gpiote.h>
#include <nrfx_grtc.h>
#include <helpers/nrfx_gppi.h>
#include <gpiote_nrfx.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(xiaosync);

/* Extract devicetree node properties */

#define SYNC_NODE DT_ALIAS(sync0)

#define SYNC_GPIO_FLAGS DT_GPIO_FLAGS(SYNC_NODE, capture_gpios)
#define SYNC_CAPTURE_EDGE DT_PROP(SYNC_NODE, capture_edge)

#define SYNC_INPUT_PIN NRF_DT_GPIOS_TO_PSEL(SYNC_NODE, capture_gpios)
#define SYNC_GPIOTE_NODE NRF_DT_GPIOTE_NODE(SYNC_NODE, capture_gpios)

#if SYNC_CAPTURE_EDGE == 0
#define SYNC_GPIOTE_TRIGGER NRFX_GPIOTE_TRIGGER_LOTOHI
#elif SYNC_CAPTURE_EDGE == 1
#define SYNC_GPIOTE_TRIGGER NRFX_GPIOTE_TRIGGER_HITOLO
#elif SYNC_CAPTURE_EDGE == 2
#define SYNC_GPIOTE_TRIGGER NRFX_GPIOTE_TRIGGER_TOGGLE
#else
#error "capture-edge must be 0 (rising), 1 (falling), or 2 (either)"
#endif

#if (SYNC_GPIO_FLAGS & GPIO_PULL_UP) != 0
#define SYNC_PULL_CONFIG NRF_GPIO_PIN_PULLUP
#elif (SYNC_GPIO_FLAGS & GPIO_PULL_DOWN) != 0
#define SYNC_PULL_CONFIG NRF_GPIO_PIN_PULLDOWN
#else
#define SYNC_PULL_CONFIG NRF_GPIO_PIN_NOPULL
#endif

/* -------------
 * Private state
 * ------------- */

/** nrfx GPIOTE instance selected by the SYNC_NODE */
static nrfx_gpiote_t* const kgGpiote =
    &GPIOTE_NRFX_INST_BY_NODE(SYNC_GPIOTE_NODE);

/** Allocated GPIOTE input channel */
static uint8_t gGpioteChannel;

/** Allocated GRTC capture/compare (CC) channel */
static uint8_t gGrtcChannel;

/** Allocated PPI connection GPIOTE event -> GRTC capture task */
static nrfx_gppi_handle_t gGppi;

/** Work item to process sync GPIOTE event outside the interrupt handler */
static struct k_work gCaptureWork;

/** Protects the epoch value */
static struct k_spinlock gEpochLock;

/** Captured SYSCOUNTER tick that defines reference time zero */
static uint64_t gEpochTicks;

/* ------------------
 * Private functions
 * ------------------ */

/** GPIOTE IRQ Handler for the edge detection event */
static void gpioIRQHandler(nrfx_gpiote_pin_t pin, nrfx_gpiote_trigger_t trigger,
                           void* context) {
  ARG_UNUSED(pin);
  ARG_UNUSED(trigger);
  ARG_UNUSED(context);

  // Mask new events
  nrfx_gpiote_trigger_disable(kgGpiote, SYNC_INPUT_PIN);

  // Schedule processing of the synchronization event
  k_work_submit(&gCaptureWork);
}

/**
 * @brief Process GRTC SYSCOUNTER captured value
 * Delayed processing submitted by the gpioIRQHandler.
 */
static void gpioteWorkHandler(struct k_work* work) {
  ARG_UNUSED(work);

  // Get captured value from CC channel
  uint64_t ticks = nrfx_grtc_sys_counter_cc_get(gGrtcChannel);

  // Atomically write the global variable
  k_spinlock_key_t key = k_spin_lock(&gEpochLock);
  gEpochTicks = ticks;
  k_spin_unlock(&gEpochLock, key);

  LOG_INF("epoch set @ SYSCOUNTER = %" PRIu64, ticks);

  // Unmask new events
  nrfx_gpiote_trigger_enable(kgGpiote, SYNC_INPUT_PIN, true);
}

/* ------------------
 * Public functions
 * ------------------ */

int syncInit(void) {
  int err;

  // configure work item for processing synchronization events
  k_work_init(&gCaptureWork, gpioteWorkHandler);

  /*
   * GRTC initialization
   */

  if (!nrfx_grtc_init_check()) {
    LOG_ERR("nrfx GRTC driver is not initialized");
    return -ENODEV;
  }

  // Prevent SYSCOUNTER from going to sleep
  nrfx_grtc_active_request_set(true);

  err = nrfx_grtc_channel_alloc(&gGrtcChannel);
  if (err) {
    LOG_ERR("failed to allocate GRTC CC channel");
    return err;
  }

  /*
   * GPIO(TE) initialization
   */

  err = nrfx_gpiote_init(kgGpiote, 0);
  if (err && (err != -EALREADY)) {
    LOG_ERR("failed to initialize nrfx GPIOTE driver");
    return err;
  }

  err = nrfx_gpiote_channel_alloc(kgGpiote, &gGpioteChannel);
  if (err) {
    LOG_ERR("failed to allocate GPIOTE channel");
    return err;
  }

  const nrf_gpio_pin_pull_t pull_cfg = SYNC_PULL_CONFIG;

  const nrfx_gpiote_handler_config_t handler_cfg = {
      .handler = gpioIRQHandler,
      .p_context = NULL,
  };

  const nrfx_gpiote_trigger_config_t trigger_cfg = {
      .trigger = SYNC_GPIOTE_TRIGGER,
      .p_in_channel = &gGpioteChannel,
  };

  nrfx_gpiote_input_pin_config_t input_cfg = {
      .p_pull_config = &pull_cfg,
      .p_trigger_config = &trigger_cfg,
      .p_handler_config = &handler_cfg,
  };

  // Configure sync pin as edge-sensitive input
  err = nrfx_gpiote_input_configure(kgGpiote, SYNC_INPUT_PIN, &input_cfg);
  if (err) {
    LOG_ERR("failed to configure sync pin with GPIOTE");
    return err;
  }

  /*
   * PPI configuration
   */

  uint32_t gpio_event =
      nrfx_gpiote_in_event_address_get(kgGpiote, SYNC_INPUT_PIN);
  uint32_t grtc_capture_task = nrfx_grtc_capture_task_address_get(gGrtcChannel);

  // Allocate a connection between sync event and GRTC capture task
  err = nrfx_gppi_conn_alloc(gpio_event, grtc_capture_task, &gGppi);
  if (err) {
    LOG_ERR("failed to configure PPI connection");
    return err;
  }

  nrfx_gppi_conn_enable(gGppi);

  return 0;
}

void syncEnable(void) {
  nrfx_gpiote_trigger_enable(kgGpiote, SYNC_INPUT_PIN, true);

  LOG_INF("waiting for sync edge on P%u.%02u ...",
          (unsigned int)(SYNC_INPUT_PIN >> 5),
          (unsigned int)(SYNC_INPUT_PIN & 0x1F));
}

int syncDisable(void) {
  nrfx_gpiote_trigger_disable(kgGpiote, SYNC_INPUT_PIN);
  return k_work_cancel(&gCaptureWork);
}

int64_t getDeltaMicroSeconds(void) {
  k_spinlock_key_t key;
  uint64_t epoch;

  key = k_spin_lock(&gEpochLock);
  epoch = gEpochTicks;
  k_spin_unlock(&gEpochLock, key);

  if (!epoch) {
    LOG_DBG("epoch not set");
    return -1;
  }

  return (int64_t)(nrfx_grtc_syscounter_get() - epoch);
}
