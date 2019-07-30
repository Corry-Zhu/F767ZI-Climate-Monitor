/*
 * si7021.h
 *
 *  Created on: Jul 25, 2019
 *      Author: paulcz
 */

#ifndef SI7021_H_
#define SI7021_H_


/*!
 *  @file Adafruit_Si7021.h
 *
 *  This is a library for the Adafruit Si7021 breakout board.
 *
 *  Designed specifically to work with the Adafruit Si7021 breakout board.
 *
 *  Pick one up today in the adafruit shop!
 *  ------> https://www.adafruit.com/product/3251
 *
 *  These sensors use I2C to communicate, 2 pins are required to interface.
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit andopen-source hardware by purchasing products
 *  from Adafruit!
 *
 *  Limor Fried (Adafruit Industries)
 *
 *  BSD license, all text above must be included in any redistribution
 */
#include "main.h"
#include "stm32f7xx_hal.h"

/*!
 *  I2C ADDRESS/BITS
 */
#define SI7021_DEFAULT_ADDRESS	0x40

#define SI7021_MEASRH_HOLD_CMD           0xE5U /**< Measure Relative Humidity, Hold Master Mode */
#define SI7021_MEASRH_NOHOLD_CMD         0xF5U /**< Measure Relative Humidity, No Hold Master Mode */
#define SI7021_MEASTEMP_HOLD_CMD         0xE3U /**< Measure Temperature, Hold Master Mode */
#define SI7021_MEASTEMP_NOHOLD_CMD       0xF3U /**< Measure Temperature, No Hold Master Mode */
#define SI7021_READPREVTEMP_CMD          0xE0U /**< Read Temperature Value from Previous RH Measurement */
#define SI7021_RESET_CMD                 0xFEU /**< Reset Command */
#define SI7021_WRITERHT_REG_CMD          0xE6U /**< Write RH/T User Register 1 */
#define SI7021_READRHT_REG_CMD           0xE7U /**< Read RH/T User Register 1 */
#define SI7021_WRITEHEATER_REG_CMD       0x51U /**< Write Heater Control Register */
#define SI7021_READHEATER_REG_CMD        0x11U /**< Read Heater Control Register */
#define SI7021_ID1_CMD                   0xFA0FU /**< Read Electronic ID 1st Byte */
#define SI7021_ID2_CMD                   0xFCC9U /**< Read Electronic ID 2nd Byte */
#define SI7021_FIRMVERS_CMD              0x84B8U /**< Read Firmware Revision */

#define SI7021_REV_1					0xFFU  /**< Sensor revision 1 */
#define SI7021_REV_2					0x20U  /**< Sensor revision 2 */

#define SI7021_HTRE_POS			 		(2U) /** D2 in user reg toggles heater -- 1:enable, 0:disable **/
#define SI7021_HTRE_MASK				(0x1U << SI7021_HTRE_POS)
#define SI7021_HEATLVL_MASK				(0x0FU) /** heater register [3:0] control heat level **/
#define SI7021_RHT_RSVD_MASK			(0x3AU) /** mask reserved bits in user register 1 **/

/** An enum to represent sensor types **/
typedef enum {
	SI_Engineering_Samples,
	SI_7013,
	SI_7020,
	SI_7021,
	SI_UNKNOWN,
} Si_SensorTypeDef;

typedef struct __Adafruit_Si7021 {
	I2C_HandleTypeDef _hi2c;
	uint32_t sernum_a; /**< Serialnum A */
	uint32_t sernum_b; /**< Serialnum B */
	Si_SensorTypeDef _model;
	uint8_t _revision;
	uint8_t  _i2caddr;
	_Bool heater;
} Adafruit_Si7021;

_Bool Adafruit_Si7021_Begin(Adafruit_Si7021 *si7021);
_Bool Adafruit_Si7021_HeaterOff(Adafruit_Si7021 *si7021);
_Bool Adafruit_Si7021_HeaterOn(Adafruit_Si7021 *si7021, uint8_t level);
float Adafruit_Si7021_ReadHumidity(Adafruit_Si7021 *si7021);
float Adafruit_Si7021_ReadTemperature(Adafruit_Si7021 *si7021);
float Adafruit_Si7021_ReadPrevTemperature(Adafruit_Si7021 *si7021);
Si_SensorTypeDef Adafruit_Si7021_GetModel(Adafruit_Si7021 *si7021);
uint8_t Adafruit_Si7021_HeaterStatus(Adafruit_Si7021 *si7021);
uint8_t getRevision();
void Adafruit_Si7021_Init(Adafruit_Si7021 *si7021, I2C_HandleTypeDef *hi2c);
void Adafruit_Si7021_ReadSerialNumber(Adafruit_Si7021 *si7021);
void Adafruit_Si7021_Reset(Adafruit_Si7021 *si7021);


#endif /* SI7021_H_ */
