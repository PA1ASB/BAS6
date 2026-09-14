#ifndef I2C_H
#define	I2C_H

#include <xc.h>
#include <stdint.h>


void I2C_Init(void);
void I2C_Write(uint8_t add, uint8_t reg, uint8_t data);

#endif