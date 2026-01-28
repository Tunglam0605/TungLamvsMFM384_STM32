#include "tou_energy_service.h"
#include <string.h>

static uint16_t tou_minutes_of_day(const datetime_t *now)
{
  return (uint16_t)((uint16_t)now->hour * 60U + now->minute);
}

static uint8_t tou_in_window(const tou_window_t *win, uint16_t minute)
{
  if (win->start_min <= win->end_min)
  {
    return (minute >= win->start_min) && (minute < win->end_min);
  }
  /* wrap around midnight */
  return (minute >= win->start_min) || (minute < win->end_min);
}

tou_bucket_t tou_energy_service_bucket_for_time(const tou_config_t *cfg, const datetime_t *now)
{
  uint16_t minute;
  if ((cfg == NULL) || (now == NULL))
  {
    return TOU_BUCKET_LOW;
  }
  minute = tou_minutes_of_day(now);
  if (tou_in_window(&cfg->high, minute))
  {
    return TOU_BUCKET_HIGH;
  }
  if (tou_in_window(&cfg->mid, minute))
  {
    return TOU_BUCKET_MID;
  }
  return TOU_BUCKET_LOW;
}

void tou_energy_service_init(tou_energy_service_t *svc, const tou_config_t *cfg, const energy_counters_t *counters)
{
  if ((svc == NULL) || (cfg == NULL))
  {
    return;
  }
  svc->config = *cfg;
  if (counters != NULL)
  {
    svc->counters = *counters;
  }
  else
  {
    (void)memset(&svc->counters, 0, sizeof(svc->counters));
  }
  (void)memset(&svc->last_date, 0, sizeof(svc->last_date));
  svc->last_tick_ms = 0U;
  svc->day_rollover = 0U;
}

static void tou_handle_rollover(tou_energy_service_t *svc, const datetime_t *now)
{
  if ((svc->last_date.year == 0U) && (svc->last_date.month == 0U) && (svc->last_date.day == 0U))
  {
    svc->last_date = *now;
    return;
  }

  if ((now->year != svc->last_date.year) || (now->month != svc->last_date.month) || (now->day != svc->last_date.day))
  {
    if (now->year != svc->last_date.year)
    {
      svc->counters.kwh_year = 0.0f;
      svc->counters.kwh_month = 0.0f;
    }
    else if (now->month != svc->last_date.month)
    {
      svc->counters.kwh_month = 0.0f;
    }
    svc->counters.kwh_day = 0.0f;
    svc->counters.kwh_day_bucket[TOU_BUCKET_LOW] = 0.0f;
    svc->counters.kwh_day_bucket[TOU_BUCKET_MID] = 0.0f;
    svc->counters.kwh_day_bucket[TOU_BUCKET_HIGH] = 0.0f;
    svc->last_date = *now;
    svc->day_rollover = 1U;
  }
}

void tou_energy_service_update(tou_energy_service_t *svc, const meter_sample_t *sample, const datetime_t *now)
{
  if ((svc == NULL) || (sample == NULL) || (now == NULL))
  {
    return;
  }

  tou_handle_rollover(svc, now);

  if (!sample->valid)
  {
    svc->last_tick_ms = sample->tick_ms;
    return;
  }

  if (svc->last_tick_ms == 0U)
  {
    svc->last_tick_ms = sample->tick_ms;
    return;
  }

  uint32_t dt_ms = sample->tick_ms - svc->last_tick_ms;
  svc->last_tick_ms = sample->tick_ms;

  float dt_hours = ((float)dt_ms) / 3600000.0f;
  float kw = sample->kw_total;

  if (dt_hours < 0.0f)
  {
    return;
  }

  float delta_kwh = kw * dt_hours;
  svc->counters.kwh_day += delta_kwh;
  svc->counters.kwh_month += delta_kwh;
  svc->counters.kwh_year += delta_kwh;

  tou_bucket_t bucket = tou_energy_service_bucket_for_time(&svc->config, now);
  svc->counters.kwh_day_bucket[bucket] += delta_kwh;
}

void tou_energy_service_get_counters(const tou_energy_service_t *svc, energy_counters_t *counters)
{
  if ((svc == NULL) || (counters == NULL))
  {
    return;
  }
  *counters = svc->counters;
}

uint8_t tou_energy_service_day_rollover(const tou_energy_service_t *svc)
{
  if (svc == NULL)
  {
    return 0U;
  }
  return svc->day_rollover;
}

void tou_energy_service_clear_rollover(tou_energy_service_t *svc)
{
  if (svc == NULL)
  {
    return;
  }
  svc->day_rollover = 0U;
}

void tou_energy_service_set_last_date(tou_energy_service_t *svc, const datetime_t *dt)
{
  if ((svc == NULL) || (dt == NULL))
  {
    return;
  }
  svc->last_date = *dt;
}

void tou_energy_service_get_last_date(const tou_energy_service_t *svc, datetime_t *dt)
{
  if ((svc == NULL) || (dt == NULL))
  {
    return;
  }
  *dt = svc->last_date;
}

uint8_t tou_test_bucket_selection(void)
{
  tou_config_t cfg;
  datetime_t dt;
  cfg.low.start_min = 0U;
  cfg.low.end_min = 360U;
  cfg.mid.start_min = 360U;
  cfg.mid.end_min = 1080U;
  cfg.high.start_min = 1080U;
  cfg.high.end_min = 1440U;

  dt.hour = 1U; dt.minute = 0U;
  if (tou_energy_service_bucket_for_time(&cfg, &dt) != TOU_BUCKET_LOW) return 0U;
  dt.hour = 8U; dt.minute = 0U;
  if (tou_energy_service_bucket_for_time(&cfg, &dt) != TOU_BUCKET_MID) return 0U;
  dt.hour = 19U; dt.minute = 0U;
  if (tou_energy_service_bucket_for_time(&cfg, &dt) != TOU_BUCKET_HIGH) return 0U;
  return 1U;
}

uint8_t tou_test_energy_integration(void)
{
  tou_energy_service_t svc;
  tou_config_t cfg;
  meter_sample_t sample;
  datetime_t dt;

  cfg.low.start_min = 0U;
  cfg.low.end_min = 1440U;
  cfg.mid.start_min = 0U;
  cfg.mid.end_min = 0U;
  cfg.high.start_min = 0U;
  cfg.high.end_min = 0U;

  (void)memset(&sample, 0, sizeof(sample));
  (void)memset(&dt, 0, sizeof(dt));

  tou_energy_service_init(&svc, &cfg, NULL);
  sample.valid = 1U;
  sample.kw_total = 1.0f;
  sample.tick_ms = 0U;
  tou_energy_service_update(&svc, &sample, &dt);
  sample.tick_ms = 3600000U;
  tou_energy_service_update(&svc, &sample, &dt);

  return (svc.counters.kwh_day > 0.99f && svc.counters.kwh_day < 1.01f) ? 1U : 0U;
}

uint8_t tou_test_rollover_day(void)
{
  tou_energy_service_t svc;
  tou_config_t cfg;
  energy_counters_t counters;
  meter_sample_t sample;
  datetime_t dt;

  cfg.low.start_min = 0U;
  cfg.low.end_min = 1440U;
  cfg.mid.start_min = 0U;
  cfg.mid.end_min = 0U;
  cfg.high.start_min = 0U;
  cfg.high.end_min = 0U;
  (void)memset(&counters, 0, sizeof(counters));
  tou_energy_service_init(&svc, &cfg, &counters);

  (void)memset(&sample, 0, sizeof(sample));
  sample.valid = 1U;
  sample.kw_total = 1.0f;
  sample.tick_ms = 1000U;

  (void)memset(&dt, 0, sizeof(dt));
  dt.year = 2026U; dt.month = 1U; dt.day = 1U;
  tou_energy_service_update(&svc, &sample, &dt);

  dt.day = 2U;
  tou_energy_service_update(&svc, &sample, &dt);

  return (svc.day_rollover != 0U) ? 1U : 0U;
}
