/******************************************************************************/
/*                                                                            */
/* FILENAME     :	main.c                                                    */
/* DESCRIPTION  :   Remote controlled 6 antenna switch controlled through a   */
/*                  2-wire pulse modulated current loop.                      */
/* WRITTEN BY   :   PA1ASB / M. van den Berg                                  */
/*                                                                            */
/* |--------------+-------+----------+---------------------------------------|*/
/* | DATE         | REV.  | BY       | CHANGES                               |*/
/* |--------------+-------+----------+---------------------------------------+*/
/* | AUG 24 2026  |     0 | MvdB     | INITIAL VERSION                       |*/
/* | SEP 13 2026  |     1 | MvdB     | FINAL VERSION                         |*/
/* |              |       |          |                                       |*/
/* |--------------+-------+----------+---------------------------------------|*/
/*                                                                            */
/* ========================================================================== */
/*                                                                            */
/* MIT License                                                                */
/*                                                                            */
/* Copyright (c) 2026 - M. van den Berg / PA1ASB                              */
/*                                                                            */
/* Permission is hereby granted,  free of charge,  to any person  obtaining a */
/* copy of this software and associated documentation files (the "Software"), */
/* to deal in the Software without restriction, including without  limitation */
/* the rights to use, copy,  modify, merge,  publish,  distribute, sublicense,*/
/* and/or  sell copies of  the Software,  and to  permit persons  to whom the */ 
/* Software is furnished to do so, subject to the following conditions:       */
/*                                                                            */
/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software.                        */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED,  INCLUDING BUT  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, */
/* FITNESS  FOR A PARTICULAR  PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL */
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY,  WHETHER IN AN ACTION OF CONTRACT,  TORT OR OTHERWISE,  ARISING */
/* FROM,  OUT OF OR IN  CONNECTION WITH  THE SOFTWARE  OR  THE  USE OR  OTHER */
/* DEALINGS IN THE SOFTWARE.                                                  */
/*                                                                            */
/******************************************************************************/
#include "main.h"

/******************************************************************************/
/* main()
** ------
*/
void main(void) 
{
    uint8_t rx_data = 0;			
	uint8_t requested_antenna = 0;
	uint16_t timeout = 0;
    bool start_found = false;

	SetCPUClockHigh();
    System_Init();					// Initialize the hardware
    MCP23017_Init(MCP23017_ADDR);	// Initialize the I/O expander that drives the relays
    TX_PIN = 0;						// Make sure the TX line is LOW
	CLRWDT();

    while(1) 
	{
        CLRWDT(); 					
        switch(currentState) 		
		{
// -------------------------------------------------------------
// STATE: START (System start)
// -------------------------------------------------------------
            case STATE_START:
                current_antenna = ReadAntennaStates(1);
				CLRWDT();
				SetCPUClockLow();
				currentState = STATE_IDLE;
                break;
// -------------------------------------------------------------
// STATE: IDLE (Wait for START condition on RX)
// -------------------------------------------------------------
            case STATE_IDLE:
	
				 CLRWDT();
// RX_PIN goes LOW when master sets TX line HIGH
 			     if (RX_PIN == 0)
				 {
// Switch to 1 MHz clock
					SetCPUClockHigh();
// Acknowledge to master
					TX_PIN = 1;
// Enter receive state, wait start condition to end
					currentState = STATE_COMMUNICATE;
	
				 }		
                 break;
// -------------------------------------------------------------
// STATE: COMMUNICATE (Receive, switch and transmit)
// -------------------------------------------------------------
            case STATE_COMMUNICATE: 
			{
                timeout = 0;
                start_found = false;

// Wait for START condition to end (RX_PIN goes HIGH)
                while (timeout < START_PULSE_TIMEOUT) 
				{
                    CLRWDT();
                    if (RX_PIN == 1) 
					{
                        start_found = true;
						rx_pulse_count=0;
                        break;
                    }
                    __delay_ms(1);
                    timeout++;
                }
// Release the TX pin in any case
				TX_PIN = 0;

                if (start_found) 
				{
					CLRWDT();
					timeout=0;
                    
// Loop while collecting pulses in the ISR
					while (timeout<COUNT_SAMPLE_TIME_WINDOW)
					{
						CLRWDT();
						__delay_ms(1);
						timeout++;
					}
					requested_antenna = rx_pulse_count;

					if ((requested_antenna >=1) && (requested_antenna<=9))
					{
						if (requested_antenna == 8)	// Error
						{
							Send_Frame(9);
							CLRWDT();
							SetCPUClockLow();
							currentState = STATE_IDLE;
						}
						else 
						{
							if (requested_antenna !=9)	// actual switching required!
							{
								if (requested_antenna !=7)
								{
// Switch OFF all relays
									SwitchAllAntennasOff();
// Switch ON the requested relay
									SwitchAntennaON(requested_antenna);
								}
								else
									SwitchAllAntennasOff();
								CLRWDT();
// Some additional contact debouncing
								__delay_ms(10);
// Get the actually enganged relay from the feedback lines
								current_antenna = ReadAntennaStates(0);
// and send back to master
								Send_Frame(current_antenna);
								CLRWDT();
// Now go back to IDLE
								SetCPUClockLow();
								currentState = STATE_IDLE;
							}
							else
							{
// Request status -> just return current antenna
								__delay_ms(100);
								current_antenna = ReadAntennaStates(0);
// and send back to master
								Send_Frame(current_antenna);
								CLRWDT();
// Now go back to IDLE
								SetCPUClockLow();
								currentState = STATE_IDLE;
							}
						}
					}
					else
					{
// Error -> return 9 pulses
							Send_Frame(9);
							CLRWDT();
							SetCPUClockLow();
							currentState = STATE_IDLE;
					}
                } 
                break;
            }
    	}
	}
}


/******************************************************************************/
/* bool Send_Frame(unit8_t data_mask)
** ----------------------------------
*/
bool Send_Frame(uint8_t pulses) 
{
    TX_PIN = 1;
    __delay_ms(START_PULSE_HIGH_TIME);
	CLRWDT();
	TX_PIN = 0;
    __delay_ms(START_PULSE_LOW_TIME);
    CLRWDT();

	for (int i=0;i<pulses;i++)
	{
		TX_PIN=1;
		__delay_ms(COUNTER_PULSE_HIGH_TIME);
		TX_PIN=0;
		__delay_ms(COUNTER_PULSE_LOW_TIME);
	}
	CLRWDT();
    return true;
}

/******************************************************************************/
/* void SetCPUClockHigh(void)
** -------------------------
*/
void SetCPUClockHigh(void)
{
// Set CPU clock to 1 MHz
	 OSCCONbits.IRCF = 0b1011; 	
// Wait until clock is stable, then return
     while(OSCSTATbits.HFIOFS == 0); 
}
/******************************************************************************/
/* void SetCPUClockLow(void)
** -------------------------
*/
void SetCPUClockLow(void)
{
// Set CPU clock to 32 kHz
	OSCCONbits.IRCF = 0b0000; 	
}
/******************************************************************************/
/* void ReadAntennaStates(char ResetOnErr)
** ---------------------------------------
*/
uint8_t ReadAntennaStates(uint8_t ResetOnErr)
{
	uint8_t feedback = (PORTC ^ 0xff) & 0x3F;
	uint8_t mask=0x01;
	uint8_t rn = 0;
	switch (feedback)
	{
		case 0x01: rn=4;
			break;
		case 0x02: rn=5;
			break;
		case 0x04: rn=6;
			break;
		case 0x08: rn=1;
			break;
		case 0x10: rn=2;
			break;
		case 0x20: rn=3;
			break;
		default: if (ResetOnErr)
			{
 				SwitchAllAntennasOff();
				rn=7;
			}
			else
				rn=7;
			break;
	}
	return rn;
}
/******************************************************************************/
/* void SwitchAllAntennasOff()
** ---------------------------
*/
void SwitchAllAntennasOff()
{
	I2C_Write(MCP23017_ADDR, 0x12, 0x2A);
	__delay_ms(25);
	I2C_Write(MCP23017_ADDR, 0x12, 0);
	__delay_ms(1);
	I2C_Write(MCP23017_ADDR, 0x13, 0x2A);
	__delay_ms(25);
	I2C_Write(MCP23017_ADDR, 0x13, 0);
	__delay_ms(1);
	CLRWDT();
}
/******************************************************************************/
/* void SwitchAntennaON(uint8_t antenna)
** --------------------------------------
*/
void SwitchAntennaON(uint8_t antenna)
{
	switch (antenna)
	{
		case 6: I2C_Write(MCP23017_ADDR, 0x12, 0x01);
				__delay_ms(25);
				I2C_Write(MCP23017_ADDR, 0x12, 0);
				__delay_ms(1);
				break;
		case 5: I2C_Write(MCP23017_ADDR, 0x12, 0x04);
				__delay_ms(25);
				I2C_Write(MCP23017_ADDR, 0x12, 0);
				__delay_ms(1);
				break;
		case 4: I2C_Write(MCP23017_ADDR, 0x12, 0x10);
				__delay_ms(25);
				I2C_Write(MCP23017_ADDR, 0x12, 0);
				__delay_ms(1);
				break;
		case 3: I2C_Write(MCP23017_ADDR, 0x13, 0x01);
				__delay_ms(25);
				I2C_Write(MCP23017_ADDR, 0x13, 0);
				__delay_ms(1);
				break;
		case 2: I2C_Write(MCP23017_ADDR, 0x13, 0x04);
				__delay_ms(25);
				I2C_Write(MCP23017_ADDR, 0x13, 0);
				__delay_ms(1);
				break;
		case 1: I2C_Write(MCP23017_ADDR, 0x13, 0x10);
				__delay_ms(25);
				I2C_Write(MCP23017_ADDR, 0x13, 0);
				__delay_ms(1);
				break;
	}
	CLRWDT();
}
