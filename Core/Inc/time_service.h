#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include "stm32f1xx_hal.h"
#include "time_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  RTC_HandleTypeDef *rtc;
} time_service_t;

typedef struct {
  void *ctx;
  void (*get_datetime)(void *ctx, datetime_t *dt);
  void (*set_datetime)(void *ctx, const datetime_t *dt);
  uint32_t (*now_epoch)(void *ctx);
} time_iface_t;

void time_service_init(time_service_t *svc, RTC_HandleTypeDef *rtc);
uint32_t time_service_now_epoch(time_service_t *svc);
void time_service_get_datetime(time_service_t *svc, datetime_t *dt);
void time_service_set_datetime(time_service_t *svc, const datetime_t *dt);
void time_service_get_iface(time_service_t *svc, time_iface_t *iface);

#ifdef __cplusplus
}
#endif

#endif /* TIME_SERVICE_H */
