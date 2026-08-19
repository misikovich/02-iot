// !!!
// NEVER EVER DELETE THIS FILE
// ALWAYS USE THE ABSTRACTION DEFINITIONS FROM THIS FILE
// !!!

#if !defined(AL_H)
#define AL_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "mcc_generated_files/system.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

#define mst(ms)          pdMS_TO_TICKS((ms))                             // ms to ticks
#define task_hold(ms)    vTaskDelay(mst((ms)))                           // vtaskdelay but looks better
#define func_hold(f, ms) do { (f); task_hold((ms)); } while(0)          // executes f then holds for ms
#define unused(x)        (void)(x)
#define forever          for(;;)
#define len(arr) (sizeof((arr)) / sizeof((arr)[0]))

#ifndef min
#define min(a, b)        ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)        ((a) > (b) ? (a) : (b))
#endif

#ifndef clamp
#define clamp(v, lo, hi) min(max((v), (lo)), (hi))
#endif

#ifndef abs
#define abs(v)           ((v) < 0 ? -(v) : (v))
#endif

#define AL_PWM_MAX       3454

/**
 * @brief Maps/interpolates an 8-bit value (0..255) to a full 16-bit range (0..65535).
 */
static inline u16 u16_lerp8(u8 val)
{
    return (u16)(((u16)val << 8) | (u16)val);
}

/**
 * @brief Maps/interpolates an 8-bit value (0..255) to the hardware PWM resolution (0..AL_PWM_MAX).
 */
static inline u16 pwm_lerp8(u8 v)
{
    return (u16)(((u32)v * AL_PWM_MAX) / 255);
}

/* -------------------------------------------------------------------------- */
/* RTOS & Hardware Abstraction Interfaces                                     */
/* -------------------------------------------------------------------------- */

/**
 * @brief Get system uptime in milliseconds.
 */
static inline u32 al_millis(void)
{
    return (u32)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/**
 * @brief Initialize all hardware peripherals configured via MCC.
 */
static inline void al_system_init(void)
{
    SYSTEM_Initialize();
}

/**
 * @brief Start the FreeRTOS real-time scheduler.
 */
static inline void al_rtos_start(void)
{
    vTaskStartScheduler();
}

/**
 * @brief Dynamically create a new task.
 */
static inline BaseType_t al_task_create(
    TaskFunction_t task_fn,
    const char *task_name,
    u16 stack_depth_words,
    void *params,
    UBaseType_t priority,
    TaskHandle_t *out_handle)
{
    return xTaskCreate(task_fn, task_name, stack_depth_words, params, priority, out_handle);
}

/**
 * @brief Delay calling task until an incremented tick count.
 */
static inline void al_task_delay_until_ms(TickType_t *previous_wake_tick, u32 increment_ms)
{
    vTaskDelayUntil(previous_wake_tick, mst(increment_ms));
}

/**
 * @brief Delete a task. Pass NULL to delete calling task.
 */
static inline void al_task_delete(TaskHandle_t task_handle)
{
    vTaskDelete(task_handle);
}

/**
 * @brief Yield execution to another ready task.
 */
static inline void al_task_yield(void)
{
    taskYIELD();
}

/**
 * @brief Enter critical section.
 */
static inline void al_critical_enter(void)
{
    taskENTER_CRITICAL();
}

/**
 * @brief Exit critical section.
 */
static inline void al_critical_exit(void)
{
    taskEXIT_CRITICAL();
}

#endif // AL_H
