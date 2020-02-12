/*
 * ModeBus.c
 *
 *  Created on: Jan 25, 2020
 *      Author: Ivan
 */
#include "ModeBus.h"

void ModBusMasterGenerateWriteReg(uint8_t slaveID, uint16_t* masterBuf, uint16_t slaveRegAddr, uint16_t slaveDataLen, uint8_t* pbuf,uint8_t* psize ){

uint8_t size=0;
int j,i;
 pbuf[size++] = slaveID;
 pbuf[size++] = 0x10;
 pbuf[size++] = (slaveRegAddr>>8)&0xFF;
 pbuf[size++] = (slaveRegAddr>>0)&0xFF;
 pbuf[size++] = (slaveDataLen>>8)&0xFF;
 pbuf[size++] = (slaveDataLen>>0)&0xFF;
 pbuf[size++] = (slaveDataLen<<1)&0xFF;

 for(i=0;i<(slaveDataLen);i++){
    j = (slaveRegAddr + i);
    pbuf[size++] = (masterBuf[j] >> 8) & 0xFF;
    pbuf[size++] = (masterBuf[j] >> 0) & 0xFF;
}
 CRC16_OUT(pbuf,&size);
 *psize = size;
}

uint8_t ModBusMasterCheckAnswerWriteReg(uint8_t slaveAddr,uint8_t* pbuf , uint8_t size){

  if (pbuf[0] != slaveAddr){
	  return 1;
  }
  if (pbuf[1] != 0x10) {
	  return 1;
  }
  if(CRC16_IN(pbuf,size)!=0){
	  return 1;
  }
  return 0;
}

void ModBusMasterGenerateReadReg(uint8_t slaveID, uint16_t slaveRegAddr, uint16_t slaveRegCounter, uint8_t* parsel_buf,uint8_t* parsel_size ){

 uint8_t size=0;

 parsel_buf[size++] = slaveID;
 parsel_buf[size++] = 0x03;
 parsel_buf[size++] = (slaveRegAddr>>8)&0xFF;
 parsel_buf[size++] = (slaveRegAddr>>0)&0xFF;
 parsel_buf[size++] = (slaveRegCounter>>8)&0xFF;
 parsel_buf[size++] = (slaveRegCounter>>0)&0xFF;

 CRC16_OUT(parsel_buf,&size);

 *parsel_size = size;
}


uint8_t ModBusMasterCheckAnswerReadReg(uint8_t slaveAddr, uint16_t* masterBuf ,uint16_t masterStartAddr, uint16_t masterDataLen, uint8_t* pbuf , uint8_t size){

  int i=0;

  if (pbuf[0] != slaveAddr){
	  return 1;
  }

  if (pbuf[1] != 0x03){
	  return 1;
  }

  if (pbuf[2] != masterDataLen*2){
	  return 1;
  }
  if(CRC16_IN(pbuf,size)!=0){
	  return 1;
  }

  for( i=0;i<masterDataLen;i++){
    if (isMRE(masterStartAddr + i)) masterBuf[masterStartAddr + i] =  256*pbuf[3+i*2]+pbuf[3+i*2+1];
  }
  return 0;

}



uint8_t MBMREMASK[ModBusMRMaskLen] = ModBusMasterReadEnabeMask;

int isMRE(uint16_t regn){

  if ((MBMREMASK[regn/8]&(0x01<<(regn%8)))>0) {
	  return 1;
  }else{
	  return 0;
  }

}

// Calculate CRC for outcoming buffer
// and place it to end.
void CRC16_OUT(uint8_t* buf,uint8_t* datacount){

  uint16_t crc = 0xFFFF;
  uint16_t pos = 0;
  uint8_t i =0;
  uint8_t lo =0;
  uint8_t hi =0;

  for (pos = 0; pos < *datacount; pos++){
    crc ^= buf[pos];

    for (i = 8; i != 0; i--){
    	if ((crc & 0x0001) != 0){
    		crc >>= 1;
    		crc ^= 0xA001;
    	}
    	else{
    		crc >>= 1;
    	}
    }
  }

  lo = crc & 0xFF;
  hi = ( crc >> 8 ) & 0xFF;

  buf[(*datacount)++] = lo;
  buf[(*datacount)++] = hi;
}

// Calculate CRC fro incoming buffer
// Return 0 - if CRC is correct
uint8_t CRC16_IN(uint8_t* buf,uint8_t datacount){

  uint16_t crc = 0xFFFF;
  uint16_t pos = 0;
  uint8_t i =0;
  uint8_t lo =0;
  uint8_t hi =0;

  for (pos = 0; pos < datacount-2; pos++){

    crc ^= buf[pos];
    for (i = 8; i != 0; i--){
    	if ((crc & 0x0001) != 0){
    		crc >>= 1;
    		crc ^= 0xA001;
    	}
    else
      crc >>= 1;
    }

  }

  lo = crc & 0xFF;
  hi = ( crc >> 8 ) & 0xFF;
  if( (buf[datacount-2] == lo) && (buf[datacount-1] == hi) ){
      return 0;
  }

  return 1;
}

