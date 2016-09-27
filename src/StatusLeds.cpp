#include "StatusLeds.h"

void offLeds()
{
  digitalWrite(LED_GREEN, 0);
  digitalWrite(LED_RED, 0);
}

void StatusLedsClass::begin()
{
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_GREEN, 0);
  digitalWrite(LED_RED, 0);
  // Turn off status leds
  timer.setInterval(500, offLeds);
}

void StatusLedsClass::loop()
{
  timer.run();
}

void StatusLedsClass::blinkTx()
{
  digitalWrite(LED_RED, !digitalRead(LED_RED));
}

void StatusLedsClass::blinkRx()
{
  digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
}

StatusLedsClass StatusLeds;
