#ifndef APP_STATE_H
#define APP_STATE_H

#include "meter_types.h"
#include "tou_types.h"
#include "time_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  meter_sample_t sample;
  energy_counters_t counters;
  datetime_t datetime;
} app_state_t;

void app_state_init(app_state_t *state);
void app_state_set_sample(app_state_t *state, const meter_sample_t *sample);
void app_state_get_sample(const app_state_t *state, meter_sample_t *sample);
void app_state_set_counters(app_state_t *state, const energy_counters_t *counters);
void app_state_get_counters(const app_state_t *state, energy_counters_t *counters);
void app_state_set_datetime(app_state_t *state, const datetime_t *dt);
void app_state_get_datetime(const app_state_t *state, datetime_t *dt);

#ifdef __cplusplus
}
#endif

#endif /* APP_STATE_H */
