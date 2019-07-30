/*!
 *  @file si7021.h
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

#ifndef SI7021_H_
#define SI7021_H_

#include "main.h"
#include "stm32f7xx_hal.h"

/*!
 *  I2C Address
 */
#define SI7021_DEFAULT_ADDRESS	0x40

/*!
 *  I2C Commands
 */
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

/*!
 * Firmware revisions
 */
#define SI7021_REV_1					0xFFU  /**< Sensor revision 1 */
#define SI7021_REV_2					0x20U  /**< Sensor revision 2 */

/*!
 * Register bit masks
 */
#define SI7021_HTRE_POS			 		(2U) /** D2 in user reg toggles heater -- 1:enable, 0:disable **/
#define SI7021_HTRE_MASK				(0x1U << SI7021_HTRE_POS)
#define SI7021_HEATLVL_MASK				(0x0FU) /** heater register [3:0] control heat level **/
#define SI7021_RHT_RSVD_MASK			(0x3AU) /** mask reserved bits in user register 1 **/

/*!
 * @typedef Si_SensorTypeDef refers to enum of sensor types
 */
typedef enum {
	SI_Engineering_Samples,
	SI_7013,
	SI_7020,
	SI_7021,
	SI_UNKNOWN,
} Si_SensorTypeDef;

/*!
 * @typedef Si7021 refers to struct __Si7021 containing sensor properties
 */
typedef struct __Si7021 {
	_Bool heater;		/**< Built-in heater status -- 0:off, 1: on **/
	I2C_HandleTypeDef _hi2c;
	Si_SensorTypeDef _model;
	uint8_t _revision;
	uint8_t  _i2caddr;
	uint32_t sernum_a; /**< Serial number A */
	uint32_t sernum_b; /**< Serial number B */
} Si7021_TypeDef;

/*!
 * Instance function prototypes
 */
_Bool Si7021_Begin(Si7021_TypeDef *si7021);
_Bool Si7021_HeaterOff(Si7021_TypeDef *si7021);
_Bool Si7021_HeaterOn(Si7021_TypeDef *si7021, uint8_t level);
float Si7021_ReadHumidity(Si7021_TypeDef *si7021);
float Si7021_ReadPrevTemperature(Si7021_TypeDef *si7021);
float Si7021_ReadTemperature(Si7021_TypeDef *si7021);
Si_SensorTypeDef Si7021_GetModel(Si7021_TypeDef *si7021);
uint8_t Si7021_GetRevision(Si7021_TypeDef *si7021);
uint8_t Si7021_HeaterStatus(Si7021_TypeDef *si7021);
void Si7021_Init(Si7021_TypeDef *si7021, I2C_HandleTypeDef *hi2c);
void Si7021_Reset(Si7021_TypeDef *si7021);


#endif /* SI7021_H_ */
