#ifndef TIMER_H
#define TIMER_H

#include "kernel.h"

// Timer frequency (Hz)
#define TIMER_FREQUENCY 100 // 100 Hz = 10ms intervals

// Timer functions
void timer_init(void);
void timer_handler(void);
uint32_t timer_get_ticks(void);
void timer_sleep(uint32_t ticks);

#endif
