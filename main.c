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
#include "asm_prototype.h"

#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)

#define UART_RX P1_3
#define UART_TX P1_2

XMC_GPIO_CONFIG_t uart_tx;
XMC_GPIO_CONFIG_t uart_rx;

bool g_adc_c0_1_flag;

/* UART configuration */
const XMC_UART_CH_CONFIG_t uart_config = { .data_bits = 8U, .stop_bits = 1U,
		.baudrate = 921600 };

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

	for (i = 0; i < len; ++i) {
		XMC_UART_CH_Transmit(XMC_UART0_CH1, *(data + i));
	}

	// return # of bytes written - as best we can tell
	return len;
}

void __attribute__((section(".ram_code"))) TestFunct(void){
	printf("%08X %s\n", TestFunct, __func__);
	printf("CPUID:%08X\n", SCB->CPUID);
}

extern void Heap_Bank1_Size(void);
extern void Heap_Bank1_Start(void);
extern void Heap_Bank1_End(void);

extern void __Vectors(void);
extern void eROData(void);
extern void DataLoadAddr(void);
extern void __data_start(void);
extern void __data_end(void);

extern void __ram_code_load(void);
extern void __ram_code_start(void);
extern void __ram_code_end(void);

extern void __bss_start(void);
extern void __bss_end(void);

extern void VeneerStart(void);
extern void VeneerEnd(void);
extern void VeneerSize(void);

extern void HardFault_Veneer(void);
extern void HardFault_Handler(void);
extern void SysTick_Veneer(void);
extern void SysTick_Handler(void);

void Reset_Handler(void) {
	//Copy veneer
	uint32_t* src = (uint32_t*)eROData;
	uint32_t* dest_ptr = (uint32_t*)VeneerStart;
	uint32_t* dest_end = (uint32_t*)VeneerEnd;
	while(dest_ptr != dest_end) {
		*dest_ptr++ = *src++;
	}

	SystemInit();

	// Initialize data
	src = (uint32_t*)DataLoadAddr;
	dest_ptr = (uint32_t*)__data_start;
	dest_end = (uint32_t*)__data_end;
	while(dest_ptr != dest_end) {
		*dest_ptr++ = *src++;
	}

	//RAM code copy
	src = (uint32_t*)__ram_code_load;
	dest_ptr = (uint32_t*)__ram_code_start;
	dest_end = (uint32_t*)__ram_code_end;
	while(dest_ptr != dest_end) {
		*dest_ptr++ = *src++;
	}


	///*  Define __SKIP_BSS_CLEAR to disable zeroing uninitialzed data in startup.
	// *  The BSS section is specified by following symbols
	// *    __bss_start__: start of the BSS section.
	// *    __bss_end__: end of the BSS section.
#ifndef __SKIP_BSS_CLEAR
	dest_ptr = (uint32_t*)__bss_start;
	dest_end = (uint32_t*)__bss_end;
	while(dest_ptr != dest_end) {
		*dest_ptr++ = 0;
	}
#endif /* __SKIP_BSS_CLEAR */

#ifndef __SKIP_LIBC_INIT_ARRAY
	__libc_init_array();
#endif

	//Should never return
	main();
}

typedef struct detailed_result_struct
{
	uint8_t channel_num;
	uint8_t group_num;
	uint16_t conversion_result;
}detailed_result_struct_t;

#define	ADC_BUF_LEN	2

uint32_t result;
bool valid_result;
detailed_result_struct_t detailed_result[ADC_BUF_LEN];
static uint8_t g_adc_ch_index;

void Adc_Measurement_Handler(){
	uint32_t result;
	valid_result = (bool)false;

	result = ADC_MEASUREMENT_GetGlobalDetailedResult();

	if((bool)(result >> VADC_GLOBRES_VF_Pos))
	{
		valid_result = (bool)true;
		detailed_result[g_adc_ch_index].channel_num = (result & VADC_GLOBRES_CHNR_Msk) >> VADC_GLOBRES_CHNR_Pos;
		detailed_result[g_adc_ch_index].group_num = ADC_MEASUREMENT_Channel_A.group_index;
		detailed_result[g_adc_ch_index].conversion_result = (result & VADC_GLOBRES_RESULT_Msk) >>
				((uint32_t)ADC_MEASUREMENT_0.iclass_config_handle->conversion_mode_standard * (uint32_t)2);
	}
	g_adc_ch_index = (g_adc_ch_index+1)%ADC_BUF_LEN;

	g_adc_c0_1_flag = true;
}

int main(void){
	__IO uint32_t tmpTick;
	__IO int32_t tmpK;
	__IO int32_t tmpL;
	uint8_t tmp_idx;

	DAVE_STATUS_t status;

	status = DAVE_Init();           /* Initialization of DAVE APPs  */

	ADC_MEASUREMENT_StartConversion(&ADC_MEASUREMENT_0);

	XMC_GPIO_SetMode(XMC_GPIO_PORT0, 5, XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN);

	printf("Test %u MHz %s\n", SystemCoreClock / 1000000, __TIME__);

	if(status != DAVE_STATUS_SUCCESS)
	{
		/* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
		XMC_DEBUG("DAVE APPs initialization failed\n");

		while(1U)
		{

		}
	}

	// System Timer initialization
	SysTick_Config(SystemCoreClock / 1000);

	/* Configure pins */
	uart_tx.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT7;
	uart_rx.mode = XMC_GPIO_MODE_INPUT_TRISTATE;
	XMC_GPIO_Init(UART_TX, &uart_tx);
	XMC_GPIO_Init(UART_RX, &uart_rx);

	/* Configure UART channel */
	XMC_UART_CH_Init(XMC_UART0_CH1, &uart_config);
	XMC_UART_CH_SetInputSource(XMC_UART0_CH1, XMC_UART_CH_INPUT_RXD,
			USIC0_C1_DX0_P1_3);
	/* Start UART channel */
	XMC_UART_CH_Start(XMC_UART0_CH1);

	/* Enable DTS */
	XMC_SCU_StartTempMeasurement();

	printf("Test Dave %u MHz %s\n", SystemCoreClock / 1000000, __TIME__);

	printf("%08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X\n",
			*(uint32_t*)0x10000F00, *(uint32_t*)0x10000F04,
			*(uint32_t*)0x10000F08, *(uint32_t*)0x10000F0C,
			*(uint32_t*)0x10000F10, *(uint32_t*)0x10000F04,
			*(uint32_t*)0x10000F18,
			SCU_GENERAL->DBGROMID, SCU_GENERAL->IDCHIP, SCU_GENERAL->ID, SCB->CPUID);


	printf("Heap Start:%08X, Heap End:%08X, Heap Size:%08X\n",
			(uint32_t)Heap_Bank1_Start, (uint32_t)Heap_Bank1_End, (uint32_t)Heap_Bank1_Size);

	printf("__Vectors:%08X\n", (uint32_t)__Vectors);
	printf("eROData:%08X\n", (uint32_t)eROData);
	printf("VeneerStart:%08X\n", (uint32_t)VeneerStart);
	printf("VeneerEnd:%08X\n", (uint32_t)VeneerEnd);
	printf("VeneerSize:%08X\n", (uint32_t)VeneerSize);
	printf("DataLoadAddr:%08X\n", (uint32_t)DataLoadAddr);
	printf("__ram_code_load:%08X\n", (uint32_t)__ram_code_load);
	printf("__ram_code_start:%08X\n", (uint32_t)__ram_code_start);
	printf("__ram_code_end:%08X\n", (uint32_t)__ram_code_end);

	__ASM volatile ("svc 0" : : : "memory");
	printf("After SVC\n");

	while(1){
		HAL_Delay(500);
		DIGITAL_IO_ToggleOutput(&DIGITAL_IO_0);
		DIGITAL_IO_ToggleOutput(&DIGITAL_IO_1);
		DIGITAL_IO_ToggleOutput(&DIGITAL_IO_2);
		DIGITAL_IO_ToggleOutput(&DIGITAL_IO_3);
		DIGITAL_IO_ToggleOutput(&DIGITAL_IO_4);

		tmpK = XMC1000_CalcTemperature()-273;
		tmpL = tmpK * 10;
		printf("%i\n",
				tmpK);


		if(g_adc_c0_1_flag) {
			for(tmp_idx=0; tmp_idx<ADC_BUF_LEN; ++tmp_idx) {
				printf("[%u] %u, %u, %u\n", tmp_idx,
						detailed_result[tmp_idx].channel_num,
						detailed_result[tmp_idx].group_num,
						detailed_result[tmp_idx].conversion_result);
			}
			printf("\n");

			g_adc_c0_1_flag = false;
			ADC_MEASUREMENT_StartConversion(&ADC_MEASUREMENT_0);
		}
	}

	return 0;
}

void SysTick_Handler(void) {
	g_Ticks++;
}
