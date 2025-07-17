#ifndef __HAL_I2C_H_
#define __HAL_I2C_H_

void Hal_I2C_SDA_Write(unsigned char BitValue);
void Hal_I2C_SCL_Write(unsigned char BitValue);

unsigned char Hal_I2C_SDA_Read(void);

void Hal_I2C_Start(void);
void Hal_I2C_Stop(void);

void Hal_I2C_SendByte(unsigned char Byte);
unsigned char Hal_I2C_ReceiveByte(void);

void Hal_I2C_SendACK(unsigned char ACKbit);
unsigned char Hal_I2C_RecACK(void);

void Hal_I2C_Delay(unsigned short t);

#endif
