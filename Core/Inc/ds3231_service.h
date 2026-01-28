#ifndef DS3231_SERVICE_H
#define DS3231_SERVICE_H

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "time_types.h"
#include "time_service.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  I2C_HandleTypeDef *i2c;
  uint8_t address;
  const time_iface_t *time_iface;
  osMutexId mutex;
  uint32_t last_sync_tick;
  uint8_t present;
} ds3231_service_t;

void ds3231_service_init(ds3231_service_t *svc, I2C_HandleTypeDef *i2c, uint8_t address, const time_iface_t *time_iface, osMutexId mutex);
uint8_t ds3231_service_is_present(ds3231_service_t *svc);
uint8_t ds3231_service_get_datetime(ds3231_service_t *svc, datetime_t *dt);
uint8_t ds3231_service_set_datetime(ds3231_service_t *svc, const datetime_t *dt);
void ds3231_service_tick(ds3231_service_t *svc);

#ifdef __cplusplus
}
#endif

#endif /* DS3231_SERVICE_H */
