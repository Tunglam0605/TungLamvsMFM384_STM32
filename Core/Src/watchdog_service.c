#include "watchdog_service.h"

void watchdog_service_init(watchdog_service_t *svc, IWDG_HandleTypeDef *iwdg, uint32_t timeout_ms)
{
  uint8_t i;
  if ((svc == NULL) || (iwdg == NULL))
  {
    return;
  }
  svc->iwdg = iwdg;
  svc->timeout_ms = timeout_ms;
  for (i = 0U; i < (uint8_t)WDG_HB_MAX; i++)
  {
    svc->last_seen[i] = HAL_GetTick();
  }
}

void watchdog_service_heartbeat(watchdog_service_t *svc, wdg_heartbeat_id_t id)
{
  if ((svc == NULL) || (id >= WDG_HB_MAX))
  {
    return;
  }
  svc->last_seen[id] = HAL_GetTick();
}

uint8_t watchdog_service_all_alive(watchdog_service_t *svc)
{
  uint32_t now;
  uint8_t i;
  if (svc == NULL)
  {
    return 0U;
  }
  now = HAL_GetTick();
  for (i = 0U; i < (uint8_t)WDG_HB_MAX; i++)
  {
    if ((now - svc->last_seen[i]) > svc->timeout_ms)
    {
      return 0U;
    }
  }
  return 1U;
}

void watchdog_service_feed_if_alive(watchdog_service_t *svc)
{
  if (svc == NULL)
  {
    return;
  }
  if (watchdog_service_all_alive(svc))
  {
    (void)HAL_IWDG_Refresh(svc->iwdg);
  }
}
