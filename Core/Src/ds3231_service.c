#include "ds3231_service.h"
#include "app_config.h"
#include <string.h>

#define DS3231_REG_TIME 0x00U

static uint8_t ds3231_bcd_to_bin(uint8_t val)
{
  return (uint8_t)((val >> 4) * 10U + (val & 0x0FU));
}

static uint8_t ds3231_bin_to_bcd(uint8_t val)
{
  return (uint8_t)(((val / 10U) << 4) | (val % 10U));
}

static void ds3231_lock(ds3231_service_t *svc)
{
  if ((svc != NULL) && (svc->mutex != NULL))
  {
    (void)osMutexWait(svc->mutex, RS485_MUTEX_TIMEOUT_MS);
  }
}

static void ds3231_unlock(ds3231_service_t *svc)
{
  if ((svc != NULL) && (svc->mutex != NULL))
  {
    (void)osMutexRelease(svc->mutex);
  }
}

static uint8_t ds3231_is_leap(uint16_t year)
{
  return ((year % 4U == 0U) && (year % 100U != 0U)) || (year % 400U == 0U);
}

static uint32_t ds3231_days_before_year(uint16_t year)
{
  uint32_t days = 0U;
  uint16_t y;
  for (y = 1970U; y < year; y++)
  {
    days += ds3231_is_leap(y) ? 366U : 365U;
  }
  return days;
}

static uint16_t ds3231_days_before_month(uint16_t year, uint8_t month)
{
  static const uint16_t days_by_month[] = {0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U};
  uint16_t days = days_by_month[month - 1U];
  if ((month > 2U) && ds3231_is_leap(year))
  {
    days += 1U;
  }
  return days;
}

static uint32_t ds3231_datetime_to_epoch(const datetime_t *dt)
{
  if (dt == NULL)
  {
    return 0U;
  }
  if (dt->year < 1970U)
  {
    return 0U;
  }
  uint32_t days = ds3231_days_before_year(dt->year);
  days += ds3231_days_before_month(dt->year, dt->month);
  days += (uint32_t)(dt->day - 1U);
  return (days * 86400U) + ((uint32_t)dt->hour * 3600U) + ((uint32_t)dt->minute * 60U) + dt->second;
}

void ds3231_service_init(ds3231_service_t *svc, I2C_HandleTypeDef *i2c, uint8_t address, const time_iface_t *time_iface, osMutexId mutex)
{
  if ((svc == NULL) || (i2c == NULL))
  {
    return;
  }
  svc->i2c = i2c;
  svc->address = address;
  svc->time_iface = time_iface;
  svc->mutex = mutex;
  svc->last_sync_tick = 0U;
  svc->present = 0U;
}

uint8_t ds3231_service_is_present(ds3231_service_t *svc)
{
  if ((svc == NULL) || (svc->i2c == NULL))
  {
    return 0U;
  }
  ds3231_lock(svc);
  HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(svc->i2c, (uint16_t)(svc->address << 1), 2U, 10U);
  ds3231_unlock(svc);
  svc->present = (status == HAL_OK) ? 1U : 0U;
  return svc->present;
}

uint8_t ds3231_service_get_datetime(ds3231_service_t *svc, datetime_t *dt)
{
  uint8_t buf[7];
  if ((svc == NULL) || (svc->i2c == NULL) || (dt == NULL))
  {
    return 0U;
  }
  ds3231_lock(svc);
  HAL_StatusTypeDef status = HAL_I2C_Mem_Read(svc->i2c, (uint16_t)(svc->address << 1), DS3231_REG_TIME, I2C_MEMADD_SIZE_8BIT, buf, sizeof(buf), 100U);
  ds3231_unlock(svc);
  if (status != HAL_OK)
  {
    return 0U;
  }

  dt->second = ds3231_bcd_to_bin((uint8_t)(buf[0] & 0x7FU));
  dt->minute = ds3231_bcd_to_bin((uint8_t)(buf[1] & 0x7FU));
  dt->hour = ds3231_bcd_to_bin((uint8_t)(buf[2] & 0x3FU));
  dt->weekday = ds3231_bcd_to_bin((uint8_t)(buf[3] & 0x07U));
  dt->day = ds3231_bcd_to_bin((uint8_t)(buf[4] & 0x3FU));
  dt->month = ds3231_bcd_to_bin((uint8_t)(buf[5] & 0x1FU));
  dt->year = (uint16_t)(2000U + ds3231_bcd_to_bin(buf[6]));
  return 1U;
}

uint8_t ds3231_service_set_datetime(ds3231_service_t *svc, const datetime_t *dt)
{
  uint8_t buf[7];
  if ((svc == NULL) || (svc->i2c == NULL) || (dt == NULL))
  {
    return 0U;
  }
  buf[0] = ds3231_bin_to_bcd(dt->second);
  buf[1] = ds3231_bin_to_bcd(dt->minute);
  buf[2] = ds3231_bin_to_bcd(dt->hour);
  buf[3] = ds3231_bin_to_bcd(dt->weekday);
  buf[4] = ds3231_bin_to_bcd(dt->day);
  buf[5] = ds3231_bin_to_bcd(dt->month);
  buf[6] = ds3231_bin_to_bcd((uint8_t)(dt->year - 2000U));

  ds3231_lock(svc);
  HAL_StatusTypeDef status = HAL_I2C_Mem_Write(svc->i2c, (uint16_t)(svc->address << 1), DS3231_REG_TIME, I2C_MEMADD_SIZE_8BIT, buf, sizeof(buf), 100U);
  ds3231_unlock(svc);
  return (status == HAL_OK) ? 1U : 0U;
}

void ds3231_service_tick(ds3231_service_t *svc)
{
  datetime_t ds_dt;
  datetime_t rtc_dt;
  uint32_t now_tick;
  if ((svc == NULL) || (svc->time_iface == NULL) || (svc->time_iface->get_datetime == NULL) || (svc->time_iface->set_datetime == NULL))
  {
    return;
  }

  now_tick = HAL_GetTick();
  if ((now_tick - svc->last_sync_tick) < DS3231_SYNC_INTERVAL_MS)
  {
    return;
  }
  svc->last_sync_tick = now_tick;

  if (!ds3231_service_is_present(svc))
  {
    return;
  }

  if (!ds3231_service_get_datetime(svc, &ds_dt))
  {
    return;
  }

  svc->time_iface->get_datetime(svc->time_iface->ctx, &rtc_dt);
  uint32_t ds_epoch = ds3231_datetime_to_epoch(&ds_dt);
  uint32_t rtc_epoch = ds3231_datetime_to_epoch(&rtc_dt);
  uint32_t diff = (ds_epoch > rtc_epoch) ? (ds_epoch - rtc_epoch) : (rtc_epoch - ds_epoch);

  if (diff > DS3231_SYNC_THRESHOLD_SEC)
  {
#if DS3231_PREFER_DS3231
    svc->time_iface->set_datetime(svc->time_iface->ctx, &ds_dt);
#else
    (void)ds3231_service_set_datetime(svc, &rtc_dt);
#endif
  }
}
