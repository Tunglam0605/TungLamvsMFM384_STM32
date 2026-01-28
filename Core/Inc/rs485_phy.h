#ifndef RS485_PHY_H
#define RS485_PHY_H

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  UART_HandleTypeDef *uart;
  GPIO_TypeDef *de_port;
  uint16_t de_pin;
  GPIO_TypeDef *re_port;
  uint16_t re_pin;
  uint8_t de_active_high;
  uint8_t re_active_high;
  uint32_t dir_settle_ms;
  osMutexId mutex;
} rs485_phy_config_t;

typedef struct {
  rs485_phy_config_t cfg;
} rs485_phy_t;

void rs485_phy_init(rs485_phy_t *phy, const rs485_phy_config_t *cfg);
HAL_StatusTypeDef rs485_phy_apply_uart_config(rs485_phy_t *phy, uint32_t baudrate, uint32_t parity, uint32_t stop_bits);
HAL_StatusTypeDef rs485_phy_transmit(rs485_phy_t *phy, const uint8_t *data, uint16_t len, uint32_t timeout_ms);
HAL_StatusTypeDef rs485_phy_receive(rs485_phy_t *phy, uint8_t *data, uint16_t len, uint32_t timeout_ms);
HAL_StatusTypeDef rs485_phy_transceive(rs485_phy_t *phy, const uint8_t *tx_data, uint16_t tx_len, uint8_t *rx_data, uint16_t rx_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* RS485_PHY_H */
