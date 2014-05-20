void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
//void write_mem8(int addr, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void)
{
	int i; /* i is an 32-bit integer */
	char *p; /* used for BYTE address */
	
	init_palette(); /* setting palette */
	
	p = (char *) 0xa0000; /* address */
	
	for (i = 0; i <= 0xffff; i++) {
		//write_mem8(i, i & 0x0f);
		//*i = i & 0x0f; // MOV [i], (i & 0x0f) // We do not know [i] is  BYTE, WORD or DWORD, so it is wrong.
		p[i] = i & 0x0f;
	}

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