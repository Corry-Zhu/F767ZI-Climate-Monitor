/*
 * si7021.c
 *
 *  Created on: Jul 25, 2019
 *      Author: paulcz
 */

#include "si7021.h"

static uint8_t _readRegister8(uint8_t reg);
static uint16_t _readRegister16(uint8_t reg);
static void _writeRegister8(uint8_t reg, uint8_t value);
static void _readRevision();

static uint8_t _readRegister8(uint8_t reg) {
	return 0;
}

static uint16_t _readRegister16(uint8_t reg) {
	return 0;
}

static void _writeRegister8(uint8_t reg, uint8_t value) {
	return;
}

static void _readRevision() {
	return;
}


/*!
 *  @brief  Instantiates a new Adafruit_Si7021 class
 *  @param  *theWire
 *          optional wire object
 */
void Adafruit_Si7021_Init(Adafruit_Si7021 *si7021, I2C_HandleTypeDef *hi2c) {
	si7021->_i2caddr = SI7021_DEFAULT_ADDRESS;
	si7021->_hi2c = *hi2c;
	si7021->sernum_a = 0; /**< Serialnum A */
	si7021->sernum_b = 0; /**< Serialnum B */
	si7021->_model = SI_7021;
	si7021->_revision = 0;
}


/*!
 *  @brief  Sets up the HW by reseting It, reading serial number and reading revision.
 *  @return Returns true if set up is successful.
 */
_Bool begin() {

	// TODO: begin I2C communication

	reset();
	if (_readRegister8(SI7021_READRHT_REG_CMD) != 0x3A)
		return 0;

	readSerialNumber();
	_readRevision();

	return 1;
}
