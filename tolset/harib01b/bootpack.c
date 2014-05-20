#include <stdio.h>

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
//void write_mem8(int addr, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int x, int y);

void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);

void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize);

struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

/* GDT/IDT */

struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void init_gdtidt(void);


void HariMain(void)
{
	//int i; /* i is an 32-bit integer */
	//char *vram; /* used for BYTE address */
	//int xsize, ysize;
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	
	char s[40], mcursor[256];
	int mx, my;
	
	mx = (binfo->scrnx - 16) / 2;  
	my = (binfo->scrny - 28 - 16) / 2;
	
	//extern char hankaku[4096];
	/*static char font_A[16] = {
		0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
		0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
	};*/
	
	//init_gdtidt();
	
	init_palette(); /* setting palette */
	
	/*
	binfo = (struct BOOTINFO *) 0x0ff0;
	xsize = (*binfo).scrnx;
	ysize = (*binfo).scrny;
	vram = (*binfo).vram;
	*/
	
	/*
	binfo_scrnx = (short *) 0x0ff4; // address
	binfo_scrny = (short *) 0x0ff6;
	binfo_vram = (int *) 0x0ff8;
	xsize = *binfo_scrnx;
	ysize = *binfo_scrny;
	vram = (char *) *binfo_vram;
	*/
	
	/*
	vram = (char *) 0xa0000; // address VRAM
	xsize = 320;
	ysize = 200;
	*/
	
	/*for (i = 0; i <= 0xffff; i++) {
		//write_mem8(i, i & 0x0f);
		//*i = i & 0x0f; // MOV [i], (i & 0x0f) // We do not know [i] is  BYTE, WORD or DWORD, so it is wrong.
		p[i] = i & 0x0f;
	}*/
	
	//boxfill8(p, 320, COL8_00FFFF, 20, 20, 120, 120);
	
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	//putfont8(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, font_A);
	/*
	putfont8(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, hankaku + 'H' * 16);
	putfont8(binfo->vram, binfo->scrnx, 16, 8, COL8_FFFFFF, hankaku + 'K' * 16);
	putfont8(binfo->vram, binfo->scrnx, 24, 8, COL8_FFFFFF, hankaku + 'U' * 16);
	*/
	
	putfonts8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "Hong Kong University, I come!");
	
	sprintf(s, "scrnx = %d", binfo->scrnx);
	
	putfonts8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, s);
	
	init_mouse_cursor8(mcursor, COL8_0000FF); //prepare mouse cursor
	
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); // show cursor
	
	for (;;) {
		io_hlt();
	}
}