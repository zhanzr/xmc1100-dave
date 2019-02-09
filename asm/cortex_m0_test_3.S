//*****************************************************************************
// File: cortex_m0_test_3.S
// Compiler: arm-none-eabi-gcc
//******************************************************************************
//Description: Instruction Test 3
//Subtract
//Operation				Description Assembler	Cycles
//Lo and Lo 				SUBS Rd, Rn, Rm 		1
//3-bit immediate 		SUBS Rd, Rn, #<imm> 	1
//8-bit immediate 		SUBS Rd, Rd, #<imm> 	1
//With carry 			SBCS Rd, Rd, Rm 		1
//Immediate from SP 		SUB SP, SP, #<imm> 		1
//Subtract Negate 		RSBS Rd, Rn, #0 		1
    .cpu cortex-m0
    .syntax unified
    .thumb

    .file   "cortex_m0_test_3.S"

    .section ".text"

    .global asm_sub20
    .global asm_simple_sub
    .global asm_get_neg

#define inputA          r0
#define inputB          r1
#define result          r0

    .align 4

asm_sub20:
	SUBS r0, r0, #20
	bx lr
	
//Simple Sub
	
asm_simple_sub:
	SUBS r0, r0, r1
	bx lr
	
//Subtract Negate
asm_get_neg:
    RSBS r1, r0, #0
	movs r0, r1
	bx lr
	
	.end
