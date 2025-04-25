#ifndef _TIMER_H
#define _TIMER_H

void pit_timer_init();
void irq_pit_timer_handler(void*);
void pit_timer_phase(int hz);
void timer_wait(long milliseconds);

#endif
