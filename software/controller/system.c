/******************************************************************************/
/* FILENAME     :	system.c                                                  */
/******************************************************************************/

#include "system.h"

/******************************************************************************/
/* void System_Init(void)
** ----------------------
** Initialize the CPU hardware
*/
void System_Init(void) 
{
// Internal oscillator, 16 MHz
    OSCCON = 0b01111000; 
    
// Watchdog (approx. 2 second timeout)
    WDTCON = 0b00010101; 

// No analog
    ANSELA = 0; 
	ANSELB = 0; 
	ANSELC = 0;

// Set pushbutton inputs    
    TRISCbits.TRISC0 = 1; // BTN_UP
    TRISCbits.TRISC1 = 1; // BTN_DN
// No weak pull-up (fitted on PCB)
    OPTION_REGbits.nWPUEN = 1; 
    
// Set other digital inputs and outputs
    TRISCbits.TRISC2 = 0; // Status LED Red
    TRISCbits.TRISC3 = 0; // Status LED Green
    TRISCbits.TRISC7 = 0; // TX Line
    TRISBbits.TRISB7 = 1; // RX Line

// Configure RB7 (RX) interrupt on falling edge only
	IOCBPbits.IOCBP7 = 0;    
    IOCBNbits.IOCBN7 = 1; 

    IOCBFbits.IOCBF7 = 0;    
    INTCONbits.IOCIF = 0;    
    INTCONbits.IOCIE   = 1;
    
// Setup Timer2 for 1ms interval interrupt
    T2CON = 0b01111100; 
    PR2 = 250;          
    PIE1bits.TMR2IE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
 
// Initialize the I2C hardware
    I2C_Init();
}