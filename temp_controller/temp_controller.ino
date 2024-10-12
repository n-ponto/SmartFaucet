#ifndef TESTING
#include <Adafruit_SoftServo.h>
#include <Arduino.h>

#include "hardware.h"
#else
#include "../tests/mocks.h"
#endif
#include "constants.h"

#define CUM_ERROR_DECAY 0.95
#define CLOSE_LOW_TEMP 10  // How close to the goal temperature to use the derivative term
#define CLOSE_HIGH_TEMP 4
#define RATE_ERR_AVG_WEIGHT 0.3  // How much the new rate error impacts the previous rate error
#define INIT_GOAL_TEMP 100
#define ROUND(x) (x + 0.5)

const float prevRateErrAvgWeight = 1.0 - RATE_ERR_AVG_WEIGHT;
const float kdMultiplier = 5;
unsigned long lastMotorUpdateTime = 0;

float kp = 0.2;
float ki = 0.0000;
float kd = 400;
float pTerm, iTerm, dTerm;
bool approachingGoal;

float goalFaucet;
uint8_t curFaucet;
uint8_t goalTemp;
float curTemp;
unsigned long curTime, prevTime, elapsedTime;
float error, rateError, cumError, lastError, prevRateError;
float input, output;

void computePID() {
  elapsedTime = curTime - prevTime;  // compute time elapsed from previous computation

  error = curTemp - goalTemp;                                                              // determine error
  cumError = (error * elapsedTime) + (cumError * CUM_ERROR_DECAY);                         // compute integral
  rateError = (error - lastError) / elapsedTime;                                           // compute derivative
  rateError = (rateError * RATE_ERR_AVG_WEIGHT) + (prevRateErrAvgWeight * prevRateError);  // Moving average of rate error

  // If near the goal temperature and getting hotter, use derivative term
  approachingGoal = (rateError > 0) && (curTemp > goalTemp - CLOSE_LOW_TEMP) && (curTemp < goalTemp - CLOSE_HIGH_TEMP);

  pTerm = (kp * error);
  iTerm = (ki * cumError);
  dTerm = (kd * rateError) * (approachingGoal ? kdMultiplier : 1.0);
  output = pTerm + iTerm + dTerm;  // PID output

  // Remember values for next iteration
  lastError = error;
  prevRateError = rateError;
  prevTime = curTime;
}

void setup() {
  pTerm = iTerm = dTerm = 0;
  approachingGoal = false;
  goalFaucet = curFaucet = 0;
  goalTemp = INIT_GOAL_TEMP;
  curTemp = 0;
  curTime = millis();
  prevTime = curTime - SAMPLE_PERIOD;
  elapsedTime = 0;
  error = rateError = cumError = lastError = prevRateError = 0;
  input = output = 0;
  hardwareInit();

  // Run initial computation to get the first output
  goalTemp = getGoalTemp();
  curTemp = getTemp();
  computePID();
}

void loop() {
  curTime = millis();
  if (curTime >= prevTime + SAMPLE_PERIOD) {
    goalTemp = getGoalTemp();
    curTemp = getTemp();
    computePID();
    goalFaucet = constrain(output + goalFaucet, 0, MAX_FAUCET);
    updateScreen(ROUND(curTemp), goalTemp);
    if (abs(ROUND(goalFaucet) - curFaucet) >= 1) {
      // Faucet setting is integer, so change must be at least 1
      curFaucet = uint8_t(ROUND(goalFaucet));
      setFaucet(curFaucet);
      lastMotorUpdateTime = curTime;
    } else if (curTime > lastMotorUpdateTime + MOTOR_DISABLE_DELAY) {
      setMotorEnable(false);  // Disable motor
    }
  }
  // Update servos
  refreshServos();
  delay(SERVO_REFRESH_DELAY);
}
