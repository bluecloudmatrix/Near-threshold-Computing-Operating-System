#include "bootpack.h"

struct TIMERCTL timerctl;

// the interrupt of IRQ0
void init_pit(void)
{
	io_out8(PIT_CTRL, 0x34);
	//0x2e9c indicates that the frequency of interrupt is 100HZ
	io_out8(PIT_CNT0, 0x9c); // lower eight bits of interrupt cycle
	io_out8(PIT_CNT0, 0x2e); // higher eight bits of interrupt cycle
	timerctl.count = 0;
	timerctl.timeout = 0;
	return;
}

// when IRQ0 happens, invoke the interrupt handler: inthandler20
void inthandler20(int *esp)
{
	io_out8(PIC0_OCW2, 0x60); // Inform PIC the information that IRQ0-00 has been received.
	timerctl.count++; // each second it adds 100
	if (timerctl.timeout > 0) {
		timerctl.timeout--;
		if (timerctl.timeout == 0) {
			fifo8_put(timerctl.fifo, timerctl.data);
		}
	}
	return;
}

void settimer(unsigned int timeout, struct FIFO8 *fifo, unsigned char data)
{
	int eflags;
	eflags = io_load_eflags();
	io_cli();
	timerctl.timeout = timeout;
	timerctl.fifo = fifo;
	timerctl.data = data;
	io_store_eflags(eflags);
	return;
}