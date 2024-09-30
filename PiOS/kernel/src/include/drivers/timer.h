#ifndef _TIMER_H
#define _TIMER_H

void delay_cycls(uint32_t cycles);
void delay_arm_microsec(uint32_t msec);
void delay_arm_millisec(uint32_t msec);
uint64_t fetch_bcm_system_timer();
void delay_bcm_microsec(uint32_t msec);

#endif
