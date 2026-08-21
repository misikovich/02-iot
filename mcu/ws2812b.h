#if !defined(WS2812B_H)
#define WS2812B_H

#include "al.h"

#if !defined(WS2812B_MAX_PIXELS)
#error "WS2812B_MAX_PIXELS must be defined by the project"
#endif

#if WS2812B_MAX_PIXELS == 0
#error "WS2812B_MAX_PIXELS must be greater than zero"
#endif

#if ((9UL * WS2812B_MAX_PIXELS) + 32UL) > 16384UL
#error "WS2812B frame exceeds the DMA transfer limit"
#endif

/**
 * @brief Direct 8-bit RGB color with no brightness or gamma transformation.
 *
 * The driver converts this caller-facing RGB order to WS2812B GRB wire order.
 */
typedef struct
{
    u8 red;
    u8 green;
    u8 blue;
} Ws2812bColor;

/**
 * @brief Claim and configure SPI2 and DMA for WS2812B output.
 *
 * Call once from task context after al_system_init().
 *
 * @return true when initialized; false when already initialized or when the
 * generated SPI2 configuration is incompatible.
 */
bool ws2812b_init(void);

/**
 * @brief Encode and asynchronously transmit an RGB frame.
 *
 * Call from task context. The colors array only needs to remain valid until
 * this function returns. Values are transmitted directly without brightness
 * scaling or gamma correction.
 *
 * @param colors Non-null caller-owned RGB array.
 * @param color_count Number of colors from 1 through WS2812B_MAX_PIXELS.
 * @return true when the frame was accepted; false for invalid state,
 * arguments, or while another frame is active.
 */
bool ws2812b_show(const Ws2812bColor *colors, u16 color_count);

/**
 * @brief Report whether a frame, including its trailing reset interval, is active.
 *
 * The DMA completion ISR uses no FreeRTOS API.
 */
bool ws2812b_busy(void);

#endif // WS2812B_H
