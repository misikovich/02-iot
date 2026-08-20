#include "blinker.h"
#include "blinker_seq.h"
#include "led.h"
#include "log.h"

#include "mcc_generated_files/oc1.h"
#include "mcc_generated_files/oc3.h"

#define BLINKER_TIMEBASE_MS 1000U
#define BLINKER_FRAME_COUNT len(BLINKER_SEQ)

#if !defined(BLINKER_SEQ_TPS)
#error "blinker_seq.h must define BLINKER_SEQ_TPS"
#endif

#if !defined(BLINKER_SEQ_PEAK_FRAME)
#error "blinker_seq.h must define BLINKER_SEQ_PEAK_FRAME"
#endif

#if BLINKER_SEQ_TPS == 0U || BLINKER_SEQ_TPS > BLINKER_TIMEBASE_MS
#error "BLINKER_SEQ_TPS must be between 1 and 1000"
#endif

typedef char BlinkerSeqFrameCountValid[
    BLINKER_FRAME_COUNT > 0U && BLINKER_FRAME_COUNT <= UINT16_MAX ? 1 : -1
];

typedef char BlinkerSeqPeakFrameValid[
    BLINKER_SEQ_PEAK_FRAME < BLINKER_FRAME_COUNT ? 1 : -1
];

typedef enum
{
    BLINKER_L,
    BLINKER_R,
    BLINKER_COUNT
} BlinkerIndex;

typedef struct
{
    Led led;
    LogBlinkerSide side;
    volatile bool requested;
    u16 frame;
    u8 output_value;
    bool running;
} Blinker;

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

static Blinker blinkers[BLINKER_COUNT] =
{
    {
        .led = blinker_l_drive,
        .side = LOG_BLINKER_L
    },
    {
        .led = blinker_r_drive,
        .side = LOG_BLINKER_R
    }
};

static void blinker_output_set(Blinker *blinker, u8 value)
{
    if (blinker->output_value == value)
    {
        return;
    }

    blinker->output_value = value;
    led_set(blinker->led, value);
}

static void blinker_stop(Blinker *blinker)
{
    blinker->running = false;
    blinker->frame = 0;
}

static void blinker_frame_advance(Blinker *blinker)
{
    blinker->frame++;

    if (blinker->frame >= BLINKER_FRAME_COUNT)
    {
        blinker->frame = 0;
    }
}

static void blinker_update(Blinker *blinker, bool requested)
{
    u16 next_frame;

    if (!blinker->running)
    {
        if (requested)
        {
            blinker->running = true;
            blinker->frame = 0;
        }
    }
    else if (requested)
    {
        blinker_frame_advance(blinker);
    }
    else if (blinker->frame < BLINKER_SEQ_PEAK_FRAME)
    {
        if (blinker->frame == 0)
        {
            blinker_stop(blinker);
        }
        else
        {
            blinker->frame--;
        }
    }
    else if (BLINKER_SEQ[blinker->frame] == 0U)
    {
        blinker_stop(blinker);
    }
    else
    {
        next_frame = (u16)(blinker->frame + 1U);

        if (
            next_frame >= BLINKER_FRAME_COUNT ||
            BLINKER_SEQ[next_frame] == 0U)
        {
            blinker_stop(blinker);
        }
        else
        {
            blinker->frame = next_frame;
        }
    }

    blinker_output_set(
        blinker,
        blinker->running ? BLINKER_SEQ[blinker->frame] : 0U
    );
}

static void blinker_requested_set(Blinker *blinker, bool requested)
{
    al_critical_enter();
    blinker->requested = requested;
    al_critical_exit();

    log_write(LOG_EVENT_BLINKER_SET, blinker->side, requested);
}

static void blinker_requested_get(bool requested[BLINKER_COUNT])
{
    u8 i;

    al_critical_enter();

    for (i = 0; i < BLINKER_COUNT; i++)
    {
        requested[i] = blinkers[i].requested;
    }

    al_critical_exit();
}

static u32 blinker_delay_ms_get(u16 *remainder)
{
    u32 accumulated_remainder;
    u32 delay_ms;

    delay_ms = BLINKER_TIMEBASE_MS / BLINKER_SEQ_TPS;
    accumulated_remainder =
        (u32)*remainder + (BLINKER_TIMEBASE_MS % BLINKER_SEQ_TPS);

    if (accumulated_remainder >= BLINKER_SEQ_TPS)
    {
        accumulated_remainder -= BLINKER_SEQ_TPS;
        delay_ms++;
    }

    *remainder = (u16)accumulated_remainder;
    return delay_ms;
}

void blinker_set_l(bool enabled)
{
    blinker_requested_set(&blinkers[BLINKER_L], enabled);
}

void blinker_set_r(bool enabled)
{
    blinker_requested_set(&blinkers[BLINKER_R], enabled);
}

void blinker_task(void *params)
{
    TickType_t previous_wake_tick;
    bool requested[BLINKER_COUNT];
    u16 delay_remainder;
    u8 i;

    drop(params);
    log_write(LOG_EVENT_BLINKER_TASK_START, 0, 0);

    for (i = 0; i < BLINKER_COUNT; i++)
    {
        led_set(blinkers[i].led, 0);
    }

    previous_wake_tick = xTaskGetTickCount();
    delay_remainder = 0;

    forever
    {
        blinker_requested_get(requested);

        for (i = 0; i < BLINKER_COUNT; i++)
        {
            blinker_update(&blinkers[i], requested[i]);
        }

        al_task_delay_until_ms(
            &previous_wake_tick,
            blinker_delay_ms_get(&delay_remainder)
        );
    }
}
