/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "SPI.h"
#include "wiring_private.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_mcspi.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom_map.h"
#include "driverlib/spi.h"
#include "driverlib/gpio.h"
#include "driverlib/prcm.h"
#include "driverlib/pin.h"

#define SSIBASE g_ulSSIBase[SSIModule]
#define NOT_ACTIVE 0xA

static const unsigned long g_ulSSIBase[] = {
	GSPI_BASE
};

//*****************************************************************************
//
// The list of SSI peripherals.
//
//*****************************************************************************
static const unsigned long g_ulSSIPeriph[] = {
	PRCM_GSPI
};

//*****************************************************************************
//
// The list of SSI gpio configurations.
//
//*****************************************************************************
static const unsigned long g_ulSSIPins[][4] = {
	{PIN_05 /* SCLK */, PIN_08 /* SS */, PIN_06 /* MISO */, PIN_07 /* MOSI */}
};

static const unsigned long g_ulSSIPinModes[][4] = {
	{PIN_MODE_7 /* SCLK */, PIN_MODE_7 /* SS */, PIN_MODE_7 /* MISO */, PIN_MODE_7 /* MOSI */}
};

SPIClass::SPIClass(void) {
	SSIModule = NOT_ACTIVE;
}

SPIClass::SPIClass(uint8_t module) {
	SSIModule = module;
}
  
void SPIClass::begin() {
	unsigned long initialData = 0;

	if(SSIModule == NOT_ACTIVE) {
		SSIModule = BOOST_PACK_SPI;
	}

	MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

	/* Configure pins as SPI */
	MAP_PinTypeSPI(g_ulSSIPins[SSIModule][0], g_ulSSIPinModes[SSIModule][0]);
	MAP_PinTypeSPI(g_ulSSIPins[SSIModule][1], g_ulSSIPinModes[SSIModule][1]);
	MAP_PinTypeSPI(g_ulSSIPins[SSIModule][2], g_ulSSIPinModes[SSIModule][2]);
	MAP_PinTypeSPI(g_ulSSIPins[SSIModule][3], g_ulSSIPinModes[SSIModule][3]);

	MAP_PRCMPeripheralReset(PRCM_GSPI);
	MAP_SPIReset(SSIBASE);

	MAP_SPIConfigSetExpClk(SSIBASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
				100000, SPI_MODE_MASTER, SPI_SUB_MODE_0,
				(SPI_SW_CTRL_CS |
				SPI_4PIN_MODE |
				SPI_TURBO_OFF |
				SPI_CS_ACTIVEHIGH |
				SPI_WL_8));

	MAP_SPIEnable(SSIBASE);
}

void SPIClass::end()
{
	MAP_SPIDisable(SSIBASE);
}

void SPIClass::setBitOrder(uint8_t ssPin, uint8_t bitOrder)
{
}

void SPIClass::setBitOrder(uint8_t bitOrder)
{
}

void SPIClass::setDataMode(uint8_t mode)
{
	HWREG(SSIBASE + MCSPI_O_MODULCTRL) &= ~SPI_MODE_MASK;
	HWREG(SSIBASE + MCSPI_O_MODULCTRL) |= mode;
}

void SPIClass::setClockDivider(uint8_t divider)
{
}

uint8_t SPIClass::transfer(uint8_t data)
{
	uint8_t rxData;

	MAP_SPITransfer(SSIBASE, &data, &rxData, 1, SPI_CS_ENABLE|SPI_CS_DISABLE);

	return (uint8_t) rxData;
}

/* Only one module available in the CC3200
 * But we leave it in here in case there will
 * be variants with more modules */
void SPIClass::setModule(uint8_t module) {
	SSIModule = module;
	begin();
}

SPIClass SPI;
