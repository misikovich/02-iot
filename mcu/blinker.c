#include "blinker.h"
#include "log.h"

#include "mcc_generated_files/oc1.h"
#include "mcc_generated_files/oc3.h"

#define BLINKER_RISE_MS      80U
#define BLINKER_FADE_MS      320U
#define BLINKER_GAP_MS       300U
#define BLINKER_STOP_FADE_MS 160U
#define BLINKER_TICK_MS      (1000U / HID_TPS)

typedef enum
{
    BLINKER_PHASE_DISABLED,
    BLINKER_PHASE_RISING,
    BLINKER_PHASE_FADING,
    BLINKER_PHASE_GAP,
    BLINKER_PHASE_STOPPING
} BlinkerPhase;

typedef struct
{
    Led led;
    u32 phase_started_at_ms;
    BlinkerPhase phase;
    LogBlinkerSide side;
} BlinkerState;

static volatile bool blinker_l_enabled;
static volatile bool blinker_r_enabled;

static void blinker_l_drive(u8 value)
{
    OC3_PrimaryValueSet(pwm_lerp8(value));
    log_write(LOG_EVENT_BLINKER_OUTPUT, LOG_BLINKER_L, value);
}

static void blinker_r_drive(u8 value)
{
    OC1_PrimaryValueSet(pwm_lerp8(value));
    log_write(LOG_EVENT_BLINKER_OUTPUT, LOG_BLINKER_R, value);
}

static const Led blinker_leds[] =
{
    blinker_l_drive,
    blinker_r_drive
};

static void blinker_phase_set(
    BlinkerState *state,
    BlinkerPhase phase,
    u32 now_ms)
{
    state->phase = phase;
    state->phase_started_at_ms = now_ms;
    log_write(LOG_EVENT_BLINKER_PHASE, state->side, phase);
}

static bool blinker_transition_start(
    BlinkerState *state,
    u8 target_value,
    u32 duration_ms,
    HidInterpolation interpolation,
    BlinkerPhase phase,
    u32 now_ms)
{
    if (!hid_transit(state->led, target_value, duration_ms, interpolation))
    {
        log_write(LOG_EVENT_BLINKER_TRANSITION_REJECTED, state->side, phase);
        return false;
    }

    blinker_phase_set(state, phase, now_ms);
    return true;
}

static void blinker_update(BlinkerState *state, bool enabled, u32 now_ms)
{
    u32 elapsed_ms = now_ms - state->phase_started_at_ms;

    if (!enabled)
    {
        if (state->phase == BLINKER_PHASE_DISABLED)
        {
            return;
        }

        if (state->phase == BLINKER_PHASE_GAP)
        {
            blinker_phase_set(state, BLINKER_PHASE_DISABLED, now_ms);
            return;
        }

        if (state->phase == BLINKER_PHASE_STOPPING)
        {
            if (elapsed_ms >= BLINKER_STOP_FADE_MS)
            {
                blinker_phase_set(state, BLINKER_PHASE_DISABLED, now_ms);
            }

            return;
        }

        blinker_transition_start(
            state,
            0,
            BLINKER_STOP_FADE_MS,
            hid_inter_smoothstep,
            BLINKER_PHASE_STOPPING,
            now_ms
        );
        return;
    }

    switch (state->phase)
    {
        case BLINKER_PHASE_DISABLED:
        case BLINKER_PHASE_STOPPING:
            blinker_transition_start(
                state,
                UINT8_MAX,
                BLINKER_RISE_MS,
                hid_inter_linear,
                BLINKER_PHASE_RISING,
                now_ms
            );
            break;

        case BLINKER_PHASE_RISING:
            if (elapsed_ms >= BLINKER_RISE_MS)
            {
                blinker_transition_start(
                    state,
                    0,
                    BLINKER_FADE_MS,
                    hid_inter_smoothstep,
                    BLINKER_PHASE_FADING,
                    now_ms
                );
            }
            break;

        case BLINKER_PHASE_FADING:
            if (elapsed_ms >= BLINKER_FADE_MS)
            {
                blinker_phase_set(state, BLINKER_PHASE_GAP, now_ms);
            }
            break;

        case BLINKER_PHASE_GAP:
            if (elapsed_ms >= BLINKER_GAP_MS)
            {
                blinker_transition_start(
                    state,
                    UINT8_MAX,
                    BLINKER_RISE_MS,
                    hid_inter_linear,
                    BLINKER_PHASE_RISING,
                    now_ms
                );
            }
            break;
    }
}

static void blinker_enabled_get(bool *l_enabled, bool *r_enabled)
{
    al_critical_enter();
    *l_enabled = blinker_l_enabled;
    *r_enabled = blinker_r_enabled;
    al_critical_exit();
}

void blinker_set_l(bool enabled)
{
    al_critical_enter();
    blinker_l_enabled = enabled;
    al_critical_exit();
    log_write(LOG_EVENT_BLINKER_SET, LOG_BLINKER_L, enabled);
}

void blinker_set_r(bool enabled)
{
    al_critical_enter();
    blinker_r_enabled = enabled;
    al_critical_exit();
    log_write(LOG_EVENT_BLINKER_SET, LOG_BLINKER_R, enabled);
}

void blinker_task(void *params)
{
    BlinkerState l_state;
    BlinkerState r_state;
    TickType_t previous_wake_tick;
    bool l_enabled;
    bool r_enabled;
    u32 now_ms;

    unused(params);
    log_write(LOG_EVENT_BLINKER_TASK_START, 0, 0);

    if (!hid_init(blinker_leds, 2))
    {
        log_write(LOG_EVENT_BLINKER_HID_INIT, false, 2);
        al_task_delete(NULL);
        return;
    }

    log_write(LOG_EVENT_BLINKER_HID_INIT, true, 2);

    l_state.led = blinker_l_drive;
    l_state.phase_started_at_ms = 0;
    l_state.phase = BLINKER_PHASE_DISABLED;
    l_state.side = LOG_BLINKER_L;

    r_state.led = blinker_r_drive;
    r_state.phase_started_at_ms = 0;
    r_state.phase = BLINKER_PHASE_DISABLED;
    r_state.side = LOG_BLINKER_R;

    previous_wake_tick = xTaskGetTickCount();

    forever
    {
        blinker_enabled_get(&l_enabled, &r_enabled);
        now_ms = al_millis();

        blinker_update(&l_state, l_enabled, now_ms);
        blinker_update(&r_state, r_enabled, now_ms);
        hid_tick();

        al_task_delay_until_ms(&previous_wake_tick, BLINKER_TICK_MS);
    }
}
