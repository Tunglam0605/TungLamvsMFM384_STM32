#include "meter_register_map.h"

/* Default map for MFM384 (adjust addresses if needed). */
const meter_register_map_t g_meter_register_map = {
  .v_avg = {30012U, 0x04U, MODBUS_WORD_ORDER_AB, MODBUS_BYTE_ORDER_AB},
  .v1n = {30000U, 0x04U, MODBUS_WORD_ORDER_AB, MODBUS_BYTE_ORDER_AB},
  .v2n = {30002U, 0x04U, MODBUS_WORD_ORDER_AB, MODBUS_BYTE_ORDER_AB},
  .v3n = {30004U, 0x04U, MODBUS_WORD_ORDER_AB, MODBUS_BYTE_ORDER_AB},
  /* TODO: update kW/kWh register addresses per MFM384 map */
  .kw_total = {0xFFFFU, 0x04U, MODBUS_WORD_ORDER_AB, MODBUS_BYTE_ORDER_AB},
  .kwh_total = {0xFFFFU, 0x04U, MODBUS_WORD_ORDER_AB, MODBUS_BYTE_ORDER_AB}
};
