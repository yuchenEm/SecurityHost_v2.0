/****************************************************
  * @Name	Hal_I2C.c
  * @Brief	Software I2C Driver
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "hal_i2c.h"
#include "hal_gpio.h"


/*-------------Internal Functions Declaration------*/


/*-------------Module Variables Declaration--------*/


/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/

  
/*---------------------------- I2C Driver Functions --------------------------*/
/*----------------------------------------------------------------------------
@Name		: Hal_I2C_SDA_Write(BitValue)
@Function	: I2C SDA（1：high，0：low）

@Name		: Hal_I2C_SCL_Write(BitValue)
@Function	: I2C SCL（1：high，0：low）

@Name		: Hal_I2C_SDA_Read()
@Function	: I2C SDA readout（Return Value： 1：high，0：low）

@Name		: Hal_I2C_Start()
@Function	: I2C start signal

@Name		: Hal_I2C_Stop()
@Function	: I2C stop signal

@Name		: Hal_I2C_SendByte(Byte)
@Function	: I2C send 1byte

@Name		: Hal_I2C_ReceiveByte()
@Function	: I2C receive 1byte

@Name		: Hal_I2C_SendACK(ACKbit)
@Function	: I2C send ACK to slave （1：NACK，0：ACK）

@Name		: Hal_I2C_RecACK()
@Function	: I2C waiting ACK from slave （Return Value： 1：NACK，0：ACK）

@Name		: Hal_I2C_Delay(t)
@Function	: I2C delay
------------------------------------------------------------------------------*/

void Hal_I2C_SDA_Write(unsigned char BitValue)
{
	GPIO_WriteBit(I2C_SDA_PORT, I2C_SDA_PIN, (BitAction)BitValue);
	Hal_I2C_Delay(1);
}

void Hal_I2C_SCL_Write(unsigned char BitValue)
{
	GPIO_WriteBit(I2C_SCL_PORT, I2C_SCL_PIN, (BitAction)BitValue);
	Hal_I2C_Delay(1);
}

unsigned char Hal_I2C_SDA_Read(void)
{
	unsigned char BitValue;
	
	BitValue = GPIO_ReadInputDataBit(I2C_SDA_PORT, I2C_SDA_PIN);
	Hal_I2C_Delay(1);
	return BitValue;
}

void Hal_I2C_Start(void)
{
	Hal_I2C_SDA_Write(1);
	Hal_I2C_SCL_Write(1);
	Hal_I2C_SDA_Write(0);
	Hal_I2C_SCL_Write(0);
}

void Hal_I2C_Stop(void)
{
	Hal_I2C_SDA_Write(0);
	Hal_I2C_SCL_Write(1);
	Hal_I2C_SDA_Write(1);
}

void Hal_I2C_SendByte(unsigned char Byte)
{
	unsigned char i;
	
	for(i=0; i<8; i++)
	{
		Hal_I2C_SDA_Write(Byte & (0x80>>i));
		Hal_I2C_SCL_Write(1);
		Hal_I2C_SCL_Write(0);
	}
	
	Hal_I2C_SCL_Write(0);
	Hal_I2C_SDA_Write(1);
}

unsigned char Hal_I2C_ReceiveByte(void)
{
	unsigned char Byte = 0x00;
	unsigned char i;
	
	Hal_I2C_SDA_Write(1);
	
	for(i=0; i<8; i++)
	{
		Hal_I2C_SCL_Write(1);
		if(Hal_I2C_SDA_Read() == 1)
		{
			Byte |= (0x80>>i);
		}
		Hal_I2C_SCL_Write(0);
	}
	
	Hal_I2C_SCL_Write(0);
	Hal_I2C_SDA_Write(1);
	
	return Byte;
}

void Hal_I2C_SendACK(unsigned char ACKbit)
{
	Hal_I2C_SDA_Write(ACKbit);
	Hal_I2C_SCL_Write(1);
	Hal_I2C_SCL_Write(0);
}

unsigned char Hal_I2C_RecACK(void)
{
	unsigned char ACKbit;
	
	Hal_I2C_SDA_Write(1);
	Hal_I2C_SCL_Write(1);			
	ACKbit = Hal_I2C_SDA_Read();
	Hal_I2C_SCL_Write(0);
	
	return ACKbit;
}

void Hal_I2C_Delay(unsigned short t)
{
	unsigned short i = 50;
	unsigned short j, k;
	k = t;
	for(j=0; j<k; j++)
	{
		while(i)
		{
			i--;
		}
	}
}

/*-------------Internal Functions Definition--------*/


/*-------------Interrupt Functions Definition--------*/


