#include <xmc_uart.h>
#include "xmc_gpio.h"

#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_BAUDRATE 921600U
#define SERIAL_UART XMC_UART0_CH1
#define SERIAL_TX_PIN P1_2
#define SERIAL_TX_PIN_AF (XMC_GPIO_MODE_t)((int32_t)XMC_GPIO_MODE_OUTPUT_PUSH_PULL | (int32_t)P1_2_AF_U0C1_DOUT0)
#define SERIAL_RX_PIN P1_3
#define SERIAL_RX_INPUT USIC0_C1_DX0_P1_3
#define SERIAL_RX_IRQN USIC0_0_IRQn

#define SERIAL_BUFFER_SIZE 128

#ifdef __cplusplus
extern "C" {
#endif

//void serial_init(void);

#ifdef __cplusplus
}
#endif

#endif
