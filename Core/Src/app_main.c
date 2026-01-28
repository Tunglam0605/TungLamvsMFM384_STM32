#include "app_main.h"
#include "queues.h"
#include "meter_register_map.h"
#include "usart.h"
#include "i2c.h"
#include "rtc.h"
#include "iwdg.h"
#include <string.h>

static app_context_t g_app;

static void app_load_default_tou(tou_config_t *cfg)
{
  if (cfg == NULL)
  {
    return;
  }
  cfg->low.start_min = TOU_LOW_START_MIN;
  cfg->low.end_min = TOU_LOW_END_MIN;
  cfg->mid.start_min = TOU_MID_START_MIN;
  cfg->mid.end_min = TOU_MID_END_MIN;
  cfg->high.start_min = TOU_HIGH_START_MIN;
  cfg->high.end_min = TOU_HIGH_END_MIN;
}

app_context_t *app_main_get(void)
{
  return &g_app;
}

void app_main_init(void)
{
  rs485_phy_config_t rs_cfg;
  modbus_rtu_config_t mb_cfg;
  tou_config_t tou_cfg;
  energy_counters_t counters;
  datetime_t stored_date;

  app_state_init(&g_app.state);
  app_log_init();

  queues_init();

  datastore_init(&g_app.datastore);
  if (!datastore_load_counters(&g_app.datastore, &counters, &stored_date))
  {
    (void)memset(&counters, 0, sizeof(counters));
    (void)memset(&stored_date, 0, sizeof(stored_date));
  }
  app_load_default_tou(&tou_cfg);

  tou_energy_service_init(&g_app.tou_service, &tou_cfg, &counters);
  if (stored_date.year != 0U)
  {
    tou_energy_service_set_last_date(&g_app.tou_service, &stored_date);
  }
  app_state_set_counters(&g_app.state, &counters);

#if APP_TEST_ENABLE
  APP_LOG("TOU test bucket: %u\r\n", tou_test_bucket_selection());
  APP_LOG("TOU test integrate: %u\r\n", tou_test_energy_integration());
  APP_LOG("TOU test rollover: %u\r\n", tou_test_rollover_day());
#endif

  time_service_init(&g_app.time_service, &hrtc);
  time_service_get_iface(&g_app.time_service, &g_app.time_iface);
  ds3231_service_init(&g_app.ds3231_service, &APP_I2C_HANDLE, DS3231_I2C_ADDRESS, &g_app.time_iface, queues_get_meter_mutex());

  lcd_service_init(&g_app.lcd_service, &APP_I2C_HANDLE, LCD_I2C_ADDRESS, queues_get_meter_mutex());
  button_service_init(&g_app.button_service, BUTTON_GPIO_Port, BUTTON_Pin, BUTTON_ACTIVE_LEVEL);

  rs_cfg.uart = &RS485_UART_HANDLE;
  rs_cfg.de_port = RS485_DE_GPIO_Port;
  rs_cfg.de_pin = RS485_DE_Pin;
  rs_cfg.re_port = RS485_RE_GPIO_Port;
  rs_cfg.re_pin = RS485_RE_Pin;
  rs_cfg.de_active_high = RS485_DE_ACTIVE_HIGH;
  rs_cfg.re_active_high = RS485_RE_ACTIVE_HIGH;
  rs_cfg.dir_settle_ms = RS485_DIR_SETTLE_MS;
  rs_cfg.mutex = queues_get_meter_mutex();
  rs485_phy_init(&g_app.rs485_phy, &rs_cfg);

  mb_cfg.slave_id = MODBUS_SLAVE_ID;
  mb_cfg.baudrate = MODBUS_BAUDRATE;
  mb_cfg.parity = MODBUS_PARITY;
  mb_cfg.stop_bits = MODBUS_STOPBITS;
  mb_cfg.timeout_ms = MODBUS_TIMEOUT_MS;
  mb_cfg.retries = MODBUS_RETRIES;
  mb_cfg.reg_addr_offset = MODBUS_REG_ADDR_OFFSET;
  modbus_rtu_init(&g_app.modbus, &g_app.rs485_phy, &mb_cfg);

  modbus_rtu_get_iface(&g_app.modbus, &g_app.modbus_iface);
  meter_service_init(&g_app.meter_service, &g_app.modbus_iface, &g_meter_register_map);

  watchdog_service_init(&g_app.watchdog_service, &hiwdg, HEARTBEAT_TIMEOUT_MS);
}
