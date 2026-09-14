#ifndef MCP23017_H
#define	MCP23017_H

#include <xc.h>
#include <stdint.h>
#include "i2c.h"

void MCP23017_Init(uint8_t add);

#endif