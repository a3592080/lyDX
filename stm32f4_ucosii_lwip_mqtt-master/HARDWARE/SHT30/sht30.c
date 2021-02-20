#include "myiic.h"
#include "sht30.h"


static u8 SHT3X_StartWriteAccess(void)
{
   
		u8 error;
    // write a start condition 
    IIC_Start();

    // write the sensor I2C address with the write flag 
    IIC_Send_Byte(_address << 1);
		error=IIC_Wait_Ack();
	
    return error;
}


static u8 SHT3X_StartReadAccess(void)
{
    u8 error; // error code

    // write a start condition 
    IIC_Start();

    // write the sensor I2C address with the read flag 
    IIC_Send_Byte(_address << 1 | 0x01);
		error=IIC_Wait_Ack();
    return error;
}

uint8_t ReadSht3xMeasure(float *fRawTemperature, float *fRawHumidity)
{
  uint16_t wRawTemperature;
  uint16_t wRawHumidity;
  uint8_t bStatus;
  
  bStatus = InternalReadSht3xMeasure(&wRawTemperature, &wRawHumidity);
  if (0 != bStatus)
  {
    return bStatus;
  }
  //return (float)dwMeasure / (float)65536.0  * (float)175.72 - (float)46.85;
  //return (float)dwMeasure;
	*fRawHumidity = (float)wRawHumidity / 65535.0 * 100.0;
	*fRawTemperature = (float)wRawTemperature / 65535.0 * 175.0 - 45.0;
	
	return 0;
}

static uint8_t InternalReadSht3xMeasure(uint16_t *wRawTemperature, uint16_t *wRawHumidity)
{
  uint8_t bSlave = 0x44 << 1;
  uint8_t bStatus = 0;
  uint8_t bMeasureData[4];
  const uint8_t bCommandData[2] = {0x24, 0x16};
  uint8_t bCheckSum[2];
  uint32_t iPollWaitCount;
  //uint32_t dwMeasure;
  
  //*pbStatus = 0;
  //dwMeasure = 0;
  IIC_Start();
  IIC_Send_Byte(bSlave);
	if (0 != IIC_Wait_Ack())
  {
    bStatus = 0xfe;
    goto Fail;
  }
  IIC_Send_Byte(bCommandData[0]);
	if (0 != IIC_Wait_Ack())
  {
    bStatus = 0xfd;
    goto Fail;
  }
  IIC_Send_Byte(bCommandData[1]);
	if (0 != IIC_Wait_Ack())
  {
    bStatus = 0xfd;
    goto Fail;
  }
  //delay_us(20);
  IIC_Stop();


  iPollWaitCount = 50;
  while (0 != iPollWaitCount) 
  {
    IIC_Start();
    IIC_Send_Byte(bSlave | 0x01);	   //·¢ËÍÐ´ÃüÁî
    if (0 == IIC_Wait_Ack())
    {
      break;
    }
    IIC_Stop();
    
    iPollWaitCount --;
    delay_ms(10);
		//HAL_Delay(10);
  }
  
  bMeasureData[0]=IIC_Read_Byte(1);	
  bMeasureData[1]=IIC_Read_Byte(1);	
  bCheckSum[0]=IIC_Read_Byte(1);	
  bMeasureData[2]=IIC_Read_Byte(1);	
  bMeasureData[3]=IIC_Read_Byte(1);	
  bCheckSum[1]=IIC_Read_Byte(0);	
  
  //dwMeasure = ((((uint32_t)bMeasureData[0]) << 8) | ((uint32_t)bMeasureData[1])) & 0xfffffffc;
  if (!SHT2x_CheckCrc(bMeasureData, 2, bCheckSum[0]))
  {
    bStatus = 0xfc;
  }
  else if (!SHT2x_CheckCrc(bMeasureData + 2, 2, bCheckSum[1]))
  {
    bStatus = 0xfb;
  }
	
	*wRawTemperature = (((uint16_t)bMeasureData[0]) << 8) | ((uint16_t)bMeasureData[1]);
	*wRawHumidity =  (((uint16_t)bMeasureData[2]) << 8) | ((uint16_t)bMeasureData[3]);

Fail:
  IIC_Stop();
  return bStatus;  
}

static uint8_t SHT2x_CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
//==============================================================================
{
  uint8_t crc = 0xFF; 
  uint8_t byteCtr;
  const uint16_t POLYNOMIAL = 0x131;
  u8 bit;
  //
  //calculates 8-Bit checksum with given polynomial
  //
  for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
  { 
    crc ^= (data[byteCtr]);
  
    for (bit = 8; bit > 0; --bit)
    { 
      if (crc & 0x80) 
      {
        crc = (crc << 1) ^ POLYNOMIAL;
      }
      else 
      {
        crc = (crc << 1);
      }
    }
  }
  
  return (crc != checksum) ? 0 : 1;
}



uint8_t IICCheckSlave(uint8_t bSlave)
{
  IIC_Start();
  IIC_Send_Byte(bSlave);	   //·¢ËÍÐ´ÃüÁî
	if (0 != IIC_Wait_Ack())
  {
    return 0;
  }
  IIC_Stop();
  return 1;
}
