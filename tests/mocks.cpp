#include "mocks.h"

#include <fstream>
#include <iostream>
#include <string>

#include "..\temp_controller\constants.h"

long mockTime = 0;
uint8_t mockGoal = 100;

float (*getTempFunction)(void);

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

float getTemp() {
  return getTempFunction();
}

uint8_t getGoalTemp() {
  return mockGoal;
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
