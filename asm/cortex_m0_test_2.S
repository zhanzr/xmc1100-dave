//*****************************************************************************
// File: cortex_m0_test_2.S
// Compiler: arm-none-eabi-gcc
//******************************************************************************
//Description: Instruction Test 2
//Add
//Operation				Description Assembler	Cycles
//3-bit immediate 		ADDS Rd, Rn, #<imm> 	1
//All registers Lo 		ADDS Rd, Rn, Rm 		1
//Any to Any 			ADD Rd, Rd, Rm 			1
//Any to PC 				ADD PC, PC, Rm 			3
//8-bit immediate 		ADDS Rd, Rd, #<imm> 	1
//With carry 			ADCS Rd, Rd, Rm 		1
//Immediate to SP 		ADD SP, SP, #<imm> 		1
//Form address from SP 	ADD Rd, SP, #<imm> 		1
//Form address from PC 	ADR Rd, <label> 		1
    .cpu cortex-m0
    .syntax unified
    .thumb

    .file   "cortex_m0_test_2.S"

    .section ".text"

    .global asm_add2
    .global asm_simple_add
    .global asm_pc_add

#define inputA          r0
#define inputB          r1
#define result          r0

    .align 4
//Add 3bit immediate
asm_add2:
	ADDS r0, r0, #2
	bx lr
	
//Simple Add
asm_simple_add:
	ADD r0, r1, r0
	bx lr
	
//PC Add
asm_pc_add:
	MOVS r0, #6
	//Cortex M0 has 3 levels of pipeline, when this instruction is to be executed, the PC points to test_label
	//Cortex M4F has 3 levels of pipeline, when this instruction is to be executed, the PC points to test_label
	ADD PC, PC, r0
	MOVS r0, #1

test_label:
	bx lr
	
	MOVS r0, #5
	bx lr
	
	MOVS r0, #7
	bx lr
	
	.end
