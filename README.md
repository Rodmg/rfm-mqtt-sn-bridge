# rfm-mqtt-sn-bridge

MQTT-SN Bridge firmware for RFM69 rf module (915Mhz) and ATmega328p based boards, designed to work with [Aquila MQTT-SN Gateway](https://github.com/Rodmg/aquila-mqtt-sn-gateway).

## Requirements

For compiling and uploading this code to the board you need the PlatformIO tools and the [PlatformIO IDE](http://platformio.org/get-started).

You also need to install the **RadioHead library** from the PlatformIO library manager.

## Using

Open the project folder from the Atom IDE with PlatformIO IDE plugin installed, connect your board and press the upload button.

Right now the project is configured to work on an Arduino nano with ATmega328 or equivalent, that has the RFM69 connected via SPI the standard way that the [RadioHead library](http://www.airspayce.com/mikem/arduino/RadioHead/) recommends.

The code has been tested with the 915Mhz version of the RFM69C module, if you want to use other version, you have to edit the code at ``SerialEndpoint.cpp``, specially the part that calls the function ``setFrequency`` of the module driver.
