#ifndef WATCHDOG_SERVICE_H
#define WATCHDOG_SERVICE_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  WDG_HB_RS485 = 0,
  WDG_HB_TOU = 1,
  WDG_HB_LCD = 2,
  WDG_HB_UI = 3,
  WDG_HB_TIME = 4,
  WDG_HB_MAX
} wdg_heartbeat_id_t;

typedef struct {
  IWDG_HandleTypeDef *iwdg;
  uint32_t timeout_ms;
  uint32_t last_seen[WDG_HB_MAX];
} watchdog_service_t;

void watchdog_service_init(watchdog_service_t *svc, IWDG_HandleTypeDef *iwdg, uint32_t timeout_ms);
void watchdog_service_heartbeat(watchdog_service_t *svc, wdg_heartbeat_id_t id);
uint8_t watchdog_service_all_alive(watchdog_service_t *svc);
void watchdog_service_feed_if_alive(watchdog_service_t *svc);

#ifdef __cplusplus
}
#endif

#endif /* WATCHDOG_SERVICE_H */
