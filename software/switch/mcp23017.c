/******************************************************************************/
/* FILENAME     :	mcp23017.c                                                */
/******************************************************************************/

#include "mcp23017.h"

/******************************************************************************/
/* void MCP23017_Init(uint8_t add)
** -------------------------------
*/
void MCP23017_Init(uint8_t add) 
{
    I2C_Write(add, 0x00, 0x00); // IODIRA: All outputs (Relay SET pins)
    I2C_Write(add, 0x01, 0x00); // IODIRB: All outputs (Relay RESET pins)
    I2C_Write(add, 0x12, 0x00); // OLATA: All OFF
    I2C_Write(add, 0x13, 0x00); // OLATB: All OFF
}