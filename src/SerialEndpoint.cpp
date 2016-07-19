#include "SerialEndpoint.h"

// Singleton instance of the radio driver
RH_RF69 driver;
// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, HUB);

static SerialEndpointClass *self;
static uint8_t recBuffer[SERIAL_BUFF_SIZE];

#define PAIRCMD 0x03
#define ENTERPAIR 0x01
#define EXITPAIR 0x00
#define PAIRREQ 0x02
#define PAIRRES 0x03

void SerialEndpointClass::sendAck()
{
  // First, lqi and rssi
  buffer[0] = 0;
  buffer[1] = 0;
  // Length, base encaspsulator len + original msg len
  buffer[2] = 2;
  // MsgType
  buffer[3] = 0x01;

  uint8_t size = buffer[2] + 2; // + lqi and rssi
  size = appendCrc((char*)buffer, size);
  slip.send((char*)buffer, size);
}

void SerialEndpointClass::sendNack()
{
  // First, lqi and rssi
  buffer[0] = 0;
  buffer[1] = 0;
  // Length, base encaspsulator len + original msg len
  buffer[2] = 2;
  // MsgType
  buffer[3] = 0x00;

  uint8_t size = buffer[2] + 2; // + lqi and rssi
  size = appendCrc((char*)buffer, size);
  slip.send((char*)buffer, size);
}

static void attendSerial(char *data, uint8_t size)
{
  StatusLeds.blinkTx();
  // Attend
  if(size < 3) return self->sendNack(); // bad data
  //uint8_t capLen = data[0];
  uint8_t msgType = data[1];
  if(msgType != 0xFE && msgType != PAIRCMD) return self->sendNack(); // unsupported message

  // Manage entering and exiting pair mode
  if(msgType == PAIRCMD)
  {
    if(data[2] == ENTERPAIR) return self->enterPairMode();
    else if(data[2] == EXITPAIR) return self->enterNormalMode();
  }

  if(size < 5) return self->sendNack(); // bad data
  //uint8_t borderRadius = data[2]; // not used
  uint16_t addr;
  addr = data[3] & 0x00FF;
  addr |= ((uint16_t)data[4] << 8) & 0xFF00;
  uint8_t plSize = data[5];
  memcpy(recBuffer, &data[5], plSize); // NOTE: Only supporting 1 byte lenght TODO: consideer supporting 3 byte header

  if(manager.sendto(recBuffer, plSize, addr))
  {
    manager.waitPacketSent();
    self->sendAck();
    return;
  }
  self->sendNack();
}

SerialEndpointClass::SerialEndpointClass()
{
  self = this;
  pairMode = false;
}

void SerialEndpointClass::enterPairMode()
{
  pairMode = true;
  self->sendAck();
}

void SerialEndpointClass::enterNormalMode()
{
  pairMode = false;
  self->sendAck();
}

void SerialEndpointClass::begin()
{
  slip.begin(115200, attendSerial);
  if (!manager.init())
    Serial.println("init failed");
  driver.setFrequency(915.0);
  driver.setModemConfig(RH_RF69::GFSK_Rb57_6Fd120);
}

void SerialEndpointClass::loop()
{
  slip.loop();

  // Wait for a message addressed to us from the client
  uint8_t len = sizeof(recBuffer);
  uint8_t from;
  if (manager.recvfrom(recBuffer, &len, &from))
  {
    // Toggle LED
  	StatusLeds.blinkRx();
    if(pairMode) SerialEndpoint.sendPair(from, recBuffer, len, driver.lastRssi(), driver.lastRssi());
    else SerialEndpoint.send(from, recBuffer, len, driver.lastRssi(), driver.lastRssi());
    return;
  }
}

void SerialEndpointClass::send(uint16_t address, uint8_t *message, uint8_t len, uint8_t lqi, uint8_t rssi)
{
  // First, lqi and rssi
  buffer[0] = lqi;
  buffer[1] = rssi;
  // Length, base encaspsulator len + original msg len
  buffer[2] = 5 + len;
  // MsgType, always 0xFE
  buffer[3] = 0xFE;
  // Ctrl, border radius, max 0x03 according to spec
  buffer[4] = 0x01;
  // Wireless node id low byte
  buffer[5] = (uint8_t)(address & 0xFF);
  // Wireless node id high byte
  buffer[6] = (uint8_t)(address >> 8);
  // original mqtt-sn msg
  memcpy(&buffer[7], message, len);

  uint8_t size = buffer[2] + 2; // + lqi and rssi
  size = appendCrc((char*)buffer, size);
  slip.send((char*)buffer, size);
}

void SerialEndpointClass::sendPair(uint16_t address, uint8_t *message, uint8_t len, uint8_t lqi, uint8_t rssi)
{
  // First, lqi and rssi
  buffer[0] = lqi;
  buffer[1] = rssi;
  // Length, base encaspsulator len + original msg len
  buffer[2] = 5 + len;
  // MsgType, always 0x03
  buffer[3] = PAIRCMD;
  // Ctrl, border radius, max 0x03 according to spec
  buffer[4] = PAIRREQ;
  // Wireless node id low byte
  buffer[5] = (uint8_t)(address & 0xFF);
  // Wireless node id high byte
  buffer[6] = (uint8_t)(address >> 8);
  // pair content
  memcpy(&buffer[7], message, len);

  uint8_t size = buffer[2] + 2; // + lqi and rssi
  size = appendCrc((char*)buffer, size);
  slip.send((char*)buffer, size);
}

SerialEndpointClass SerialEndpoint;
