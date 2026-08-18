#if !defined(HID_H)
#define HID_H

#include "led.h"

#if !defined(HID_TPS)
#define HID_TPS 50U
#endif

#if !defined(HID_MAX_LEDS)
#define HID_MAX_LEDS 8U
#endif

#if !defined(HID_COMMAND_CAPACITY)
#define HID_COMMAND_CAPACITY 8U
#endif

#if HID_TPS == 0U || HID_TPS > 1000U || (1000U % HID_TPS) != 0U
#error "HID_TPS must be a non-zero divisor of 1000"
#endif

#if HID_MAX_LEDS == 0U || HID_MAX_LEDS > 255U
#error "HID_MAX_LEDS must be between 1 and 255"
#endif

#if HID_COMMAND_CAPACITY == 0U || HID_COMMAND_CAPACITY > 255U
#error "HID_COMMAND_CAPACITY must be between 1 and 255"
#endif

#define HID_MAX_TRANSITION_MS (UINT32_MAX / 255U)

/**
 * @brief Maps normalized transition progress from 0..255 to 0..255.
 *
 * Interpolation functions must be deterministic and non-blocking.
 */
typedef u8 (*HidInterpolation)(u8 progress);

/**
 * @brief Initialize HID with a fixed set of LED drivers.
 *
 * Every LED is set to zero during initialization. Call once after hardware
 * initialization and before starting hid_task().
 *
 * @return true when initialized; false for invalid or repeated initialization.
 */
bool hid_init(const Led *leds, u8 led_count);

/**
 * @brief Queue an LED value to be applied on the next HID tick.
 *
 * @return true when queued; false when HID is unavailable, the LED is unknown,
 * or the command queue is full.
 */
bool hid_set(Led led, u8 value);

/**
 * @brief Queue a transition from the last rendered value to a target value.
 *
 * @return true when queued; false for invalid arguments, an unknown LED, or a
 * full command queue.
 */
bool hid_transit(
    Led led,
    u8 value,
    u32 duration_ms,
    HidInterpolation interpolation
);

/**
 * @brief Process pending commands and render one transition frame.
 *
 * Call manually or through hid_task(), never concurrently from both.
 */
void hid_tick(void);

/**
 * @brief FreeRTOS task entry that runs hid_tick() at HID_TPS.
 */
void hid_task(void *params);

/**
 * @brief Linear normalized interpolation.
 */
u8 hid_inter_linear(u8 progress);

/**
 * @brief Smoothstep normalized interpolation.
 */
u8 hid_inter_smoothstep(u8 progress);

#endif // HID_H
