/**
 * @file main.c
 * @brief Application entry point with FreeRTOS 1kHz tick scheduler.
 */

#include "al.h"

#define APP_MAIN_TASK_STACK_WORDS   configMINIMAL_STACK_SIZE
#define APP_MAIN_TASK_PRIORITY      AL_PRIORITY_NORMAL
#define APP_HEARTBEAT_PERIOD_MS     1000UL

static void app_main_task(void *params)
{
    (void)params;

    AlTick last_wake_time = al_tick_get();

    for (;;)
    {
        // Periodic task running with 1 kHz FreeRTOS tick rate
        al_task_delay_until_ms(&last_wake_time, APP_HEARTBEAT_PERIOD_MS);
    }
}

int main(void)
{
    // Initialize system clock and configured peripherals
    al_system_init();

    // Create the default main application task
    al_task_create(
        app_main_task,
        "main_task",
        APP_MAIN_TASK_STACK_WORDS,
        NULL,
        APP_MAIN_TASK_PRIORITY,
        NULL
    );

    // Start the FreeRTOS real-time scheduler (1kHz tick rate)
    al_rtos_start();

    // The scheduler should never return
    for (;;)
    {
    }

    return 0;
}
