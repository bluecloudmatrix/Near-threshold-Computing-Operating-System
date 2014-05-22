#include "bootpack.h"

#define PORT_KEYSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

struct FIFO8 keyfifo;

void wait_KBC_sendready(void)
/* waiting for ready of KBC*/
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	/* initialize KBC */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

void inthandler21(int *esp)
/* interrupt from PS/2 keyboard */
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61); // inform PIC that IRQ-01 has been handled
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
	return;
}