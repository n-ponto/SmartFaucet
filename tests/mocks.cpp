#include "mocks.h"

#include <stdio.h>

extern const unsigned long samplePeriod;
extern const int tempDelayMs;
extern double hotWaterTemp;

unsigned long time = 0;

_Serial Serial;  // Mock Serial object

void _Serial::begin(int baud) {
  // Do nothing
}

bool _Serial::operator!() {
  return false;
}

unsigned char faucetValue = 0;

double constrain(double value, double min, double max) {
  if (value < min) {
    value = min;
  } else if (value > max) {
    value = max;
  }
  return value;
}

void delay(unsigned long ms) {
  // Do nothing
}

unsigned long millis() {
  unsigned long temp = time;
  time += samplePeriod;
  return temp;
}

void hardwareInit() {
  // Do nothing
}

void setFaucet(double value) {
  // printf("Setting faucet to %f\n", value);
  faucetValue = (int)value;
}

unsigned char getTemp() {
  // Return weighted average of hot and cold water
  // printf("FAUCET: %d\n", faucetValue);
  double coldPercent = (double)(faucetValue) / 2 / maxFaucet;
  // printf("coldPercent: %f\n", coldPercent);
  double hotPercent = 1.0 - coldPercent;
  // printf("hotPercent: %f\n", hotPercent);
  // printf("hotWaterTemp: %f\n", hotWaterTemp);
  return (unsigned char)((hotWaterTemp * hotPercent) + (COLD_WATER * coldPercent));
}