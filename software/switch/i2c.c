/******************************************************************************/
/* FILENAME     :	i2c.c                                                     */
/******************************************************************************/

#include "i2c.h"

/******************************************************************************/
/* void I2C_Init(void) 
** -------------------
*/
void I2C_Init(void) 
{
    SSP1STAT = 0x80; 		// Slew rate disabled for Standard Speed (100 kHz)
    SSP1CON1 = 0x28; 		// Master mode, clock = FOSC / (4 * (SSP1ADD+1))
    SSP1ADD  = 39;   		// 100 kHz at 16 MHz FOSC
    TRISBbits.TRISB4 = 1; 	// SDA
    TRISBbits.TRISB6 = 1; 	// SCL
}

/******************************************************************************/
/* I2C_Write(uint8_t add, uint8_t reg, uint8_t data) 
** -------------------------------------------------
*/
void I2C_Write(uint8_t add, uint8_t reg, uint8_t data) 
{
    SSP1CON2bits.SEN = 1;            // Start
    while(SSP1CON2bits.SEN);
    
    SSP1BUF = add;         			 // Addrese + Write
    while(SSP1STATbits.R_nW);
    
    SSP1BUF = reg;                   // Register
    while(SSP1STATbits.R_nW);
    
    SSP1BUF = data;                  // Data
    while(SSP1STATbits.R_nW);
    
    SSP1CON2bits.PEN = 1;            // Stop
    while(SSP1CON2bits.PEN);
}