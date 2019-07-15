/*
 * dust_sensor.c
 *
 *  Created on: Jun 6, 2019
 *      Author: Ivan
 */
#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h>
#include <stdio.h>

#include "dust_sensor.h"
#include "DEV_Config.h"


int Dust_Sensor_PMS_7003_Set_Mode(int fd, uint8_t mode){

	uint8_t i=0, counter_get_data=0;
	uint8_t get_buf[20]={0};
	uint16_t counter=0;

	if( mode == MODE_PASSIVE ){
		const uint8_t buf_passive_mode[7] = {0x42,0x4D,0xE1,0x00,0x00,0x01,0x70}; // set passive mode
		const uint8_t buf_passive_mode_answer[8] = {0x42,0x4D,0x00,0x04,0xE1,0x00,0x01,0x74}; //answer at passive mode command
		for( i=0; i<7; i++){
			serialPutchar ( fd, buf_passive_mode[i] );
		}

		usleep(70000);

		serialFlush ( fd );

		for( i=0; i<7; i++){
			serialPutchar ( fd, buf_passive_mode[i] );
		}

		counter = 0;
		counter_get_data = 0;
		while(counter_get_data != 8){
			usleep(1000);
			counter_get_data = serialDataAvail (fd);
			counter++;
			if( counter == 1000 ){
				perror("Set passive mode PMS-7003 - FAILUR");
				return -1;
			}
		}

		for( i=0; i < counter_get_data; i++){
			get_buf[i] = serialGetchar (fd);
		}

		for( i=0; i < 8; i++ ){
			if( get_buf[i] != buf_passive_mode_answer[i] ) {
				perror("Set passive mode PMS-7003 - answer is wrong");
				return -1;
			}
		}

	}else if( mode == MODE_ACTIVE ){
		const uint8_t buf_active_mode[7] = {0x42,0x4D,0xE1,0x00,0x01,0x01,0x71}; // set active mode
		//const uint8_t buf_active_mode_answer[8] = {0x42,0x4D,0x00,0x04,0xE1,0x01,0x01,0x75}; //answer at active mode command
		for( i=0; i<7; i++){
			serialPutchar ( fd, buf_active_mode[i] );
		}
		//no answer check,  because we will get automatically data from sensor

	}else if( mode == MODE_SLEEP){
		//We can use command or pin GPIO to set mode sleep.
		//const uint8_t buf_slepp_mode[7] = {0x42,0x4D,0xE4,0x00,0x00,0x01,0x73}; // set sleep mode
		//const uint8_t buf_slepp_mode_answer[8] = {0x42,0x4D,0x00,0x04,0xE4,0x00,0x01,0x77}; //answer at sleep mode command
		//Use pin :
		PMS_7003_Set_LOW;

	}else if( mode == MODE_WAKEUP){
		//We can use command or pin GPIO to set mode wakeup.
		//const uint8_t buf_wakeup_mode[7] = {0x42,0x4D,0xE4,0x00,0x01,0x01,0x74}; // set wakeup mode
		//no answer at comman wakeup;
		//Use pin:
		PMS_7003_Set_HIGHT;
	}

	usleep(50000); // need for sensor some times sensor no answer after command and get answer. Next command blocked sensor without this sleep

	return 0;
}


void Dust_Sensor_PMS_7003_Reset(void){

	PMS_7003_Reset_LOW;
	usleep(300000);
	PMS_7003_Reset_HIGHT;
	PMS_7003_Set_HIGHT;
	sleep(2);
}


int Dust_Sensor_PMS_7003_Read_Data_Passive_Mode (int fd,  _data_pms_7003 *data_pms_7003){

	uint8_t i = 0;
	uint8_t counter_get_data = 0;
	uint16_t counter = 0;
	uint8_t get_data_buf[32];
	uint16_t pseudo_CRC_culculate = 0;
	uint16_t pseudo_CRC_get = 0;

	const uint8_t buf_read_data_passive_mode[7] = {0x42,0x4D,0xE2,0x00,0x00,0x01,0x71}; // passive mode read instruction, answer: one data telegram

	serialFlush ( fd );

	for( i=0; i<7; i++ ){
		serialPutchar ( fd, buf_read_data_passive_mode[i] );
	}

	counter = 0;
	counter_get_data = 0;
	while(counter_get_data != 32){
		usleep(1000);
		counter_get_data = serialDataAvail (fd);
		counter++;
		if( counter == 2000 ){
			perror("Request data PMS-7003, no get data");
			return -1;
		}
	}

	for( i=0; i < counter_get_data; i++){
		get_data_buf[i] = serialGetchar (fd);
	}

	//check pseudo crc
	pseudo_CRC_get = (get_data_buf[counter_get_data - 2]<<8) | get_data_buf[counter_get_data-1];
	pseudo_CRC_culculate = 0;
	for( i=0; i<(counter_get_data-2); i++ ){
		pseudo_CRC_culculate = pseudo_CRC_culculate + get_data_buf[i];
	}

	if(pseudo_CRC_culculate != pseudo_CRC_get ){
		perror("Request data PMS-7003, got data is not correct");
		return -1;
	}

	//data parsing

	data_pms_7003->pm1_0_standard_particle =  (get_data_buf[4]<<8) | get_data_buf[5];
	data_pms_7003->pm2_5_standard_particle =  (get_data_buf[6]<<8) | get_data_buf[7];
	data_pms_7003->pm10_standard_particle =  (get_data_buf[8]<<8) | get_data_buf[9];
	data_pms_7003->pm1_0_atmospheric_envir = (get_data_buf[10]<<8) | get_data_buf[11];
	data_pms_7003->pm2_5_atmospheric_envir = (get_data_buf[12]<<8) | get_data_buf[13];
	data_pms_7003->pm10_atmospheric_envir =  (get_data_buf[14]<<8) | get_data_buf[15];
	data_pms_7003->diameter_0_3_um_in_0_1_L_air = (get_data_buf[16]<<8) | get_data_buf[17];
	data_pms_7003->diameter_0_5_um_in_0_1_L_air = (get_data_buf[18]<<8) | get_data_buf[19];
	data_pms_7003->diameter_1_0_um_in_0_1_L_air = (get_data_buf[20]<<8) | get_data_buf[21];
	data_pms_7003->diameter_2_5_um_in_0_1_L_air = (get_data_buf[22]<<8) | get_data_buf[23];
	data_pms_7003->diameter_5_0_um_in_0_1_L_air = (get_data_buf[24]<<8) | get_data_buf[25];
	data_pms_7003->diameter_10_um_in_0_1_L_air = (get_data_buf[26]<<8) | get_data_buf[27];

	return 0;
}




int Dust_Sensor_SDS198_Set_Mode(int fd, uint8_t mode){

	uint8_t i=0, counter_get_data=0;
	uint8_t get_buf[20]={0};
	uint16_t counter=0;

	uint8_t const *buf_send_command;
	uint8_t const *buf_get_answer;

	if( mode == MODE_PASSIVE ){
		const uint8_t buf_passive_mode[19] = {0xAA, 0xB4, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDA, 0xC8, 0xA6, 0xAB}; // set passive mode
		const uint8_t buf_passive_mode_answer[10] = {0xAA, 0xC5, 0x02, 0x01, 0x01, 0x00, 0xDA, 0xC8, 0xA6, 0xAB }; //answer at passive mode command
		buf_send_command = buf_passive_mode;
		buf_get_answer = buf_passive_mode_answer;

		/*
		for( i=0; i<19; i++){
			serialPutchar ( fd, buf_send_command[i] );
		}
		usleep(100000);*/

	}else if( mode == MODE_ACTIVE ){
		//TO DO

	}else if( mode == MODE_SLEEP){
		const uint8_t buf_sleep_mode[19] = {0xAA, 0xB4, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDA, 0xC8, 0xA9, 0xAB}; // set sleep mode
		const uint8_t buf_sleep_mode_answer[10] = {0xAA, 0xC5, 0x06, 0x01, 0x00, 0x00, 0xDA, 0xC8, 0xA9, 0xAB}; //answer at sleep mode command
		buf_send_command = buf_sleep_mode;
		buf_get_answer = buf_sleep_mode_answer;

	}else if( mode == MODE_WAKEUP){
		const uint8_t buf_wakeup_mode[19] = {0xAA, 0xB4, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDA, 0xC8, 0xAA, 0xAB}; // set wakeup mode
		const uint8_t buf_wakeup_mode_answer[10] = { 0xAA, 0xC5, 0x06, 0x01, 0x01, 0x00, 0xDA, 0xC8, 0xAA, 0xAB};// answer at wakeup mode command
		buf_send_command = buf_wakeup_mode;
		buf_get_answer = buf_wakeup_mode_answer;
	}else if( mode == SET_DEV_ID){
		const uint8_t buf_set_dev_id[19] = {0xAA, 0xB4, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDA, 0xC8, 0xFF, 0xFF, 0xA5, 0xAB}; // set wakeup mode
		const uint8_t buf_set_dev_id_answer[10] = { 0xAA, 0xC5, 0x05, 0x00, 0x00, 0x00, 0xDA, 0xC8, 0xA7, 0xAB};// answer at wakeup mode command
		buf_send_command =  buf_set_dev_id;
		buf_get_answer = buf_set_dev_id_answer;

	}


	serialFlush ( fd );

	for( i=0; i<19; i++){
		serialPutchar ( fd, buf_send_command[i] );
	}

	counter = 0;
	counter_get_data = 0;
	while(counter_get_data != 10){
		usleep(1000);
		counter_get_data = serialDataAvail (fd);
		counter++;
		if( counter == 1000 ){
			perror("No get answer at command from SDS198 sensor");
			return -1;
		}
	}

	for( i=0; i < counter_get_data; i++){
		get_buf[i] = serialGetchar (fd);
	}

	for( i=0; i < 10; i++ ){
		if( get_buf[i] != buf_get_answer[i] ) {
			perror("Sensor SDS198 - answer is wrong");
			return -1;
		}
	}

	 if( mode == MODE_WAKEUP){
		 //A little bit of magic. The sensor does not wake up without repeating (does not respond to commands)
		 for( i=0; i<19; i++){
			 serialPutchar ( fd, buf_send_command[i] );
		 }
		 usleep(50000);

		 for( i=0; i<19; i++){
			 serialPutchar ( fd, buf_send_command[i] );
		 }
		 usleep(50000);
		 /////////////////////////////////////////////
	//	 serialFlush ( fd );
	 }

	//usleep(50000); // need for sensor some times sensor no answer after command and get answer. Next command blocked sensor without this sleep
	return 0;
}

int Dust_Sensor_SDS198_Read_Data_Passive_Mode (int fd, uint16_t *data_sds198){

	uint8_t i = 0;
	uint8_t counter_get_data = 0;
	uint16_t counter = 0;
	uint8_t get_data_buf[32];
	uint16_t pseudo_CRC_culculate = 0;
	uint8_t pseudo_CRC_get = 0;

	const uint8_t buf_read_data_passive_mode[19] = {0xAA, 0xB4, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDA, 0xC8, 0xA6, 0xAB}; // passive mode read instruction, answer: one data telegram

	serialFlush ( fd );

	for( i=0; i<19; i++ ){
		serialPutchar ( fd, buf_read_data_passive_mode[i] );
	}

	counter = 0;
	counter_get_data = 0;
	while(counter_get_data != 10){
		usleep(1000);
		counter_get_data = serialDataAvail (fd);
		counter++;
		if( counter == 2000 ){
			perror("Request data SDS198, no get data");
			return -1;
		}
	}

	for( i=0; i < counter_get_data; i++){
		get_data_buf[i] = serialGetchar (fd);
	}

	//check pseudo crc
	pseudo_CRC_get = get_data_buf[8];
	pseudo_CRC_culculate = 0;
	for( i=2; i<(counter_get_data-2); i++ ){
		pseudo_CRC_culculate = pseudo_CRC_culculate + get_data_buf[i];
	}

	if((uint8_t)pseudo_CRC_culculate != pseudo_CRC_get ){
		perror("Request data SDS198, got data is not correct");
		return -1;
	}

	//data parsing

	*data_sds198 =  (get_data_buf[5]<<8) | get_data_buf[4];
	return 0;
}


