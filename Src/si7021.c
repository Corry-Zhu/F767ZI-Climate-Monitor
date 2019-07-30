/*
 * si7021.c
 *
 *  Created on: Jul 25, 2019
 *      Author: paulcz
 */

#include "si7021.h"
#include <math.h>

const static uint32_t _TRANSACTION_TIMEOUT = 100; // Wire NAK/Busy timeout in ms

static uint8_t _readRegister8(Adafruit_Si7021 *si7021, uint8_t reg);
static uint16_t _readRegister16(Adafruit_Si7021 *si7021, uint8_t reg);
static void _writeRegister8(Adafruit_Si7021 *si7021, uint8_t reg, uint8_t value);
static void _readRevision();

static uint8_t _readRegister8(Adafruit_Si7021 *si7021, uint8_t reg) {
	uint8_t cmd[] = {reg};
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	uint8_t val[] = {0};
	if (HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, val, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	return val[0];
}

static uint16_t _readRegister16(Adafruit_Si7021 *si7021, uint8_t reg) {
	uint8_t cmd[] = {reg};
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	uint8_t ibuf[2];
	if (HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, ibuf, 2, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	uint16_t value = ibuf[0] << 8 | ibuf[1];
	return value;
}

static void _writeRegister8(Adafruit_Si7021 *si7021, uint8_t reg, uint8_t value) {
	uint8_t cmd[] = {reg, value};
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 2, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}
}

static void _readRevision(Adafruit_Si7021 *si7021) {
	uint8_t cmd[] = {SI7021_FIRMVERS_CMD >> 8, SI7021_FIRMVERS_CMD & 0xFF};
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 2, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	uint8_t firmvers;
	if (HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, &firmvers, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	switch (firmvers) {
	case SI7021_REV_1:
		si7021->_revision = 1;
		break;
	case SI7021_REV_2:
		si7021->_revision = 2;
		break;
	default:
		si7021->_revision = 0;
	}
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
	si7021->heater = 0;
}


/*!
 *  @brief  Sets up the HW by reseting It, reading serial number and reading revision.
 *  @return Returns true if set up is successful.
 */
_Bool Adafruit_Si7021_Begin(Adafruit_Si7021 *si7021) {
	Adafruit_Si7021_Reset(si7021);
	if (_readRegister8(si7021, SI7021_READRHT_REG_CMD) != 0x3AU)
		return 0;

	Adafruit_Si7021_ReadSerialNumber(si7021);
	_readRevision(si7021);

	return 1;
}

_Bool Adafruit_Si7021_HeaterOn(Adafruit_Si7021 *si7021, uint8_t level) {
	uint8_t usr_val = _readRegister8(si7021, SI7021_READRHT_REG_CMD);
	usr_val |= SI7021_HTRE_MASK;

	_writeRegister8(si7021, SI7021_WRITERHT_REG_CMD, usr_val);
	if (_readRegister8(si7021, SI7021_READRHT_REG_CMD) != usr_val) {
		return 0;
	}
	level &= SI7021_HEATLVL_MASK; /** [7:4] are reserved bits in heater register **/
	_writeRegister8(si7021, SI7021_WRITEHEATER_REG_CMD, level);
	if (_readRegister8(si7021, SI7021_READHEATER_REG_CMD) != level) {
		return 0;
	}

	si7021->heater = 1;
	return 1;
}

_Bool Adafruit_Si7021_HeaterOff(Adafruit_Si7021 *si7021) {
	uint8_t usr_val = _readRegister8(si7021, SI7021_READRHT_REG_CMD);
	usr_val &= ~SI7021_HTRE_MASK;

	_writeRegister8(si7021, SI7021_WRITERHT_REG_CMD, usr_val);
	if (_readRegister8(si7021, SI7021_READRHT_REG_CMD) != usr_val) {
		return 0;
	}

	si7021->heater = 0;
	return 1;
}

/*!
 *  @brief  Obtains heater enable status from usr reg and heater level from heater reg
 *  @return Returns uint8_t [7:0] where bit 4 is enable status and [3:0] is heater level
 */
uint8_t Adafruit_Si7021_HeaterStatus(Adafruit_Si7021 *si7021) {
	uint8_t status = 0x00;
	if (_readRegister8(si7021, SI7021_READRHT_REG_CMD) & SI7021_HTRE_MASK) {
		status |= (1U << 4); /** heater enabled **/
	}

	status |= (_readRegister8(si7021, SI7021_READHEATER_REG_CMD) & SI7021_HEATLVL_MASK);

	return status;
}

/*!
 *  @brief  Reads the humidity value from Si7021 (Master hold)
 *  @return Returns humidity as float value or NAN when there is error timeout
 */
float Adafruit_Si7021_ReadHumidity(Adafruit_Si7021 *si7021) {
	uint8_t cmd[] = {SI7021_MEASRH_HOLD_CMD};
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		return NAN;
	}

	uint8_t resp[3];
	HAL_StatusTypeDef rxStatus = HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, resp, 3, _TRANSACTION_TIMEOUT);
	if(rxStatus != HAL_OK) {
		return NAN;
	}
	uint16_t hum = resp[0] << 8 | resp[1];
	// uint8_t chxsum = resp[2];

	float humidity = hum;
	humidity *= 125;
	humidity /= 65536;
	humidity -= 6;

	return humidity;
}

/*!
 *  @brief  Reads the temperature value from Si7021 (Master hold)
 *  @return Returns temperature as float value or NAN when there is error timeout
 */
float Adafruit_Si7021_ReadTemperature(Adafruit_Si7021 *si7021) {
	uint8_t cmd[] = {SI7021_MEASTEMP_HOLD_CMD};
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		return NAN;
	}

	uint8_t resp[3];
	HAL_StatusTypeDef rxStatus = HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, resp, 3, _TRANSACTION_TIMEOUT);
	if(rxStatus != HAL_OK) {
		return NAN;
	}
	uint16_t temp = resp[0] << 8 | resp[1];
	// uint8_t chxsum = resp[2];

	float temperature = temp;
	temperature *= 175.72;
	temperature /= 65536;
	temperature -= 46.85;
	return temperature;
}

/*!
 *  @brief  Provides access to temperature value from previous humidity conversion
 *  @return Returns temperature as float value or NAN when there is error timeout
 */
float Adafruit_Si7021_ReadPrevTemperature(Adafruit_Si7021 *si7021) {
	uint8_t cmd[] = {SI7021_READPREVTEMP_CMD};
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		return NAN;
	}

	uint8_t resp[2];
	HAL_StatusTypeDef rxStatus = HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, resp, 2, _TRANSACTION_TIMEOUT);
	if(rxStatus != HAL_OK) {
		return NAN;
	}
	uint16_t temp = resp[0] << 8 | resp[1];

	float temperature = temp;
	temperature *= 175.72;
	temperature /= 65536;
	temperature -= 46.85;
	return temperature;
}

/*!
 *  @brief  Provides the caller with the model of the sensor
 *  @return Model number
 */
Si_SensorTypeDef Adafruit_Si7021_GetModel(Adafruit_Si7021 *si7021) {
	return si7021->_model;
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
		Error_Handler(); // TODO: Handle gracefully
	}

	uint8_t sernum[8];
	if (HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, sernum, 8, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	si7021->sernum_a = (sernum[0]<<24 | sernum[1]<<16 | sernum[2]<<8 | sernum[3]);

	cmd[0] = SI7021_ID2_CMD >> 8;
	cmd[1] = SI7021_ID2_CMD & 0xFF;
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 2, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
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

