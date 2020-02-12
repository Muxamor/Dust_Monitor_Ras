/*
 * ModeBus.h
 *
 *  Created on: Jan 25, 2020
 *      Author: Ivan
 */


#ifndef INCLUDE_MODEBUS_H_
#define INCLUDE_MODEBUS_H_

#include <stdint.h>


#define ModBusMRMaskLen 12
#define ModBusMasterReadEnabeMask {0xFF,0xFF,0x00,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}

void ModBusMasterGenerateWriteReg(uint8_t slaveID, uint16_t* masterBuf, uint16_t slaveRegAddr, uint16_t slaveDataLen, uint8_t* pbuf,uint8_t* psize );
uint8_t ModBusMasterCheckAnswerWriteReg(uint8_t slaveAddr,uint8_t* pbuf , uint8_t size);

void ModBusMasterGenerateReadReg(uint8_t slaveID, uint16_t slaveRegAddr, uint16_t slaveCounterData, uint8_t* parsel_buf,uint8_t* parsel_size );
uint8_t ModBusMasterCheckAnswerReadReg(uint8_t slaveAddr, uint16_t* masterBuf ,uint16_t masterStartAddr, uint16_t masterDataLen, uint8_t* pbuf , uint8_t size);

int isMRE(uint16_t regn);
void CRC16_OUT(uint8_t* buf,uint8_t* datacount);
uint8_t CRC16_IN(uint8_t* buf,uint8_t datacount);

#endif /* INCLUDE_MODEBUS_H_ */
