#include "led.h"

void led_set(Led led, u8 value)
{
    led(value);
}
