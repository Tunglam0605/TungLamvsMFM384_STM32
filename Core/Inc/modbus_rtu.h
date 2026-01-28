#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "rs485_phy.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint8_t slave_id;
  uint32_t baudrate;
  uint32_t parity;
  uint32_t stop_bits;
  uint32_t timeout_ms;
  uint8_t retries;
  uint16_t reg_addr_offset;
} modbus_rtu_config_t;

typedef struct {
  rs485_phy_t *phy;
  modbus_rtu_config_t cfg;
  uint8_t last_error;
} modbus_rtu_t;

typedef struct {
  void *ctx;
  int (*read_registers)(void *ctx, uint8_t function, uint16_t reg_addr, uint16_t count, uint16_t *out_regs);
} modbus_iface_t;

void modbus_rtu_init(modbus_rtu_t *mb, rs485_phy_t *phy, const modbus_rtu_config_t *cfg);
int modbus_rtu_read_registers(modbus_rtu_t *mb, uint8_t function, uint16_t reg_addr, uint16_t count, uint16_t *out_regs);
uint8_t modbus_rtu_get_last_error(const modbus_rtu_t *mb);
void modbus_rtu_get_iface(modbus_rtu_t *mb, modbus_iface_t *iface);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_RTU_H */
