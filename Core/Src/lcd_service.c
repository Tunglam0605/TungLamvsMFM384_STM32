#include "lcd_service.h"
#include "app_config.h"
#include <stdio.h>
#include <string.h>

#define LCD_RS 0x01U
#define LCD_RW 0x02U
#define LCD_EN 0x04U
#define LCD_BL 0x08U

static void lcd_lock(lcd_service_t *lcd)
{
  if ((lcd != NULL) && (lcd->mutex != NULL))
  {
    (void)osMutexWait(lcd->mutex, RS485_MUTEX_TIMEOUT_MS);
  }
}

static void lcd_unlock(lcd_service_t *lcd)
{
  if ((lcd != NULL) && (lcd->mutex != NULL))
  {
    (void)osMutexRelease(lcd->mutex);
  }
}

static void lcd_write_i2c(lcd_service_t *lcd, uint8_t data)
{
  uint8_t buf = data | (lcd->backlight ? LCD_BL : 0U);
  (void)HAL_I2C_Master_Transmit(lcd->i2c, (uint16_t)(lcd->address << 1), &buf, 1U, 100U);
}

static void lcd_pulse_enable(lcd_service_t *lcd, uint8_t data)
{
  lcd_write_i2c(lcd, data | LCD_EN);
  HAL_Delay(1);
  lcd_write_i2c(lcd, data & (uint8_t)~LCD_EN);
}

static void lcd_write4bits(lcd_service_t *lcd, uint8_t data)
{
  lcd_write_i2c(lcd, data);
  lcd_pulse_enable(lcd, data);
}

static void lcd_send(lcd_service_t *lcd, uint8_t value, uint8_t mode)
{
  uint8_t high = (uint8_t)(value & 0xF0U);
  uint8_t low = (uint8_t)((value << 4) & 0xF0U);
  lcd_write4bits(lcd, (uint8_t)(high | mode));
  lcd_write4bits(lcd, (uint8_t)(low | mode));
}

static void lcd_command(lcd_service_t *lcd, uint8_t cmd)
{
  lcd_send(lcd, cmd, 0U);
  HAL_Delay(2);
}

static void lcd_write_char(lcd_service_t *lcd, char c)
{
  lcd_send(lcd, (uint8_t)c, LCD_RS);
}

static void lcd_set_cursor(lcd_service_t *lcd, uint8_t row, uint8_t col)
{
  static const uint8_t row_offsets[] = {0x00U, 0x40U};
  if (row > 1U)
  {
    row = 1U;
  }
  lcd_command(lcd, (uint8_t)(0x80U | (row_offsets[row] + col)));
}

static void lcd_write_str(lcd_service_t *lcd, const char *str)
{
  while ((*str != '\0'))
  {
    lcd_write_char(lcd, *str++);
  }
}

static void lcd_init_hw(lcd_service_t *lcd)
{
  HAL_Delay(50);
  lcd_write4bits(lcd, 0x30U);
  HAL_Delay(5);
  lcd_write4bits(lcd, 0x30U);
  HAL_Delay(5);
  lcd_write4bits(lcd, 0x30U);
  HAL_Delay(5);
  lcd_write4bits(lcd, 0x20U);

  lcd_command(lcd, 0x28U); /* 4-bit, 2 line */
  lcd_command(lcd, 0x0CU); /* display on */
  lcd_command(lcd, 0x06U); /* entry mode */
  lcd_command(lcd, 0x01U); /* clear */
  HAL_Delay(5);
}

void lcd_service_init(lcd_service_t *lcd, I2C_HandleTypeDef *i2c, uint8_t address, osMutexId mutex)
{
  if ((lcd == NULL) || (i2c == NULL))
  {
    return;
  }
  lcd->i2c = i2c;
  lcd->address = address;
  lcd->backlight = LCD_BACKLIGHT_DEFAULT ? 1U : 0U;
  lcd->page = LCD_PAGE_TOTAL;
  lcd->mutex = mutex;

  lcd_lock(lcd);
  lcd_init_hw(lcd);
  lcd_unlock(lcd);
}

void lcd_service_set_page(lcd_service_t *lcd, lcd_page_t page)
{
  if (lcd == NULL)
  {
    return;
  }
  if (page >= LCD_PAGE_MAX)
  {
    page = LCD_PAGE_TOTAL;
  }
  lcd->page = page;
}

void lcd_service_next_page(lcd_service_t *lcd)
{
  if (lcd == NULL)
  {
    return;
  }
  lcd->page = (lcd_page_t)((lcd->page + 1U) % LCD_PAGE_MAX);
}

static float lcd_select_energy(const energy_counters_t *counters, lcd_page_t page)
{
  switch (page)
  {
    case LCD_PAGE_TODAY:
      return counters->kwh_day;
    case LCD_PAGE_MONTH:
      return counters->kwh_month;
    case LCD_PAGE_YEAR:
      return counters->kwh_year;
    case LCD_PAGE_TOU_HIGH:
      return counters->kwh_day_bucket[TOU_BUCKET_HIGH];
    case LCD_PAGE_TOU_MID:
      return counters->kwh_day_bucket[TOU_BUCKET_MID];
    case LCD_PAGE_TOU_LOW:
      return counters->kwh_day_bucket[TOU_BUCKET_LOW];
    case LCD_PAGE_TOTAL:
    default:
      return counters->kwh_day;
  }
}

static const char *lcd_page_label(lcd_page_t page)
{
  switch (page)
  {
    case LCD_PAGE_TOTAL:
      return "T";
    case LCD_PAGE_TODAY:
      return "D";
    case LCD_PAGE_MONTH:
      return "M";
    case LCD_PAGE_YEAR:
      return "Y";
    case LCD_PAGE_TOU_HIGH:
      return "H";
    case LCD_PAGE_TOU_MID:
      return "M";
    case LCD_PAGE_TOU_LOW:
      return "L";
    default:
      return " ";
  }
}

void lcd_service_render(lcd_service_t *lcd, const meter_sample_t *sample, const energy_counters_t *counters, const datetime_t *dt)
{
  char line1[17];
  char line2[17];
  float kw = 0.0f;
  float kwh = 0.0f;
  const char *status = "--";

  if ((lcd == NULL) || (sample == NULL) || (counters == NULL) || (dt == NULL))
  {
    return;
  }

  kw = sample->kw_total;
  status = sample->rs485_ok ? "OK" : "ER";

  if (lcd->page == LCD_PAGE_TOTAL)
  {
    kwh = (sample->kwh_total > 0.0f) ? sample->kwh_total : counters->kwh_day;
  }
  else
  {
    kwh = lcd_select_energy(counters, lcd->page);
  }

  (void)snprintf(line1, sizeof(line1), "P:%4.1fkW %s", kw, status);
  (void)snprintf(line2, sizeof(line2), "E%s:%5.1f %02u:%02u", lcd_page_label(lcd->page), kwh, dt->hour, dt->minute);

  lcd_lock(lcd);
  lcd_set_cursor(lcd, 0U, 0U);
  lcd_write_str(lcd, "                ");
  lcd_set_cursor(lcd, 1U, 0U);
  lcd_write_str(lcd, "                ");
  lcd_set_cursor(lcd, 0U, 0U);
  lcd_write_str(lcd, line1);
  lcd_set_cursor(lcd, 1U, 0U);
  lcd_write_str(lcd, line2);
  lcd_unlock(lcd);
}
