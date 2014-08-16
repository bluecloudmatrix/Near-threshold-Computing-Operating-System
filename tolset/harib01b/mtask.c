/** used for task switching 
 *  2014/8/16
 */

#include "bootpack.h"
 
struct TIMER *mt_timer; // the timer used for task switching
int mt_tr; // representing TR register

void mt_init(void)
{
	mt_timer = timer_alloc();
	timer_settime(mt_timer, 2); // every 0.02 second, task switching
	mt_tr = 3 * 8;
	return;
}

// set the next mt_tr, and then reset the timer as 0.02s
void mt_taskswitch(void)
{
	if (mt_tr == 3 * 8) {
		mt_tr = 4 * 8;
	} else {
		mt_tr = 3 * 8;
	}
	timer_settime(mt_timer, 2);
	farjmp(0, mt_tr);
	return;
}