/*
 * config_station.c
 *
 *  Created on: Jun 23, 2019
 *      Author: Ivan
 */

#include <stdio.h>		//printf()
#include <string.h>
#include <stdlib.h>		//exit()
#include <stdint.h>
#include <errno.h>
#include "config_station.h"
#include <time.h>

int LogERR( char *f_path, struct tm *cur_time, char *Message ){

	FILE *LogFile;
	char LogPath[1000];
	sprintf( LogPath, "%s%s", f_path, "Log.txt");
	LogFile = fopen( LogPath, "a");

	if ( LogFile == NULL ){
	//	fprintf(stderr, "Can't open file.\n");
		return 0;
	}

	fprintf( LogFile, " %02d.%02d.%02d %02d:%02d:%02d %s;\r\n", cur_time->tm_mday, cur_time->tm_mon + 1, cur_time->tm_year + 1900, cur_time->tm_hour, cur_time->tm_min, cur_time->tm_sec, Message );

	fclose( LogFile );
	return 1;
}


int Get_Config( char *f_path, int8_t TimeArray[3][2]){

	FILE *ConfigFile;
	char ConfigPath[1000];
	char String[1000];
	char *string_ptr;
	int Time;
	int i;

	sprintf( ConfigPath, "%s%s", f_path, "config.txt");
	ConfigFile = fopen( ConfigPath, "r");
	if ( ConfigFile == NULL ) {
	//	fprintf(stderr, "Can't open file.\n");
		return 0;
	}

	for (i = 0; (i < 3) && (NULL != fgets( String, 1000, ConfigFile)); ++i) {
		if ( strstr( String, "Next start" ) != NULL ) {
			string_ptr = strstr( String, ":" );
			if ( string_ptr != NULL ) {
				sscanf( string_ptr + 1, "%d:%d", &TimeArray[0][0], &TimeArray[0][1]);
			}
			continue;
		}
		if ( strstr( String, "Preparing time" ) != NULL ) {
			string_ptr = strstr( String, ":" );
			if ( string_ptr != NULL ) {
				sscanf( string_ptr + 1, "%d:%d", &TimeArray[1][0], &TimeArray[1][1]);
				if (TimeArray[1][0] == 0 && TimeArray[1][1] < 30 ){
					TimeArray[1][1] = 30;
				}
			}
			continue;
		}
		if ( strstr( String, "Measuring time" ) != NULL ) {
			string_ptr = strstr( String, ":" );
			if ( string_ptr != NULL ) {
				sscanf( string_ptr + 1, "%d:%d", &TimeArray[2][0], &TimeArray[2][1]);
			}
			continue;
		}

	}

	fclose( ConfigFile );
	return 1;
}




int Get_Flash_disk_path(char *f_path){

	FILE *pipe_read;
	char string_console[100];
	char *ret_str;
	char *ret_strstr;
	uint16_t i=0;

	pipe_read = popen("mount" ,"r");

	if(pipe_read==NULL){
		return -1; // pipe no open
	}

	while(1){
		ret_str = fgets(string_console,100,pipe_read );
		 if( ret_str == NULL){
			 return -1;
		 }

		 ret_strstr = strstr(string_console, "/dev/sda1");
		 if(ret_strstr != NULL){
			break;
		 }
	}

	 ret_str = strstr(string_console, "/media/");

	 if(ret_str != NULL){
		 for(i=0; *ret_str != ' ' ;i++){
			 f_path[i] = *ret_str;
			 ret_str++;
		 }
		 f_path[i]= '/';
		 f_path[++i]= '\0';
	 }
	 pclose(pipe_read);

	 return 0;
}
