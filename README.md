# TungLamvsMFM384_STM32

Firmware STM32 đọc công tơ SELEC MFM384-C-CE qua RS485 Modbus RTU, hiển thị LCD 16x2 I2C (PCF8574), chạy FreeRTOS (CMSIS V1). Kiến trúc SOLID, module hóa rõ ràng, dễ mở rộng và không đụng code HAL/generated ngoài USER CODE.

## Cấu trúc thư mục (đã gộp vào Core/Inc & Core/Src)
- `Core/Inc`: tất cả header của project (APP + Services + Models + FreeRTOS wrappers).
- `Core/Src`: tất cả source của project (APP + Services + FreeRTOS task loops).

### Các module chính (tổng quan)
- **app_main**: composition root, init tất cả services + DI, không chứa logic nghiệp vụ.
- **rs485_phy**: lớp vật lý UART + DE/RE (không chứa logic Modbus).
- **modbus_rtu**: Modbus RTU client (FC04) + retry/timeout/CRC.
- **meter_service**: đọc register map -> struct `meter_sample_t`.
- **meter_register_map**: map địa chỉ, hỗ trợ config word/byte order.
- **time_service**: RTC nội LSI, cung cấp now_epoch/get/set.
- **ds3231_service**: DS3231 I2C sync RTC nội với ngưỡng lệch.
- **tou_energy_service**: tính điện năng theo khung giờ + rollover ngày/tháng/năm.
- **lcd_service**: LCD 16x2 I2C + render pages.
- **button_service**: debounce + long-press.
- **watchdog_service**: heartbeat các task + feed IWDG.
- **datastore**: persist counters vào Flash.
- **tasks / queues**: loop task + queue/event wrapper.

## Build (Keil / STM32CubeIDE)
### Keil
1. Add tất cả file `.c` trong `Core/Src` vào project.
2. Add include path: `Core/Inc`.
3. Build.

### STM32CubeIDE
1. Mở `TungLamvsMFM384_STM32.ioc`.
2. Generate code.
3. Build.

## Cấu hình nhanh (app_config.h)
- RS485/Modbus: `MODBUS_SLAVE_ID`, `MODBUS_BAUDRATE`, `MODBUS_PARITY`, `MODBUS_STOPBITS`, `MODBUS_TIMEOUT_MS`, `MODBUS_RETRIES`.
- Offset địa chỉ Modbus: `MODBUS_REG_ADDR_OFFSET` (mặc định 30000).
- LCD I2C: `LCD_I2C_ADDRESS` (0x27/0x3F).
- DS3231: `DS3231_I2C_ADDRESS` (0x68) + `DS3231_SYNC_INTERVAL_MS`.
- TOU: `TOU_*_START_MIN`, `TOU_*_END_MIN`.
- Queue: `USE_QUEUE_STRUCT`.
- Datastore Flash: `DATASTORE_FLASH_BASE`, `DATASTORE_FLASH_PAGE_SIZE`.

## Register map MFM384
Sửa tại `Core/Src/meter_register_map.c`.
- Voltage đang đặt mặc định theo 30000..30012 (Input Reg, FC04).
- `kw_total` và `kwh_total` đang để `0xFFFF` (TODO) – cần thay bằng địa chỉ đúng trong tài liệu MFM384.

## Datastore (persist)
Lưu counters vào Flash trang cuối (mặc định `0x0800FC00`, page 1KB). Nếu MCU khác, hãy chỉnh `DATASTORE_FLASH_BASE`/`DATASTORE_FLASH_PAGE_SIZE` cho đúng vùng flash trống.

## Log & test
- Bật log: `APP_LOG_ENABLE 1` (mặc định off để tránh ảnh hưởng RS485).
- Bật test TOU: `APP_TEST_ENABLE 1` (in kết quả qua log).

## Ghi chú phần cứng
- RS485 DE/RE: cấu hình trong `app_config.h` nếu module không auto-direction.
- LCD + DS3231 dùng chung I2C; mutex `mMeter` lock chung bus.
