/*
 ************************************************************************
 *	TwoWire.cpp
 *
 *	Arduino core files for MSP430
 *		Copyright (c) 2012 Robert Wessels. All right reserved.
 *
 *
 ***********************************************************************
  Derived from:
  TwoWire.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "wiring_private.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_i2c.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "Wire.h"

#define TX_BUFFER_EMPTY    txWriteIndex == 0
#define TX_BUFFER_FULL     txWriteIndex == BUFFER_LENGTH

#define RX_BUFFER_EMPTY    (rxReadIndex == rxWriteIndex)
#define RX_BUFFER_FULL     (((rxWriteIndex + 1) % BUFFER_LENGTH) == rxReadIndex)

#define STOP_CONDITION	0x4

static const unsigned long g_uli2cMasterBase[4] =
{
    I2C0_MASTER_BASE, I2C1_MASTER_BASE, 
	I2C2_MASTER_BASE, I2C3_MASTER_BASE
};
static const unsigned long g_uli2cSlaveBase[4] =
{
    I2C0_SLAVE_BASE, I2C1_SLAVE_BASE, 
	I2C2_SLAVE_BASE, I2C3_SLAVE_BASE
};

//*****************************************************************************
//
// The list of possible interrupts for the console i2c.
//
//*****************************************************************************
static const unsigned long g_uli2cInt[4] =
{
    INT_I2C0, INT_I2C1, INT_I2C2, INT_I2C3
};

//*****************************************************************************
//
// The list of i2c peripherals.
//
//*****************************************************************************
static const unsigned long g_uli2cPeriph[4] =
{
    SYSCTL_PERIPH_I2C0, SYSCTL_PERIPH_I2C1, 
    SYSCTL_PERIPH_I2C2, SYSCTL_PERIPH_I2C3
};

//*****************************************************************************
//
// The list of i2c gpio configurations.
//
//*****************************************************************************
static const unsigned long g_uli2cConfig[4][2] =
{
    {GPIO_PB2_I2C0SCL, GPIO_PB3_I2C0SDA},
    {GPIO_PA6_I2C1SCL, GPIO_PA7_I2C1SDA},
    {GPIO_PE4_I2C2SCL, GPIO_PE5_I2C2SDA},
    {GPIO_PD0_I2C3SCL, GPIO_PD1_I2C3SDA}
};

//*****************************************************************************
//
// The list of i2c gpio configurations.
//
//*****************************************************************************
static const unsigned long g_uli2cBase[4] =
{
    GPIO_PORTB_BASE, GPIO_PORTA_BASE, GPIO_PORTE_BASE, GPIO_PORTD_BASE
};

//*****************************************************************************
//
// The list of i2c gpio configurations.
//
//*****************************************************************************
static const unsigned long g_uli2cSDAPins[4] =
{
    GPIO_PIN_3, GPIO_PIN_7, GPIO_PIN_5, GPIO_PIN_1
};
static const unsigned long g_uli2cSCLPins[4] =
{
    GPIO_PIN_2, GPIO_PIN_6, GPIO_PIN_4, GPIO_PIN_0
};

#define MASTER_BASE g_uli2cMasterBase[i2cModule]
#define SLAVE_BASE g_uli2cSlaveBase[i2cModule]

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t TwoWire::rxBuffer[BUFFER_LENGTH];
uint8_t TwoWire::rxReadIndex = 0;
uint8_t TwoWire::rxWriteIndex = 0;
uint8_t TwoWire::txAddress = 0;

uint8_t TwoWire::txBuffer[BUFFER_LENGTH];
uint8_t TwoWire::txReadIndex = 0;
uint8_t TwoWire::txWriteIndex = 0;

uint8_t TwoWire::transmitting = 0;
uint8_t TwoWire::currentState = 0;

void (*TwoWire::user_onRequest)(void);
void (*TwoWire::user_onReceive)(int);

uint8_t TwoWire::i2cModule = 0;
uint8_t TwoWire::slaveAddress = 0;
// Constructors ////////////////////////////////////////////////////////////////

TwoWire::TwoWire()
{
}

// Private Methods //////////////////////////////////////////////////////////////
//Process any errors

uint8_t getError(uint8_t thrownError) {
  if(thrownError == I2C_MASTER_ERR_ADDR_ACK) return(2);
  else if(thrownError == I2C_MASTER_ERR_DATA_ACK) return(3);
  else if(thrownError != 0) return (4);
  else return(0);
}

uint8_t TwoWire::getRxData(unsigned long cmd) {

	if (currentState == IDLE) while(I2CMasterBusBusy(MASTER_BASE));
	HWREG(MASTER_BASE + I2C_O_MCS) = cmd;
	while(I2CMasterBusy(MASTER_BASE));
	uint8_t error = I2CMasterErr(MASTER_BASE);
	if (error != I2C_MASTER_ERR_NONE) {
        I2CMasterControl(MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
	}
	else {
		rxBuffer[rxWriteIndex] = I2CMasterDataGet(MASTER_BASE);
		rxWriteIndex = (rxWriteIndex + 1) % BUFFER_LENGTH;
	}
	return error;

}

uint8_t TwoWire::sendTxData(unsigned long cmd, uint8_t data) {

    I2CMasterDataPut(MASTER_BASE, data);

    //if (currentState == IDLE) while(I2CMasterBusBusy(MASTER_BASE));
    HWREG(MASTER_BASE + I2C_O_MCS) = cmd;
    while(I2CMasterBusy(MASTER_BASE));
    uint8_t error = I2CMasterErr(MASTER_BASE);
    if (error != I2C_MASTER_ERR_NONE)
		  I2CMasterControl(MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
    return(getError(error));
}

void TwoWire::forceStop(void) {

	//force a stop to release the bus
	HWREG(MASTER_BASE + 0x52C) = 0;//GPIO_PCTL
	GPIOPinTypeGPIOOutput(g_uli2cBase[i2cModule],
		  g_uli2cSCLPins[i2cModule] | g_uli2cSDAPins[i2cModule]);
    GPIOPinWrite(g_uli2cBase[i2cModule], g_uli2cSDAPins[i2cModule], 0);
    GPIOPinWrite(g_uli2cBase[i2cModule],
  		  g_uli2cSCLPins[i2cModule], g_uli2cSCLPins[i2cModule]);
    GPIOPinWrite(g_uli2cBase[i2cModule],
    	  g_uli2cSDAPins[i2cModule], g_uli2cSDAPins[i2cModule]);

    //reset I2C controller
    //without resetting the I2C controller, the I2C module will
    //bring the bus back to it's erroneous state
    SysCtlPeripheralReset(g_uli2cPeriph[i2cModule]);
    while(!SysCtlPeripheralReady(g_uli2cPeriph[i2cModule]));
}

// Public Methods //////////////////////////////////////////////////////////////

//Initialize as a master
void TwoWire::begin(void)
{
  rxReadIndex = 0;
  rxWriteIndex = 0;

  txReadIndex = 0;
  txWriteIndex = 0;

  currentState = IDLE;

  SysCtlPeripheralEnable(g_uli2cPeriph[i2cModule]);

  //force a stop condition
  if(!GPIOPinRead(g_uli2cBase[i2cModule], g_uli2cSCLPins[i2cModule]))
	  forceStop();

  //Configure GPIO pins for I2C operation
  GPIOPinConfigure(g_uli2cConfig[i2cModule][0]);
  GPIOPinConfigure(g_uli2cConfig[i2cModule][1]);
  GPIOPinTypeI2C(g_uli2cBase[i2cModule], g_uli2cSDAPins[i2cModule]);
  GPIOPinTypeI2CSCL(g_uli2cBase[i2cModule], g_uli2cSCLPins[i2cModule]);

  //Enable and initialize the I2Cx master module
  //false indicates that we're not using fast-speed transfers
  I2CMasterInitExpClk(MASTER_BASE, SysCtlClockGet(), false);//max bus speed=400kHz for gyroscope
  slaveAddress = 0;
  i2cModule = 0;

  //Handle any startup issues by pulsing SCL
  if(I2CMasterBusBusy(MASTER_BASE) || I2CMasterErr(MASTER_BASE)){
	  uint8_t doI = 0;
	  HWREG(MASTER_BASE + 0x52C) = 0;//GPIO_PCTL
  	  GPIOPinTypeGPIOOutput(g_uli2cBase[i2cModule], g_uli2cSCLPins[i2cModule]);
  	  unsigned long mask = 0;
  	  do{
  		  for(unsigned long i = 0; i < 10 ; i++) {
  			  SysCtlDelay(SysCtlClockGet()/100000/3);//100Hz=desired frequency, delay iteration=3 cycles
  			  mask = (i%2) ? g_uli2cSCLPins[i2cModule] : 0;
  			  GPIOPinWrite(g_uli2cBase[i2cModule], g_uli2cSCLPins[i2cModule], mask);
  		  }
  		  doI++;
  	  }while(I2CMasterBusBusy(MASTER_BASE) && doI < 100);
  	  GPIOPinConfigure(g_uli2cConfig[i2cModule][0]);
  	  GPIOPinConfigure(g_uli2cConfig[i2cModule][1]);
  	  GPIOPinTypeI2C(g_uli2cBase[i2cModule], g_uli2cSDAPins[i2cModule]);
  	  GPIOPinTypeI2CSCL(g_uli2cBase[i2cModule], g_uli2cSCLPins[i2cModule]);
  }

}

//Initialize as a slave
void TwoWire::begin(uint8_t address)
{

  begin();
  slaveAddress = address;
  //Enable slave interrupts
  IntEnable(g_uli2cInt[i2cModule]);
  I2CSlaveIntEnableEx(SLAVE_BASE, I2C_SLAVE_INT_DATA);
  
  //Setup as a slave device
  I2CMasterDisable(MASTER_BASE);
  I2CSlaveEnable(SLAVE_BASE);
  I2CSlaveInit(SLAVE_BASE, address); 
  
  IntMasterEnable();

}

void TwoWire::selectModule(unsigned long _i2cModule)
{
    i2cModule = _i2cModule;
    if(slaveAddress != 0) begin(slaveAddress);
    else begin();
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop)
{
  uint8_t error = 0;
  uint8_t oldWriteIndex = rxWriteIndex;
  uint8_t spaceAvailable = (rxWriteIndex >= rxReadIndex) ?
		 BUFFER_LENGTH - (rxWriteIndex - rxReadIndex) : (rxReadIndex - rxWriteIndex);
  if (quantity > spaceAvailable)
	  quantity = spaceAvailable;
  if (!quantity) return 0;
  //Select which slave we are requesting data from
  //true indicates we are reading from the slave
  I2CMasterSlaveAddrSet(MASTER_BASE, address, true);

  unsigned long cmd = 0;
  uint8_t runBit = (quantity) ? 1: 0;
  //uint8_t startBit = (currentState == IDLE) ? 2 : 0;//currentState ? 0 : 2;
  uint8_t startBit = (currentState == MASTER_RX) ? 0 : 2;//currentState ? 0 : 2;
  uint8_t ackBit = 0x8;

  cmd = runBit | startBit | ackBit;
  error = getRxData(cmd);
  if(error) return 0;

  int i = 1;

  for (; i < quantity; i++) {
	  if(i == (quantity - 1)) {
		  ackBit = 0;
	  }
	  cmd = runBit | ackBit;
	  error = getRxData(cmd);
	  if(error) return i;
  }


  if(sendStop) {
	  HWREG(MASTER_BASE + I2C_O_MCS) = 0x4;
	  while(I2CMasterBusy(MASTER_BASE));
	  currentState = IDLE;
  }
  else currentState = MASTER_RX;

  uint8_t bytesWritten = (rxWriteIndex >= oldWriteIndex) ?
		 BUFFER_LENGTH - (rxWriteIndex - oldWriteIndex) : (oldWriteIndex - rxWriteIndex);

  return(bytesWritten);

}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}
uint8_t TwoWire::requestFrom(int address, int quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}
uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

void TwoWire::beginTransmission(uint8_t address)
{
  transmitting = 1;
  // set address of targeted slave
  txAddress = address;
  // reset tx buffer iterator vars
  txWriteIndex = 0;
}

void TwoWire::beginTransmission(int address)
{
  beginTransmission((uint8_t)address);
}

uint8_t TwoWire::endTransmission(uint8_t sendStop)
{
  uint8_t error = I2C_MASTER_ERR_NONE;
  unsigned long cmd = 0;

  if(TX_BUFFER_EMPTY) return 0;

  //Select which slave we are requesting data from
  //false indicates we are writing to the slave
  I2CMasterSlaveAddrSet(MASTER_BASE, txAddress, false);
  //Wait for bus to open up in the case of multiple masters present

  uint8_t startBit = (currentState == MASTER_TX) ? 0 : 2;
  uint8_t runBit = (txWriteIndex) ? 1 : 0;

  cmd = runBit | startBit;

  error = sendTxData(cmd,txBuffer[0]);
  if(error) return error;

  for (int i = 1; i < txWriteIndex; i++) {
	  error = sendTxData(runBit,txBuffer[i]);
	  if(error) return getError(error);
  }

  if(sendStop) {
	  HWREG(MASTER_BASE + I2C_O_MCS) = STOP_CONDITION;
	  while(I2CMasterBusy(MASTER_BASE));
	  currentState = IDLE;
  }
  else {
	  currentState = MASTER_TX;
  }

  txWriteIndex = 0;
  // indicate that we are done transmitting
  transmitting = 0;
  return error;

}

//	This provides backwards compatibility with the original
//	definition, and expected behaviour, of endTransmission
//
uint8_t TwoWire::endTransmission(void)
{
  return endTransmission(true);
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(uint8_t data)
{
  if(transmitting){
  // in master transmitter mode
    // don't bother if buffer is full
    if(TX_BUFFER_FULL){
      setWriteError();
      return 0;
    }
    // put byte in tx buffer
    txBuffer[txWriteIndex] = data;
    txWriteIndex++;

  }else{
  // in slave send mode
    // reply to master
	I2CSlaveDataPut(SLAVE_BASE, data);
  }
  return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
  for(size_t i = 0; i < quantity; i++){
      write(data[i]);
  }
  return quantity;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void)
{
    return((rxWriteIndex >= rxReadIndex) ?
		(rxWriteIndex - rxReadIndex) : BUFFER_LENGTH - (rxReadIndex - rxWriteIndex));
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void)
{
  int value = -1;
  
  // get each successive byte on each call
  if(!RX_BUFFER_FULL){
    value = rxBuffer[rxReadIndex];
    rxReadIndex = (rxReadIndex + 1) % BUFFER_LENGTH;
  }

  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek(void)
{
  int value = -1;
  
  if(!RX_BUFFER_EMPTY){
    value = rxBuffer[rxReadIndex];
  }
  return value;
}
void TwoWire::flush(void)
{
	txWriteIndex = 0;
	rxReadIndex = rxWriteIndex;
}

// sets function called on slave write
void TwoWire::onReceive( void (*function)(int) )
{
  user_onReceive = function;
}

// sets function called on slave read
void TwoWire::onRequest( void (*function)(void) )
{
  user_onRequest = function;
}

void TwoWire::I2CIntHandler(void) {

    I2CSlaveIntClear(SLAVE_BASE);
	switch(I2CSlaveStatus(SLAVE_BASE)) {
		case(I2C_SLAVE_ACT_RREQ)://data received
		    user_onReceive(available());
			break;
		case(I2C_SLAVE_ACT_TREQ)://data requested 
		    user_onRequest();
			break;
		default:
			break;
	}
}
void
I2CIntHandler(void)
{
    Wire.I2CIntHandler();
}

//Preinstantiate Object
TwoWire Wire;
