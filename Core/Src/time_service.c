#include "time_service.h"
#include <string.h>

static uint8_t time_service_is_leap(uint16_t year)
{
  return ((year % 4U == 0U) && (year % 100U != 0U)) || (year % 400U == 0U);
}

static uint8_t time_service_weekday(uint16_t y, uint8_t m, uint8_t d)
{
  static const uint8_t t[] = {0U, 3U, 2U, 5U, 0U, 3U, 5U, 1U, 4U, 6U, 2U, 4U};
  if (m < 3U)
  {
    y -= 1U;
  }
  uint16_t w = (uint16_t)((y + (y / 4U) - (y / 100U) + (y / 400U) + t[m - 1U] + d) % 7U);
  if (w == 0U)
  {
    return 7U; /* Sunday */
  }
  return (uint8_t)w; /* Monday = 1 */
}

static uint32_t time_service_days_before_year(uint16_t year)
{
  uint32_t days = 0U;
  uint16_t y;
  for (y = 1970U; y < year; y++)
  {
    days += time_service_is_leap(y) ? 366U : 365U;
  }
  return days;
}

static uint16_t time_service_days_before_month(uint16_t year, uint8_t month)
{
  static const uint16_t days_by_month[] = {0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U};
  uint16_t days = days_by_month[month - 1U];
  if ((month > 2U) && time_service_is_leap(year))
  {
    days += 1U;
  }
  return days;
}

static uint32_t time_service_datetime_to_epoch(const datetime_t *dt)
{
  if (dt == NULL)
  {
    return 0U;
  }
  if (dt->year < 1970U)
  {
    return 0U;
  }
  uint32_t days = time_service_days_before_year(dt->year);
  days += time_service_days_before_month(dt->year, dt->month);
  days += (uint32_t)(dt->day - 1U);
  return (days * 86400U) + ((uint32_t)dt->hour * 3600U) + ((uint32_t)dt->minute * 60U) + dt->second;
}

void time_service_init(time_service_t *svc, RTC_HandleTypeDef *rtc)
{
  if ((svc == NULL) || (rtc == NULL))
  {
    return;
  }
  svc->rtc = rtc;
}

uint32_t time_service_now_epoch(time_service_t *svc)
{
  datetime_t dt;
  if (svc == NULL)
  {
    return 0U;
  }
  time_service_get_datetime(svc, &dt);
  return time_service_datetime_to_epoch(&dt);
}

void time_service_get_datetime(time_service_t *svc, datetime_t *dt)
{
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;
  if ((svc == NULL) || (svc->rtc == NULL) || (dt == NULL))
  {
    return;
  }

  (void)HAL_RTC_GetTime(svc->rtc, &time, RTC_FORMAT_BIN);
  (void)HAL_RTC_GetDate(svc->rtc, &date, RTC_FORMAT_BIN);

  dt->year = (uint16_t)(2000U + date.Year);
  dt->month = date.Month;
  dt->day = date.Date;
  dt->hour = time.Hours;
  dt->minute = time.Minutes;
  dt->second = time.Seconds;
  dt->weekday = date.WeekDay;
}

void time_service_set_datetime(time_service_t *svc, const datetime_t *dt)
{
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;
  if ((svc == NULL) || (svc->rtc == NULL) || (dt == NULL))
  {
    return;
  }

  (void)memset(&time, 0, sizeof(time));
  (void)memset(&date, 0, sizeof(date));

  time.Hours = dt->hour;
  time.Minutes = dt->minute;
  time.Seconds = dt->second;

  date.Year = (uint8_t)((dt->year >= 2000U) ? (dt->year - 2000U) : 0U);
  date.Month = dt->month;
  date.Date = dt->day;
  date.WeekDay = time_service_weekday(dt->year, dt->month, dt->day);

  (void)HAL_RTC_SetTime(svc->rtc, &time, RTC_FORMAT_BIN);
  (void)HAL_RTC_SetDate(svc->rtc, &date, RTC_FORMAT_BIN);
}

static void time_iface_get(void *ctx, datetime_t *dt)
{
  time_service_get_datetime((time_service_t *)ctx, dt);
}

static void time_iface_set(void *ctx, const datetime_t *dt)
{
  time_service_set_datetime((time_service_t *)ctx, dt);
}

static uint32_t time_iface_now(void *ctx)
{
  return time_service_now_epoch((time_service_t *)ctx);
}

void time_service_get_iface(time_service_t *svc, time_iface_t *iface)
{
  if ((svc == NULL) || (iface == NULL))
  {
    return;
  }
  iface->ctx = svc;
  iface->get_datetime = time_iface_get;
  iface->set_datetime = time_iface_set;
  iface->now_epoch = time_iface_now;
}
