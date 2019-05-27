/*
 * Date:	01.5.2019
 *
 * Develop: Ivan Neskorodev
 * Email: ivan.neskorodev@gmail.com
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>		//printf()
#include <string.h>
#include <stdlib.h>		//exit()

#include "main.h"
#include "OLED_Driver.h"
#include "OLED_GUI.h"
#include "DEV_Config.h"

#include <time.h>

int main(void){

	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;
	void * tmpAddrPtr=NULL;

	getifaddrs(&ifAddrStruct);
	uint8_t i=0;

	//1.System Initialization
	if(System_Init())
		exit(0);

	printf("**********Init OLED**********\r\n");
	OLED_SCAN_DIR OLED_ScanDir = SCAN_DIR_DFT;//SCAN_DIR_DFT = D2U_L2R
	OLED_Init(OLED_ScanDir );

	GUI_DisString_EN(10 , 30, "ICT SB RAS", &Font16, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(25 , 60, "Dust monitor", &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(28 , 80, "PM1   PM2.5", &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(28 , 100, "PM10  PM100", &Font12, FONT_BACKGROUND, WHITE);
	OLED_Display();
    Driver_Delay_ms(5000);

//	printf("OLED Show \r\n");
//	GUI_Show(); //Use only for example

	OLED_Clear(OLED_BACKGROUND);//OLED_BACKGROUND
	OLED_Display();

	GUI_DisString_EN(10 , 40, "PM1   = 256", &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(10 , 55, "PM2.5 = 240", &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN(10 , 70, "PM10  = 230", &Font12, FONT_BACKGROUND, WHITE);
    GUI_DisString_EN(10 , 85,"PM100 = 220", &Font12, FONT_BACKGROUND, WHITE);


    for (ifa = ifAddrStruct, i=0 ; ifa != NULL; i++, ifa = ifa->ifa_next ) {
   	        if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
   	            // is a valid IP4 Address
   	            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
   	            char addressBuffer[INET_ADDRSTRLEN];
   	            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
   	            printf("%s: %s\n", ifa->ifa_name, addressBuffer);

   	            if(strcmp(ifa->ifa_name, "eth0")==0){
   	            	GUI_DisString_EN(0 , 105, "IPe", &Font12, FONT_BACKGROUND, WHITE);
   	            	GUI_DisString_EN(23 , 105, addressBuffer , &Font12, FONT_BACKGROUND, WHITE);
   	            }else if(strcmp(ifa->ifa_name, "wlan0")==0){
   	            	GUI_DisString_EN(0 , 115, "IPw" , &Font12, FONT_BACKGROUND, WHITE);
   	            	GUI_DisString_EN(23 , 115, addressBuffer , &Font12, FONT_BACKGROUND, WHITE);
   	            }
   	        } /* else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
   	            // is a valid IP6 Address
   	            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
   	            char addressBuffer[INET6_ADDRSTRLEN];
   	            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
   	            printf("%s: %s\n", ifa->ifa_name, addressBuffer);
   	        }*/
   	    }
   	    if (ifAddrStruct!=NULL)
   	        freeifaddrs(ifAddrStruct);//remember to free ifAddrStruct

    OLED_Display();

	printf("Show time\r\n");
	time_t now;
	struct tm *timenow;
	uint8_t now_time,new_time = 11;
	DEV_TIME sDev_time;

	while(1){



		time(&now);
		timenow = localtime(&now);

		sDev_time.Hour = (uint8_t)timenow->tm_hour;
		sDev_time.Min = (uint8_t)timenow->tm_min;
		sDev_time.Sec = (uint8_t)timenow->tm_sec;
		sDev_time.Day = (uint8_t)timenow->tm_mday;
		sDev_time.Month = (uint8_t)timenow->tm_mon;
		sDev_time.Year = (uint16_t)(timenow->tm_year+1900);
		now_time = (uint8_t)sDev_time.Sec;

		OLED_ClearWindow(0, 0, 127, 12, WHITE);
		GUI_ShowTimeDate(0, 0, 127, 12, &sDev_time, WHITE);
		if(now_time != new_time){
			OLED_DisWindow(0, 0, 127, 12);
			new_time = now_time;
		}
	}

	//3.System Exit
	System_Exit();
	return 0;

}


