#ifndef TESTS_H_
#define TESTS_H_

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define COLD_WATER 55
#define HOT_WATER 120

typedef unsigned char uint8_t;

extern uint8_t goalTemp;
extern float curTemp;
extern float pTerm, iTerm, dTerm, output;
extern float kp, ki, kd;

void assert(char check, std::string message);
void assertionFailure(std::string message);

class _Serial {
 public:
  void begin(int baud);
  bool operator!();
  void println(const char* message);
};

extern class _Serial Serial;

// Mock hardware functions
void refreshServos();
void hardwareInit();
float getTemp();
uint8_t getGoalTemp();
void setFaucet(uint8_t value);
void manualControlMotor();
void setMotorEnable(bool enable);
void updateScreen(uint8_t temp, uint8_t goal);

// Arduino build-in functions
float constrain(float value, float min, float max);
void delay(unsigned long ms);
unsigned long millis();
// uint8_t abs(uint8_t value);

#endif  // TESTS_H_
