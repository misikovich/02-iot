#if !defined(BLINKER_H)
#define BLINKER_H

#include "hid.h"

/**
 * @brief Enable or disable the left blinker.
 */
void blinker_set_l(bool enabled);

/**
 * @brief Enable or disable the right blinker.
 */
void blinker_set_r(bool enabled);

/**
 * @brief FreeRTOS task that owns HID ticking and both blinker animations.
 *
 * The task initializes both channels to zero. Do not start hid_task()
 * separately while this task is running.
 */
void blinker_task(void *params);

#endif // BLINKER_H
