#include "rs485_phy.h"
#include "app_config.h"

static void rs485_phy_set_pin(GPIO_TypeDef *port, uint16_t pin, uint8_t active_high, uint8_t enable)
{
  if (port == NULL)
  {
    return;
  }
  GPIO_PinState state = GPIO_PIN_RESET;
  if (enable)
  {
    state = active_high ? GPIO_PIN_SET : GPIO_PIN_RESET;
  }
  else
  {
    state = active_high ? GPIO_PIN_RESET : GPIO_PIN_SET;
  }
  HAL_GPIO_WritePin(port, pin, state);
}

static void rs485_phy_set_dir(rs485_phy_t *phy, uint8_t tx_enable)
{
  if (phy == NULL)
  {
    return;
  }
  rs485_phy_set_pin(phy->cfg.de_port, phy->cfg.de_pin, phy->cfg.de_active_high, tx_enable);
  rs485_phy_set_pin(phy->cfg.re_port, phy->cfg.re_pin, phy->cfg.re_active_high, !tx_enable);
  if (phy->cfg.dir_settle_ms > 0U)
  {
    HAL_Delay(phy->cfg.dir_settle_ms);
  }
}

static void rs485_phy_lock(rs485_phy_t *phy)
{
  if ((phy != NULL) && (phy->cfg.mutex != NULL))
  {
    (void)osMutexWait(phy->cfg.mutex, RS485_MUTEX_TIMEOUT_MS);
  }
}

static void rs485_phy_unlock(rs485_phy_t *phy)
{
  if ((phy != NULL) && (phy->cfg.mutex != NULL))
  {
    (void)osMutexRelease(phy->cfg.mutex);
  }
}

void rs485_phy_init(rs485_phy_t *phy, const rs485_phy_config_t *cfg)
{
  if ((phy == NULL) || (cfg == NULL))
  {
    return;
  }
  phy->cfg = *cfg;
  rs485_phy_set_dir(phy, 0U);
}

HAL_StatusTypeDef rs485_phy_apply_uart_config(rs485_phy_t *phy, uint32_t baudrate, uint32_t parity, uint32_t stop_bits)
{
  if ((phy == NULL) || (phy->cfg.uart == NULL))
  {
    return HAL_ERROR;
  }
  rs485_phy_lock(phy);
  (void)HAL_UART_DeInit(phy->cfg.uart);
  phy->cfg.uart->Init.BaudRate = baudrate;
  phy->cfg.uart->Init.Parity = parity;
  phy->cfg.uart->Init.StopBits = stop_bits;
  phy->cfg.uart->Init.WordLength = (parity == UART_PARITY_NONE) ? UART_WORDLENGTH_8B : UART_WORDLENGTH_9B;
  phy->cfg.uart->Init.Mode = UART_MODE_TX_RX;
  phy->cfg.uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  phy->cfg.uart->Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_StatusTypeDef status = HAL_UART_Init(phy->cfg.uart);
  rs485_phy_unlock(phy);
  return status;
}

HAL_StatusTypeDef rs485_phy_transmit(rs485_phy_t *phy, const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
  if ((phy == NULL) || (phy->cfg.uart == NULL) || (data == NULL) || (len == 0U))
  {
    return HAL_ERROR;
  }
  rs485_phy_lock(phy);
  rs485_phy_set_dir(phy, 1U);
  HAL_StatusTypeDef status = HAL_UART_Transmit(phy->cfg.uart, (uint8_t *)data, len, timeout_ms);
  rs485_phy_set_dir(phy, 0U);
  rs485_phy_unlock(phy);
  return status;
}

HAL_StatusTypeDef rs485_phy_receive(rs485_phy_t *phy, uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
  if ((phy == NULL) || (phy->cfg.uart == NULL) || (data == NULL) || (len == 0U))
  {
    return HAL_ERROR;
  }
  rs485_phy_lock(phy);
  rs485_phy_set_dir(phy, 0U);
  HAL_StatusTypeDef status = HAL_UART_Receive(phy->cfg.uart, data, len, timeout_ms);
  rs485_phy_unlock(phy);
  return status;
}

HAL_StatusTypeDef rs485_phy_transceive(rs485_phy_t *phy, const uint8_t *tx_data, uint16_t tx_len, uint8_t *rx_data, uint16_t rx_len, uint32_t timeout_ms)
{
  if ((phy == NULL) || (phy->cfg.uart == NULL) || (tx_data == NULL) || (tx_len == 0U) || (rx_data == NULL) || (rx_len == 0U))
  {
    return HAL_ERROR;
  }
  rs485_phy_lock(phy);
  rs485_phy_set_dir(phy, 1U);
  HAL_StatusTypeDef status = HAL_UART_Transmit(phy->cfg.uart, (uint8_t *)tx_data, tx_len, timeout_ms);
  rs485_phy_set_dir(phy, 0U);
  if (status == HAL_OK)
  {
    status = HAL_UART_Receive(phy->cfg.uart, rx_data, rx_len, timeout_ms);
  }
  rs485_phy_unlock(phy);
  return status;
}
