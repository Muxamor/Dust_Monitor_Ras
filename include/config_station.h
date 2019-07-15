/*
 * config_station.h
 *
 *  Created on: Jun 23, 2019
 *      Author: Ivan
 */

#ifndef INCLUDE_CONFIG_STATION_H_
#define INCLUDE_CONFIG_STATION_H_

#include <time.h>


int LogERR( char *f_path, struct tm *cur_time, char *Message );

int Get_Config( char *f_path, int8_t TimeArray[3][2]);
int Get_Flash_disk_path(char *f_path);



#endif /* INCLUDE_CONFIG_STATION_H_ */
