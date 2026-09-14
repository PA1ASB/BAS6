/******************************************************************************/
/* FILENAME     :	system.c                                                  */
/******************************************************************************/

#include "system.h"

/******************************************************************************/
/* void System_Init(void)
** ----------------------
*/
void System_Init(void) 
{
// OSCCON: Internal Oscillator 1 MHz
    OSCCON = 0b01011000; 
    
// WDTCON: Watchdog Timer (approx. 2 second timeout)
    WDTCON = 0b00010101; 

// Disable analog 
    ANSELA = 0; 
	ANSELB = 0; 
	ANSELC = 0;

// Set I/O directions
    TRISCbits.TRISC0 = 1; // K1 feedback
    TRISCbits.TRISC1 = 1; // K2 feedback
    TRISCbits.TRISC2 = 1; // K3 feedback
    TRISCbits.TRISC3 = 1; // K4 feedback
	TRISCbits.TRISC4 = 1; // K5 feedback
	TRISCbits.TRISC5 = 1; // K6 feedback
	TRISCbits.TRISC6 = 0; // TX

    TRISBbits.TRISB7 = 1; // RX Line
    
// Configure RB7 (RX) interrupt on falling edge
	IOCBPbits.IOCBP7 = 0;    
    IOCBNbits.IOCBN7 = 1;    

    IOCBFbits.IOCBF7 = 0;    
    INTCONbits.IOCIF = 0;    
    INTCONbits.IOCIE = 1; 
	INTCONbits.PEIE = 1;
	INTCONbits.GIE = 1;    

// Initialize I2C hardware
    I2C_Init();
}