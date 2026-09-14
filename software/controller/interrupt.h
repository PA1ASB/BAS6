#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>
#include <stdbool.h>

void interrupt ISR(void);

volatile uint16_t timer_1sec_counter = 0;
volatile bool timer_1sec_active = false;
volatile bool timer_1sec_expired = false;
volatile uint8_t rx_pulse_count = 0;
#endif