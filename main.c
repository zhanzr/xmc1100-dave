/******************************************************************************
 * @file     EasyMain.c
 * @brief    Uses a system timer to toggle the  ports P0.5, P0.6, P1.5 and P1.6 with 200ms frequency.
 *			 LEDs that are connected to these ports will toggle respectively.
 * 			 In addition it uses the UART of USIC Channel 1 to send a message every 2s to a terminal
 *			 emulator. The communication settings are 57.6kbps/8N1
 *			 P1.3 is configured as input (RXD) and P1.2 is configured as output(TXD)
 *			 This project runs without modifications on the XMC1100 kit for ARDUINO
 * @version  V1.0
 * @date     20. February 2015
 * @note
 * Copyright (C) 2015 Infineon Technologies AG. All rights reserved.
 ******************************************************************************
 * @par
 * Infineon Technologies AG (Infineon) is supplying this software for use with
 * Infineon�s microcontrollers.
 * This file can be freely distributed within development tools that are
 * supporting such microcontrollers.
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO
#include <stdfix.h>
#include <math.h>
#include <complex.h>

#include <XMC1100.h>
#include <xmc_scu.h>
#include <xmc_rtc.h>
#include <xmc_uart.h>
#include <xmc_gpio.h>
#include <xmc_flash.h>
#include <xmc_spi.h>

#include "XMC1000_TSE.h"
#include "serial.h"

#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)

__IO uint32_t g_Ticks;
void HAL_Delay(uint32_t d){
	uint32_t tmpTick = g_Ticks;
	while((tmpTick+d) > g_Ticks){
		__NOP();
	}
}

//Retarget IO
int _write(int file, char *data, int len) {
	int i;

	if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
		errno = EBADF;
		return -1;
	}

	//	for (i = 0; i < len; ++i) {
	//		XMC_UART_CH_Transmit(XMC_UART0_CH1, *(data + i));
	//	}
	UART_Transmit(&UART_0, data, len);

	// return # of bytes written - as best we can tell
	return len;
}

static uint8_t g_rcv_byte;
void uart_rx_cb(void) {
	g_rcv_byte = XMC_UART_CH_GetReceivedData(SERIAL_UART);
}

int main(void){
	DAVE_STATUS_t status;

	status = DAVE_Init();           /* Initialization of DAVE APPs  */

	printf("Test %u MHz %s\n", SystemCoreClock / 1000000, __TIME__);

	if(status != DAVE_STATUS_SUCCESS)
	{
		/* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
		XMC_DEBUG("DAVE APPs initialization failed\n");
		while(1U) {
		}
	}

	// System Timer initialization
	SysTick_Config(SystemCoreClock / 1000);

	/* Enable DTS */
	XMC_SCU_StartTempMeasurement();

	while(1){
		uint8_t tmp = 0;
		UART_Receive(&UART_0, &tmp, 1);
		if(0 != g_rcv_byte) {
			DIGITAL_IO_ToggleOutput(&DIGITAL_IO_0);

			__IO int32_t tmpK = XMC1000_CalcTemperature()-273;
			__IO int32_t tmpL = tmpK * 10;
			printf("%c Cel = %i\n", g_rcv_byte, tmpK);
			g_rcv_byte = 0;
		}
	}

	return 0;
}

void SysTick_Handler(void) {
	g_Ticks++;
}
