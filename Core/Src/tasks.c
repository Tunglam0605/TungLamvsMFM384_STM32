#include "tasks.h"
#include "app_main.h"
#include "app_config.h"
#include "app_events.h"
#include "queues.h"
#include "cmsis_os.h"
#include <string.h>

typedef enum {
  UI_MODE_NORMAL = 0,
  UI_MODE_SET_HOUR,
  UI_MODE_SET_MIN,
  UI_MODE_SET_DAY,
  UI_MODE_SET_MONTH,
  UI_MODE_SET_YEAR
} ui_mode_t;

static void ui_increment_field(datetime_t *dt, ui_mode_t mode)
{
  if (dt == NULL)
  {
    return;
  }
  switch (mode)
  {
    case UI_MODE_SET_HOUR:
      dt->hour = (uint8_t)((dt->hour + 1U) % 24U);
      break;
    case UI_MODE_SET_MIN:
      dt->minute = (uint8_t)((dt->minute + 1U) % 60U);
      break;
    case UI_MODE_SET_DAY:
      dt->day = (uint8_t)((dt->day % 31U) + 1U);
      break;
    case UI_MODE_SET_MONTH:
      dt->month = (uint8_t)((dt->month % 12U) + 1U);
      break;
    case UI_MODE_SET_YEAR:
      dt->year = (uint16_t)((dt->year < 2099U) ? (dt->year + 1U) : 2000U);
      break;
    default:
      break;
  }
}

void tasks_rs485(void const *argument)
{
  (void)argument;
  app_context_t *app = app_main_get();
  meter_sample_t sample;

  for (;;)
  {
    if (meter_service_read(&app->meter_service, &sample))
    {
      app_state_set_sample(&app->state, &sample);
    }
    queues_publish_meter_sample(&sample);
    watchdog_service_heartbeat(&app->watchdog_service, WDG_HB_RS485);
    osDelay(METER_POLL_INTERVAL_MS);
  }
}

void tasks_tou(void const *argument)
{
  (void)argument;
  app_context_t *app = app_main_get();
  uint32_t last_save = HAL_GetTick();
  energy_counters_t counters;
  datetime_t now;
  datetime_t last_date;
  meter_sample_t last_sample;

  (void)memset(&last_sample, 0, sizeof(last_sample));

  for (;;)
  {
    uint32_t evt_id = 0U;
    uint8_t got_sample = 0U;
    if (queues_wait_event(1000U, &evt_id))
    {
      if (evt_id == APP_EVENT_METER_SAMPLE_READY)
      {
        if (queues_get_latest_meter_sample(&last_sample))
        {
          got_sample = 1U;
        }
      }
    }

    if (!got_sample)
    {
      last_sample.valid = 0U;
      last_sample.tick_ms = HAL_GetTick();
    }

    time_service_get_datetime(&app->time_service, &now);
    app_state_set_datetime(&app->state, &now);
    tou_energy_service_update(&app->tou_service, &last_sample, &now);
    tou_energy_service_get_counters(&app->tou_service, &counters);
    app_state_set_counters(&app->state, &counters);

    if ((HAL_GetTick() - last_save) >= DATASTORE_SAVE_INTERVAL_MS || tou_energy_service_day_rollover(&app->tou_service))
    {
      tou_energy_service_get_counters(&app->tou_service, &counters);
      tou_energy_service_get_last_date(&app->tou_service, &last_date);
      (void)datastore_save_counters(&app->datastore, &counters, &last_date);
      tou_energy_service_clear_rollover(&app->tou_service);
      last_save = HAL_GetTick();
    }

    watchdog_service_heartbeat(&app->watchdog_service, WDG_HB_TOU);
  }
}

void tasks_lcd(void const *argument)
{
  (void)argument;
  app_context_t *app = app_main_get();
  meter_sample_t sample;
  energy_counters_t counters;
  datetime_t now;

  (void)memset(&sample, 0, sizeof(sample));

  for (;;)
  {
    uint32_t evt_id = 0U;
    (void)queues_wait_event(LCD_REFRESH_MS, &evt_id);

    meter_sample_t temp_sample;
    if (queues_get_latest_meter_sample(&temp_sample))
    {
      sample = temp_sample;
    }
    app_state_get_counters(&app->state, &counters);
    time_service_get_datetime(&app->time_service, &now);
    app_state_set_datetime(&app->state, &now);

    lcd_service_render(&app->lcd_service, &sample, &counters, &now);
    watchdog_service_heartbeat(&app->watchdog_service, WDG_HB_LCD);
  }
}

void tasks_ui(void const *argument)
{
  (void)argument;
  app_context_t *app = app_main_get();
  ui_mode_t mode = UI_MODE_NORMAL;
  datetime_t edit_dt;
  uint8_t ignore_release = 0U;
  uint32_t last_time_tick = 0U;

  (void)memset(&edit_dt, 0, sizeof(edit_dt));

  for (;;)
  {
    uint8_t evt = button_service_update(&app->button_service, BUTTON_DEBOUNCE_MS, BUTTON_LONG_PRESS_MS);

    if (evt & BUTTON_EVENT_LONG_PRESS)
    {
      ignore_release = 1U;
      if (mode == UI_MODE_NORMAL)
      {
        time_service_get_datetime(&app->time_service, &edit_dt);
        mode = UI_MODE_SET_HOUR;
      }
      else
      {
        if (mode == UI_MODE_SET_YEAR)
        {
          time_service_set_datetime(&app->time_service, &edit_dt);
#if DS3231_SET_ON_RTC_SET
          if (ds3231_service_is_present(&app->ds3231_service))
          {
            datetime_t rtc_dt;
            time_service_get_datetime(&app->time_service, &rtc_dt);
            (void)ds3231_service_set_datetime(&app->ds3231_service, &rtc_dt);
          }
#endif
          mode = UI_MODE_NORMAL;
        }
        else
        {
          mode = (ui_mode_t)(mode + 1U);
        }
      }
    }

    if (evt & BUTTON_EVENT_RELEASED)
    {
      if (ignore_release)
      {
        ignore_release = 0U;
      }
      else
      {
        if (mode == UI_MODE_NORMAL)
        {
          lcd_service_next_page(&app->lcd_service);
        }
        else
        {
          ui_increment_field(&edit_dt, mode);
        }
      }
    }

    if ((HAL_GetTick() - last_time_tick) >= 1000U)
    {
      ds3231_service_tick(&app->ds3231_service);
      watchdog_service_heartbeat(&app->watchdog_service, WDG_HB_TIME);
      last_time_tick = HAL_GetTick();
    }

    watchdog_service_heartbeat(&app->watchdog_service, WDG_HB_UI);
    osDelay(UI_POLL_MS);
  }
}

void tasks_wdg(void const *argument)
{
  (void)argument;
  app_context_t *app = app_main_get();

  for (;;)
  {
    watchdog_service_feed_if_alive(&app->watchdog_service);
    osDelay(WDG_FEED_INTERVAL_MS);
  }
}
