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
	int8_t Value;
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
		OLED_DisWindow( ErrorArea.x1, ErrorArea.y1, ErrorArea.x2, ErrorArea.y2 );
	}else{
		OLED_ClearWindow( ErrorArea.x1, ErrorArea.y1, ErrorArea.x2, ErrorArea.y2, WHITE);
	}
}


int main(void){
	printf("**********System Initialization**********\r\n");
	if(System_Init()){ //wiringPi System Initialization
		//fprintf (stdout, "WiringPi System Initialization: %s\n", strerror (errno)) ;
		perror("WiringPi System Initialization - FAILUR");
		exit(0);
	}

	//int fd_UART;
	//fd_UART = serialOpen ("/dev/ttyAMA0", 115200);
	//serialPutchar (fd_UART, 'A');
	//serialGetchar (fd_UART);

	printf("**********Init OLED**********\r\n");

	OLED_SCAN_DIR OLED_ScanDir = SCAN_DIR_DFT;//SCAN_DIR_DFT = D2U_L2R
	OLED_Init(OLED_ScanDir );

	GUI_OLED_Show_Start_screan(1);

	OLED_Clear(OLED_BACKGROUND);//OLED_BACKGROUND
	OLED_Display();

	GUI_DisString_EN(0 , 15,"Next start:   ", &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(20 , 30, "PM1   =    ", &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(20 , 45, "PM2.5 =    ", &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(20 , 60, "PM10  =    ", &Font12, FONT_BACKGROUND, WHITE);
    GUI_DisString_EN(20 , 75, "PM100 =    ", &Font12, FONT_BACKGROUND, WHITE);
    GUI_OLED_Show_IP_address( 0, 103, 127, 127, WHITE);
    OLED_Display();

 //   GUI_OLED_Show_IP_address( 0, 103, 127, 127, WHITE);
   // OLED_DisWindow(0, 103, 127, 127);

	printf("Show time\r\n");
	time_t now;
	struct tm *timenow;

	FILE *OutputCSV;
	char *FileNameTemp = "/home/pi/Dust_monitor_";
	char CurFileName[100];
	int8_t TimeGap[3][2] = { { 0, 2 }, { 0, 2 }, { 0, 2 } }; // массив таймеров (мин, сек) для периодов ожидания, продувки и измерения соответственно
	int8_t Period;// Возможные значения 0 - ожидание, 1 -продувка датчиков и 2 - измерение
	int8_t NumOfPeriods = 3; // Количество периодов


	struct MeasureResult Result[4]; // массив измерений PM1, PM2.5, PM10, PM100

	struct DisplayArea TimeDateArea = { 0, 0, 127, 12 };
	struct DisplayArea PeriodDescrArea = { 0, 15, 83, 27 };
	struct DisplayArea PeriodTimerArea = { 84, 15, 119, 27 };
	struct DisplayArea ErrorArea = { 0, 88, 127, 102 };
	struct DisplayArea IPArea = { 0, 103, 127, 127 };

	int i;

	uint8_t cur_time_day,new_time_day = 0;
	uint8_t now_time_sec,new_time_sec = 0;
	int8_t back_timer_min, back_timer_sec=0;

	back_timer_min = TimeGap[0][0];
	back_timer_sec = TimeGap[0][1];
	Period = 0;

	CoordInit( &Result[0].Area, 77, 30 , 127, 44);
	CoordInit( &Result[1].Area, 77, 45 , 127, 59);
	CoordInit( &Result[2].Area, 77, 60 , 127, 74);
	CoordInit( &Result[3].Area, 77, 75 , 127, 87);

	while(1){

		//usleep(250000);
		time(&now);
		timenow = localtime(&now);

		now_time_sec = (uint8_t)timenow->tm_sec;
		cur_time_day = (uint8_t)timenow->tm_mday;

		if( cur_time_day != new_time_day ){

			new_time_day = cur_time_day;

			sprintf( CurFileName, "%s%02d_%02d_%02d.csv", FileNameTemp, timenow->tm_mday,timenow->tm_mon + 1, timenow->tm_year + 1900);

			//Создадим файл для нового дня
			OutputCSV = fopen( CurFileName, "w");

			ShowFileError( OutputCSV, ErrorArea);

			if ( OutputCSV != NULL ) {
				//Если файл открыт успешно, выведем заголовок таблицы
		//		fprintf( OutputCSV, "Срок замера,,Метео,,\"Массовые концентрации, мг/мг3 или мкг/м3\",,,,,,;\n");
		//		fprintf( OutputCSV, "Дата, Время, \"Та, c\",\"Pa, мм рт ст\",NO2,SO2,PM1,PM2.5,PM10,PM100;\n");
				fprintf( OutputCSV, "Срок замера,,\"Массовые концентрации, мг/мг3 или мкг/м3\",,,,;\n");
				fprintf( OutputCSV, "Дата, Время, PM1,PM2.5,PM10,PM100;\n");
				fclose( OutputCSV );
			}
		}

		if(now_time_sec != new_time_sec){
			GUI_ShowTimeDate( TimeDateArea.x1, TimeDateArea.y1, TimeDateArea.x2, TimeDateArea.y2 , timenow, &Font12, WHITE);
			OLED_DisWindow( TimeDateArea.x1, TimeDateArea.y1, TimeDateArea.x2, TimeDateArea.y2 );
			new_time_sec = now_time_sec;

			//OLED_ClearWindow(0, 15, 42, 27, WHITE);
			//GUI_DisString_EN(0 , 15,"09:36", &Font12, FONT_BACKGROUND, WHITE);

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
						GUI_Show_OLED_string( PeriodDescrArea.x1, PeriodDescrArea.y1, PeriodDescrArea.x2, PeriodDescrArea.y2, &Font12, "Preparing:",WHITE);
					} else if ( Period == 1 ){
						// Окончание периода продувки датчиков, переход к измерению
						GUI_Show_OLED_string( PeriodDescrArea.x1, PeriodDescrArea.y1, PeriodDescrArea.x2, PeriodDescrArea.y2, &Font12, "Measuring:",WHITE);
					} else if ( Period == 2 ){
						// Окончание периода измерения, переход к ожиданию

						//Откроем файл текущего дня
						OutputCSV = fopen( CurFileName, "a");
						ShowFileError( OutputCSV, ErrorArea);
						if ( OutputCSV != NULL ) {
							fprintf( OutputCSV, "%02d.%02d.%02d, %02d:%02d:%02d,", timenow->tm_mday, timenow->tm_mon + 1, timenow->tm_year + 1900, timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
							fprintf( OutputCSV, " %d, %d, %d, %d;\r\n", Result[0].Value, Result[1].Value, Result[2].Value, Result[3].Value);
							fclose( OutputCSV );
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
				for( i = 0; i < 4; i++ ){

					Result[i].Value = rand()%1000;//имулируется получение значения с датчика

					OLED_ClearWindow( Result[i].Area.x1, Result[i].Area.y1, Result[i].Area.x2, Result[i].Area.y2, WHITE);
					if (Result[i].Value < 0 ){ // ошибка получения данных с датчика

						GUI_DisString_EN( Result[i].Area.x1, Result[i].Area.y1, "Error", &Font12, FONT_BACKGROUND, WHITE  );

					} else {

						GUI_DisNum( Result[i].Area.x1, Result[i].Area.y1, Result[i].Value, &Font12, FONT_BACKGROUND, WHITE );
					}

					OLED_DisWindow( Result[i].Area.x1, Result[i].Area.y1, Result[i].Area.x2, Result[i].Area.y2 );
				}
			}

			GUI_Show_OLED_min_sec( PeriodTimerArea.x1, PeriodTimerArea.y1, PeriodTimerArea.x2, PeriodTimerArea.y2, &Font12, back_timer_min, back_timer_sec,WHITE);
			OLED_DisWindow( PeriodTimerArea.x1, PeriodTimerArea.y1, PeriodTimerArea.x2, PeriodTimerArea.y2 );

		}

	}

	//3.System Exit
	System_Exit();
	return 0;

}


