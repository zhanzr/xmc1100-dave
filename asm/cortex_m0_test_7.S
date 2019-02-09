//*****************************************************************************
// File: cortex_m0_test_7.S
// Compiler: arm-none-eabi-gcc
//******************************************************************************
//Description: Instruction Test 7
//Extend, Reverse
//Operation                          Description Assembler   Cycles
//Signed halfword to word            SXTH Rd, Rm             1
//Signed byte to word                SXTB Rd, Rm             1
//Unsigned halfword                  UXTH Rd, Rm             1
//Unsigned byte                      UXTB Rd, Rm             1
//Bytes in word                      REV Rd, Rm              1
//Bytes in both halfwords            REV16 Rd, Rm            1
//Signed bottom half word            REVSH Rd, Rm            1
    .cpu cortex-m0
    .syntax unified
    .thumb

    .file   "cortex_m0_test_7.S"

    .section ".text"

    .global asm_s16ext
    .global asm_s8ext
    .global asm_u16ext
    .global asm_u8ext
    .global asm_rev
    .global asm_rev16
    .global asm_revsh

#define inputA          r0
#define inputB          r1
#define result          r0

    .align 4

//Signed halfword to word
asm_s16ext:
    SXTH r0, r0
    bx lr

//Signed byte to word
asm_s8ext:
    SXTB r0, r0
    bx lr

//Unsigned halfword to word
asm_u16ext:
    UXTH r0, r0
    bx lr

//Unsigned byte to word
asm_u8ext:
    UXTB r0, r0
    bx lr

//Reverse Bytes in word
asm_rev:
    REV r0, r0
    bx lr

//Reverse Bytes in both halfwords
asm_rev16:
    REV16 r0, r0
    bx lr

//Reverse Signed bottom half word
asm_revsh:
    REVSH r0, r0
    bx lr

    .end
