/*
  MQTT-SN Bridge
  LWM to Serial MQTT-SN Forwarder
  Complies with Forwarder Encapsulation of MQTT_SN spec v1.2
  (at Serial endpoint)

  Frame format:
  [Length][MsgType "0xFE" ][Ctrl][LWM short Address (2)][MQTT-SN message (n)]

    Length: 1-octet long, specifies the number of octets up to the end of the “Wireless Node Id” field (incl. the Length octet itself)
    MsgType: coded “0xFE”, see Table 3
    Ctrl: The Ctrl octet contains control information exchanged between the GW and the forwarder. Its format is shown in Table 29:
      – Radius: broadcast radius (only relevant in direction GW to forwarder)
      – All remaing bits are reserved
    Wireless Node Id: identifies the wireless node which has sent or should receive the encapsulated MQTT-SN message. The mapping between this Id and the address of the wireless node is implemented by the for- warder, if needed.
    MQTT-SN message: the MQTT-SN message, encoded according to Table 1.

	The frame is encapsulated inside a SLIP packet alongside 2 bytes of CRC at the end.

	Custom additions to the protocol:

		send ACK or NACK messages when receiving packet:

		NACK -> MsgType "0x00"
		ACK -> MsgType "0x01"

	**Note:** When sending from forwarder to pc, it has 2 extra bytes at the begining corresponding to lqi and rssi

*/

#include <RHReliableDatagram.h>
#include <RH_RF69_PAN.h>
#include <SPI.h>
#include "SerialEndpoint.h"
#include "StatusLeds.h"
#include <avr/wdt.h>

void yield()
{
  wdt_reset();
  StatusLeds.loop();
  SerialEndpoint.loop();
}

void setup()
{
  // Watchdog timer setup
  wdt_disable();
  wdt_reset();
  wdt_enable(WDTO_1S);
  // Activity indicator led
  StatusLeds.begin();
  SerialEndpoint.begin();
}

void loop()
{
  yield();
}
