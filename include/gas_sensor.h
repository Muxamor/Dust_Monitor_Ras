/*
 * gas_sensor.h
 *
 *  Created on: Jan 25, 2020
 *      Author: Ivan
 */

#ifndef INCLUDE_GAS_SENSOR_H_
#define INCLUDE_GAS_SENSOR_H_

#include <stdint.h>

#define GAS_CON_CO 0x0000
#define GAS_CON_SO2 0x0002
#define GAS_CON_NO2 0x0004
#define GAS_CON_CO2 0x0006

typedef struct{
	float gas_CO_value;
	float gas_SO2_value;
	float gas_NO2_value;
	float gas_CO2_value;
}_data_gas_senser;


int Gas_senser_get_CO_SO2_NO2_CO2 (int fd, uint8_t IDsensor, _data_gas_senser* Gas_sensor_value );
int Gas_Sensor_get (int fd, uint8_t IDsensor, uint16_t RegAddr, float* Gas_sensor_value );

#endif /* INCLUDE_GAS_SENSOR_H_ */
