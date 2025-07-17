#ifndef __MID_EEPROM_H_
#define __MID_EEPROM_H_

/* EEPROM(AT24C128) Page size: */
#define EEPROM_PAGE_SIZE 	64



void Mid_EEPROM_ByteWrite(unsigned short address, unsigned char Data);
unsigned char Mid_EEPROM_RandomRead(unsigned short address);
void Mid_EEPROM_PageWrite(unsigned short address, unsigned char *pDat, unsigned short Num);
void Mid_EEPROM_SequentialRead(unsigned short address, unsigned char *pBuffer, unsigned short Num);

#endif
