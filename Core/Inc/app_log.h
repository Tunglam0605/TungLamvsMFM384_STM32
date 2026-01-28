#ifndef APP_LOG_H
#define APP_LOG_H

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void app_log_init(void);
void app_log_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* APP_LOG_H */
