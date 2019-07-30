/*!
 *  @file si7021.c
 *  @author Paul Czeresko <p.czeresko.3@gmail.com>
 *	@date 25 July 2019
 *
 *	@section Description
 *
 *  This is a library for the Silicon Labs Si7021 I2C temperature/humidity
 *  sensor. This code has been tested using the Adafruit Si7021 breakout board.
 *
 *  The fundamental operations are based on the Arduino library provided by
 *  Adafruit, but all functions are set to work on top of the STM32F7 HAL.
 */

#include "si7021.h"
#include <math.h>

const static uint32_t _TRANSACTION_TIMEOUT = 100; // Wire NAK/Busy timeout in ms

/*!
 * Static function prototypes
 */
static uint8_t _readRegister8(Si7021_TypeDef *si7021, uint8_t reg);
static void _writeRegister8(Si7021_TypeDef *si7021, uint8_t reg, uint8_t value);
static void _readRevision(Si7021_TypeDef *si7021);
void _readSerialNumber(Si7021_TypeDef *si7021);

/*!
 * Static function definitions
 */

/*!
 * @brief Reads 8 bits from the specified register
 * @param *si7021 Pointer to the handle of the target device
 * @param reg Register to be read
 * @return value Acquired data as uint8_t
 */
static uint8_t _readRegister8(Si7021_TypeDef *si7021, uint8_t reg) {
	uint8_t cmd[] = {reg};
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	uint8_t value[] = {0};
	if (HAL_I2C_Master_Receive(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, value, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}

	return value[0];
}

/*!
 * @brief Writes 8 bits to the specified register
 * @param si7021 Pointer to the handle of the target device
 * @param reg Register to be written
 *
 * Note there is no bitmasking protection. It is currently left to the user to
 * first read the register, do any necessary masking, and apply that to the
 * byte to be written.
 */
static void _writeRegister8(Si7021_TypeDef *si7021, uint8_t reg, uint8_t value) {
	uint8_t cmd[] = {reg, value};
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, cmd, 2, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler(); // TODO: Handle gracefully
	}
}

/*!
 * @brief Reads firmware revision from device and updates struct
 * @param si7021 Pointer to the handle of the target device
 */
static void _readRevision(Si7021_TypeDef *si7021) {
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
 *  @brief Reads serial number and updates properties of target structure
 *  @param *si7021 Pointer to the handle of the target device
 */
void _readSerialNumber(Si7021_TypeDef *si7021) {
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

/*!
 * Instance function definitions
 */

/*!
 *  @brief Sets up the HW by reseting It, reading serial number and reading revision
 *  @param *si7021 Pointer to the handle of the target device
 *  @return Returns true if set up is successful
 */
_Bool Si7021_Begin(Si7021_TypeDef *si7021) {
	Si7021_Reset(si7021);
	if (_readRegister8(si7021, SI7021_READRHT_REG_CMD) != 0x3AU)
		return 0;

	_readSerialNumber(si7021);
	_readRevision(si7021);

	return 1;
}

/*!
 *  @brief Enables on-chip heater to specified level
 *  @param *si7021 Pointer to the handle of the target device
 *  @param level Heater level -- 0-15, lowest-highest. [7:4] don't care.
 *  @return True if successful, otherwise false
 */
_Bool Si7021_HeaterOn(Si7021_TypeDef *si7021, uint8_t level) {
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

/*!
 *  @brief Disables on-chip heater
 *  @param *si7021 Pointer to the handle of the target device
 *  @return True if successful, otherwise false
 */
_Bool Si7021_HeaterOff(Si7021_TypeDef *si7021) {
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
 *  @brief Reads the humidity value from Si7021 (Master hold)
 *  @param *si7021 Pointer to the handle of the target device
 *  @return humidity Humidity as float value or NAN if I2C link is unsuccessful
 */
float Si7021_ReadHumidity(Si7021_TypeDef *si7021) {
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
 *  @brief Provides access to temperature value from previous humidity conversion
 *  @param *si7021 Pointer to the handle of the target device
 *  @return temperature Temperature as float value or NAN if I2C link is unsuccessful
 *
 *  This function allows access to temperature data from the previous conversion
 *  without having to resample.
 */
float Si7021_ReadPrevTemperature(Si7021_TypeDef *si7021) {
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
 *  @brief Reads the humidity value from Si7021 (Master hold)
 *  @param *si7021 Pointer to the handle of the target device
 *  @return temperature Temperature as float value or NAN if I2C link is unsuccessful
 */
float Si7021_ReadTemperature(Si7021_TypeDef *si7021) {
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
 *  @brief Provides the caller with the model of the sensor
 *  @param *si7021 Pointer to the handle of the target device
 *  @return Model number
 */
Si_SensorTypeDef Si7021_GetModel(Si7021_TypeDef *si7021) {
	return si7021->_model;
}

/*!
 *  @brief  Obtains heater status from usr reg and heater level from heater reg
 *  @param *si7021 Pointer to the handle of the target device
 *  @return status Heater status as uint8_t as described below
 *
 *  status bit 4 is enable status -- 0:off, 1:on
 *  status bits [3:0] represent heater level 0-15, lowest-highest
 *
 *  @example
 *  Enable/disable status = status >> 4
 *  Heater level = status & 0x0F
 */
uint8_t Si7021_HeaterStatus(Si7021_TypeDef *si7021) {
	uint8_t status = 0x00;
	if (_readRegister8(si7021, SI7021_READRHT_REG_CMD) & SI7021_HTRE_MASK) {
		status |= (1U << 4); /** heater enabled **/
	}

	status |= (_readRegister8(si7021, SI7021_READHEATER_REG_CMD) & SI7021_HEATLVL_MASK);

	return status;
}

/*!
 *  @brief Instantiates a new Si7021_TypeDef struct
 *  @param *si7021 Pointer to the handle of the target device
 *  @param *hi2c Pointer to handle of I2C channel
 */
void Si7021_Init(Si7021_TypeDef *si7021, I2C_HandleTypeDef *hi2c) {
	si7021->heater = 0;
	si7021->_hi2c = *hi2c;
	si7021->_model = SI_7021;
	si7021->_revision = 0;
	si7021->_i2caddr = SI7021_DEFAULT_ADDRESS << 1; /**< 7b address as MSB **/
	si7021->sernum_a = 0;
	si7021->sernum_b = 0;
	// TODO: add address as param
}

/*!
 *  @brief Sends the reset command to Si7021
 *  @param *si7021 Pointer to the handle of the target device
 */
void Si7021_Reset(Si7021_TypeDef *si7021) {
	uint8_t cmd = SI7021_RESET_CMD;
	if (HAL_I2C_Master_Transmit(&(si7021->_hi2c), (uint16_t)si7021->_i2caddr, &cmd, 1, _TRANSACTION_TIMEOUT) != HAL_OK) {
		Error_Handler();
	}
	HAL_Delay(50);
}

