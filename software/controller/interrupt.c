/******************************************************************************/
/* FILENAME     :	interrupt.c                                               */
/******************************************************************************/

#include <xc.h>
#include "interrupt.h"


/* void interrupt ISR(void) 
** ------------------------
*/
void interrupt ISR(void) 
{
// RX line interrupt on falling edge; counts pulses
	if (INTCONbits.IOCIF) 
	{
        if (IOCBFbits.IOCBF7) 
		{
			rx_pulse_count++;
            IOCBFbits.IOCBF7 = 0; 
        }
        INTCONbits.IOCIF = 0;
    }

// 1 ms timer on Timer 2
    if (PIR1bits.TMR2IF) 
	{
        PIR1bits.TMR2IF = 0;
        if (timer_1sec_active) 
		{
            timer_1sec_counter++;
            if (timer_1sec_counter >= 1000)
			{ 
                timer_1sec_active = false;
                timer_1sec_expired = true;
            }
        }
    }
}