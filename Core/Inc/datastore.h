#ifndef DATASTORE_H
#define DATASTORE_H

#include "stm32f1xx_hal.h"
#include "tou_types.h"
#include "time_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t base_addr;
  uint32_t page_size;
} datastore_t;

void datastore_init(datastore_t *ds);
uint8_t datastore_load_counters(datastore_t *ds, energy_counters_t *counters, datetime_t *last_date);
uint8_t datastore_save_counters(datastore_t *ds, const energy_counters_t *counters, const datetime_t *last_date);

#ifdef __cplusplus
}
#endif

#endif /* DATASTORE_H */
