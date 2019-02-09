/*
 * asm_prototype.h
 *
 *  Created on: Jan 22, 2019
 *      Author: zzr
 */

#ifndef ASM_PROTOTYPE_H_
#define ASM_PROTOTYPE_H_

//The Cortex-M4 instruction set includes pairs of synchronization primitives. These provide a
//non-blocking mechanism that a thread or process can use to obtain exclusive access to a memory
//location. Software can use them to perform a guaranteed read-modify-write memory update
//sequence, or for a semaphore mechanism.

//A pair of synchronization primitives comprises:
//A Load-Exclusive instruction
//Used to read the value of a memory location, requesting exclusive access to that
//location.
//A Store-Exclusive instruction
//Used to attempt to write to the same memory location, returning a status bit to a
//register. If this bit is:
//0 It indicates that the thread or process gained exclusive access to the
//memory, and the write succeeds.
//1 It indicates that the thread or process did not gain exclusive access to
//the memory, and no write was performed.
//The pairs of Load-Exclusive and Store-Exclusive instructions are:
//• the word instructions LDREX and STREX
//• the halfword instructions LDREXH and STREXH
//• the byte instructions LDREXB and STREXB.
//Software must use a Load-Exclusive instruction with the corresponding Store-Exclusive
//instruction.
//To perform an exclusive read-modify-write of a memory location, software must:
//1. Use a Load-Exclusive instruction to read the value of the location.
//2. Modify the value, as required.
//3. Use a Store-Exclusive instruction to attempt to write the new value back to the memory
//location.
//4. Test the returned status bit. If this bit is:
//0 The read-modify-write completed successfully.
//1 No write was performed. This indicates that the value returned at step 1 might
//be out of date. The software must retry the entire read-modify-write sequence.
//Software can use the synchronization primitives to implement a semaphores as follows:
//1. Use a Load-Exclusive instruction to read from the semaphore address to check whether
//the semaphore is free.
//2. If the semaphore is free, use a Store-Exclusive to write the claim value to the semaphore
//address.
//3. If the returned status bit from step 2 indicates that the Store-Exclusive succeeded then the
//software has claimed the semaphore. However, if the Store-Exclusive failed, another
//process might have claimed the semaphore after the software performed step 1.
//The Cortex-M4 includes an exclusive access monitor, that tags the fact that the processor has
//executed a Load-Exclusive instruction. If the processor is part of a multiprocessor system, the
//system also globally tags the memory locations addressed by exclusive accesses by each
//processor.
//The processor removes its exclusive access tag if:
//• It executes a CLREX instruction
//• It executes a Store-Exclusive instruction, regardless of whether the write succeeds.
//
//• An exception occurs. This means the processor can resolve semaphore conflicts between
//different threads.
//In a multiprocessor implementation:
//• executing a CLREX instruction removes only the local exclusive access tag for the processor
//• executing a Store-Exclusive instruction, or an exception. removes the local exclusive
//access tags, and global exclusive access tags for the processor.
#include <stdint.h>
#include <stdbool.h>

typedef union union_pack16
{
	uint16_t u16;
	int16_t i16;
	uint8_t u8[2];
	int8_t i8[2];
}pack16;

typedef union union_pack32
{
	uint32_t u32;
	int32_t i32;
	uint16_t u16[2];
	int16_t i16[2];
	uint8_t u8[4];
	int8_t i8[4];
}pack32;

typedef union union_pack64
{
	uint64_t u64;
	int64_t i64;
	uint32_t u32[2];
	int32_t i32[2];
	uint16_t u16[4];
	int16_t i16[4];
	uint8_t u8[8];
	int8_t i8[8];
}pack64;

uint32_t asm_get_8bit_number(void);
uint32_t asm_get_xor(uint32_t in, uint32_t key);
void asm_direct_jump_1(void(*fptr)(void));
void asm_direct_jump_2(void(*fptr)(void));

uint32_t asm_add2(uint32_t in);
uint32_t asm_simple_add(uint32_t i1, uint32_t i2);
uint32_t asm_pc_add(void);

int32_t asm_sub20(int32_t in);
int32_t asm_simple_sub(int32_t i1, int32_t i2);
int32_t asm_get_neg(int32_t in);

uint32_t asm_simple_mul(uint32_t i1, uint32_t i2);
uint32_t asm_test_cmp(uint32_t i1, uint32_t i2);
uint32_t asm_test_cmn(uint32_t i1, uint32_t i2);
uint32_t asm_get_and(uint32_t in, uint32_t key);
uint32_t asm_get_or(uint32_t in, uint32_t key);
int32_t asm_get_not(int32_t in);

uint32_t asm_logic_left(uint32_t in, uint32_t key);
uint32_t asm_logic_right(uint32_t in, uint32_t key);
int32_t asm_arithm_right(int32_t in, uint32_t key);
uint32_t asm_rotate_right(uint32_t in, uint32_t key);

uint32_t asm_ldr32(uint32_t* addr);
uint32_t asm_str32(uint32_t* addr, uint32_t v);
uint32_t asm_test_push_pop(uint32_t i1, uint32_t i2);

int32_t asm_s16ext(int16_t in);
int32_t asm_s8ext(int8_t in);
int32_t asm_u16ext(uint16_t in);
uint32_t asm_rev(uint32_t in);
uint32_t asm_rev16(uint32_t in);
uint32_t asm_revsh(uint32_t in);

void asm_svc_1(uint32_t in);
void asm_svc_10(uint32_t in);
void asm_test_msr(uint32_t in);
uint32_t asm_test_mrs(void);

uint32_t asm_usat(uint32_t in);
pack32 asm_usat16(pack32 in);
int32_t asm_ssat(int32_t in);
pack32 asm_ssat16(pack32 in);
uint32_t asm_usat(uint32_t in);
int32_t asm_qadd(int32_t inA, int32_t inB);
int32_t asm_qsub(int32_t inA, int32_t inB);
pack32 asm_uqadd16(pack32 inA, pack32 inB);
pack32 asm_uqsub16(pack32 inA, pack32 inB);

uint32_t asm_test_tbb(uint8_t inA);
uint32_t asm_test_tbh(uint16_t inA);

uint32_t asm_simple_udiv(uint32_t i1, uint32_t i2);
int32_t asm_simple_sdiv(int32_t i1, int32_t i2);

float asm_vabs(float inFA);
float asm_vsqrt(float inFA);
float asm_vadd(float inFA, float inFB);
float asm_vsub(float inFA, float inFB);
uint32_t asm_vcmp(float inFA, float inFB);

int32_t asm_vcvt_s32(float inFA);
uint32_t asm_vcvt_u32(float inFA);
float asm_vmul(float inFA, float inFB);
float asm_vdiv(float inFA, float inFB);

float asm_vfma(float inFA, float inFB, float inFC);
float asm_vfms(float inFA, float inFB, float inFC);
float asm_vmla(float inFA, float inFB, float inFC);
float asm_vmls(float inFA, float inFB, float inFC);

#endif /* ASM_PROTOTYPE_H_ */
