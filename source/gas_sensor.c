/*
 * gas_sensor.c
 *
 *  Created on: Jan 25, 2020
 *      Author: Ivan
 */
#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h>
#include <stdio.h>

#include "gas_sensor.h"
#include "DEV_Config.h"


int Gas_senser_get_CO_SO2_NO2_CO2 (int fd, uint8_t IDsensor,  _data_gas_senser* Gas_sensor_value ){

	int ret = 0;
	float gas_value=0;

	//Get CO gas value
    ret = Gas_Sensor_get ( fd , IDsensor, GAS_CON_CO, &gas_value );
    if (ret == -1){
    	return -1;
    	Gas_sensor_value->gas_CO_value = 0;
    }else{
    	Gas_sensor_value->gas_CO_value = gas_value;
    }

	//Get SO2 gas value
    ret = Gas_Sensor_get ( fd , IDsensor, GAS_CON_SO2, &gas_value );
    if (ret == -1){
    	return -1;
    	Gas_sensor_value->gas_SO2_value = 0;
    }else{
    	Gas_sensor_value->gas_SO2_value = gas_value;
    }

	//Get NO2 gas value
    ret = Gas_Sensor_get ( fd , IDsensor, GAS_CON_NO2, &gas_value );
    if (ret == -1){
    	return -1;
    	Gas_sensor_value->gas_NO2_value = 0;
    }else{
    	Gas_sensor_value->gas_NO2_value = gas_value;
    }

	//Get CO2 gas value
    ret = Gas_Sensor_get ( fd , IDsensor, GAS_CON_CO2, &gas_value );
    if (ret == -1){
    	return -1;
    	Gas_sensor_value->gas_CO2_value = 0;
    }else{
    	Gas_sensor_value->gas_CO2_value = gas_value;
    }

    return 0;
}

int Gas_Sensor_get (int fd, uint8_t IDsensor, uint16_t RegAddr, float* Gas_sensor_value ){

	uint8_t parsel_buffer[50];
	uint16_t answer_from_senser[50] = {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};
	uint8_t parsel_size=0;
	uint32_t u32_ffloat_data;
	uint8_t i = 0;
	uint8_t counter_get_data = 0;
	uint16_t counter = 0;

	ModBusMasterGenerateReadReg( IDsensor, RegAddr , 0x02, parsel_buffer, &parsel_size);

	serialFlush ( fd );

	for( i=0; i<parsel_size; i++ ){
		serialPutchar ( fd, parsel_buffer[i] );
	}

	counter = 0;
	counter_get_data = 0;
	while(counter_get_data != 9){
		usleep(10);
		counter_get_data = serialDataAvail (fd);
		counter++;
		if( counter == 1500 ){
			perror("Request Gas Sensor, no get data");
			return -1;
		}
	}

	for( i=0; i < counter_get_data; i++){
		parsel_buffer[i] = serialGetchar (fd);
	}

	if ( ModBusMasterCheckAnswerReadReg(IDsensor, answer_from_senser , 0x0000, 0x0002, parsel_buffer , counter_get_data) == 1) {
		perror("Answer from Gas Sensor, is no correct");
		*Gas_sensor_value = 0;
		return -1;
	}

	u32_ffloat_data = (answer_from_senser[1] << 16) | answer_from_senser[0];

	*Gas_sensor_value = *((float*)&u32_ffloat_data);

	return 0;
}

