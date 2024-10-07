#ifndef TESTING
#include <Adafruit_SoftServo.h>
#include <Arduino.h>

#include "hardware.h"
#else
#include "../tests/mocks.h"
#endif
#include "constants.h"

#define CUM_ERROR_DECAY 0.95

unsigned long lastSampleTime = 0;
unsigned long lastMotorUpdateTime = 0;

const float closeLowTempRange = 10;
const float closeHighTempRange = 2;  // The range of temperatures that are considered "close" to the goal temperature
float kp = 0.6;
float ki = 0.0000;
float kd = 10;
float closeKdMultiplier = 350;   // Multiply derivative term by this amount when close to the goal temperature
float rateErrorAvgWeight = 0.3;  // How much the new rate error impacts the previous rate error
float pTerm, iTerm, dTerm;

float goalFaucet, curFaucet;
uint8_t goalTemp;
float curTemp;
unsigned long curTime;

unsigned long prevTime;
float elapsedTime;
float error, rateError, cumError, lastError, prevRateError;
float input, output;

void computePID() {
  elapsedTime = (float)(curTime - prevTime);  // compute time elapsed from previous computation

  error = curTemp - goalTemp;                                       // determine error
  cumError = (error * elapsedTime) + (cumError * CUM_ERROR_DECAY);  // compute integral
  rateError = (error - lastError) / elapsedTime;                    // compute derivative
  rateError = (rateError * rateErrorAvgWeight) + ((1.0 - rateErrorAvgWeight) * prevRateError);

  // If near the goal temperature and getting hotter, increase the derivative term
  bool approachingGoal = ((curTemp > goalTemp - closeLowTempRange) || (curTemp < goalTemp + closeHighTempRange)) && rateError > 0;

  pTerm = (kp * error);
  iTerm = (ki * cumError);
  dTerm = (kd * rateError) * (approachingGoal ? closeKdMultiplier : 1);

  output = pTerm + iTerm + dTerm;  // PID output

  lastError = error;   // remember current error
  prevTime = curTime;  // remember current time
  prevRateError = rateError;
}

void setup() {
  Serial.begin(9600);
  curFaucet = 0;
  cumError = 0;
  lastError = 0;
  input = 0;
  output = 0;
  rateError = prevRateError = 0;
  error = 0;
  goalTemp = 100;
  curTime = millis();
  prevTime = curTime - SAMPLE_PERIOD;
  hardwareInit();
  // manualControlMotor();

  // Run initial computation to get the first output
  goalTemp = getGoalTemp();
  curTemp = getTemp();
  computePID();
}

void loop() {
  curTime = millis();
  if (curTime >= lastSampleTime + SAMPLE_PERIOD) {
    goalTemp = getGoalTemp();
    curTemp = getTemp();
    computePID();
    goalFaucet = constrain(output + goalFaucet, 0, MAX_FAUCET);
    updateScreen(curTemp + 0.5, goalTemp);
    if (abs(goalFaucet - curFaucet) >= 1) {
      // Faucet setting is integer, so change must be at least 1
      setFaucet(goalFaucet);
      curFaucet = goalFaucet;
      lastMotorUpdateTime = curTime;
    } else if (curTime > lastMotorUpdateTime + MOTOR_DISABLE_DELAY) {
      setMotorEnable(false);  // Disable motor
    }
    lastSampleTime = curTime;
  }
  // Update servos
  refreshServos();
  delay(SERVO_REFRESH_DELAY);
}
