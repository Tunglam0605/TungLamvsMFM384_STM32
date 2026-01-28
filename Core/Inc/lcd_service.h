#ifndef LCD_SERVICE_H
#define LCD_SERVICE_H

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "meter_types.h"
#include "tou_types.h"
#include "time_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  LCD_PAGE_TOTAL = 0,
  LCD_PAGE_TODAY = 1,
  LCD_PAGE_MONTH = 2,
  LCD_PAGE_YEAR = 3,
  LCD_PAGE_TOU_HIGH = 4,
  LCD_PAGE_TOU_MID = 5,
  LCD_PAGE_TOU_LOW = 6,
  LCD_PAGE_MAX
} lcd_page_t;

typedef struct {
  I2C_HandleTypeDef *i2c;
  uint8_t address;
  uint8_t backlight;
  lcd_page_t page;
  osMutexId mutex;
} lcd_service_t;

void lcd_service_init(lcd_service_t *lcd, I2C_HandleTypeDef *i2c, uint8_t address, osMutexId mutex);
void lcd_service_set_page(lcd_service_t *lcd, lcd_page_t page);
void lcd_service_next_page(lcd_service_t *lcd);
void lcd_service_render(lcd_service_t *lcd, const meter_sample_t *sample, const energy_counters_t *counters, const datetime_t *dt);

#ifdef __cplusplus
}
#endif

#endif /* LCD_SERVICE_H */
