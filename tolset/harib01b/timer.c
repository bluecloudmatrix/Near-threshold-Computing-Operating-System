#include "bootpack.h"

#define TIMER_FLAGS_ALLOC 	1 // has been set
#define TIMER_FLAGS_USING	2 // the timer is running 

struct TIMERCTL timerctl;

// the interrupt of IRQ0
// add sentry
void init_pit(void)
{
	int i;
	struct TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	//0x2e9c indicates that the frequency of interrupt is 100HZ
	io_out8(PIT_CNT0, 0x9c); // lower eight bits of interrupt cycle
	io_out8(PIT_CNT0, 0x2e); // higher eight bits of interrupt cycle
	timerctl.count = 0;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0;
	}
	t = timer_alloc(); // get one
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	t->next = 0; // sentry is always at the end
	timerctl.t0 = t; // because now there is only sentry, so put it on the front
	timerctl.next = 0xffffffff; // also because there is only sentry, so the next timeout is from sentry
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0; // not found
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0; // not use
	return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

// set a timer's timeout, need forbid interrupt
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli(); // forbid interrupt
	
	/*
	if (timerctl.using == 1) {
		// there is only one timer running, but when using sentry, the case does not exist any more
		timerctl.t0 = timer;
		timer->next = 0; // no next timer
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}*/ 
	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		// put the timer to timerctl.timers[0]
		timerctl.t0 = timer;
		timer->next = t;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	// find where to insert
	for (;;) {
		s = t;
		t = t->next;
		//if (t == 0) {
		//	break; // the end, but when using sentry, the case does not exist any more
		//}
		
		if (timer->timeout <= t->timeout) {
			// insert between s and t
			s->next = timer;
			timer->next = t;
			io_store_eflags(e);
			return;
		}
	}
	// in the case that insert into the end
	//s->next = timer;
	//timer->next = 0;
	//io_store_eflags(e);
	return;
	
}

// when IRQ0(timer interrupt) happens, invoke the interrupt handler: inthandler20
void inthandler20(int *esp)
{
	int i;
	struct TIMER *timer;
	char ts = 0;
	io_out8(PIC0_OCW2, 0x60); // Inform PIC the information that IRQ0-00 has been received.
	timerctl.count++; // each second it adds 100
	if (timerctl.next > timerctl.count) {
		return; // the next time has not arrived, so finish
	}
	timer = timerctl.t0; // first, put the first address in timers to timer
	// now we do not have to do MAX_TIMER times of 'if'
	for (;;) {
		if (timer->timeout > timerctl.count) {
			break;
		}
		// timeout
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer) {
			fifo32_put(timer->fifo, timer->data);
		} else {
			ts = 1; // task_timer timeout
		}
		timer = timer->next;
	}

	// we do not have to do shift due to timer->next
	timerctl.t0 = timer;
	timerctl.next = timerctl.t0->timeout;
	if (ts != 0) {
		task_switch(); // when the interrupt handling finishes, executing mt_taskswitch
	}
	return;
}
