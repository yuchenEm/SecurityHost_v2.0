/****************************************************
  * @Name	Mid_EEPROM.c
  * @Brief	
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "mid_eeprom.h"
#include "hal_i2c.h"
#include "device.h"

/*-------------Internal Functions Declaration------*/


/*-------------Module Variables Declaration--------*/


/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
  
/*-------------------------- EEPROM Funcrion --------------------------------------*/
/* Device: AT24C128, Device address: (MSB->LSB) 1 0 1 0 0 A1 A0 R/W		      */
/* A1, A0 ground --> Device address: (MSB->LSB) 1 0 1 0 0 0 0 R/W	              */
/* write data to EEPROM  --> Device address: (R/W = 0) 1 0 1 0 0 0 0 0	          */
/* read data from EEPROM --> Device address: (R/W = 1) 1 0 1 0 0 0 0 1	          */

/*----------------------------------------------------------------------------
@Name		: Mid_EEPROM_ByteWrite(address, Byte)
@Function	: Write 1 byte of data to EEPROM
@Parameter	: Device Address: (R/W = 0) 1 0 1 0 0 0 0 0 (0xA0)
		--> address	: The EEPROM address where the data will be written
		--> Byte	: The byte of data to be written
------------------------------------------------------------------------------*/
void Mid_EEPROM_ByteWrite(unsigned short address, unsigned char Data)
{
	Hal_I2C_Start();
	
	Hal_I2C_SendByte(0xA0);
	Hal_I2C_RecACK();
	
	Hal_I2C_SendByte((address >> 8) & 0xFF); // high byte
	Hal_I2C_RecACK();
	
	Hal_I2C_SendByte(address & 0xFF); 		// low byte
	Hal_I2C_RecACK();
	
	Hal_I2C_SendByte(Data);
	Hal_I2C_RecACK();
	
	Hal_I2C_Stop();
	
	Hal_I2C_Delay(20000);		
}

/*----------------------------------------------------------------------------
@Name		: Mid_EEPROM_RandomRead(address)
@Function	:  Read 1 byte of data from any address in the EEPROM
				@Random mode： A random read requires a “dummy” byte write sequence 
					to load in the data word address.
@Parameter	: Device address: (R/W = 1) 1 0 1 0 0 0 0 1 (0xA1)
		--> address: The memory address from which to read the data
------------------------------------------------------------------------------*/
unsigned char Mid_EEPROM_RandomRead(unsigned short address)
{
	unsigned char RxByte;
	
	Hal_I2C_Start();
	
	Hal_I2C_SendByte(0xA0);
	Hal_I2C_RecACK();
	
	Hal_I2C_SendByte((address >> 8) & 0xFF); 
	Hal_I2C_RecACK();
	
	Hal_I2C_SendByte(address & 0xFF); 		
	Hal_I2C_RecACK();
	
	Hal_I2C_Start();
	
	Hal_I2C_SendByte(0xA1);
	Hal_I2C_RecACK();
	
	RxByte = Hal_I2C_ReceiveByte();
	Hal_I2C_SendACK(1);				// Send NACK to slave and end communication
	
	Hal_I2C_Stop();
	
	return RxByte;
}

/*----------------------------------------------------------------------------
@Name		: Mid_EEPROM_PageWrite(address, pDat, Num)
@Function	: Write data to the EEPROM at the specified address in pages (with automatic page turning)
@Parameter	: Device address: (R/W = 0) 1 0 1 0 0 0 0 0 (0xA0)
		--> address	： The EEPROM address where the data will be written
		--> pDat	： A pointer to the continuous block of data to be written
		--> Num		： Number of bytes to write (1–65536, i.e., up to 64KB)
@Note：
		This is an EEPROM page write function. The AT24C128 supports writing up to 64 bytes per page. Writing more than 64 bytes will overwrite existing data on the same page.
		Automatic page turning: It determines whether to move to the next page based on the starting address and the number of bytes to be written.

	*** Since Num is of type short, this function can write up to 64KB of data at a time. To support larger data sizes, change the type to int ***
------------------------------------------------------------------------------*/
void Mid_EEPROM_PageWrite(unsigned short address, unsigned char *pDat, unsigned short Num)
{
	unsigned short i, j;
	unsigned short temp = 0;
	unsigned short RemainedBytes; // remaining bytes in the page
	unsigned short Page;
	unsigned char* pBuffer;
	pBuffer = pDat;
	
	if(address % EEPROM_PAGE_SIZE)
	{
		temp = EEPROM_PAGE_SIZE - (address % EEPROM_PAGE_SIZE); // remaining bytes in the page
		if(temp >= Num)
		{
			temp = Num;			
		}
	}	
	
	if(temp) 		
	{
		Hal_I2C_Start();
		
		Hal_I2C_SendByte(0xA0);
		Hal_I2C_RecACK();
		
		Hal_I2C_SendByte((address >> 8) & 0xFF); 
		Hal_I2C_RecACK();
		
		Hal_I2C_SendByte(address & 0xFF); 		
		Hal_I2C_RecACK();
		
		for(i=0; i<temp; i++)
		{
			Hal_I2C_SendByte(pBuffer[i]);
			Hal_I2C_RecACK();
			
		}
		
		Hal_I2C_Stop();
		
		Hal_I2C_Delay(20000);				
	}
	
	Num -= temp; 					
	address += temp;				
	
	Page = Num / EEPROM_PAGE_SIZE;			// remaining full pages
	RemainedBytes = Num % EEPROM_PAGE_SIZE;	// last page bytes
	
	for(i=0; i<Page; i++)
	{
		Hal_I2C_Start();
		
		Hal_I2C_SendByte(0xA0);
		Hal_I2C_RecACK();
		
		Hal_I2C_SendByte((address >> 8) & 0xFF); 
		Hal_I2C_RecACK();
		
		Hal_I2C_SendByte(address & 0xFF); 		
		Hal_I2C_RecACK();
		
		for(j=0; j<EEPROM_PAGE_SIZE; j++)
		{
			Hal_I2C_SendByte(pBuffer[temp + j]);
			Hal_I2C_RecACK();
			
		}
		
		Hal_I2C_Stop();
		
		Hal_I2C_Delay(20000);				
		
		address += EEPROM_PAGE_SIZE;
		temp += EEPROM_PAGE_SIZE;
	}
	
	if(RemainedBytes)
	{
		Hal_I2C_Start();
		
		Hal_I2C_SendByte(0xA0);
		Hal_I2C_RecACK();
		
		Hal_I2C_SendByte((address >> 8) & 0xFF); 
		Hal_I2C_RecACK();
		
		Hal_I2C_SendByte(address & 0xFF); 		
		Hal_I2C_RecACK();
		
		for(j=0; j<RemainedBytes; j++)
		{
			Hal_I2C_SendByte(pBuffer[temp + j]);
			Hal_I2C_RecACK();
			
		}
		
		Hal_I2C_Stop();
		
		Hal_I2C_Delay(20000);				
	}
}


/*----------------------------------------------------------------------------
@Name		: Mid_EEPROM_SequentialRead(address, pBuffer, Num)
@Function	: Read a sequence of data continuously from a specified EEPROM address
@Parameter	: Device address: (R/W = 1) 1 0 1 0 0 0 0 1 (0xA1)
		--> address	： The starting memory address from which to read the data
		--> pBuffer	： A pointer to the buffer where the read data will be stored
		--> Num		： The number of bytes to read

	*** The Num parameter is of type short, which means this function can read up to 64KB of data at a time. To support reading more than 64KB, change the type of Num to int ***
------------------------------------------------------------------------------*/
// When the data address reaches the storage address limit, it rolls over. The reading continues until the host sends a stop signal
void Mid_EEPROM_SequentialRead(unsigned short address, unsigned char *pBuffer, unsigned short Num)
{
	unsigned short len;
	len = Num;
	
	Hal_I2C_Start();
	
	Hal_I2C_SendByte(0xA0);
	Hal_I2C_RecACK();
	
	Hal_I2C_SendByte((address >> 8) & 0xFF); 
	Hal_I2C_RecACK();
	
	Hal_I2C_SendByte(address & 0xFF); 		
	Hal_I2C_RecACK();
	
	Hal_I2C_Start();
	
	Hal_I2C_SendByte(0xA1);
	Hal_I2C_RecACK();
	
	while(len)
	{
		*pBuffer = Hal_I2C_ReceiveByte();
		
		if(len == 1)
		{
			Hal_I2C_SendACK(1);
		}
		else
		{
			Hal_I2C_SendACK(0);
		}
		pBuffer++;
		len--;
	}
	
	Hal_I2C_Stop();
}

/*-------------Internal Functions Definition--------*/


/*-------------Interrupt Functions Definition--------*/
