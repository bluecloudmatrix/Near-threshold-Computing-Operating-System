#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo;

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	
	char s[40], mcursor[256], keybuf[32];
	int mx, my, i;
	
	mx = (binfo->scrnx - 16) / 2;  
	my = (binfo->scrny - 28 - 16) / 2;

	init_gdtidt();
	
	init_pic();
	
	io_sti();
	
	fifo8_init(&keyfifo, 32, keybuf);
	
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
		//io_hlt();
		io_cli();
		if (fifo8_status(&keyfifo) == 0) {
			io_stihlt();
		} else {
			i = fifo8_get(&keyfifo);
			io_sti();
			sprintf(s, "%02X", i);
			boxfill8(binfo->vram, binfo->scrnx, COL8_0000FF, 0, 16, 15, 31);
			putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		}
	}
}