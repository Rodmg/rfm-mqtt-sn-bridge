#ifndef CRC_H
#define CRC_H

#include <Arduino.h>

uint16_t calcCrc(char *data, int size);

int appendCrc(char *buffer, int size);

bool checkCrc(char *data, int size);

#endif //CRC_H