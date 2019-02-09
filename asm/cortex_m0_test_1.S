//*****************************************************************************
// File: cortex_m0_test_1.S
// Compiler: arm-none-eabi-gcc
//******************************************************************************
//Description: Instruction Test 1
//Move
//Operation              Description Assembler   Cycles
//8-bit immediate        MOVS Rd, #<imm>         1
//Lo to Lo               MOVS Rd, Rm             1
//Any to Any             MOV Rd, Rm              1
//Any to PC              MOV PC, Rm              3
    .cpu cortex-m0
    .syntax unified
    .thumb

    .file   "cortex_m0_test_1.S"

    .section ".text"

    .global asm_get_8bit_number
    .global asm_get_xor
    .global asm_direct_jump_1
    .global asm_direct_jump_2

#define inputA          r0
#define inputB          r1
#define result          r0

    .align 4

//Get an 8bit immediate number
asm_get_8bit_number:
    MOVS result, #127
    bx lr

//Get the XOR result
asm_get_xor:
    eors inputB, inputA, inputB
    movs result, inputB
    bx lr

//Direct Jump Test 1
asm_direct_jump_1:
//The following line will get warning:
//MOV pc,<rn> instruction used, but BX <rn> is preferred
    mov PC,inputA

//Direct Jump Test 2
asm_direct_jump_2:
    bx inputA

    .end
