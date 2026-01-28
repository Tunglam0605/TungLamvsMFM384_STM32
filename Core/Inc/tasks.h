#ifndef TASKS_H
#define TASKS_H

#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

void tasks_rs485(void const *argument);
void tasks_tou(void const *argument);
void tasks_lcd(void const *argument);
void tasks_ui(void const *argument);
void tasks_wdg(void const *argument);

#ifdef __cplusplus
}
#endif

#endif /* TASKS_H */
