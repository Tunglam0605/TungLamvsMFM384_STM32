#include "modbus_rtu.h"
#include <string.h>

#define MODBUS_MAX_REGS 64U

static uint16_t modbus_crc16(const uint8_t *data, uint16_t len)
{
  uint16_t crc = 0xFFFFU;
  uint16_t i;
  for (i = 0; i < len; i++)
  {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8U; j++)
    {
      if (crc & 0x0001U)
      {
        crc = (uint16_t)((crc >> 1) ^ 0xA001U);
      }
      else
      {
        crc >>= 1;
      }
    }
  }
  return crc;
}

void modbus_rtu_init(modbus_rtu_t *mb, rs485_phy_t *phy, const modbus_rtu_config_t *cfg)
{
  if ((mb == NULL) || (phy == NULL) || (cfg == NULL))
  {
    return;
  }
  mb->phy = phy;
  mb->cfg = *cfg;
  mb->last_error = 0U;
  (void)rs485_phy_apply_uart_config(phy, cfg->baudrate, cfg->parity, cfg->stop_bits);
}

uint8_t modbus_rtu_get_last_error(const modbus_rtu_t *mb)
{
  if (mb == NULL)
  {
    return 0U;
  }
  return mb->last_error;
}

static int modbus_rtu_read_once(modbus_rtu_t *mb, uint8_t function, uint16_t reg_addr, uint16_t count, uint16_t *out_regs)
{
  uint8_t req[8];
  uint8_t resp[5 + (2U * MODBUS_MAX_REGS)];
  uint16_t req_crc;
  uint16_t resp_len;
  uint16_t i;
  uint16_t crc_calc;
  uint16_t crc_recv;
  uint16_t reg_zero;

  if ((mb == NULL) || (mb->phy == NULL) || (out_regs == NULL))
  {
    return -1;
  }
  if ((count == 0U) || (count > MODBUS_MAX_REGS))
  {
    return -1;
  }

  if (reg_addr >= mb->cfg.reg_addr_offset)
  {
    reg_zero = (uint16_t)(reg_addr - mb->cfg.reg_addr_offset);
  }
  else
  {
    reg_zero = reg_addr;
  }

  req[0] = mb->cfg.slave_id;
  req[1] = function;
  req[2] = (uint8_t)(reg_zero >> 8);
  req[3] = (uint8_t)(reg_zero & 0xFFU);
  req[4] = (uint8_t)(count >> 8);
  req[5] = (uint8_t)(count & 0xFFU);
  req_crc = modbus_crc16(req, 6U);
  req[6] = (uint8_t)(req_crc & 0xFFU);
  req[7] = (uint8_t)(req_crc >> 8);

  resp_len = (uint16_t)(5U + (2U * count));
  if (rs485_phy_transceive(mb->phy, req, sizeof(req), resp, resp_len, mb->cfg.timeout_ms) != HAL_OK)
  {
    mb->last_error = 3U;
    return -1;
  }

  crc_calc = modbus_crc16(resp, (uint16_t)(resp_len - 2U));
  crc_recv = (uint16_t)(resp[resp_len - 2U] | ((uint16_t)resp[resp_len - 1U] << 8));
  if (crc_calc != crc_recv)
  {
    mb->last_error = 4U;
    return -1;
  }

  if ((resp[0] != mb->cfg.slave_id) || (resp[1] != function))
  {
    mb->last_error = 5U;
    return -1;
  }
  if (resp[2] != (uint8_t)(2U * count))
  {
    mb->last_error = 5U;
    return -1;
  }

  for (i = 0; i < count; i++)
  {
    uint16_t idx = (uint16_t)(3U + (2U * i));
    out_regs[i] = (uint16_t)(((uint16_t)resp[idx] << 8) | resp[idx + 1U]);
  }

  mb->last_error = 0U;
  return 0;
}

int modbus_rtu_read_registers(modbus_rtu_t *mb, uint8_t function, uint16_t reg_addr, uint16_t count, uint16_t *out_regs)
{
  uint8_t attempt;
  if (mb == NULL)
  {
    return -1;
  }
  for (attempt = 0U; attempt <= mb->cfg.retries; attempt++)
  {
    if (modbus_rtu_read_once(mb, function, reg_addr, count, out_regs) == 0)
    {
      return 0;
    }
  }
  return -1;
}

static int modbus_iface_read(void *ctx, uint8_t function, uint16_t reg_addr, uint16_t count, uint16_t *out_regs)
{
  return modbus_rtu_read_registers((modbus_rtu_t *)ctx, function, reg_addr, count, out_regs);
}

void modbus_rtu_get_iface(modbus_rtu_t *mb, modbus_iface_t *iface)
{
  if ((mb == NULL) || (iface == NULL))
  {
    return;
  }
  iface->ctx = mb;
  iface->read_registers = modbus_iface_read;
}
