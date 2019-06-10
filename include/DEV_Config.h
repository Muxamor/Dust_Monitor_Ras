#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_
/***********************************************************************************************************************
			------------------------------------------------------------------------
			|\\\																///|
			|\\\					Hardware interface							///|
			------------------------------------------------------------------------
***********************************************************************************************************************/
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>

#define USE_SPI_4W 1
#define USE_IIC 0
#define IIC_CMD        0X00
#define IIC_RAM        0X40


//OLED
//#define SPI_MOSI 10
//#define SPI_SCK  11
//#define I2C_SCL 3
//#define I2C_SDA 2
#define OLED_CS         8
#define OLED_RST        25
#define OLED_DC         24

#define OLED_CS_0      digitalWrite(OLED_CS,LOW)
#define OLED_CS_1      digitalWrite(OLED_CS,HIGH)

#define OLED_RST_0      digitalWrite(OLED_RST,LOW)
#define OLED_RST_1      digitalWrite(OLED_RST,HIGH)
#define OLED_RST_RD     digitalRead(OLED_RST)

#define OLED_DC_0       digitalWrite(OLED_DC,LOW)
#define OLED_DC_1       digitalWrite(OLED_DC,HIGH)

//SN74LV4052A - multiplexor UART
#define SN74_SET_A     digitalWrite(17,HIGH)
#define SN74_SET_B     digitalWrite(27,HIGH)
#define SN74_SET_INH     digitalWrite(22,HIGH)
#define SN74_RST_A     digitalWrite(17,LOW)
#define SN74_RST_B     digitalWrite(27,LOW)
#define SN74_RST_INH     digitalWrite(22,LOW)

/*------------------------------------------------------------------------------------------------------*/

uint8_t System_Init(void);
void    System_Exit(void);

void SPI4W_Write_Byte(uint8_t value);
void I2C_Write_Byte(uint8_t value, uint8_t Cmd);

void SN74_MUX_to_PMS_7003(void);
void SN74_MUX_to_SDS198(void);
void SN74_MUX_to_SIM7000E(void);
void SN74_MUX_to_External_Gas_Sensor(void);

void Driver_Delay_ms(uint32_t xms);
void Driver_Delay_us(uint32_t xus);

#endif
