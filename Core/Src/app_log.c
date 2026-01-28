#include "app_log.h"
#include "app_config.h"
#include "usart.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>

void app_log_init(void)
{
  /* Placeholder for future log init */
}

void app_log_printf(const char *fmt, ...)
{
#if APP_LOG_ENABLE
  char buffer[128];
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  if (len > 0)
  {
    if ((size_t)len > sizeof(buffer))
    {
      len = (int)sizeof(buffer);
    }
    (void)HAL_UART_Transmit(&APP_LOG_UART_HANDLE, (uint8_t *)buffer, (uint16_t)len, APP_LOG_UART_TIMEOUT_MS);
  }
#else
  (void)fmt;
#endif
}
