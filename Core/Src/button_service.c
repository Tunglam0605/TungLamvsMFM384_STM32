#include "button_service.h"
#include "app_config.h"

void button_service_init(button_service_t *btn, GPIO_TypeDef *port, uint16_t pin, uint8_t active_level)
{
  if (btn == NULL)
  {
    return;
  }
  btn->port = port;
  btn->pin = pin;
  btn->active_level = active_level;
  btn->stable_state = 0U;
  btn->last_state = 0U;
  btn->last_change_ms = HAL_GetTick();
  btn->pressed_ms = 0U;
  btn->long_reported = 0U;
}

uint8_t button_service_update(button_service_t *btn, uint32_t debounce_ms, uint32_t long_press_ms)
{
  uint8_t event = BUTTON_EVENT_NONE;
  uint8_t raw_state;
  uint32_t now;

  if (btn == NULL)
  {
    return BUTTON_EVENT_NONE;
  }

  raw_state = (HAL_GPIO_ReadPin(btn->port, btn->pin) == (btn->active_level ? GPIO_PIN_SET : GPIO_PIN_RESET)) ? 1U : 0U;
  now = HAL_GetTick();

  if (raw_state != btn->last_state)
  {
    btn->last_state = raw_state;
    btn->last_change_ms = now;
  }

  if ((now - btn->last_change_ms) >= debounce_ms)
  {
    if (btn->stable_state != raw_state)
    {
      btn->stable_state = raw_state;
      if (raw_state)
      {
        btn->pressed_ms = now;
        btn->long_reported = 0U;
        event |= BUTTON_EVENT_PRESSED;
      }
      else
      {
        event |= BUTTON_EVENT_RELEASED;
      }
    }
  }

  if ((btn->stable_state != 0U) && (!btn->long_reported))
  {
    if ((now - btn->pressed_ms) >= long_press_ms)
    {
      btn->long_reported = 1U;
      event |= BUTTON_EVENT_LONG_PRESS;
    }
  }

  return event;
}
