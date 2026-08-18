#if !defined(LOG_H)
#define LOG_H

#include "al.h"

#if !defined(LOG_CAPACITY)
#define LOG_CAPACITY 64U
#endif

#if !defined(LOG_UART_QUEUE_CAPACITY)
#define LOG_UART_QUEUE_CAPACITY 32U
#endif

#if LOG_CAPACITY == 0U || LOG_CAPACITY > 65535U
#error "LOG_CAPACITY must be between 1 and 65535"
#endif

#if LOG_UART_QUEUE_CAPACITY == 0U || LOG_UART_QUEUE_CAPACITY > 255U
#error "LOG_UART_QUEUE_CAPACITY must be between 1 and 255"
#endif

typedef enum
{
    LOG_EVENT_APP_START = 1,
    LOG_EVENT_TASK_CREATE,
    LOG_EVENT_SCHEDULER_RETURNED,
    LOG_EVENT_LOG_INIT,

    LOG_EVENT_BLINKER_SET = 10,
    LOG_EVENT_BLINKER_TASK_START,
    LOG_EVENT_BLINKER_HID_INIT,
    LOG_EVENT_BLINKER_PHASE,
    LOG_EVENT_BLINKER_TRANSITION_REJECTED,
    LOG_EVENT_BLINKER_OUTPUT
} LogEvent;

typedef enum
{
    LOG_TASK_LOG = 1,
    LOG_TASK_BLINKER,
    LOG_TASK_DEMO
} LogTask;

typedef enum
{
    LOG_BLINKER_L = 0,
    LOG_BLINKER_R
} LogBlinkerSide;

typedef enum
{
    LOG_BLINKER_PHASE_DISABLED = 0,
    LOG_BLINKER_PHASE_RISING,
    LOG_BLINKER_PHASE_FADING,
    LOG_BLINKER_PHASE_GAP,
    LOG_BLINKER_PHASE_STOPPING
} LogBlinkerPhase;

typedef struct
{
    u32 timestamp_ms;
    u16 sequence;
    u16 event;
    i16 arg0;
    i16 arg1;
} LogEntry;

/**
 * Debugger-visible circular log. Watch log_entries, log_write_index, and
 * log_sequence in MPLAB. The newest complete entry has the highest sequence.
 *
 * Event arguments:
 * - LOG_EVENT_TASK_CREATE: task, xTaskCreate result
 * - LOG_EVENT_BLINKER_SET: side, enabled
 * - LOG_EVENT_BLINKER_HID_INIT: success, LED count
 * - LOG_EVENT_BLINKER_PHASE: side, phase
 * - LOG_EVENT_BLINKER_TRANSITION_REJECTED: side, requested phase
 * - LOG_EVENT_BLINKER_OUTPUT: side, normalized output
 */
extern volatile LogEntry log_entries[LOG_CAPACITY];
extern volatile u16 log_write_index;
extern volatile u16 log_sequence;
extern volatile u16 log_uart_dropped;

/**
 * @brief Initialize the UART log queue after hardware initialization.
 */
bool log_init(void);

/**
 * @brief UART1 log writer task.
 */
void log_task(void *params);

void log_write(LogEvent event, i16 arg0, i16 arg1);

#endif // LOG_H
