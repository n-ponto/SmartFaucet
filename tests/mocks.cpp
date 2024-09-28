#include "mocks.h"

#include <fstream>
#include <iostream>
#include <string>

#include "..\temp_controller\constants.h"

extern double hotWaterTemp;

unsigned long mockTime = 0;

void assertionFailure(std::string message) {
  std::cout << "\n[ASSERT FAILURE]: " << message << std::endl;
  throw std::exception();
}

void assert(char check, std::string message) {
  if (!check) {
    assertionFailure(message);
  }
}

_Serial Serial;  // Mock Serial object

void _Serial::begin(int baud) {
  // Do nothing
}

bool _Serial::operator!() {
  return false;
}

void refreshServos() {
  // Do nothing
}

void hardwareInit() {
  // Do nothing
}

uint8_t getTemp() {
  // Return weighted average of hot and cold water
  // printf("FAUCET: %d\n", curFaucet);
  double coldPercent = (double)(curFaucet) / 2 / MAX_FAUCET;
  // printf("coldPercent: %f\n", coldPercent);
  double hotPercent = 1.0 - coldPercent;
  // printf("hotPercent: %f\n", hotPercent);
  // printf("hotWaterTemp: %f\n", hotWaterTemp);
  return (unsigned char)((hotWaterTemp * hotPercent) + (COLD_WATER * coldPercent));
}

uint8_t getGoalTemp() {
  return 90;
}

void setFaucet(uint8_t value) {
  // printf("Setting faucet to %f\n", value);
  curFaucet = (int)value;
}

void setMotorEnable(bool enable) {
  // Do nothing
}

void updateScreen(uint8_t temp, uint8_t goal) {
  // Do nothing
}

void _Serial::println(const char* message) {
  printf("%s\n", message);
}

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
  unsigned long temp = mockTime;
  mockTime += SAMPLE_PERIOD;
  return temp;
}

uint8_t abs(uint8_t value) {
  return value < 0 ? -value : value;
}
