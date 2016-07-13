#ifndef STATUS_LEDS_H
#define STATUS_LEDS_H

#include <Arduino.h>
#include <SimpleTimer.h>

#define LED_GREEN 3
#define LED_RED 4

class StatusLedsClass
{
private:
  SimpleTimer timer;
public:
  void begin();
  void loop();
  void blinkTx();
  void blinkRx();
};

extern StatusLedsClass StatusLeds;

#endif //STATUS_LEDS_H
