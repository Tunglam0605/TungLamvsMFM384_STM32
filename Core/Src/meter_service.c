#include "meter_service.h"
#include "stm32f1xx_hal.h"
#include <string.h>

static float modbus_decode_float(const uint16_t regs[2], modbus_word_order_t word_order, modbus_byte_order_t byte_order)
{
  uint16_t w0 = regs[0];
  uint16_t w1 = regs[1];
  uint8_t bytes[4];
  float value = 0.0f;

  if (word_order == MODBUS_WORD_ORDER_BA)
  {
    uint16_t tmp = w0;
    w0 = w1;
    w1 = tmp;
  }

  if (byte_order == MODBUS_BYTE_ORDER_AB)
  {
    bytes[0] = (uint8_t)(w0 >> 8);
    bytes[1] = (uint8_t)(w0 & 0xFFU);
    bytes[2] = (uint8_t)(w1 >> 8);
    bytes[3] = (uint8_t)(w1 & 0xFFU);
  }
  else
  {
    bytes[0] = (uint8_t)(w0 & 0xFFU);
    bytes[1] = (uint8_t)(w0 >> 8);
    bytes[2] = (uint8_t)(w1 & 0xFFU);
    bytes[3] = (uint8_t)(w1 >> 8);
  }

  (void)memcpy(&value, bytes, sizeof(value));
  return value;
}

static uint8_t meter_read_entry(meter_service_t *svc, const meter_reg_map_entry_t *entry, float *out_value)
{
  uint16_t regs[2];
  if ((entry == NULL) || (out_value == NULL) || (svc == NULL) || (svc->modbus.read_registers == NULL))
  {
    return 0U;
  }
  if (entry->address == 0xFFFFU)
  {
    return 0U;
  }
  if (svc->modbus.read_registers(svc->modbus.ctx, entry->function, entry->address, 2U, regs) != 0)
  {
    return 0U;
  }
  *out_value = modbus_decode_float(regs, entry->word_order, entry->byte_order);
  return 1U;
}

void meter_service_init(meter_service_t *svc, const modbus_iface_t *modbus, const meter_register_map_t *map)
{
  if ((svc == NULL) || (modbus == NULL) || (map == NULL))
  {
    return;
  }
  svc->modbus = *modbus;
  svc->map = map;
  svc->link_ok = 0U;
}

uint8_t meter_service_read(meter_service_t *svc, meter_sample_t *sample)
{
  uint8_t ok = 1U;
  uint8_t any = 0U;
  if ((svc == NULL) || (sample == NULL) || (svc->map == NULL))
  {
    return 0U;
  }

  (void)memset(sample, 0, sizeof(*sample));
  sample->tick_ms = HAL_GetTick();

  if (svc->map->v_avg.address != 0xFFFFU)
  {
    any = 1U;
    if (!meter_read_entry(svc, &svc->map->v_avg, &sample->v_avg))
    {
      ok = 0U;
    }
  }
  if (svc->map->v1n.address != 0xFFFFU)
  {
    any = 1U;
    if (!meter_read_entry(svc, &svc->map->v1n, &sample->v1n))
    {
      ok = 0U;
    }
  }
  if (svc->map->v2n.address != 0xFFFFU)
  {
    any = 1U;
    if (!meter_read_entry(svc, &svc->map->v2n, &sample->v2n))
    {
      ok = 0U;
    }
  }
  if (svc->map->v3n.address != 0xFFFFU)
  {
    any = 1U;
    if (!meter_read_entry(svc, &svc->map->v3n, &sample->v3n))
    {
      ok = 0U;
    }
  }
  if (svc->map->kw_total.address != 0xFFFFU)
  {
    any = 1U;
    if (!meter_read_entry(svc, &svc->map->kw_total, &sample->kw_total))
    {
      ok = 0U;
    }
  }
  if (svc->map->kwh_total.address != 0xFFFFU)
  {
    any = 1U;
    if (!meter_read_entry(svc, &svc->map->kwh_total, &sample->kwh_total))
    {
      ok = 0U;
    }
  }

  sample->valid = (any && ok) ? 1U : 0U;
  sample->rs485_ok = ok ? 1U : 0U;
  svc->link_ok = sample->rs485_ok;
  return sample->valid;
}

uint8_t meter_service_link_ok(const meter_service_t *svc)
{
  if (svc == NULL)
  {
    return 0U;
  }
  return svc->link_ok;
}
