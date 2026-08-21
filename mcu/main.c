/**
 * @file main.c
 * @brief Application entry point with FreeRTOS 1kHz tick scheduler.
 */

#include "al.h"
#include "blinker.h"
#include "log.h"

#define APP_DEMO_TASK_STACK_WORDS    configMINIMAL_STACK_SIZE
#define APP_DEMO_TASK_PRIORITY       (tskIDLE_PRIORITY + 1)
#define APP_BLINKER_TASK_STACK_WORDS configMINIMAL_STACK_SIZE
#define APP_BLINKER_TASK_PRIORITY    (tskIDLE_PRIORITY + 2)
#define APP_LOG_TASK_STACK_WORDS     configMINIMAL_STACK_SIZE
#define APP_LOG_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)
#define APP_DEMO_L_STOP_MS           850UL
#define APP_DEMO_R_STOP_DELAY_MS     180UL
#define APP_DEMO_RESTART_DELAY_MS    1000UL

static void app_blinker_demo_task(void *params)
{
    drop(params);

    forever;

    blinker_set_r(true);

    forever;

    forever {
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
    BaseType_t task_result;
    bool log_ready;

    al_system_init();
    log_ready = log_init();
    log_write(LOG_EVENT_LOG_INIT, log_ready, LOG_UART_QUEUE_CAPACITY);
    log_write(LOG_EVENT_APP_START, 0, 0);

    if (log_ready) {
        task_result = al_task_create(
            log_task,
            "log_task",
            APP_LOG_TASK_STACK_WORDS,
            NULL,
            APP_LOG_TASK_PRIORITY,
            NULL
        );
        log_write(LOG_EVENT_TASK_CREATE, LOG_TASK_LOG, (i16)task_result);
    }

    task_result = al_task_create(
        blinker_task,
        "blinker_task",
        APP_BLINKER_TASK_STACK_WORDS,
        NULL,
        APP_BLINKER_TASK_PRIORITY,
        NULL
    );
    log_write(LOG_EVENT_TASK_CREATE, LOG_TASK_BLINKER, (i16)task_result);

    task_result = al_task_create(
        app_blinker_demo_task,
        "blinker_demo",
        APP_DEMO_TASK_STACK_WORDS,
        NULL,
        APP_DEMO_TASK_PRIORITY,
        NULL
    );
    log_write(LOG_EVENT_TASK_CREATE, LOG_TASK_DEMO, (i16)task_result);

    al_rtos_start();
    log_write(LOG_EVENT_SCHEDULER_RETURNED, 0, 0);

    forever;

    return 0;
}
