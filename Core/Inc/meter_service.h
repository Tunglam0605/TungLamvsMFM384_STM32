#ifndef METER_SERVICE_H
#define METER_SERVICE_H

#include "modbus_rtu.h"
#include "meter_register_map.h"
#include "meter_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  modbus_iface_t modbus;
  const meter_register_map_t *map;
  uint8_t link_ok;
} meter_service_t;

void meter_service_init(meter_service_t *svc, const modbus_iface_t *modbus, const meter_register_map_t *map);
uint8_t meter_service_read(meter_service_t *svc, meter_sample_t *sample);
uint8_t meter_service_link_ok(const meter_service_t *svc);

#ifdef __cplusplus
}
#endif

#endif /* METER_SERVICE_H */
