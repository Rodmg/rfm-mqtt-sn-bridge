#include "SerialEndpoint.h"

// Singleton instance of the radio driver
RH_RF69_PAN driver;
// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, HUB);

static SerialEndpointClass *self;
static uint8_t recBuffer[SERIAL_BUFF_SIZE];

// Commands:
#define MSG_NACK    0x00
#define MSG_ACK     0x01
#define MSG_CONFIG  0x02
#define MSG_PAIR    0x03
#define MSG_MQTT    0xFE
// Pair subcomands:
#define EXITPAIR  0x00
#define ENTERPAIR 0x01
#define PAIRREQ   0x02
#define PAIRRES   0x03

static uint8_t voidKey[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

static bool isVoidKey(uint8_t * key)
{
  if(key == NULL) return true;
  for(uint8_t i = 0; i < KEY_SIZE; i++)
  {
    if(key[i] != voidKey[i]) return false;
  }
  return true;
}

void SerialEndpointClass::sendAck()
{
  // First, lqi and rssi
  buffer[0] = 0;
  buffer[1] = 0;
  // Length, base encaspsulator len + original msg len
  buffer[2] = 2;
  // MsgType
  buffer[3] = MSG_ACK;

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
  buffer[3] = MSG_NACK;

  uint8_t size = buffer[2] + 2; // + lqi and rssi
  size = appendCrc((char*)buffer, size);
  slip.send((char*)buffer, size);
}

static void attendSerial(char *data, uint8_t size)
{
  // Data format here:
  // [len][msgType][...]
  StatusLeds.blinkTx();
  // Attend
  if(size < 3) return self->sendNack(); // bad data
  //uint8_t capLen = data[0];
  uint8_t msgType = data[1];
  if(msgType != MSG_MQTT && msgType != MSG_PAIR && msgType != MSG_CONFIG) return self->sendNack(); // unsupported message

  // Manage CONFIG
  if(msgType == MSG_CONFIG)
  {
    // Data format here:
    // [len][msgType][PAN][KEY1]...[KEY16]
    if(size < 19) return self->sendNack(); // bad data, min len, command, pan and 16x key
    uint8_t pan = data[2];
    Storage.setPan(pan);
    Storage.setKey((uint8_t*)&data[3]);
    self->loadPreferences();
    return;
  }

  // Manage entering and exiting pair mode
  if(msgType == MSG_PAIR)
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
  driver.setHeaderPan(0x00);
  driver.setEncryptionKey(NULL);
  self->sendAck();
}

void SerialEndpointClass::enterNormalMode()
{
  pairMode = false;
  loadPreferences();
  self->sendAck();
}

void SerialEndpointClass::begin()
{
  Storage.begin();
  slip.begin(115200, attendSerial);
  if (!manager.init())
    Serial.println("init failed");
  driver.setFrequency(915.0);
  driver.setModemConfig(RH_RF69_PAN::GFSK_Rb57_6Fd120);
  loadPreferences();
  sendConfigReq(); // Request PAN and Key config to Gateway
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
  buffer[3] = MSG_MQTT;
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
  buffer[3] = MSG_PAIR;
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

void SerialEndpointClass::loadPreferences()
{
  uint8_t pan = Storage.getPan();
  uint8_t key[KEY_SIZE];
  Storage.getKey(key);
  if(pan == 0 || pan == 0xFF) driver.setHeaderPan(DEFAULT_PAN);
  else driver.setHeaderPan(pan);
  if(isVoidKey(key)) driver.setEncryptionKey(NULL);
  else driver.setEncryptionKey(key);
}

void SerialEndpointClass::sendConfigReq()
{
  // First, lqi and rssi
  buffer[0] = 0;
  buffer[1] = 0;
  // Length, base encaspsulator len + original msg len
  buffer[2] = 2;
  // MsgType
  buffer[3] = MSG_CONFIG;

  uint8_t size = buffer[2] + 2; // + lqi and rssi
  size = appendCrc((char*)buffer, size);
  slip.send((char*)buffer, size);
}



SerialEndpointClass SerialEndpoint;
