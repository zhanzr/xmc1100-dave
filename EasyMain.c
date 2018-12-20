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
#include  <errno.h>
#include  <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO
#include <stdfix.h>
#include <complex.h>

#include <XMC1100.h>
#include <xmc_scu.h>
#include <xmc_rtc.h>
#include <xmc_uart.h>
#include <xmc_gpio.h>
#include <xmc_flash.h>

#include "GPIO.h"
#include "XMC1000_TSE.h"

#define UART_RX P1_3
#define UART_TX P1_2

XMC_GPIO_CONFIG_t uart_tx;
XMC_GPIO_CONFIG_t uart_rx;

/* UART configuration */
const XMC_UART_CH_CONFIG_t uart_config = { .data_bits = 8U, .stop_bits = 1U,
		.baudrate = 256000 };

__IO uint32_t g_Ticks;

//Retarget IO
int _write(int file, char *data, int len) {
	int i;

	if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
		errno = EBADF;
		return -1;
	}

	for (i = 0; i < len; ++i) {
		XMC_UART_CH_Transmit(XMC_UART0_CH1, *(data + i));
	}

	// return # of bytes written - as best we can tell
	return len;
}

void test_fix(void)
{
	printf("Ref: http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1169.pdf \n");

	printf("fract size:%u accum size:%u sat fract size:%u sat accum size:%u\n",
			sizeof(fract), sizeof(accum), sizeof(sat fract), sizeof(sat accum));

	{
		fract f = 0.1r;
		fract res;
		for(int i=0; i<10; ++i)
		{
			res = f * i;

			printf("%u %04X\n", res, *((uint16_t*)&res));
		}

		f = -0.1r;
		for(int i=0; i<10; ++i)
		{
			res = f * i;

			printf("%u %04X\n", res, *((uint16_t*)&res));
		}
	}
}

int main(void)
{
	__IO uint32_t tmpTick;
	__IO uint32_t deltaTick;
	__IO int8_t tmpK;
	__IO uint32_t i = 0;
	// Clock configuration
	XMC_SCU_CLOCK_CONFIG_t clock_config = { .rtc_src =
			XMC_SCU_CLOCK_RTCCLKSRC_DCO2, .pclk_src =
					XMC_SCU_CLOCK_PCLKSRC_DOUBLE_MCLK, .fdiv = 0, .idiv = 1 };
	XMC_SCU_CLOCK_Init(&clock_config);
	SystemCoreClockUpdate();

	/*Initialize the UART driver */
	uart_tx.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT7;
	uart_rx.mode = XMC_GPIO_MODE_INPUT_TRISTATE;
	/* Configure UART channel */
	XMC_UART_CH_Init(XMC_UART0_CH1, &uart_config);
	XMC_UART_CH_SetInputSource(XMC_UART0_CH1, XMC_UART_CH_INPUT_RXD,
			USIC0_C1_DX0_P1_3);

	/* Start UART channel */
	XMC_UART_CH_Start(XMC_UART0_CH1);

	/* Configure pins */
	XMC_GPIO_Init(UART_TX, &uart_tx);
	XMC_GPIO_Init(UART_RX, &uart_rx);

	// LEDs configuration (P1.2 and P1.3 are used for serial comm)
	P0_5_set_mode(OUTPUT_OD_GP);
	P0_6_set_mode(OUTPUT_OD_GP);
	P1_4_set_mode(OUTPUT_OD_GP);
	P1_5_set_mode(OUTPUT_OD_GP);

	// Turn ON all LEDs
	P0_5_reset();
	P0_6_reset();
	P1_4_reset();
	P1_5_reset();

	// System Timer initialization
	SysTick_Config(SystemCoreClock / 1000);

	/* Enable DTS */
	XMC_SCU_StartTempMeasurement();

	printf("Test %u MHz %s\n", SystemCoreClock / 1000000, __TIME__);

	test_fix();

	while(1)
	{
		P0_5_reset();
		P0_6_reset();
		P1_4_reset();
		P1_5_reset();

		tmpTick = g_Ticks;
		while((tmpTick+1000) > g_Ticks)
		{
			__NOP();
		}
		P0_5_set();
		P0_6_set();
		P1_4_set();
		P1_5_set();

		tmpTick = g_Ticks;
		while((tmpTick+1000) > g_Ticks)
		{
			__NOP();
		}

		tmpK = XMC1000_CalcTemperature()-273;

		printf("%i %s\n", tmpK, _NEWLIB_VERSION);

	    double PI = acos(-1);
	    double complex z = cexp(I * PI); // Euler's formula
	    printf("exp(i*pi) = %.1f%+.1fi\n", creal(z), cimag(z));
	}

	return 0;
}

void SysTick_Handler(void) {
	g_Ticks++;
}
