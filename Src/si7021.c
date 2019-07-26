/*
 * si7021.c
 *
 *  Created on: Jul 25, 2019
 *      Author: paulcz
 */

#include "si7021.h"

const static uint32_t _TRANSACTION_TIMEOUT = 100; // Wire NAK/Busy timeout in ms

static uint8_t _readRegister8(Adafruit_Si7021 *si7021, uint8_t reg);
static uint16_t _readRegister16(uint8_t reg);
static void _writeRegister8(uint8_t reg, uint8_t value);
static void _readRevision();

static uint8_t _readRegister8(Adafruit_Si7021 *si7021, uint8_t reg) {
	uint8_t value;
	uint8_t cmd = reg;

	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, &cmd, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}
	HAL_Delay(20);
	if (HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, &value, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	return value;
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
	si7021->_i2caddr = SI7021_DEFAULT_ADDRESS << 1;
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
_Bool Adafruit_Si7021_Begin(Adafruit_Si7021 *si7021) {
	Adafruit_Si7021_Reset(si7021);
	if (_readRegister8(si7021, SI7021_READRHT_REG_CMD) != 0x3AU)
		return 0;

	Adafruit_Si7021_ReadSerialNumber();
	_readRevision();

	return 1;
}

/*!
 *  @brief  Sends the reset command to Si7021.
 */
void Adafruit_Si7021_Reset(Adafruit_Si7021 *si7021) {
	uint8_t cmd = SI7021_RESET_CMD;
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, &cmd, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler();
	}
	HAL_Delay(50);
}

/*!
 *  @brief  Reads serial number and stores It in sernum_a and sernum_b variable
 */
void Adafruit_Si7021_ReadSerialNumber(Adafruit_Si7021 *si7021) {
	uint8_t cmd[] = {SI7021_ID1_CMD >> 8, SI7021_ID1_CMD & 0xFF};
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 2, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler();
	}

	uint8_t sernum[8];
	if (HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, sernum, 8, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	si7021->sernum_a = (sernum[0]<<24 | sernum[1]<<16 | sernum[2]<<8 | sernum[3]);

	cmd[0] = SI7021_ID2_CMD >> 8;
	cmd[1] = SI7021_ID2_CMD & 0xFF;
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 2, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler();
	}

	if (HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, sernum, 8, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	si7021->sernum_b = (sernum[0]<<24 | sernum[1]<<16 | sernum[2]<<8 | sernum[3]);

	switch(si7021->sernum_b >> 24) {
	case 0:
	case 0xFF:
		si7021->_model = SI_Engineering_Samples;
		break;
	case 0x0D:
		si7021->_model = SI_7013;
		break;
	case 0x14:
		si7021->_model = SI_7020;
		break;
	case 0x15:
		si7021->_model = SI_7021;
		break;
	default:
		si7021->_model = SI_UNKNOWN;
	}
}
