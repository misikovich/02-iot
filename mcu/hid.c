#include "hid.h"

#define HID_PERIOD_MS (1000U / HID_TPS)

typedef struct
{
    HidInterpolation interpolation;
    u32 duration_ms;
    u8 led_index;
    u8 target_value;
} HidCommand;

typedef struct
{
    Led led;
    HidInterpolation interpolation;
    u32 started_at_ms;
    u32 duration_ms;
    u8 start_value;
    u8 current_value;
    u8 target_value;
    bool active;
    bool dirty;
} HidLedState;

static HidLedState hid_leds[HID_MAX_LEDS];
static u8 hid_led_count;

static StaticQueue_t hid_queue_control;
static u8 hid_queue_storage[HID_COMMAND_CAPACITY * sizeof(HidCommand)];
static QueueHandle_t hid_queue;

static i16 hid_led_find(Led led)
{
    u8 i;

    for (i = 0; i < hid_led_count; i++)
    {
        if (hid_leds[i].led == led)
        {
            return (i16)i;
        }
    }

    return -1;
}

static bool hid_command_send(
    Led led,
    u8 target_value,
    u32 duration_ms,
    HidInterpolation interpolation)
{
    HidCommand command;
    i16 led_index;

    if (hid_queue == NULL)
    {
        return false;
    }

    led_index = hid_led_find(led);

    if (led_index < 0)
    {
        return false;
    }

    command.interpolation = interpolation;
    command.duration_ms = duration_ms;
    command.led_index = (u8)led_index;
    command.target_value = target_value;

    return xQueueSend(hid_queue, &command, 0) == pdPASS;
}

static void hid_command_apply(const HidCommand *command, u32 now_ms)
{
    HidLedState *state = &hid_leds[command->led_index];

    if (command->duration_ms == 0)
    {
        state->start_value = command->target_value;
        state->target_value = command->target_value;
        state->duration_ms = 0;
        state->active = false;

        if (state->current_value != command->target_value)
        {
            state->current_value = command->target_value;
            state->dirty = true;
        }

        return;
    }

    state->interpolation = command->interpolation;
    state->started_at_ms = now_ms;
    state->duration_ms = command->duration_ms;
    state->start_value = state->current_value;
    state->target_value = command->target_value;
    state->active = state->current_value != command->target_value;
}

static u8 hid_value_interpolate(u8 start, u8 target, u8 progress)
{
    u32 offset;

    if (target >= start)
    {
        offset = ((u32)(target - start) * progress + 127U) / 255U;
        return (u8)(start + offset);
    }

    offset = ((u32)(start - target) * progress + 127U) / 255U;
    return (u8)(start - offset);
}

static void hid_led_update(HidLedState *state, u32 now_ms)
{
    u32 elapsed_ms;
    u8 progress;
    u8 value;

    if (!state->active)
    {
        return;
    }

    elapsed_ms = now_ms - state->started_at_ms;

    if (elapsed_ms >= state->duration_ms)
    {
        value = state->target_value;
        state->active = false;
    }
    else
    {
        progress = (u8)((elapsed_ms * 255U) / state->duration_ms);
        value = hid_value_interpolate(
            state->start_value,
            state->target_value,
            state->interpolation(progress)
        );
    }

    if (state->current_value != value)
    {
        state->current_value = value;
        state->dirty = true;
    }
}

bool hid_init(const Led *leds, u8 led_count)
{
    QueueHandle_t queue;
    u8 i;
    u8 j;

    if (hid_queue != NULL || leds == NULL || led_count == 0 || led_count > HID_MAX_LEDS)
    {
        return false;
    }

    for (i = 0; i < led_count; i++)
    {
        if (leds[i] == NULL)
        {
            return false;
        }

        for (j = 0; j < i; j++)
        {
            if (leds[j] == leds[i])
            {
                return false;
            }
        }
    }

    queue = xQueueCreateStatic(
        HID_COMMAND_CAPACITY,
        sizeof(HidCommand),
        hid_queue_storage,
        &hid_queue_control
    );

    if (queue == NULL)
    {
        return false;
    }

    for (i = 0; i < led_count; i++)
    {
        hid_leds[i].led = leds[i];
        hid_leds[i].interpolation = hid_inter_linear;
        hid_leds[i].started_at_ms = 0;
        hid_leds[i].duration_ms = 0;
        hid_leds[i].start_value = 0;
        hid_leds[i].current_value = 0;
        hid_leds[i].target_value = 0;
        hid_leds[i].active = false;
        hid_leds[i].dirty = false;
    }

    hid_led_count = led_count;
    hid_queue = queue;

    for (i = 0; i < led_count; i++)
    {
        led_set(leds[i], 0);
    }

    return true;
}

bool hid_set(Led led, u8 value)
{
    return hid_command_send(led, value, 0, hid_inter_linear);
}

bool hid_transit(
    Led led,
    u8 value,
    u32 duration_ms,
    HidInterpolation interpolation)
{
    if (duration_ms == 0 || duration_ms > HID_MAX_TRANSITION_MS || interpolation == NULL)
    {
        return false;
    }

    return hid_command_send(led, value, duration_ms, interpolation);
}

void hid_tick(void)
{
    HidCommand command;
    u32 now_ms;
    u8 command_count;
    u8 i;

    if (hid_queue == NULL)
    {
        return;
    }

    now_ms = al_millis();
    for (i = 0; i < hid_led_count; i++)
    {
        hid_led_update(&hid_leds[i], now_ms);
    }

    command_count = 0;

    while (
        command_count < HID_COMMAND_CAPACITY &&
        xQueueReceive(hid_queue, &command, 0) == pdPASS)
    {
        hid_command_apply(&command, now_ms);
        command_count++;
    }

    for (i = 0; i < hid_led_count; i++)
    {

        if (hid_leds[i].dirty)
        {
            led_set(hid_leds[i].led, hid_leds[i].current_value);
            hid_leds[i].dirty = false;
        }
    }
}

void hid_task(void *params)
{
    TickType_t previous_wake_tick = xTaskGetTickCount();

    unused(params);

    forever
    {
        hid_tick();
        al_task_delay_until_ms(&previous_wake_tick, HID_PERIOD_MS);
    }
}

u8 hid_inter_linear(u8 progress)
{
    return progress;
}

u8 hid_inter_smoothstep(u8 progress)
{
    u32 value = progress;

    return (u8)(
        ((value * value * (765U - (2U * value))) + 32512U) / 65025U
    );
}
