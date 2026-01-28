#ifndef TIME_TYPES_H
#define TIME_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t weekday;
} datetime_t;

#ifdef __cplusplus
}
#endif

#endif /* TIME_TYPES_H */
