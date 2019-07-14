/*
 * dust_sensor.h
 *
 *  Created on: Jun 6, 2019
 *      Author: Ivan
 */

#include <stdint.h>

#ifndef INCLUDE_DUST_SENSOR_H_
#define INCLUDE_DUST_SENSOR_H_

#define MODE_ACTIVE  1
#define MODE_PASSIVE 2
#define MODE_SLEEP   3
#define MODE_WAKEUP  4
#define SET_DEV_ID   5

typedef struct{
	uint16_t pm1_0_standard_particle; // ug/m3 should be used in the factory environment
	uint16_t pm2_5_standard_particle; //ug/m3 should be used in the factory environment
	uint16_t pm10_standard_particle;  //ug/m3 should be used in the factory environment
	uint16_t pm1_0_atmospheric_envir; // ug/m3
	uint16_t pm2_5_atmospheric_envir; // ug/m3
	uint16_t pm10_atmospheric_envir; // ug/m3
	uint16_t diameter_0_3_um_in_0_1_L_air;
	uint16_t diameter_0_5_um_in_0_1_L_air;
	uint16_t diameter_1_0_um_in_0_1_L_air;
	uint16_t diameter_2_5_um_in_0_1_L_air;
	uint16_t diameter_5_0_um_in_0_1_L_air;
	uint16_t diameter_10_um_in_0_1_L_air;
}_data_pms_7003;

#define PMS_7003_Reset_HIGHT     digitalWrite(6,HIGH)
#define PMS_7003_Reset_LOW       digitalWrite(6,LOW)

#define PMS_7003_Set_HIGHT     digitalWrite(5,HIGH)
#define PMS_7003_Set_LOW       digitalWrite(5,LOW)


int Dust_Sensor_PMS_7003_Set_Mode(int fd, uint8_t mode);
void Dust_Sensor_PMS_7003_Reset(void);
int Dust_Sensor_PMS_7003_Read_Data_Passive_Mode (int fd,  _data_pms_7003 *data_pms_7003);
int Dust_Sensor_SDS198_Set_Mode(int fd, uint8_t mode);
int Dust_Sensor_SDS198_Read_Data_Passive_Mode (int fd, uint16_t *data_sds198);



#endif /* INCLUDE_DUST_SENSOR_H_ */
