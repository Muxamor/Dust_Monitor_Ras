/*
 * Date:	01.5.2019
 *
 * Develop: Ivan Neskorodev
 * Email: ivan.neskorodev@gmail.com
*/



#include <stdio.h>		//printf()
#include <string.h>
#include <stdlib.h>		//exit()
#include <errno.h>

#include "main.h"
#include "dust_sensor.h"
#include "OLED_Driver.h"
#include "OLED_GUI.h"
#include "DEV_Config.h"
#include <time.h>
//#include <wiringSerial.h>

struct DisplayArea{
	POINT x1;
	POINT y1;
	POINT x2;
	POINT y2;
};

struct MeasureResult{
	struct DisplayArea Area;
	int16_t Value;
};

void CoordInit ( struct DisplayArea *Area, POINT x_start, POINT y_start, POINT x_end, POINT y_end){
	Area->x1 = x_start;
	Area->y1 = y_start;
	Area->x2 = x_end;
	Area->y2 = y_end;
}

void ShowFileError( FILE *File,  struct DisplayArea ErrorArea ){
	//Обработка ошибок открытия файла
	if ( File == NULL ){
		GUI_Show_OLED_string( ErrorArea.x1, ErrorArea.y1, ErrorArea.x2, ErrorArea.y2, &Font12, "Flash disk error!", WHITE);
	}else{
		OLED_ClearWindow( ErrorArea.x1, ErrorArea.y1, ErrorArea.x2, ErrorArea.y2, WHITE);
	}
	OLED_DisWindow( ErrorArea.x1, ErrorArea.y1, ErrorArea.x2, ErrorArea.y2 );

}


int main(void){

	_data_pms_7003 DATA_PMS_7003, *data_pms_7003 = &DATA_PMS_7003;
	uint16_t DATA_SDS198, *data_sds198 = &DATA_SDS198;
	int fd_UART,counter_data_uart;
	int retval_pms_7003 = 0, retval_SDS198 = 0;
	int i;
	uint8_t count_error_sensor_pms_7003 = 0;
	uint8_t count_error_sensor_SDS198 = 0;

	time_t now;
	struct tm *timenow;

	FILE *OutputCSV;
	char *FileNameTemp = "Dust_Monitoring/Dust_monitor_";
	char Flash_path[100];
	char CurFileName[100];
	int8_t TimeGap[3][2]; // массив таймеров (мин, сек) для периодов ожидания, продувки и измерения соответственно
	int8_t Period;// Возможные значения 0 - ожидание, 1 -продувка датчиков и 2 - измерение
	int8_t NumOfPeriods = 3; // Количество периодов

	struct MeasureResult Result[4]; // массив измерений PM1, PM2.5, PM10, PM100

	struct DisplayArea TimeDateArea = { 0, 0, 127, 12 };
	struct DisplayArea PeriodDescrArea = { 0, 15, 83, 27 };
	struct DisplayArea PeriodTimerArea = { 84, 15, 119, 27 };
	struct DisplayArea ErrorArea = { 0, 88, 127, 102 };
	struct DisplayArea IPArea = { 0, 103, 127, 127 };

	uint8_t cur_time_day,new_time_day = 0;
	uint8_t now_time_sec,new_time_sec = 0;
	int8_t back_timer_min, back_timer_sec=0;

	Get_Flash_disk_path(Flash_path);

	Get_Config( Flash_path, TimeGap );



	fd_UART = serialOpen("/dev/ttyAMA0", 9600);

	if(System_Init()){ //wiringPi System Initialization
		//fprintf (stdout, "WiringPi System Initialization: %s\n", strerror (errno)) ;
		perror("WiringPi System Initialization - FAILUR");
		exit(0);
	}

	//printf("**********Init OLED**********\r\n");
	OLED_SCAN_DIR OLED_ScanDir = SCAN_DIR_DFT;//SCAN_DIR_DFT = D2U_L2R
	OLED_Init(OLED_ScanDir );

	GUI_OLED_Show_Start_screan(3000);
	OLED_Clear(OLED_BACKGROUND);
	OLED_Display();

	time(&now);
	timenow = localtime(&now);


	GUI_DisString_EN(0 , 15,"Initialization: ", &Font12, FONT_BACKGROUND, WHITE);
	OLED_Display();

	Dust_Sensor_PMS_7003_Reset();
	SN74_MUX_to_PMS_7003();
	retval_pms_7003 =  Dust_Sensor_PMS_7003_Set_Mode(fd_UART, MODE_PASSIVE);
	if(retval_pms_7003 == -1){
		GUI_DisString_EN(0 , 30,"PMS-7003     ERROR", &Font12, FONT_BACKGROUND, WHITE);
		LogERR( Flash_path, timenow, "Start app. Initialization PMS-7003 error");
	}else{
		GUI_DisString_EN(0 , 30,"PMS-7003        OK", &Font12, FONT_BACKGROUND, WHITE);
	}
	Dust_Sensor_PMS_7003_Set_Mode(fd_UART, MODE_SLEEP);
	OLED_Display();

	SN74_MUX_to_SDS198();
	retval_SDS198 =  Dust_Sensor_SDS198_Set_Mode(fd_UART, MODE_PASSIVE);
	if(retval_SDS198 == -1){
		GUI_DisString_EN(0 , 45,"SDS198       ERROR", &Font12, FONT_BACKGROUND, WHITE);
		LogERR( Flash_path, timenow, "Start app. Initialization SDS198 error");
	}else{
		GUI_DisString_EN(0 , 45,"SDS198          OK", &Font12, FONT_BACKGROUND, WHITE);
	}
	Dust_Sensor_SDS198_Set_Mode(fd_UART, MODE_SLEEP);

	OLED_Display();
	Driver_Delay_ms(2000);

	OLED_Clear(OLED_BACKGROUND);
	OLED_Display();

	GUI_DisString_EN(0 , 15,"Next start:   ", &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(20 , 30, "PM1   =    ", &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(20 , 45, "PM2.5 =    ", &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(20 , 60, "PM10  =    ", &Font12, FONT_BACKGROUND, WHITE);
    GUI_DisString_EN(20 , 75, "PM100 =    ", &Font12, FONT_BACKGROUND, WHITE);
    GUI_OLED_Show_IP_address( 0, 103, 127, 127, WHITE);
    OLED_Display();


	back_timer_min = TimeGap[0][0];
	back_timer_sec = TimeGap[0][1];
	Period = 0;

	CoordInit( &Result[0].Area, 77, 30 , 127, 44);
	CoordInit( &Result[1].Area, 77, 45 , 127, 59);
	CoordInit( &Result[2].Area, 77, 60 , 127, 74);
	CoordInit( &Result[3].Area, 77, 75 , 127, 87);

	while(1){

		usleep(200000);
		time(&now);
		timenow = localtime(&now);

		now_time_sec = (uint8_t)timenow->tm_sec;
		cur_time_day = (uint8_t)timenow->tm_mday;

		if( cur_time_day != new_time_day ){

			new_time_day = cur_time_day;

			sprintf( CurFileName, "%s%s%02d_%02d_%02d.csv", Flash_path, FileNameTemp, timenow->tm_mday,timenow->tm_mon + 1, timenow->tm_year + 1900);

			//Создадим файл для нового дня
			OutputCSV = fopen( CurFileName, "w");
			ShowFileError( OutputCSV, ErrorArea);

			if ( OutputCSV != NULL ) {
				//Если файл открыт успешно, выведем заголовок таблицы
		//		fprintf( OutputCSV, "Срок замера,,Метео,,\"Массовые концентрации, мг/мг3 или мкг/м3\",,,,,,;\n");
		//		fprintf( OutputCSV, "Дата, Время, \"Та, c\",\"Pa, мм рт ст\",NO2,SO2,PM1,PM2.5,PM10,PM100;\n");
				fprintf( OutputCSV, "Срок замера,,\"Массовые концентрации, млг/м3 или мкг/м3\",,,,;\n");
				fprintf( OutputCSV, "Дата, Время, PM1,PM2.5,PM10,PM100;\n");
				fclose( OutputCSV );
				fsync( OutputCSV );
			}
		}

		if(now_time_sec != new_time_sec){
			GUI_ShowTimeDate( TimeDateArea.x1, TimeDateArea.y1, TimeDateArea.x2, TimeDateArea.y2 , timenow, &Font12, WHITE);
			OLED_DisWindow( TimeDateArea.x1, TimeDateArea.y1, TimeDateArea.x2, TimeDateArea.y2 );
			new_time_sec = now_time_sec;

			back_timer_sec = back_timer_sec-1;

			if ( new_time_sec % 30 == 0 ){
				GUI_OLED_Show_IP_address( IPArea.x1, IPArea.y1, IPArea.x2, IPArea.y2, WHITE);
				OLED_DisWindow( IPArea.x1, IPArea.y1, IPArea.x2, IPArea.y2 );
			}

			if(back_timer_sec < 0){
				back_timer_sec = 59;
				back_timer_min = back_timer_min - 1;

				if(back_timer_min < 0){ //Запуск очередного периода

					if ( Period == 0 ){
						// Окончание периода ожидания, переход к Продувке датчиков
						SN74_MUX_to_PMS_7003();
						retval_pms_7003 = 0;
						retval_pms_7003 = Dust_Sensor_PMS_7003_Set_Mode(fd_UART, MODE_WAKEUP);
						if (retval_pms_7003 == -1){
							LogERR( Flash_path, timenow, "Period0 to Period1  Set sensor PMS7003 mode wake up - error");
						}

						SN74_MUX_to_SDS198();
						retval_SDS198 = 0;
						retval_SDS198 = Dust_Sensor_SDS198_Set_Mode(fd_UART,  MODE_WAKEUP);
						if (retval_SDS198 == -1){
							LogERR( Flash_path, timenow, "Period0 to Period1 Set sensor SDS198 mode wake up - error");
						}
						GUI_Show_OLED_string( PeriodDescrArea.x1, PeriodDescrArea.y1, PeriodDescrArea.x2, PeriodDescrArea.y2, &Font12, "Preparing:",WHITE);

					} else if ( Period == 1 ){
						// Окончание периода продувки датчиков, переход к измерению
						SN74_MUX_to_PMS_7003();
						retval_pms_7003 = 0;
						retval_pms_7003 = Dust_Sensor_PMS_7003_Set_Mode(fd_UART, MODE_PASSIVE);
						if (retval_pms_7003 == -1){
							LogERR( Flash_path, timenow, "Period1 to Period2 Set sensor PMS7003 mode passive - error");
						}
						//Sensor SDS198 have flash to  save setting passive mode.
						GUI_Show_OLED_string( PeriodDescrArea.x1, PeriodDescrArea.y1, PeriodDescrArea.x2, PeriodDescrArea.y2, &Font12, "Measuring:",WHITE);

					} else if ( Period == 2 ){
						// Окончание периода измерения, переход к ожиданию
						SN74_MUX_to_PMS_7003();
						retval_pms_7003 = 0;
						retval_pms_7003 = Dust_Sensor_PMS_7003_Set_Mode(fd_UART, MODE_SLEEP);
						if (retval_pms_7003 == -1){
							LogERR( Flash_path, timenow, "Period2 to Period0 Set sensor PMS7003 mode sleep - error");
						}

						SN74_MUX_to_SDS198();
						retval_SDS198 = 0;
						retval_SDS198 = Dust_Sensor_SDS198_Set_Mode(fd_UART,  MODE_SLEEP);
						if (retval_SDS198 == -1){
							LogERR( Flash_path, timenow, "Period2 to Period0 Set sensor SDS198 mode sleep - error");
						}

						//Откроем файл текущего дня
						OutputCSV = fopen( CurFileName, "a");
						ShowFileError( OutputCSV, ErrorArea);
						if ( OutputCSV != NULL ) {
							fprintf( OutputCSV, "%02d.%02d.%02d, %02d:%02d:%02d,", timenow->tm_mday, timenow->tm_mon + 1, timenow->tm_year + 1900, timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
							fprintf( OutputCSV, " %d, %d, %d, %d;\r\n", Result[0].Value, Result[1].Value, Result[2].Value, Result[3].Value);
							fclose( OutputCSV );
							fsync( OutputCSV );
						}

						GUI_Show_OLED_string( PeriodDescrArea.x1, PeriodDescrArea.y1, PeriodDescrArea.x2, PeriodDescrArea.y2, &Font12, "Next start:",WHITE);
					}
					//обновим PeriodDescrArea
					OLED_DisWindow( PeriodDescrArea.x1, PeriodDescrArea.y1, PeriodDescrArea.x2, PeriodDescrArea.y2 );

					Period = (Period + 1) % NumOfPeriods;
					back_timer_min = TimeGap[Period][0];
					back_timer_sec = TimeGap[Period][1];

				}

			}
			if ( Period == 2 ){
				//Выведем полученные значения измерений
				SN74_MUX_to_PMS_7003();
				retval_pms_7003 = 0;
				retval_pms_7003 = Dust_Sensor_PMS_7003_Read_Data_Passive_Mode (fd_UART, data_pms_7003);
				if (retval_pms_7003 == -1 ){
					LogERR( Flash_path, timenow, "Error get data from sensor PMS_7003. Period 2");
					count_error_sensor_pms_7003++;
					serialFlush ( fd_UART );
				}else{
					count_error_sensor_pms_7003 = 0;
				}

				if(count_error_sensor_pms_7003 >= 2 ){ //если происходит три ошибки подряд ресетим датчик
					LogERR( Flash_path, timenow, "Error get data from sensor PMS_7003 three times. Period 2");
					Dust_Sensor_PMS_7003_Reset();
					SN74_MUX_to_PMS_7003();
					retval_pms_7003 = 0;
					retval_pms_7003 =  Dust_Sensor_PMS_7003_Set_Mode(fd_UART, MODE_PASSIVE);
					if (retval_pms_7003 == -1 ){
						LogERR( Flash_path, timenow, "Restart sensor PMS_7003 error ");
					}
				}

				Result[0].Value = data_pms_7003->pm1_0_atmospheric_envir;
				Result[1].Value = data_pms_7003->pm2_5_atmospheric_envir;
				Result[2].Value = data_pms_7003->pm10_atmospheric_envir;

				SN74_MUX_to_SDS198();
				retval_SDS198 = 0;
				retval_SDS198 = Dust_Sensor_SDS198_Read_Data_Passive_Mode (fd_UART, data_sds198);
				if (retval_SDS198 == -1 ){
					LogERR( Flash_path, timenow, "Error get data from sensor SDS198. Period 2");
					count_error_sensor_SDS198++;
					serialFlush ( fd_UART );
				}else{
					count_error_sensor_SDS198 = 0;
				}

				if(count_error_sensor_SDS198 >= 2 ){ //если происходит три ошибки подряд ресетим датчик
					LogERR( Flash_path, timenow, "Error get data sensor SDS198 three times. Period 2");

					//TO DO. Need add power switch on board.
				}

				Result[3].Value = *data_sds198;

				for( i = 0; i < 4; i++ ){

					OLED_ClearWindow( Result[i].Area.x1, Result[i].Area.y1, Result[i].Area.x2, Result[i].Area.y2, WHITE);

					if ( (retval_pms_7003 < 0) && (i < 3)){ // errors sensor
						GUI_DisString_EN( Result[i].Area.x1, Result[i].Area.y1, "Error", &Font12, FONT_BACKGROUND, WHITE  );
					}else if(  (retval_pms_7003 == 0) && (i < 3) ) {
						GUI_DisNum( Result[i].Area.x1, Result[i].Area.y1, Result[i].Value, &Font12, FONT_BACKGROUND, WHITE );
					}

					if( (retval_SDS198 < 0) && (i == 3) ){
						GUI_DisString_EN( Result[i].Area.x1, Result[i].Area.y1, "Error", &Font12, FONT_BACKGROUND, WHITE  );
					}else if( (retval_SDS198 == 0) && (i == 3) ) {
						GUI_DisNum( Result[i].Area.x1, Result[i].Area.y1, Result[i].Value, &Font12, FONT_BACKGROUND, WHITE );
					}

					OLED_DisWindow( Result[i].Area.x1, Result[i].Area.y1, Result[i].Area.x2, Result[i].Area.y2 );
				}
			}

			GUI_Show_OLED_min_sec( PeriodTimerArea.x1, PeriodTimerArea.y1, PeriodTimerArea.x2, PeriodTimerArea.y2, &Font12, back_timer_min, back_timer_sec, WHITE );
			OLED_DisWindow( PeriodTimerArea.x1, PeriodTimerArea.y1, PeriodTimerArea.x2, PeriodTimerArea.y2 );

		}

	}

	//3.System Exit
	System_Exit();
	return 0;

}


