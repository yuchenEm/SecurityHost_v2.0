#ifndef __CRC16_H_
#define __CRC16_H_

unsigned short Mid_CRC16_Modbus(unsigned char *ptr, unsigned int len);
unsigned short Mid_CRC16_Modbus_Continuous(unsigned char *ptr, unsigned int len, unsigned short crc16);

#endif
