/*
 * uart.h
 *
 *  Created on: Jul 12, 2019
 *      Author: Alex
 */

#ifndef UART_H_
#define UART_H_

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "common/include/sci.h"

extern SCI_Handle mySci;

void scia_init(void);
void scia_fifo_init(void);
void scia_msg(char * msg);
uint16_t scia_xmit(int a);

#endif /* UART_H_ */
