/*
 * startup_xmc1100.c
 *
 *  Created on: Sep 13, 2019
 *      Author: Administrator
 */

#include <stdint.h>

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

extern int main(void);

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

