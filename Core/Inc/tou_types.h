#ifndef TOU_TYPES_H
#define TOU_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  TOU_BUCKET_LOW = 0,
  TOU_BUCKET_MID = 1,
  TOU_BUCKET_HIGH = 2
} tou_bucket_t;

typedef struct {
  uint16_t start_min;
  uint16_t end_min;
} tou_window_t;

typedef struct {
  tou_window_t low;
  tou_window_t mid;
  tou_window_t high;
} tou_config_t;

typedef struct {
  float kwh_day;
  float kwh_month;
  float kwh_year;
  float kwh_day_bucket[3];
} energy_counters_t;

#ifdef __cplusplus
}
#endif

#endif /* TOU_TYPES_H */
