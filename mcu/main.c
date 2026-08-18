/**
 * @file main.c
 * @brief Application entry point with FreeRTOS 1kHz tick scheduler.
 */

#include "al.h"

#define APP_MAIN_TASK_STACK_WORDS configMINIMAL_STACK_SIZE
#define APP_MAIN_TASK_PRIORITY    (tskIDLE_PRIORITY + 2)
#define APP_HEARTBEAT_PERIOD_MS   1000UL

static void app_main_task(void *params)
{
    unused(params);

    forever
    {
        task_hold(APP_HEARTBEAT_PERIOD_MS);
    }
}

int main(void)
{
    al_system_init();

    al_task_create(
        app_main_task,
        "main_task",
        APP_MAIN_TASK_STACK_WORDS,
        NULL,
        APP_MAIN_TASK_PRIORITY,
        NULL
    );

    al_rtos_start();

    forever
    {
    }

    return 0;
}
