#ifndef SERIAL_ENDPOINT_H
#define SERIAL_ENDPOINT_H

#include <Arduino.h>
#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include <SPI.h>
#include "StatusLeds.h"
#include "Slip.h"
#include "CRC.h"

#define SERIAL_BUFF_SIZE RH_RF69_MAX_MESSAGE_LEN
#define LEN_INDEX 0
#define PLLEN_INDEX 5

#define HUB 0xF0

class SerialEndpointClass
{
private:
  Slip slip;
  bool pairMode;
  uint8_t buffer[SERIAL_BUFF_SIZE];
public:
  SerialEndpointClass();
  void begin();
  void loop();
  void send(uint16_t address, uint8_t *message, uint8_t len, uint8_t lqi, uint8_t rssi);
  void sendAck();
  void sendNack();

  void enterPairMode();
  void enterNormalMode();
  void sendPair(uint16_t address, uint8_t *message, uint8_t len, uint8_t lqi, uint8_t rssi);

};

extern SerialEndpointClass SerialEndpoint;

#endif //SERIAL_ENDPOINT_H
