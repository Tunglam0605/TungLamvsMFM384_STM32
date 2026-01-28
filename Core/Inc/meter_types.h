#ifndef METER_TYPES_H
#define METER_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint8_t valid;
  uint8_t rs485_ok;
  float v_avg;
  float v1n;
  float v2n;
  float v3n;
  float kw_total;
  float kwh_total;
  uint32_t tick_ms;
} meter_sample_t;

#ifdef __cplusplus
}
#endif

#endif /* METER_TYPES_H */
