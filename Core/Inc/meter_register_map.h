#ifndef METER_REGISTER_MAP_H
#define METER_REGISTER_MAP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  MODBUS_WORD_ORDER_AB = 0,
  MODBUS_WORD_ORDER_BA = 1
} modbus_word_order_t;

typedef enum {
  MODBUS_BYTE_ORDER_AB = 0,
  MODBUS_BYTE_ORDER_BA = 1
} modbus_byte_order_t;

typedef struct {
  uint16_t address;
  uint8_t function;
  modbus_word_order_t word_order;
  modbus_byte_order_t byte_order;
} meter_reg_map_entry_t;

typedef struct {
  meter_reg_map_entry_t v_avg;
  meter_reg_map_entry_t v1n;
  meter_reg_map_entry_t v2n;
  meter_reg_map_entry_t v3n;
  meter_reg_map_entry_t kw_total;
  meter_reg_map_entry_t kwh_total;
} meter_register_map_t;

extern const meter_register_map_t g_meter_register_map;

#ifdef __cplusplus
}
#endif

#endif /* METER_REGISTER_MAP_H */
