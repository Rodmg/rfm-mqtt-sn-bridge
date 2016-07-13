#include "CRC.h"

uint16_t calcCrc(char *data, int size)
{
  int crc;
  char i;
  crc = 0;
  while (--size >= 0)
  {
    crc = crc ^ (int) *data++ << 8;
    i = 8;
    do
    {
      if (crc & 0x8000)
      crc = crc << 1 ^ 0x1021;
      else
      crc = crc << 1;
    } while(--i);
  }
  return (crc);
}

// returns new size
int appendCrc(char *buffer, int size)
{
  uint16_t crc = calcCrc(buffer, size);
  // low
  buffer[size] = (uint8_t)(crc);
  // high
  buffer[size + 1] = (uint8_t)(crc >> 8);

  // new size
  return size + 2;
}

bool checkCrc(char *data, int size)
{
	uint16_t crc, calcdCrc;
	// Extract crc
	// high
	crc = data[size-1];
	crc = crc << 8;
	// low
	crc |= data[size-2]&0x00FF;	// 0x00FF IMPORTANT

	size -= 2;

	calcdCrc = calcCrc(data, size);

	return crc == calcdCrc;
}