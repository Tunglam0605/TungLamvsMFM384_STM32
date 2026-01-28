# TungLamvsMFM384_STM32

Firmware STM32 ??c c?ng t? SELEC MFM384-C-CE qua RS485 Modbus RTU, hi?n th? LCD 16x2 I2C (PCF8574), ch?y FreeRTOS (CMSIS V1). Ki?n tr?c SOLID, module h?a r? r?ng, d? m? r?ng v? kh?ng ??ng code HAL/generated ngo?i USER CODE.

## C?u tr?c th? m?c (?? g?p v?o Core/Inc & Core/Src)
- `Core/Inc`: t?t c? header c?a project (APP + Services + Models + FreeRTOS wrappers).
- `Core/Src`: t?t c? source c?a project (APP + Services + FreeRTOS task loops).

### C?c module ch?nh (t?ng quan)
- **app_main**: composition root, init t?t c? services + DI, kh?ng ch?a logic nghi?p v?.
- **rs485_phy**: l?p v?t l? UART + DE/RE (kh?ng ch?a logic Modbus).
- **modbus_rtu**: Modbus RTU client (FC04) + retry/timeout/CRC.
- **meter_service**: ??c register map -> struct `meter_sample_t`.
- **meter_register_map**: map ??a ch?, h? tr? config word/byte order.
- **time_service**: RTC n?i LSI, cung c?p now_epoch/get/set.
- **ds3231_service**: DS3231 I2C sync RTC n?i v?i ng??ng l?ch.
- **tou_energy_service**: t?nh ?i?n n?ng theo khung gi? + rollover ng?y/th?ng/n?m.
- **lcd_service**: LCD 16x2 I2C + render pages.
- **button_service**: debounce + long-press.
- **watchdog_service**: heartbeat c?c task + feed IWDG.
- **datastore**: persist counters v?o Flash.
- **tasks / queues**: loop task + queue/event wrapper.

## Build (Keil / STM32CubeIDE)
### Keil
1. Add t?t c? file `.c` trong `Core/Src` v?o project.
2. Add include path: `Core/Inc`.
3. Build.

### STM32CubeIDE
1. M? `TungLamvsMFM384_STM32.ioc`.
2. Generate code.
3. Build.

## C?u h?nh nhanh (app_config.h)
- RS485/Modbus: `MODBUS_SLAVE_ID`, `MODBUS_BAUDRATE`, `MODBUS_PARITY`, `MODBUS_STOPBITS`, `MODBUS_TIMEOUT_MS`, `MODBUS_RETRIES`.
- Offset ??a ch? Modbus: `MODBUS_REG_ADDR_OFFSET` (m?c ??nh 30000).
- LCD I2C: `LCD_I2C_ADDRESS` (0x27/0x3F).
- DS3231: `DS3231_I2C_ADDRESS` (0x68) + `DS3231_SYNC_INTERVAL_MS`.
- TOU: `TOU_*_START_MIN`, `TOU_*_END_MIN`.
- Queue: `USE_QUEUE_STRUCT`.
- Datastore Flash: `DATASTORE_FLASH_BASE`, `DATASTORE_FLASH_PAGE_SIZE`.

## Register map MFM384
S?a t?i `Core/Src/meter_register_map.c`.
- Voltage ?ang ??t m?c ??nh theo 30000..30012 (Input Reg, FC04).
- `kw_total` v? `kwh_total` ?ang ?? `0xFFFF` (TODO) ? c?n thay b?ng ??a ch? ??ng trong t?i li?u MFM384.

## Datastore (persist)
L?u counters v?o Flash trang cu?i (m?c ??nh `0x0800FC00`, page 1KB). N?u MCU kh?c, h?y ch?nh `DATASTORE_FLASH_BASE`/`DATASTORE_FLASH_PAGE_SIZE` cho ??ng v?ng flash tr?ng.

## Log & test
- B?t log: `APP_LOG_ENABLE 1` (m?c ??nh off ?? tr?nh ?nh h??ng RS485).
- B?t test TOU: `APP_TEST_ENABLE 1` (in k?t qu? qua log).

## Ghi ch? ph?n c?ng
- RS485 DE/RE: c?u h?nh trong `app_config.h` n?u module kh?ng auto-direction.
- LCD + DS3231 d?ng chung I2C; mutex `mMeter` lock chung bus.
