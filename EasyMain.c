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

#include "GPIO.h"
#include "XMC1000_TSE.h"
#include "asm_prototype.h"

#define UART_RX P1_3
#define UART_TX P1_2

XMC_GPIO_CONFIG_t uart_tx;
XMC_GPIO_CONFIG_t uart_rx;

/* UART configuration */
const XMC_UART_CH_CONFIG_t uart_config = { .data_bits = 8U, .stop_bits = 1U,
		.baudrate = 256000 };

__IO uint32_t g_Ticks;
void HAL_Delay(uint32_t d){
	uint32_t tmpTick = g_Ticks;
	while((tmpTick+d) > g_Ticks){
		__NOP();
	}
}

void LD2_ON(void){
}

void LD2_OFF(void){
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


XMC_GPIO_CONFIG_t rx_pin_config;
XMC_GPIO_CONFIG_t tx_pin_config;
XMC_GPIO_CONFIG_t selo_pin_config;
XMC_GPIO_CONFIG_t clk_pin_config;

/**
 * @brief SPI configuration structure
 */
XMC_SPI_CH_CONFIG_t spi_config =
{
		.baudrate = 1000000U,
		.bus_mode = XMC_SPI_CH_BUS_MODE_MASTER,
		.selo_inversion = XMC_SPI_CH_SLAVE_SEL_INV_TO_MSLS,
		.parity_mode = XMC_USIC_CH_PARITY_MODE_NONE
};

void spi_init(void){
	/* Initialize and Start SPI */
	XMC_SPI_CH_Init(XMC_SPI0_CH0, &spi_config);
	XMC_SPI_CH_SetInputSource(XMC_SPI0_CH0, XMC_SPI_CH_INPUT_DIN0, USIC0_C0_DX0_P2_0);

	XMC_SPI_CH_SetFrameLength(XMC_SPI0_CH0, (uint8_t)64);

	XMC_SPI_CH_Start(XMC_SPI0_CH0);

	/* GPIO Input pin configuration */
	rx_pin_config.mode = XMC_GPIO_MODE_INPUT_TRISTATE;
	XMC_GPIO_Init(XMC_GPIO_PORT2,0, &rx_pin_config);

	/* GPIO Output pin configuration */
	tx_pin_config.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT7;
	XMC_GPIO_Init(XMC_GPIO_PORT1,0, &tx_pin_config);

	/* GPIO Slave Select line pin configuration */
	selo_pin_config.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT6;
	XMC_GPIO_Init(XMC_GPIO_PORT0,9, &selo_pin_config);

	/* GPIO Clock pin configuration */
	clk_pin_config.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT6;
	XMC_GPIO_Init(XMC_GPIO_PORT0,8, &clk_pin_config);
}

extern void interface_init(void);
extern void protocol_init(void);
extern void server_loop(void);

volatile uint32_t g_sp_vals[3];

int main(void){
	__IO uint32_t tmpTick;
	__IO int32_t tmpK;
	__IO int32_t tmpL;
	__IO uint32_t deltaTick;

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

	//	spi_init();

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

	//	interface_init();
	//	protocol_init();
	//
	//	printf("MAC Rev: 0x%02X\n", enc28j60getrev());
	//
	//	server_loop();

	while(1){
		HAL_Delay(2000);
		P0_5_toggle();
		//		P0_6_toggle();
		//		P1_4_toggle();
		//		P1_5_toggle();

		//		printf("ASM Test 1 Result:%u\n", asm_get_8bit_number());
		//		printf("ASM Test 2 Result:%08X\t[%08X]\n", asm_get_xor(0x12345678, 0x34567890), 0x12345678^0x34567890);
		//		printf("ASM Test 3 Direct Jump:%08X\n", TestFunct);
		//		printf("Jump 1, Before.%08X\n", __get_MSP());
		//		asm_direct_jump_1(TestFunct);
		//		printf("Jump 1, After.%08X\n\n", __get_MSP());
		//
		//		printf("Jump 2, Before.%08X\n", __get_MSP());
		//		asm_direct_jump_2(TestFunct);
		//		printf("Jump 2, After.%08X\n\n", __get_MSP());
		//
		//
		//		printf("ASM Test 4 :%u\t[%u]\n", asm_add2(34), 34+2);
		//		printf("ASM Test 5 :%u\t[%u]\n", asm_simple_add(123, 456), 123+456);
		//		printf("ASM Test 6 :%u\t[%u]\n", asm_pc_add(), 7);
		//
		//		printf("ASM Test 7 :%d\t[%d]\n", asm_sub20(34), 34-20);
		//		printf("ASM Test 8 :%d\t[%d]\n", asm_simple_sub(123, 456), 123-456);
		//		printf("ASM Test 9 :%d\t[%d]\n", asm_get_neg(1024), 0-1024);
		//
		//		printf("ASM Test 10 Result:%u\t[%u]\n", asm_simple_mul(123, 456), 123*456);
		//
		//		//Test Addition/Mulitiplication Cycles
		//#define	TEST_ADD_MUL_NUM	500000
		//		//If the multiplication takes similar cycles, it is a single cycle multiplication implementation
		//		uint32_t tmpTick = g_Ticks;
		//		for(uint32_t i=0; i<TEST_ADD_MUL_NUM; ++i)
		//		{
		//			uint32_t tn = 101;
		//			asm_simple_add(tn, 456);
		//		}
		//		tmpTick = g_Ticks-tmpTick;
		//		printf("A:%u\n", tmpTick);
		//
		//		tmpTick = g_Ticks;
		//		for(uint32_t i=0; i<TEST_ADD_MUL_NUM; ++i)
		//		{
		//			uint32_t tn = 101;
		//			asm_simple_mul(tn, 456);
		//		}
		//		tmpTick = g_Ticks-tmpTick;
		//		printf("M:%u\n", tmpTick);
		//
		//		//Test Division
		//		{
		//			uint32_t ta = 10;
		//			uint32_t tb = 2;
		//			uint32_t tc = ta/tb;
		//			printf("%u %u %u\n", ta, tb, tc);
		//		}
		//
		//		printf("ASM Test 11 Result:%u\t[%u]\n", asm_test_cmp(123, 456), (123==456));
		//		printf("ASM Test 12 Result:%u\t[%u]\n", asm_test_cmn(123, 456), (123!=456));
		//		printf("ASM Test 13 Result:%08X\t[%08X]\n", asm_get_and(0x12345678, 0x34567890), 0x12345678 & 0x34567890);
		//		printf("ASM Test 14 Result:%08X\t[%08X]\n", asm_get_or(0x12345678, 0x34567890), (0x12345678 | 0x34567890));
		//		printf("ASM Test 15 Result:%08X\t[%08X]\n", asm_get_not(0x12345678), 0-0x12345678);
		//
		//		uint32_t g_TestVar32 = 0x12345678;
		//		printf("ASM Test 20 Result:%08X\t[%08X]\n", asm_ldr32(&g_TestVar32), g_TestVar32);
		//		asm_str32(&g_TestVar32, 0x78904563);
		//		printf("ASM Test 21 Result:%08X\t[%08X]\n", asm_ldr32(&g_TestVar32), g_TestVar32);
		//		printf("ASM Test 22 Result:%u\t[%d]\n", asm_test_push_pop(123, 456), 123+456+456+4);
		//
		//		//Part 7: Test Extend, Reverse
		//		printf("Part 7\n");
		//		printf("ASM Test 23 Result:%08X\t[%08X]\n", asm_s16ext((int16_t)0x8001), (int32_t)0x8001);
		//		printf("ASM Test 24 Result:%08X\t[%08X]\n", asm_s8ext((int8_t)0xC4), (int32_t)0xC4);
		//		printf("ASM Test 25 Result:%08X\t[%08X]\n", asm_u16ext((uint16_t)0x8001), (uint32_t)0x8001);
		//		printf("ASM Test 26 Result:%08X\t[%08X]\n", asm_rev(0x123456C8), __REV(0x123456C8));
		//		printf("ASM Test 27 Result:%08X\t[%08X]\n", asm_rev16(0x123456C8), __REV16(0x123456C8));
		//		printf("ASM Test 28 Result:%08X\t[%08X]\n", asm_revsh(0x123456C8), __REVSH(0x123456C8));
		//
		//Part 8: Test SVC, MSR, MRS
		printf("Part 8\n");
		printf("ASM Test 29, Before SVC\n");
		g_sp_vals[0]=__get_MSP();
		//destroy the 8 byte alignment
		//		__ASM volatile ("MSR msp, %0\n" : : "r" (g_sp_vals[0]-4) : "sp");
//		__ASM volatile ("svc 1" : : : "memory");
		g_sp_vals[2]=__get_MSP();
		printf("sp vals.%08X %08X %08X\n\n", g_sp_vals[0], g_sp_vals[1], g_sp_vals[2]);
		//		asm_svc_1(1000);
		printf("After SVC\n");
		//
		//		printf("ASM Test 30 Result:%08X\n", asm_test_mrs());
		//		printf("ASM Test 31 Tick:%u\n", SysTick->VAL);
		//		asm_test_msr(0x00000001);
		//		uint32_t p1 = asm_test_mrs();
		//		asm_test_msr(0x00000000);
		//		uint32_t p2 = asm_test_mrs();
		//		printf("%08X\t%08X\n", p1, p2);
		//
		//bkpt when no debugger will cause hardfault
		printf("Before A breakpoint\n");
		__BKPT(10);
		printf("After breakpoint\n");

		//unaligned access
		uint8_t tmpU8A[]={0x12, 0x34, 0x56, 0x78};
		uint16_t* pU16 = (uint16_t*)&tmpU8A[0];
		printf("%p %04X\n", pU16, *pU16);
		pU16 = (uint16_t*)&tmpU8A[2];
		printf("%p %04X\n", pU16, *pU16);

		printf("Before Unaligned access\n");
		pU16 = (uint16_t*)&tmpU8A[1];
		printf("%p %04X\n", pU16, *pU16);
		printf("After Unaligned access\n");

		//• a system-generated bus error on a load or store
		uint32_t* pU32_NonExist = 0x60000000;
		printf("Before LDR non exist\n");
		printf("%08X\t[%08X]\n", asm_ldr32(pU32_NonExist), *pU32_NonExist);
		printf("After LDR non exist\n");
		printf("Before STR non exist\n");
		asm_str32(pU32_NonExist, 0x78904563);
		printf("%08X\t[%08X]\n", 0x78904563, pU32_NonExist);
		printf("After STR non exist\n");

		//• execution of an instruction from an XN memory address
		//• execution of an instruction when not in Thumb-State as a result of the T-bit being previously cleared to 0
		printf("Before exe non exist\n");
		asm_direct_jump_2(pU32_NonExist);
		printf("After exe non exist\n");

		printf("Before ram function\n");
		TestFunct();
		printf("After ram function\n");
		//• execution of an Undefined instruction
		uint32_t tmpU32 = ((uint32_t)TestFunct) - 1;
		asm_str32(tmpU32, 0x78904563);
		printf("%s\n", __func__);
		__ISB();
		__DSB();
		__DMB();
		tmpTick = g_Ticks;
		while((tmpTick+100) > g_Ticks){
			__NOP();
		}
		printf("Before ram function modified\n");
		TestFunct();
		printf("After ram function modified\n");
		//
		//		tmpTick = g_Ticks;
		//		while((tmpTick+2000) > g_Ticks){
		//			__NOP();
		//		}
		//		deltaTick = tmpTick * 2;
		//
		//		tmpK = XMC1000_CalcTemperature()-273;
		//		tmpL = tmpK * 10;
		//		printf("%i %i %i %i\n",
		//				tmpK, tmpL,
		//				tmpTick, deltaTick);
	}

	return 0;
}

void SysTick_Handler(void) {
	g_Ticks++;
}
