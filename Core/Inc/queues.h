#ifndef QUEUES_H
#define QUEUES_H

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "meter_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void queues_init(void);
QueueHandle_t queues_get_meter_struct(void);
osMutexId queues_get_meter_mutex(void);

void queues_publish_meter_sample(const meter_sample_t *sample);
uint8_t queues_get_latest_meter_sample(meter_sample_t *sample);
uint8_t queues_wait_event(uint32_t timeout_ms, uint32_t *event_id);
void queues_signal_event(uint8_t event_id, uint8_t repeat);

#ifdef __cplusplus
}
#endif

#endif /* QUEUES_H */
