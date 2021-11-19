/*
 * i2c.h - Header file for I2C0 functionality
 * Attributes - Prof.David Sluiter IOT and Embedded Firmware Lecture 5 & Lecture 6
 *
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "em_gpio.h"
#include "src/timers.h"
#include "app.h"
#include "em_i2c.h"
#include <sl_i2cspm.h>
#include "em_letimer.h"
#include "efr32bg13p632f512gm48.h"

/*
 * Function to initialize i2c0
 * PARAMTERS - NONE
 * RETURNS   - NONE
 */
void I2C_init(void);

/*
 * Function to read from buffer of Si7021
 * PARAMTERS - NONE
 * RETURNS   - read buffer
 */
uint8_t* I2C_Read_Si7021(void);

/*
 * Function to write command to Si7021
 * PARAMTERS - NONE
 * RETURNS   - True for Success , False for failure
 */
bool I2C_Write_Si7021(void);

/*
 * Function to enable/disable Si7021
 * PARAMTERS - State to enable/disable
 * RETURNS   - NONE
 */
void Enable_si7021(bool state);

/*
 * Function to perform temperature read fro Si7021
 * PARAMTERS - NONE
 * RETURNS   - Converted temperature value
 */
uint16_t read_temp_si7021(void);

bool I2C_Write_VEML6030init(void);

#endif /* SRC_I2C_H_ */
