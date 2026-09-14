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

// RX line interrupt on falling edge
	if (INTCONbits.IOCIF) 
	{
        if (IOCBFbits.IOCBF7) 
		{
// Count pulses on RX line
			rx_pulse_count++;
			INTCONbits.IOCIF = 0;
        }
        
		IOCBFbits.IOCBF7=0;
    }
}