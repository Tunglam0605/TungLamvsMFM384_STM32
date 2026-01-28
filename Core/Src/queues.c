#include "queues.h"
#include "app_config.h"
#include "app_events.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

extern osMessageQId qMeterHandle;
extern osMutexId mMeterHandle;

static QueueHandle_t g_meterStructQueue = NULL;
#if !USE_QUEUE_STRUCT
static meter_sample_t g_sharedSample;
#endif

void queues_init(void)
{
#if USE_QUEUE_STRUCT
  g_meterStructQueue = xQueueCreate(APP_METER_STRUCT_QUEUE_LEN, sizeof(meter_sample_t));
#else
  (void)memset(&g_sharedSample, 0, sizeof(g_sharedSample));
#endif
}

QueueHandle_t queues_get_meter_struct(void)
{
  return g_meterStructQueue;
}

osMutexId queues_get_meter_mutex(void)
{
  return mMeterHandle;
}

#if !USE_QUEUE_STRUCT
static void queues_copy_sample(meter_sample_t *dst, const meter_sample_t *src)
{
  taskENTER_CRITICAL();
  (void)memcpy(dst, src, sizeof(*src));
  taskEXIT_CRITICAL();
}
#endif

void queues_publish_meter_sample(const meter_sample_t *sample)
{
  if (sample == NULL)
  {
    return;
  }
#if USE_QUEUE_STRUCT
  if (g_meterStructQueue != NULL)
  {
#if APP_METER_STRUCT_QUEUE_LEN == 1
    (void)xQueueOverwrite(g_meterStructQueue, sample);
#else
    (void)xQueueSend(g_meterStructQueue, sample, 0);
#endif
  }
#else
  queues_copy_sample(&g_sharedSample, sample);
#endif

  queues_signal_event(APP_EVENT_METER_SAMPLE_READY, APP_METER_EVENT_CONSUMERS);
}

uint8_t queues_get_latest_meter_sample(meter_sample_t *sample)
{
  if (sample == NULL)
  {
    return 0U;
  }
#if USE_QUEUE_STRUCT
  if (g_meterStructQueue == NULL)
  {
    return 0U;
  }
  if (xQueuePeek(g_meterStructQueue, sample, 0) == pdTRUE)
  {
    return 1U;
  }
  return 0U;
#else
  queues_copy_sample(sample, &g_sharedSample);
  return 1U;
#endif
}

uint8_t queues_wait_event(uint32_t timeout_ms, uint32_t *event_id)
{
  osEvent evt = osMessageGet(qMeterHandle, timeout_ms);
  if (evt.status == osEventMessage)
  {
    if (event_id != NULL)
    {
      *event_id = evt.value.v;
    }
    return 1U;
  }
  return 0U;
}

void queues_signal_event(uint8_t event_id, uint8_t repeat)
{
  uint8_t i;
  for (i = 0U; i < repeat; i++)
  {
    (void)osMessagePut(qMeterHandle, event_id, 0U);
  }
}
