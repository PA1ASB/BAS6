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

#define _XTAL_FREQ 16000000     // 16 MHz; required for delay macros

// I/O pin definitions
#define BTN_UP      PORTCbits.RC0
#define BTN_DN      PORTCbits.RC1
#define LED_RED     PORTCbits.RC2
#define LED_GREEN   PORTCbits.RC3
#define RX_PIN      PORTBbits.RB7
#define TX_PIN      LATCbits.LATC7

// MCP23017 I2C Address, A0, A1, A2 tied to GND
#define MCP23017_ADDR 0x40

// State machine definitions
typedef enum {
    STATE_START,
	STATE_REQ_INFO,
    STATE_IDLE,
    STATE_SEND,
    STATE_RECEIVE,
    STATE_ERROR
} MasterState_t;

// Some timing stuff
#define START_PULSE_TIMEOUT		 ((uint16_t)1500)
#define COUNT_SAMPLE_TIME_WINDOW ((uint16_t)300)
#define START_PULSE_HIGH_TIME	 ((uint8_t)15)
#define START_PULSE_LOW_TIME	 ((uint8_t)5)
#define	COUNTER_PULSE_HIGH_TIME	 ((uint8_t)3)
#define	COUNTER_PULSE_LOW_TIME	 ((uint8_t)3)

// Globals
MasterState_t currentState = STATE_START;
uint8_t current_antenna = 0;   // 1..6 = antenna 1..6, 7 = all OFF, 0 = unknown
uint8_t selected_antenna = 1;  // Antenna selected by operator

void DisplayAntennaSelection(uint8_t active, uint8_t selected);
void Set_Status_LED(uint8_t color); // 0=OFF, 1=GREEN, 2=RED, 3=ORANGE
bool Send_Frame(uint8_t data_mask);

// LED color definitions
#define COLOR_OFF    0
#define COLOR_GREEN  1
#define COLOR_RED    2
#define COLOR_ORANGE 3

#endif