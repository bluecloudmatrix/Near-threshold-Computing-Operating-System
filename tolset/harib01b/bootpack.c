#include <stdio.h>
#include "bootpack.h"

#define MEMMAN_ADDR		0x003c0000

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);

void HariMain(void)
{
	// preparing all kinds of variables
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo; // shared FIFO buffer
	int fifobuf[128];
	char s[40];
	struct TIMER *timer, *timer2, *timer3;
	int mx, my, i;
	struct MOUSE_DEC mdec;
	unsigned int memtotal, count = 0;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, buf_mouse[256], *buf_win;

	
	// interrupt setting
	init_gdtidt();
	init_pic();	
	io_sti();	// The initialize of IDT/PIC has been over, so relieve the ban of CPU interrupt
	init_pit(); // about timer
	
	// shared FIFO buffer setting
	fifo32_init(&fifo, 128, fifobuf);            // used for receiving interrupt information
	init_keyboard(&fifo, 256);	
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); // PIT and PIC1 and keyboard are set to permission: 11111000
	io_out8(PIC1_IMR, 0xef); // Mouse is set to permission: 11101111 

	// memory operation
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); // 0x00001000 - 0x0009efff 
	memman_free(memman, 0x00400000, memtotal - 0x00400000);	

	// sheet control
	init_palette(); // setting palette 
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny); // setting sheets control structure
	
	// build background sheet
	sht_back  = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); 
	init_screen8(buf_back, binfo->scrnx, binfo->scrny); // initialize the show of screen
	sheet_slide(sht_back, 0, 0);
	sheet_updown(sht_back,  0);
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	putfonts8_asc(buf_back, binfo->scrnx, 8, 56, COL8_FFFFFF, "Hong Kong University");
	
	// build mouse sheet
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_mouse_cursor8(buf_mouse, 99);        // initialize the show of mouse
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_mouse, 2);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	
	// build a window
	sht_win = sheet_alloc(shtctl);
	buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 160, 52, -1); //There is no transparent color
	make_window8(buf_win, 160, 52, "counter");
	//putfonts8_asc(buf_win, 160, 24, 28, COL8_000000, "Welcome to");
	//putfonts8_asc(buf_win, 160, 24, 44, COL8_000000, "Seer OS!");
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_win, 1);
	
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, 80);  // show the vram

	
	// timer buf
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);
	
	for (;;) {
		count++; // high-speed counting to test the performance 
		//sprintf(s, "%010d", count);
		
		//sprintf(s, "%010d", timerctl.count);
		//boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
		//putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, s);
		//sheet_refresh(sht_win, 40, 28, 120, 44);
	
		//io_hlt(); // in order to count in a high speed, do not use htl to let CPU sleep
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			//io_stihlt();
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) { // keyboard data
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_0000FF, s, 2);
			} else if (512 <= i && i <= 767) { // mouse data
				if (mouse_decode(&mdec, i - 512) != 0) {
					// there are three bytes that have been received, so showing them
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

					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_0000FF, s, 15);
					
					// move mouse cursor
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					
					// when mouse moves out of the screen, it could be hidden
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					
					sprintf(s, "(%3d, %3d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_0000FF, s, 15);
					sheet_slide(sht_mouse, mx, my);
				}
			} else if (i == 10) { // the 10-second timer
				putfonts8_asc_sht(sht_back, 0, 84, COL8_FFFFFF, COL8_0000FF, "10[sec]", 7);
				// testing the performance, showing the value of count after 10 seconds
				sprintf(s, "%010d", count);
				putfonts8_asc_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, s, 10);	
			} else if (i == 3) { // the 3-second timer
				putfonts8_asc_sht(sht_back, 0, 104, COL8_FFFFFF, COL8_0000FF, "3[sec]", 7);
				count = 0; 	// for testing the performance, after 3 seconds, start testing, because the initialization needs some time, 
							// we do the count after initialization to decrease error.
			} else if (i == 1) { // the cursor timer
				timer_init(timer3, &fifo, 0);
				boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 124, 15, 143);
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 124, 16, 144);
			} else if (i == 0) { // the cursor timer
				timer_init(timer3, &fifo, 1);
				boxfill8(buf_back, binfo->scrnx, COL8_0000FF, 8, 124, 15, 143);
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 124, 16, 144);
			}
		}
	}
}

// paint a window
void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15); // paint background color
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s); // write characters
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}