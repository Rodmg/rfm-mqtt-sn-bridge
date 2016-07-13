#include "StatusLeds.h"

void offLeds()
{
  digitalWrite(LED_GREEN, 1);
  digitalWrite(LED_RED, 1);
}

void StatusLedsClass::begin()
{
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_GREEN, 1);
  digitalWrite(LED_RED, 1);
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
