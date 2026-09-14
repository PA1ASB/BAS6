#ifndef MAIN_H
#define	MAIN_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "interrupt.h"
#include "i2c.h"
#include "mcp23017.h"
#include "system.h"

// Configuration Bits (PIC16F1829)
#pragma config FOSC = INTOSC    // Internal oscillator
#pragma config WDTE = ON        // Watchdog Timer ON 
#pragma config PWRTE = ON       // Power-up Timer ON
#pragma config MCLRE = ON       // MCLR pin active
#pragma config CP = OFF         // Code Protection OFF
#pragma config BOREN = ON       // Brown-out Reset ON
#pragma config CLKOUTEN = OFF   // Clock Out function OFF
#pragma config IESO = OFF       // Internal/External Switchover OFF
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor OFF

#define _XTAL_FREQ 1000000      // 1 MHz; required for delay macros

// Pin definitions
#define RX_PIN      PORTBbits.RB7
#define TX_PIN      LATCbits.LATC6

// MCP23017 I2C Adress, A0, A1, A2 tied to GND
#define MCP23017_ADDR 0x40  

// State machine definitions
typedef enum {
    STATE_START,
    STATE_IDLE,
    STATE_COMMUNICATE,
    STATE_ERROR
} MasterState_t;

#define START_PULSE_TIMEOUT		 ((uint16_t)1500)
#define COUNT_SAMPLE_TIME_WINDOW ((uint16_t)300)
#define START_PULSE_HIGH_TIME	 ((uint8_t)5)
#define START_PULSE_LOW_TIME	 ((uint8_t)5)
#define	COUNTER_PULSE_HIGH_TIME	 ((uint8_t)3)
#define	COUNTER_PULSE_LOW_TIME	 ((uint8_t)3)

// Globals
MasterState_t currentState = STATE_START;

uint8_t current_antenna = 0;   // 1..6 = antenna 1..6, 7 = all OFF, 0 = unknown

void SetCPUClockLow(void);
void SetCPUClockHigh(void);
uint8_t ReadAntennaStates(uint8_t ResetOnErr);
bool Send_Frame(uint8_t data_mask);
void SwitchAntennaON(uint8_t antenna);
void SwitchAllAntennasOff(void);
void OutputPulses(uint8_t count);

#endif