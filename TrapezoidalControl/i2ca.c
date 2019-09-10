/*
 * i2ca.c
 *
 *  Created on: Sep 1, 2019
 *      Author: Alex
 */

#include "i2ca.h"

void i2ca_init(void){
    //
       // Initialize I2C
       //
       I2caRegs.I2CSAR = I2C_SLAVE_ADDR_FUEL_GAUGE;        // Slave address - EEPROM control code

       //
       // I2CCLK = SYSCLK/(I2CPSC+1)
       //
   #if (CPU_FRQ_40MHZ||CPU_FRQ_50MHZ)
       I2caRegs.I2CPSC.all = 4;       // Prescaler - need 7-12 Mhz on module clk
   #endif

   #if (CPU_FRQ_60MHZ)
       I2caRegs.I2CPSC.all = 6;       // Prescaler - need 7-12 Mhz on module clk
   #endif
       I2caRegs.I2CCLKL = 10;           // NOTE: must be non zero
       I2caRegs.I2CCLKH = 5;            // NOTE: must be non zero

       //
       // Take I2C out of reset. Stop I2C when suspended
       //
       I2caRegs.I2CMDR.all = 0x0020;
       I2caRegs.I2CMDR.bit.TRX = 1;                 //Set to Transmit mode
       I2caRegs.I2CMDR.bit.MST = 1;                 //Set to Master mode
       I2caRegs.I2CMDR.bit.FREE = 1;                //Run in FREE mode
       I2caRegs.I2CMDR.bit.STP = 1;                 //Stop when internal counter becomes 0

       return;
}

//
/* I2CA_WriteData -
 * 1. Send a start sequence
 * 2. Send the I2C address of the slave with the R/W bit low (Perform a write)
 * 3. Send the internal register number you want to write to
 * 4. Send the data byte
 * 5. [Optionally, send any further data bytes]
 * 6. Send the stop sequence.
 */
void i2ca_writeData(char *TxData)
{
    int i;
    I2caRegs.I2CSAR = I2C_SLAVE_ADDR_FUEL_GAUGE;           //Set slave address
       I2caRegs.I2CCNT = I2C_NUMBYTES + 2;          //Set count to 5 characters plus 2 address bytes
       I2caRegs.I2CDXR = I2C_REG_VOLTAGE;            //Send eeprom high address
       I2caRegs.I2CMDR.bit.TRX = 1;                 //Set to Transmit mode
       I2caRegs.I2CMDR.bit.MST = 1;                 //Set to Master mode
       I2caRegs.I2CMDR.bit.FREE = 1;                //Run in FREE mode
       I2caRegs.I2CMDR.bit.STP = 1;                 //Stop when internal counter becomes 0
       I2caRegs.I2CMDR.bit.STT = 1;                 //Send the start bit, transmission will follow
       while(I2caRegs.I2CSTR.bit.XRDY == 0){};      //Do nothing till data is shifted out

       for(i = 0; i < I2C_NUMBYTES; i++){
        while(I2caRegs.I2CSTR.bit.XRDY == 0){};     //Do nothing till data is shifted out
        I2caRegs.I2CDXR = TxData[i];                  //Send out the message
       }
}

//
/* I2CA_ReadData -
 * 1. Send a start sequence
 * 2. Send I2C Address of Slave with R/W bit low (Perform a write)
 * 3. Send the internal register number you want to read from
 * 4. Send a start sequence again (repeated start)
 * 5. Send I2C Address of Slave with R/W bit high (Perform a read)
 * 6. Read data byte from Slave
 * 7. Send the stop sequence.
 */
void i2ca_readData(char *RxData)
{
    int i = 0;
    I2caRegs.I2CSAR = I2C_SLAVE_ADDR_FUEL_GAUGE;            //Set slave address
       I2caRegs.I2CCNT = 1;                         //Set count to 1 address byte
       I2caRegs.I2CDXR = I2C_REG_VOLTAGE;            //Send register address
       I2caRegs.I2CMDR.bit.TRX = 1;                 //Set to Transmit mode
       I2caRegs.I2CMDR.bit.MST = 1;                 //Set to Master mode
       I2caRegs.I2CMDR.bit.FREE = 1;                //Run in FREE mode
       I2caRegs.I2CMDR.bit.STP = 0;                 //Dont release the bus after Tx
       I2caRegs.I2CMDR.bit.STT = 1;                 //Send the start bit, transmission will follow

       while(I2caRegs.I2CSTR.bit.XRDY == 0){};      //Do nothing till data is shifted out
       I2caRegs.I2CCNT = 2;              //read 2 bytes
       I2caRegs.I2CMDR.bit.TRX = 0;                 //Set to Recieve mode
       I2caRegs.I2CMDR.bit.MST = 1;                 //Set to Master mode
       I2caRegs.I2CMDR.bit.FREE = 1;                //Run in FREE mode
       I2caRegs.I2CMDR.bit.STP = 1;                 //Stop when internal counter becomes 0
       I2caRegs.I2CMDR.bit.STT = 1; //Repeated start, Reception will follow
       for(i = 0; i < 2; i++){
        while(I2caRegs.I2CSTR.bit.RRDY == 0){};     //I2CDRR not ready to read?
        RxData[i] = I2caRegs.I2CDRR;
       }
}
