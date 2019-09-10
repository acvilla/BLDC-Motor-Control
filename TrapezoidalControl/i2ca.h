/*
 * i2ca.h
 *
 *  Created on: Sep 1, 2019
 *      Author: Alex
 */




#ifndef I2CA_H_
#define I2CA_H_

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "i2ca.h"

#define I2C_SLAVE_ADDR_FUEL_GAUGE 0xAA
#define I2C_REG_VOLTAGE 0x08
#define I2C_NUMBYTES 5

void i2ca_init(void);
void i2ca_writeData(char *);
void i2ca_ReadData(char *);

#endif
