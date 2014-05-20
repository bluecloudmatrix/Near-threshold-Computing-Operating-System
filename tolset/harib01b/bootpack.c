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

struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

void HariMain(void)
{
	//int i; /* i is an 32-bit integer */
	//char *vram; /* used for BYTE address */
	//int xsize, ysize;
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	
	//extern char hankaku[4096];
	/*static char font_A[16] = {
		0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
		0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
	};*/
	
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
	
	for (;;) {
		io_hlt();
	}
}

void init_palette(void)
{
	/* char a[3]; 
	
	a:
		RESB 3
	
	*/
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,   /* 0:black */
		0xff, 0x00, 0x00,	/*  1:bright red */
		0x00, 0xff, 0x00,	/*  2:bright green */
		0xff, 0xff, 0x00,	/*  3:bright yellow */
		0x00, 0x00, 0xff,	/*  4:bright blue */
		0xff, 0x00, 0xff,	/*  5:bright purple */
		0x00, 0xff, 0xff,	/*  6:shallow bright blue */
		0xff, 0xff, 0xff,	/*  7:white */
		0xc6, 0xc6, 0xc6,	/*  8:bright grey */
		0x84, 0x00, 0x00,	/*  9:dark red */
		0x00, 0x84, 0x00,	/* 10:dark green */
		0x84, 0x84, 0x00,	/* 11:dark yellow */
		0x00, 0x00, 0x84,	/* 12:dark blue */
		0x84, 0x00, 0x84,	/* 13:dark purple */
		0x00, 0x84, 0x84,	/* 14:shallow dark blue */
		0x84, 0x84, 0x84	/* 15:dark grey */
	};
	
	set_palette(0, 15, table_rgb);
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags(); /* record the value of interrupt enable */
	io_cli(); /* set eflags to 0, disable interrupt */
	io_out8(0x03c8, start); /* send data to specific device */ /* 0x03c8 represents the palette index register, used for write */
	for(i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4); /* 0x03c8 represents the palette data register, saving red, green and blue */
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags); /* recover eflags */
	return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for(y = y0; y <= y1; y++) {
		for(x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}

void init_screen(char *vram, int x, int y)
{
	/* background */
	boxfill8(vram, x, COL8_0000FF, 0, 0         , x - 1, y - 29);
	boxfill8(vram, x, COL8_FFFFFF, 0, y - 28, x - 1, y - 28);
	boxfill8(vram, x, COL8_C6C6C6, 0, y - 27, x - 1, y - 27);
	boxfill8(vram, x, COL8_FFFFFF, 0, y - 26, x - 1, y - 1);
	
	/* left down */
	boxfill8(vram, x, COL8_FFFFFF,  3,         y - 24, 59,         y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,         y - 24,  2,         y -  4);
	boxfill8(vram, x, COL8_848484,  3,         y -  4, 59,         y -  4);
	boxfill8(vram, x, COL8_848484, 59,         y - 23, 59,         y -  5);
	boxfill8(vram, x, COL8_000000,  2,         y -  3, 59,         y -  3);
	boxfill8(vram, x, COL8_000000, 60,         y - 24, 60,         y -  3);
	
	return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i; 
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) p[0] = c;
		if ((d & 0x40) != 0) p[1] = c;
		if ((d & 0x20) != 0) p[2] = c;
		if ((d & 0x10) != 0) p[3] = c;
		if ((d & 0x08) != 0) p[4] = c;
		if ((d & 0x04) != 0) p[5] = c;
		if ((d & 0x02) != 0) p[6] = c;
		if ((d & 0x01) != 0) p[7] = c;
	}
	return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];
	for (; *s != 0x00; s++) {
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}