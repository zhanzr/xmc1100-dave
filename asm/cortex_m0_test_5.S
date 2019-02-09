//*****************************************************************************
// File: cortex_m0_test_5.S
// Compiler: arm-none-eabi-gcc
//******************************************************************************
//Description: Instruction Test 5
//Shift, Rotate
//Operation							Description Assembler	Cycles
//Logical shift left by immediate 	LSLS Rd, Rm, #<shift> 	1
//Logical shift left by register 	LSLS Rd, Rd, Rs 		1
//Logical shift right by immediate 	LSRS Rd, Rm, #<shift> 	1
//Logical shift right by register 	LSRS Rd, Rd, Rs 		1
//Arithmetic shift right 			ASRS Rd, Rm, #<shift> 	1
//Arithmetic shift right by register ASRS Rd, Rd, Rs 		1
//Rotate right by register 			RORS Rd, Rd, Rs 		1
    .cpu cortex-m0
    .syntax unified
    .thumb

    .file   "cortex_m0_test_5.S"

    .section ".text"

    .global asm_logic_left
    .global asm_logic_right
    .global asm_arithm_right
    .global asm_rotate_right

#define inputA          r0
#define inputB          r1
#define result          r0

    .align 4
//Logical shift left by register
asm_logic_left:
	LSLS r0, r0, r1
	bx lr
	
//Logical shift right by register
asm_logic_right:
	LSRS r0, r0, r1
	bx lr
				
//Arithmetic shift right by register
asm_arithm_right:
	ASRS r0, r0, r1
	bx lr
	
//Rotate right by register
asm_rotate_right:
	RORS r0, r0, r1
	bx lr

	.end
