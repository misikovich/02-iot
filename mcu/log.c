#include "log.h"

#include "mcc_generated_files/uart1.h"

static StaticQueue_t log_uart_queue_control;
static u8 log_uart_queue_storage[LOG_UART_QUEUE_CAPACITY * sizeof(LogEntry)];
static QueueHandle_t log_uart_queue;

volatile LogEntry log_entries[LOG_CAPACITY];
volatile u16 log_write_index;
volatile u16 log_sequence;
volatile u16 log_uart_dropped;

static const char *log_event_name(u16 event)
{
    switch ((LogEvent)event)
    {
        case LOG_EVENT_APP_START:
            return "APP_START";
        case LOG_EVENT_TASK_CREATE:
            return "TASK_CREATE";
        case LOG_EVENT_SCHEDULER_RETURNED:
            return "SCHEDULER_RETURNED";
        case LOG_EVENT_LOG_INIT:
            return "LOG_INIT";
        case LOG_EVENT_BLINKER_SET:
            return "BLINKER_SET";
        case LOG_EVENT_BLINKER_TASK_START:
            return "BLINKER_TASK_START";
        case LOG_EVENT_BLINKER_HID_INIT:
            return "BLINKER_HID_INIT";
        case LOG_EVENT_BLINKER_PHASE:
            return "BLINKER_PHASE";
        case LOG_EVENT_BLINKER_TRANSITION_REJECTED:
            return "BLINKER_TRANSITION_REJECTED";
        case LOG_EVENT_BLINKER_OUTPUT:
            return "BLINKER_OUTPUT";
    }

    return "UNKNOWN";
}

static void log_uart_write_text(const char *text)
{
    while (*text != '\0')
    {
        UART1_Write((u8)*text);
        text++;
    }
}

static void log_uart_write_u32(u32 value)
{
    char digits[10];
    u8 digit_count = 0;

    do
    {
        digits[digit_count] = (char)('0' + (value % 10U));
        digit_count++;
        value /= 10U;
    }
    while (value != 0);

    while (digit_count > 0)
    {
        digit_count--;
        UART1_Write((u8)digits[digit_count]);
    }
}

static void log_uart_write_i16(i16 value)
{
    u16 magnitude;

    if (value < 0)
    {
        UART1_Write('-');
        magnitude = (u16)(-(i32)value);
    }
    else
    {
        magnitude = (u16)value;
    }

    log_uart_write_u32(magnitude);
}

static void log_uart_write_entry(const LogEntry *entry)
{
    log_uart_write_u32(entry->timestamp_ms);
    UART1_Write(' ');
    log_uart_write_u32(entry->sequence);
    UART1_Write(' ');
    log_uart_write_text(log_event_name(entry->event));
    UART1_Write(' ');
    log_uart_write_i16(entry->arg0);
    UART1_Write(' ');
    log_uart_write_i16(entry->arg1);
    log_uart_write_text("\r\n");
}

bool log_init(void)
{
    QueueHandle_t queue;

    if (log_uart_queue != NULL)
    {
        return false;
    }

    queue = xQueueCreateStatic(
        LOG_UART_QUEUE_CAPACITY,
        sizeof(LogEntry),
        log_uart_queue_storage,
        &log_uart_queue_control
    );

    if (queue == NULL)
    {
        return false;
    }

    log_uart_queue = queue;
    return true;
}

void log_task(void *params)
{
    LogEntry entry;

    unused(params);

    if (log_uart_queue == NULL)
    {
        al_task_delete(NULL);
        return;
    }

    log_uart_write_text("ms seq event arg0 arg1\r\n");

    forever
    {
        if (xQueueReceive(log_uart_queue, &entry, portMAX_DELAY) == pdPASS)
        {
            log_uart_write_entry(&entry);
        }
    }
}

void log_write(LogEvent event, i16 arg0, i16 arg1)
{
    volatile LogEntry *entry;
    LogEntry queued_entry;
    u32 timestamp_ms = al_millis();
    u16 index;
    u16 sequence;

    al_critical_enter();

    index = log_write_index;
    sequence = (u16)(log_sequence + 1U);

    if (sequence == 0)
    {
        sequence = 1;
    }

    entry = &log_entries[index];
    entry->sequence = 0;
    entry->timestamp_ms = timestamp_ms;
    entry->event = (u16)event;
    entry->arg0 = arg0;
    entry->arg1 = arg1;
    entry->sequence = sequence;

    queued_entry.timestamp_ms = timestamp_ms;
    queued_entry.sequence = sequence;
    queued_entry.event = (u16)event;
    queued_entry.arg0 = arg0;
    queued_entry.arg1 = arg1;

    index++;

    if (index >= LOG_CAPACITY)
    {
        index = 0;
    }

    log_sequence = sequence;
    log_write_index = index;

    al_critical_exit();

    if (
        log_uart_queue != NULL &&
        xQueueSend(log_uart_queue, &queued_entry, 0) != pdPASS)
    {
        al_critical_enter();
        log_uart_dropped++;
        al_critical_exit();
    }
}
