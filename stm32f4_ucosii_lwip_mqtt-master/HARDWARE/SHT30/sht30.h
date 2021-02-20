#ifndef __SHT30_H
#define __SHT30_H

#include "sys.h"
#include "delay.h"

#define _address 0x44

uint8_t ReadSht3xMeasure(float *fRawTemperature, float *fRawHumidity);
static uint8_t InternalReadSht3xMeasure(uint16_t *wRawTemperature, uint16_t *wRawHumidity);

static uint8_t SHT2x_CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum);
uint8_t IICCheckSlave(uint8_t bSlave);





#endif
