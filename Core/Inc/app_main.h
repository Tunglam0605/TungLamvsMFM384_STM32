#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "app_config.h"
#include "app_log.h"
#include "app_state.h"
#include "rs485_phy.h"
#include "modbus_rtu.h"
#include "meter_service.h"
#include "time_service.h"
#include "ds3231_service.h"
#include "tou_energy_service.h"
#include "lcd_service.h"
#include "button_service.h"
#include "watchdog_service.h"
#include "datastore.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  rs485_phy_t rs485_phy;
  modbus_rtu_t modbus;
  modbus_iface_t modbus_iface;
  meter_service_t meter_service;
  time_service_t time_service;
  time_iface_t time_iface;
  ds3231_service_t ds3231_service;
  tou_energy_service_t tou_service;
  lcd_service_t lcd_service;
  button_service_t button_service;
  watchdog_service_t watchdog_service;
  datastore_t datastore;
  app_state_t state;
} app_context_t;

void app_main_init(void);
app_context_t *app_main_get(void);

#if APP_LOG_ENABLE
#define APP_LOG(...) app_log_printf(__VA_ARGS__)
#else
#define APP_LOG(...) do { } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H */
