/**
 * @file main.c
 * @brief Application entry point with FreeRTOS 1kHz tick scheduler.
 */

#include "al.h"
#include "blinker.h"

#define APP_DEMO_TASK_STACK_WORDS    configMINIMAL_STACK_SIZE
#define APP_DEMO_TASK_PRIORITY       (tskIDLE_PRIORITY + 1)
#define APP_BLINKER_TASK_STACK_WORDS configMINIMAL_STACK_SIZE
#define APP_BLINKER_TASK_PRIORITY    (tskIDLE_PRIORITY + 2)
#define APP_DEMO_L_STOP_MS           850UL
#define APP_DEMO_R_STOP_DELAY_MS     180UL
#define APP_DEMO_RESTART_DELAY_MS    1000UL

static void app_blinker_demo_task(void *params)
{
    unused(params);

    forever
    {
        blinker_set_l(true);
        blinker_set_r(true);
        task_hold(APP_DEMO_L_STOP_MS);

        blinker_set_l(false);
        task_hold(APP_DEMO_R_STOP_DELAY_MS);

        blinker_set_r(false);
        task_hold(APP_DEMO_RESTART_DELAY_MS);
    }
}

int main(void)
{
    al_system_init();

    al_task_create(
        blinker_task,
        "blinker_task",
        APP_BLINKER_TASK_STACK_WORDS,
        NULL,
        APP_BLINKER_TASK_PRIORITY,
        NULL
    );

    al_task_create(
        app_blinker_demo_task,
        "blinker_demo",
        APP_DEMO_TASK_STACK_WORDS,
        NULL,
        APP_DEMO_TASK_PRIORITY,
        NULL
    );

    al_rtos_start();

    forever
    {
    }

    return 0;
}
