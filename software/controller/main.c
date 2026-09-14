/******************************************************************************/
/*                                                                            */
/* FILENAME     :	main.c                                                    */
/* DESCRIPTION  :   Control unit for driving a remote operated 6 relay switch */
/*                  through a 2-wire pulse modulated current loop.            */
/* WRITTEN BY   :   PA1ASB / M. van den Berg                                  */
/*                                                                            */
/* |--------------+-------+----------+---------------------------------------|*/
/* | DATE         | REV.  | BY       | CHANGES                               |*/
/* |--------------+-------+----------+---------------------------------------+*/
/* | AUG 30 2026  |     0 | MvdB     | INITIAL VERSION                       |*/
/* | SEP 13 2026  |     1 | MvdB     | FINAL RELEASE VERSION                 |*/
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
	bool no_ant_req = true;			// First enquiry without request!
	uint16_t start_delay = 0;
    uint8_t rx_data = 0;	
	bool start_found = false;
    uint16_t timeout = 0;		
    System_Init();					// Initialize the hardware
    MCP23017_Init(MCP23017_ADDR);	// Initialize the I/O expander that drives the LED's
    TX_PIN = 0;						// Make sure the TX line is LOW
	currentState = STATE_START;		// Initial machine state
	Set_Status_LED(COLOR_RED);

// Wait 1 second to allow slave to start
	while (start_delay < 1000)
	{
		CLRWDT();
		start_delay++;
		__delay_ms(1);
	}

    while(1) 
	{
        CLRWDT();
        switch (currentState) 		
		{            
// -------------------------------------------------------------
// STATE: START (System start / request relay state)
// -------------------------------------------------------------
            case STATE_START:
                currentState = STATE_REQ_INFO;
				no_ant_req = true;
                break;
// -------------------------------------------------------------
// STATE: REQ_INFO (Get relay state from slave)
// -------------------------------------------------------------
			case STATE_REQ_INFO:
				if(Send_Frame(9))
	               	currentState = STATE_RECEIVE;
				else
				{
					currentState = STATE_ERROR;
					DisplayAntennaSelection(0, 0);
                	Set_Status_LED(COLOR_RED);
				}
			
				break;
// -------------------------------------------------------------
// STATE: IDLE (Wait for button press and 1 sec timeout)
// -------------------------------------------------------------
            case STATE_IDLE:
// UP button
                if (BTN_UP == 0) 
				{
                    __delay_ms(50); // debounce
                    if (BTN_UP == 0) 
					{
                        selected_antenna++;
                        if (selected_antenna > 7) 
							selected_antenna = 1;
                        timer_1sec_counter = 0;
                        timer_1sec_active = true;
                        timer_1sec_expired = false;
						DisplayAntennaSelection(current_antenna, selected_antenna);
                        while(BTN_UP == 0) 
							CLRWDT();
                    }
                }
// DOWN button  
                if (BTN_DN == 0) 
				{
                    __delay_ms(50);
                    if (BTN_DN == 0) 
					{
                        if (selected_antenna <= 1) 
							selected_antenna = 7;
                        else 
							selected_antenna--;
                        timer_1sec_counter = 0;
                        timer_1sec_active = true;
                        timer_1sec_expired = false;
						DisplayAntennaSelection(current_antenna, selected_antenna);
                        while(BTN_DN == 0) 
							CLRWDT();
                    }
                }
                
                if (timer_1sec_expired) 
				{
// UP/DOWN untouched for >1 sec
                    timer_1sec_expired = false;
                    if (selected_antenna != current_antenna) 
					{
                        currentState = STATE_SEND;
                    }
					else
						currentState = STATE_IDLE;
                }
                break;
// -------------------------------------------------------------
// STATE: RECEIVE (Detect START condition and count pulses)
// -------------------------------------------------------------
            case STATE_RECEIVE:
                
				start_found=false;
				while (timeout < START_PULSE_TIMEOUT)
				{
					CLRWDT();
                    if (RX_PIN == 0) // RX line goes HIGH
					{
						start_found=true;
                        break;
                    }
                    __delay_ms(1);
                    timeout++;
				}
				if (start_found)
				{
					start_found = false;
// Wait for RX line to go LOW again
					while (timeout < START_PULSE_TIMEOUT)
					{
						CLRWDT();
                    	if (RX_PIN == 1) 
						{
                        	start_found = true;
// Prepare RX counter. Counting is handled in the ISR.
							rx_pulse_count=0;
                        	break;
                    	}
                    	__delay_ms(1);
                    	timeout++;
					}
				}
// When correct START condition is met, start counting pulses for a period of 300 ms
				if (start_found)
				{
					timeout = 0;
					while (timeout < COUNT_SAMPLE_TIME_WINDOW)
					{
						__delay_ms(1);
						timeout++;
					}
// Get reply from slave
					uint8_t reply=rx_pulse_count;
					CLRWDT();

					if ((reply>=1) && (reply<8))
					{
						current_antenna = reply;
						if ((current_antenna == selected_antenna) || (no_ant_req))
						{
// Show the currently switched antenna on the LED's
							DisplayAntennaSelection(current_antenna, 0);
							currentState = STATE_IDLE;
// After first enquiry, make selected antenna equal to current antenna
							if (no_ant_req)
								selected_antenna = current_antenna;
						}
						else
						{
							currentState = STATE_ERROR;
						}
					}
					else // reply >= 9 or out of range -> error
					{
// Switch OFF the antenna LED's
						DisplayAntennaSelection(0, 0);
// Switch ON the FAIL LED (red status)
						Set_Status_LED(COLOR_RED);
						currentState = STATE_ERROR;
					}
				}
				else
				{
// No ACK -> error
						DisplayAntennaSelection(0, 0);
						Set_Status_LED(COLOR_RED);
						currentState = STATE_ERROR;
				}
// Reset first cycle flag
				no_ant_req = false;
                break;
// -------------------------------------------------------------
// STATE: SEND (Send switch command) 
// -------------------------------------------------------------
            case STATE_SEND: 
				if ((selected_antenna >= 1) && (selected_antenna <= 7))
				{
                	if(Send_Frame(selected_antenna))
	                	currentState = STATE_RECEIVE;
					else
					{
						currentState = STATE_ERROR;
						DisplayAntennaSelection(0, 0);
                		Set_Status_LED(COLOR_RED);
					}
				}
                break;
// -------------------------------------------------------------
// STATE: ERROR 
// -------------------------------------------------------------
            case STATE_ERROR:
// Actual relay states are unknown
                current_antenna = 0; 
				DisplayAntennaSelection(0, 0);
                Set_Status_LED(COLOR_RED);
				CLRWDT();
// Wait for button press to reset error condition
                if ((BTN_UP == 0) || (BTN_DN == 0))
				{
                    __delay_ms(50);
                    while ((BTN_UP == 0) || (BTN_DN == 0))
						CLRWDT();
                    currentState = STATE_IDLE;
					Set_Status_LED(COLOR_OFF);
                }
                break;
        }
    }
}

/******************************************************************************/
/* bool Send_Frame(unit8_t data_mask)
** ----------------------------------
*/
bool Send_Frame(uint8_t pulses) 
{
	
// Make START condition on TX
    TX_PIN = 1;
    __delay_ms(START_PULSE_HIGH_TIME);
	CLRWDT();
// Check if slave has acknowledged; if not, return with error
	if (RX_PIN == 1)
	{
		TX_PIN = 0;
		return false;	// No ACK so return with an error
	}

    TX_PIN = 0;
    __delay_ms(START_PULSE_LOW_TIME);
// START condition has ended, ready to output pulses
	CLRWDT();

	for (uint8_t i = 0; i < pulses; i++)
	{
		TX_PIN = 1;
		__delay_ms(COUNTER_PULSE_HIGH_TIME);
		TX_PIN = 0;
		__delay_ms(COUNTER_PULSE_LOW_TIME);
	}
// TX line remains low, ready for next cycle. Return with "no error".
    return true;	
}

/******************************************************************************/
/* void Set_Status_LED(uint8_t color) 
** ----------------------------------
*/
void Set_Status_LED(uint8_t color) 
{
    switch (color) 
	{
        case COLOR_OFF:    
			LED_RED = 0; 
			LED_GREEN = 0; 
			break;
        case COLOR_GREEN:  
			LED_RED = 0; 
			LED_GREEN = 1; 
			break;
        case COLOR_RED:    
			LED_RED = 1; 
			LED_GREEN = 0; 
			break;
        case COLOR_ORANGE: 
			LED_RED = 1; 
			LED_GREEN = 1; 
			break;	
		default: 
			LED_RED = 1;
			LED_GREEN = 0;
			break;
    }
}
/******************************************************************************/
/* void DisplayAntennaSelection(uint8_t active, uint8_t selected)
** --------------------------------------------------------------
*/
void DisplayAntennaSelection(uint8_t active, uint8_t selected)
{
	uint8_t porta = 0;
	uint8_t portb = 0;

	if (currentState != STATE_ERROR)
		Set_Status_LED(COLOR_OFF);	
// Set the GREEN LED for the active antenna 
	switch (active)
	{
		case 3: porta = 0x20;
				break;
		case 2: porta = 0x08;
				break;
		case 1: porta = 0x02;
				break;
		case 6: portb = 0x20;
				break;
		case 5: portb = 0x08;
				break;
		case 4: portb = 0x02;
				break;
		case 7: Set_Status_LED(COLOR_GREEN);
				break;
	}
// Set GREEN + RED (= ORANGE) for selected antenna if required
	if (selected!=active)
	{
		switch (selected)
		{
			case 3: porta |= 0x30;
				break;
			case 2: porta |= 0x0C;
				break;
			case 1: porta |= 0x03;
				break;
			case 6: portb |= 0x30;
				break;
			case 5: portb |= 0x0C;
				break;
			case 4: portb |= 0x03;
				break;
			case 7: Set_Status_LED(COLOR_ORANGE);
				break;
		}
	}

	I2C_Write(MCP23017_ADDR, 0x12, porta); 
	__delay_ms(1);
    I2C_Write(MCP23017_ADDR, 0x13, portb); 
}
