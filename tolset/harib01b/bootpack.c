#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	struct MOUSE_DEC mdec;
	
	mx = (binfo->scrnx - 16) / 2;  
	my = (binfo->scrny - 28 - 16) / 2;

	init_gdtidt();
	
	init_pic();
	
	init_keyboard();
	
	io_sti();
	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	
	io_out8(PIC0_IMR, 0xf9); /* (11111001) */
	io_out8(PIC1_IMR, 0xef); /* (11101111) */
	
	init_palette(); /* setting palette */
	
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	putfonts8_asc(binfo->vram, binfo->scrnx, 8, 56, COL8_FFFFFF, "Hong Kong University, I come!");
	init_mouse_cursor8(mcursor, COL8_0000FF); //prepare mouse cursor
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); // show cursor
	//sprintf(s, "scrnx = %d", binfo->scrnx);	
	//putfonts8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, s);

	enable_mouse(&mdec);
	
	for (;;) {
		//io_hlt();
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if(fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_0000FF, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {
				/*i = fifo8_get(&mousefifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_0000FF, 32, 16, 47, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);*/
				
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
				
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_0000FF, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					
					// move mouse cursor
					boxfill8(binfo->vram, binfo->scrnx, COL8_0000FF, mx, my, mx + 15, my + 15); // hide mouse
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_0000FF, 0, 0, 79, 15); // hide coordinate
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); // show coordinate
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); // draw mouse
				}
			}
		}
	}
}

/* memory checking */

unsigned int memtest_sub(unsigned int start, unsigned int end)
{
	unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
	for (i = start; i <= end; i += 4) {
		p = (unsigned int *) i;
		old = *p;
		*p = pat0;
		*p ^= 0xffffffff;
		if (*p != pat1) {
not_memory:
			*p = old;
			break;
		}
		*p ^= 0xffffffff;
		if (*p != pat0) {
			goto not_memory;
		}
		*p = old;
	}
	return i;
}

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
	
	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; // enable cache
		store_cr0(cr0);
	}
	
	return i;
}
