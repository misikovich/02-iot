/**
 * @file al.h
 * @brief Abstraction Layer (AL) and FreeRTOS wrapper interfaces for MCU firmware.
 */

#ifndef AL_H
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

/* -------------------------------------------------------------------------- */
/* Constants and Configuration Macros                                         */
/* -------------------------------------------------------------------------- */

#define AL_TICK_RATE_HZ             configTICK_RATE_HZ
#define AL_TICK_PERIOD_MS           portTICK_PERIOD_MS
#define AL_CPU_CLOCK_HZ             configCPU_CLOCK_HZ
#define AL_MINIMAL_STACK_SIZE       configMINIMAL_STACK_SIZE
#define AL_MAX_PRIORITIES           configMAX_PRIORITIES

#define AL_PRIORITY_IDLE            ( ( UBaseType_t ) tskIDLE_PRIORITY )
#define AL_PRIORITY_LOW             ( ( UBaseType_t ) ( tskIDLE_PRIORITY + 1 ) )
#define AL_PRIORITY_NORMAL          ( ( UBaseType_t ) ( tskIDLE_PRIORITY + 2 ) )
#define AL_PRIORITY_HIGH            ( ( UBaseType_t ) ( tskIDLE_PRIORITY + 3 ) )
#define AL_PRIORITY_CRITICAL        ( ( UBaseType_t ) ( configMAX_PRIORITIES - 1 ) )

#define AL_OK                       pdPASS
#define AL_FAIL                     pdFAIL

/* -------------------------------------------------------------------------- */
/* Type Definitions                                                           */
/* -------------------------------------------------------------------------- */

typedef TaskHandle_t        AlTaskHandle;
typedef QueueHandle_t       AlQueueHandle;
typedef SemaphoreHandle_t   AlSemaphoreHandle;
typedef TimerHandle_t       AlTimerHandle;
typedef EventGroupHandle_t  AlEventGroupHandle;
typedef TickType_t          AlTick;
typedef BaseType_t          AlStatus;
typedef TaskFunction_t      AlTaskFunction;

/* -------------------------------------------------------------------------- */
/* System & RTOS Lifecycle API                                                */
/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */
/* Timing & Delay API                                                         */
/* -------------------------------------------------------------------------- */

/**
 * @brief Convert milliseconds to RTOS ticks.
 */
static inline AlTick al_ms_to_ticks(uint32_t ms)
{
    return ( AlTick ) pdMS_TO_TICKS( ms );
}

/**
 * @brief Convert RTOS ticks to milliseconds.
 */
static inline uint32_t al_ticks_to_ms(AlTick ticks)
{
    return ( uint32_t ) ( ( uint32_t ) ticks * ( uint32_t ) portTICK_PERIOD_MS );
}

/**
 * @brief Delay calling task for specified milliseconds.
 */
static inline void al_task_delay_ms(uint32_t ms)
{
    vTaskDelay( pdMS_TO_TICKS( ms ) );
}

/**
 * @brief Delay calling task until an incremented tick count.
 */
static inline void al_task_delay_until_ms(AlTick *previous_wake_tick, uint32_t increment_ms)
{
    vTaskDelayUntil( previous_wake_tick, pdMS_TO_TICKS( increment_ms ) );
}

/**
 * @brief Get the current system uptime in ticks (1 tick = 1 ms at 1kHz).
 */
static inline AlTick al_tick_get(void)
{
    return xTaskGetTickCount();
}

/**
 * @brief Get the current system uptime in milliseconds.
 */
static inline uint32_t al_time_ms_get(void)
{
    return al_ticks_to_ms( xTaskGetTickCount() );
}

/* -------------------------------------------------------------------------- */
/* Task Management API                                                        */
/* -------------------------------------------------------------------------- */

/**
 * @brief Dynamically create a new task.
 */
static inline AlStatus al_task_create(
    AlTaskFunction task_fn,
    const char *task_name,
    uint16_t stack_depth_words,
    void *params,
    UBaseType_t priority,
    AlTaskHandle *out_handle)
{
    return xTaskCreate( task_fn, task_name, stack_depth_words, params, priority, out_handle );
}

/**
 * @brief Delete a task. Pass NULL to delete the calling task.
 */
static inline void al_task_delete(AlTaskHandle task_handle)
{
    vTaskDelete( task_handle );
}

/**
 * @brief Yield execution to another ready task of equal/higher priority.
 */
static inline void al_task_yield(void)
{
    taskYIELD();
}

/* -------------------------------------------------------------------------- */
/* Critical Section API                                                       */
/* -------------------------------------------------------------------------- */

/**
 * @brief Enter critical section (disables kernel interrupts).
 */
static inline void al_critical_enter(void)
{
    taskENTER_CRITICAL();
}

/**
 * @brief Exit critical section (restores interrupt state).
 */
static inline void al_critical_exit(void)
{
    taskEXIT_CRITICAL();
}

#endif /* AL_H */
