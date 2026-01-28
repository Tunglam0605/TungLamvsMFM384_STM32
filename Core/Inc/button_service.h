#ifndef BUTTON_SERVICE_H
#define BUTTON_SERVICE_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUTTON_EVENT_NONE 0U
#define BUTTON_EVENT_PRESSED 0x01U
#define BUTTON_EVENT_RELEASED 0x02U
#define BUTTON_EVENT_LONG_PRESS 0x04U

typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
  uint8_t active_level;
  uint8_t stable_state;
  uint8_t last_state;
  uint32_t last_change_ms;
  uint32_t pressed_ms;
  uint8_t long_reported;
} button_service_t;

void button_service_init(button_service_t *btn, GPIO_TypeDef *port, uint16_t pin, uint8_t active_level);
uint8_t button_service_update(button_service_t *btn, uint32_t debounce_ms, uint32_t long_press_ms);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_SERVICE_H */
