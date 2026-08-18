#if !defined(LED_H)
#define LED_H

#include "al.h"

/**
 * @brief Universal single-channel LED driver interface.
 *
 * A value of 0 turns the LED off and 255 requests maximum output. The driver
 * defines how intermediate values map to its hardware.
 */
typedef void (*Led)(u8 value);

/**
 * @brief Set the LED output through its driver.
 *
 * @param led Non-null LED driver.
 * @param value Normalized output value from 0 to 255.
 */
void led_set(Led led, u8 value);

#endif // LED_H
