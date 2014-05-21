#include "bootpack.h"

void init_pic(void)
/* initialize PIC */
{
	io_out8(PIC0_IMR,  0xff  ); /* disable all interrupts */
	io_out8(PIC1_IMR,  0xff  ); /* disable all interrupts */

	io_out8(PIC0_ICW1, 0x11  ); /* edge trigger mode */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7 received by INT20-27 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1 connected by IRQ2 */
	io_out8(PIC0_ICW4, 0x01  ); /* no buffer mode */

	io_out8(PIC1_ICW1, 0x11  ); /* edge trigger mode */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15 received by INT28-2f */
	io_out8(PIC1_ICW3, 2     ); /* PIC1 connected by IRQ2 */
	io_out8(PIC1_ICW4, 0x01  ); /* no buffer mode */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 except PIC1, disable all interrupts */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 disable all interrupts */

	return;
}

/* interrupt handler */

#define PORT_KEYDAT 0x0060

struct FIFO8 keyfifo;

void inthandler21(int *esp)
/* interrupt from PS/2 keyboard */
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61); // inform PIC that IRQ-01 has been handled
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
	return;
}