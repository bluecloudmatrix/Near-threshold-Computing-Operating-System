#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;

void inthandler2c(int *esp)
/* PS/2 mouse */
{
	int data;
	io_out8(PIC1_OCW2, 0x64); // inform the information which IRQ-12 has finished receiving signal to PIC1
	io_out8(PIC0_OCW2, 0x62); //inform the information which IRQ-02 has finished receiving signal to PIC0
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data + mousedata0);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	// the information of FIFO is saved in global variables
	mousefifo = fifo;
	mousedata0 = data0;
	
	/* activate mouse */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	// if things go well, ACK(0xfa) will be sent
	mdec->phase = 0; // waiting for mouse's 0xfa
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		// waiting for mouse
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		// first byte from mouse
		if ((dat & 0xc8) == 0x08) {
			/* if right */
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		// second byte from mouse
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		// third byte from mouse
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; /* the direction of y is opposite to the screen */
		return 1;
	}
	return -1; /* It should not come here */
}