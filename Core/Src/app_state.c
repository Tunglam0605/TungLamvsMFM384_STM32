#include "app_state.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

static void app_state_copy(void *dst, const void *src, size_t len)
{
  taskENTER_CRITICAL();
  (void)memcpy(dst, src, len);
  taskEXIT_CRITICAL();
}

void app_state_init(app_state_t *state)
{
  if (state == NULL)
  {
    return;
  }
  taskENTER_CRITICAL();
  (void)memset(state, 0, sizeof(*state));
  taskEXIT_CRITICAL();
}

void app_state_set_sample(app_state_t *state, const meter_sample_t *sample)
{
  if ((state == NULL) || (sample == NULL))
  {
    return;
  }
  app_state_copy(&state->sample, sample, sizeof(*sample));
}

void app_state_get_sample(const app_state_t *state, meter_sample_t *sample)
{
  if ((state == NULL) || (sample == NULL))
  {
    return;
  }
  app_state_copy(sample, &state->sample, sizeof(*sample));
}

void app_state_set_counters(app_state_t *state, const energy_counters_t *counters)
{
  if ((state == NULL) || (counters == NULL))
  {
    return;
  }
  app_state_copy(&state->counters, counters, sizeof(*counters));
}

void app_state_get_counters(const app_state_t *state, energy_counters_t *counters)
{
  if ((state == NULL) || (counters == NULL))
  {
    return;
  }
  app_state_copy(counters, &state->counters, sizeof(*counters));
}

void app_state_set_datetime(app_state_t *state, const datetime_t *dt)
{
  if ((state == NULL) || (dt == NULL))
  {
    return;
  }
  app_state_copy(&state->datetime, dt, sizeof(*dt));
}

void app_state_get_datetime(const app_state_t *state, datetime_t *dt)
{
  if ((state == NULL) || (dt == NULL))
  {
    return;
  }
  app_state_copy(dt, &state->datetime, sizeof(*dt));
}
