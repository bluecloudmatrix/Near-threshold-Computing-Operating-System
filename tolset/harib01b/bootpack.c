#include <stdio.h>
#include "bootpack.h"

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	
	char s[40], mcursor[256];
	int mx, my;
	
	mx = (binfo->scrnx - 16) / 2;  
	my = (binfo->scrny - 28 - 16) / 2;

	init_gdtidt();
	
	init_pic();
	
	io_sti();
	
	init_palette(); /* setting palette */
	
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	putfonts8_asc(binfo->vram, binfo->scrnx, 8, 56, COL8_FFFFFF, "Hong Kong University, I come!");
	
	//sprintf(s, "scrnx = %d", binfo->scrnx);
	
	//putfonts8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, s);
	
	init_mouse_cursor8(mcursor, COL8_0000FF); //prepare mouse cursor
	
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); // show cursor
	
	io_out8(PIC0_IMR, 0xf9); /* PIC1 (11111001) */
	io_out8(PIC1_IMR, 0xef); /* (11101111) */
	
	for (;;) {
		io_hlt();
	}
}