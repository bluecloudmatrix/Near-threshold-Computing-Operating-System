#include "bootpack.h"

/*
unsigned int memtest_sub(unsigned int start, unsigned int end)
{
	unsigned int i;
	for (i = start; i <= end; i += 0x1000) {}
	return i;
}
*/

/*unsigned int memtest_sub(unsigned int start, unsigned int end)
{
	unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
	//for (i = start; i <= end; i += 4) {
	//	p = (unsigned int *) i;
	for (i = start; i <= end; i+= 0x1000) {
		p = (unsigned int *) (i + 0xffc);
		old = *p;
		*p = pat0; // every time we check 4 bytes
		
		// When get the data in p, some machines will read pat0 not *p, so we reverse
		*p ^= 0xffffffff;
		if (*p != pat1) {
not_memory:
			*p = old;
			break;
		}
		*p ^= 0xffffffff;
		//
		if (*p != pat0) {
			goto not_memory;
		}
		*p = old;
	}
	return i;
}
*/

/* memory checking */

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;
	//verify CPU is 386 or 486(as well as later)
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; //AC-bit = 1
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { // If 386, even though set AC=1, AC will be back to 0 automatically
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; // AC-bit = 0
	io_store_eflags(eflg);
	
	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; // disable cache
		store_cr0(cr0);
	}
	
	i = memtest_sub(start, end); // realize memory checking

	//test
	//struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	//char s[40];
	//sprintf(s, "%d", i);
	//putfonts8_asc(binfo->vram, binfo->scrnx, 0, 80, COL8_FFFFFF, s);
	//
	
	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; // enable cache
		store_cr0(cr0);
	}
	
	return i;
}
