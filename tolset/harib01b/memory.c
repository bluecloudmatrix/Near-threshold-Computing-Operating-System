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

/* memory management */


void memman_init(struct MEMMAN *man)
{
	man->frees = 0;              // the numbers of free memory 
	man->maxfrees = 0;           // the maximum of frees 
	man->lostsize = 0;           // the total of memory that free unsuccessfully 
	man->losts = 0;              // the number of free failure
	return;
}

// count the total of free memory
unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

// allocate
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			// find enough free memory
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			// at the moment man->free[i].size=0, adjust man->free again
			if (man->free[i].size == 0) {
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1];
				}
			}
			return 0;
		}
	}
	return 0; // there is no free memory 
}

// important: the free of memory
// we need to consider the problem of merge and adjust man->free again
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	// we organize man->free according the order of address
	// so first we should find the index of man->free to record the free memory 
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;  // find the index to record the memory 
		}
	}
	// free[i-1].addr < addr < free[i].addr 
	if (i > 0) {		
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			// the new free memory can merge with free[i-1]
			man->free[i - 1].size += size;
			if (i < man->frees) {
				if (addr + size == man->free[i].addr) {
					// the new free memory can merge with free[i]
					man->free[i - 1].size += man->free[i].size;
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1];
					}
				}
			}
			return 0;
		}
	}
	
	if (i < man->frees) {
		if (addr + size == man->free[i].addr) {
			// the new free memory can merge with free[i]
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}
	
	// cannot merge with rear or front
	if (man->frees < MEMMAN_FREES) {
		for (j = man->frees; j > i; j--) {      // interesting move 
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	
	// beyond the maximum of free array
	man->losts++;
	man->lostsize += size;
	return -1;
}

// 4K
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}