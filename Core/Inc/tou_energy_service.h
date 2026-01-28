#ifndef TOU_ENERGY_SERVICE_H
#define TOU_ENERGY_SERVICE_H

#include "tou_types.h"
#include "meter_types.h"
#include "time_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  tou_config_t config;
  energy_counters_t counters;
  datetime_t last_date;
  uint32_t last_tick_ms;
  uint8_t day_rollover;
} tou_energy_service_t;

void tou_energy_service_init(tou_energy_service_t *svc, const tou_config_t *cfg, const energy_counters_t *counters);
void tou_energy_service_update(tou_energy_service_t *svc, const meter_sample_t *sample, const datetime_t *now);
void tou_energy_service_get_counters(const tou_energy_service_t *svc, energy_counters_t *counters);
uint8_t tou_energy_service_day_rollover(const tou_energy_service_t *svc);
void tou_energy_service_clear_rollover(tou_energy_service_t *svc);
void tou_energy_service_set_last_date(tou_energy_service_t *svc, const datetime_t *dt);
void tou_energy_service_get_last_date(const tou_energy_service_t *svc, datetime_t *dt);

tou_bucket_t tou_energy_service_bucket_for_time(const tou_config_t *cfg, const datetime_t *now);

/* Unit-test-like helpers */
uint8_t tou_test_bucket_selection(void);
uint8_t tou_test_energy_integration(void);
uint8_t tou_test_rollover_day(void);

#ifdef __cplusplus
}
#endif

#endif /* TOU_ENERGY_SERVICE_H */
