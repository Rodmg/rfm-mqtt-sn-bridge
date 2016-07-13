#ifndef SLIP_H
#define SLIP_H

#include <Arduino.h>

#define SLIP_MAX_SIZE 255

#define SLIP_STATE_IDLE 0
#define SLIP_STATE_RECEIVING 1
#define SLIP_STATE_ERROR 2

class Slip
{
private:
	char buffer[SLIP_MAX_SIZE];
	uint8_t bufIndex;
	uint8_t state;
	void(*onReceive)(char* data, uint8_t size);
	void parseEscapes();

public:
	Slip();
	void begin(unsigned long baud, void(*_onReceive)(char* data, uint8_t size));
	void loop();
	void send(char* data, uint8_t size);
};

#endif //SLIP_H
