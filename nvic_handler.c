/*
 * nvic_handler.c
 *
 *  Created on: Jan 12, 2019
 *      Author: zzr
 */
#include <xmc1100.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************/
static void delay(uint32_t cycles)
{
  volatile uint32_t i;

  for(i = 0UL; i < cycles ;++i) {
    __NOP();
  }
}

void HardFault_Handler(uint32_t lr, uint32_t msp, uint32_t psp){
	__ASM volatile ("nop");
//	printf("%s\n", __func__);
//	__ISB();
//	__DSB();
//	__DMB();
//	delay(1000);

//	while(1){
//		;
//	}
}

void SVC_Handler(void){
	printf("%s\n", __func__);
	__ISB();
	__DSB();
	__DMB();
	delay(1000);

	//Cause a hardfault
	__ASM volatile ("svc 0" : : : "memory");
}

void DebugMon_Handler(void){
	;
}

void PendSV_Handler(void){
	;
}


#if       (__CORTEX_M >= 0x03U)
void NMI_Handler(void){
	;
}

void MemManage_Handler(void){
	;
}

void BusFault_Handler(void){
	;
}

void UsageFault_Handler(void){
	;
}
#endif

void SCU_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void ERU0_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void ERU0_1_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void ERU0_2_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void ERU0_3_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void ERU1_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void ERU1_1_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void ERU1_2_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void ERU1_3_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void PMU0_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_C0_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

//void VADC0_C0_1_IRQHandler(void){
//	printf(__func__);
//	while(1)
//	{;}
//}

void VADC0_C0_2_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_C0_3_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G0_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G0_1_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G0_2_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G0_3_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G1_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G1_1_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G1_2_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G1_3_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G2_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G2_1_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G2_2_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G2_3_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}


void VADC0_G3_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G3_1_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}


void VADC0_G3_2_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void VADC0_G3_3_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void CCU40_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void CCU40_1_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void CCU40_2_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}


void CCU40_3_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void CCU41_0_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void CCU41_1_IRQHandler(void){
	printf(__func__);
	while(1)
	{;}
}

void CCU41_2_IRQHandler(void)
{
	;
}

void CCU41_3_IRQHandler(void)
{
	;
}

void CCU42_0_IRQHandler(void)
{
	;
}

void CCU42_1_IRQHandler(void)
{
	;
}

void CCU42_2_IRQHandler(void)
{
	;
}

void CCU42_3_IRQHandler(void)
{
	;
}

void CCU43_0_IRQHandler(void)
{
	;
}

void CCU43_1_IRQHandler(void)
{
	;
}

void CCU43_2_IRQHandler(void)
{
	;
}

void CCU43_3_IRQHandler(void)
{
	;
}

void CCU80_0_IRQHandler(void)
{
	;
}

void CCU80_1_IRQHandler(void)
{
	;
}

void CCU80_2_IRQHandler(void)
{
	;
}

void CCU80_3_IRQHandler(void)
{
	;
}

void CCU81_0_IRQHandler(void)
{
	;
}

void CCU81_1_IRQHandler(void)
{
	;
}

void CCU81_2_IRQHandler(void)
{
	;
}

void CCU81_3_IRQHandler(void)
{
	;
}

void POSIF0_0_IRQHandler(void)
{
	;
}

void POSIF0_1_IRQHandler(void)
{
	;
}

void POSIF1_0_IRQHandler(void)
{
	;
}

void POSIF1_1_IRQHandler(void)
{
	;
}

void USIC0_2_IRQHandler(void)
{
	;
}

void USIC0_3_IRQHandler(void)
{
	;
}

void USIC0_4_IRQHandler(void)
{
	;
}

void USIC0_5_IRQHandler(void)
{
	;
}

void USIC1_0_IRQHandler(void)
{
	;
}

void USIC1_1_IRQHandler(void)
{
	;
}

void USIC1_2_IRQHandler(void)
{
	;
}

void USIC1_3_IRQHandler(void)
{
	;
}

void USIC1_4_IRQHandler(void)
{
	;
}

void USIC1_5_IRQHandler(void)
{
	;
}

void USIC2_0_IRQHandler(void)
{
	;
}

void USIC2_1_IRQHandler(void)
{
	;
}

void USIC2_2_IRQHandler(void)
{
	;
}

void USIC2_3_IRQHandler(void)
{
	;
}

void USIC2_4_IRQHandler(void)
{
	;
}

void USIC2_5_IRQHandler(void)
{
	;
}
