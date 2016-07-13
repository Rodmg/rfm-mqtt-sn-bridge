#include "Slip.h"

#define SLIP_END (char)0xC0
#define SLIP_ESC (char)0xDB
#define SLIP_ESC_END (char)0xDC
#define SLIP_ESC_ESC (char)0xDD

Slip::Slip()
{
	state = SLIP_STATE_IDLE;
	onReceive = NULL;
	bufIndex = 0;
}

void Slip::parseEscapes()
{
	uint8_t size = bufIndex + 1;
	static char bufCopy[SLIP_MAX_SIZE];
	memcpy(bufCopy, buffer, size);

	// Will rewrite buffer with parsed escapes
	bufIndex = 0;
	for(uint8_t i = 0; i < size; i++)
	{
		if(bufCopy[i] == SLIP_ESC)
		{
			i++;
			if(bufCopy[i] == SLIP_ESC_END) buffer[bufIndex++] = SLIP_END;
			else if(bufCopy[i] == SLIP_ESC_ESC) buffer[bufIndex++] = SLIP_ESC;
			else buffer[bufIndex++] = bufCopy[i];	// This is a Escape error, shouldn't happen
		}
		else
		{
			buffer[bufIndex++] = bufCopy[i];
		}
	}
}

void Slip::begin(unsigned long baud, void(*_onReceive)(char* data, uint8_t size))
{
	Serial.begin(baud);
	onReceive = _onReceive;
}

void Slip::loop()
{
	if(Serial.available())
	{
		char recv = Serial.read();
		if(SLIP_STATE_IDLE == state)
		{
			if(SLIP_END == recv)
			{
				bufIndex = 0;
				state = SLIP_STATE_RECEIVING;
			}
			// else Framming error, ignore
		}

		if(SLIP_STATE_RECEIVING == state)
		{
			// Handle buffer overflow:
			// ignore data until now
			if(bufIndex >= SLIP_MAX_SIZE - 1)
			{
				state = SLIP_STATE_IDLE;
				return;
			}

			if(SLIP_END == recv)
			{
				// Handle error states (duplicated SLIP_END)
				if(bufIndex == 0) return;

				parseEscapes();
				if(onReceive != NULL) onReceive(buffer, bufIndex - 1);
				state = SLIP_STATE_IDLE;
			}
			else
			{
				buffer[bufIndex++] = recv;
			}
		}
	}
}

void Slip::send(char* data, uint8_t size)
{
	Serial.write(SLIP_END);
	for(uint8_t i = 0; i < size; i++)
	{
		if(data[i] == SLIP_END)
		{
			Serial.write(SLIP_ESC);
			Serial.write(SLIP_ESC_END);
		}
		else if(data[i] == SLIP_ESC)
		{
			Serial.write(SLIP_ESC);
			Serial.write(SLIP_ESC_ESC);
		}
		else
		{
			Serial.write(data[i]);
		}
	}
	Serial.write(SLIP_END);
}
