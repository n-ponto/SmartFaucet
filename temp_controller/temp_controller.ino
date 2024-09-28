#ifndef TESTING
#include <Adafruit_SoftServo.h>
#include <Arduino.h>

#include "hardware.h"
#else
#include "../tests/mocks.h"
#endif
#include "constants.h"

unsigned long lastSampleTime = 0;
unsigned long lastMotorUpdateTime = 0;

uint8_t curFaucet, goalFaucet;
uint8_t curTemp, goalTemp;

// PID constants
double kp = 5;
double ki = 0;
double kd = 0;

unsigned long curTime, prevTime;
double elapsedTime;
double error, rateError, cumError, lastError;
double input, output;

void setup() {
  Serial.begin(9600);
  curFaucet = 0;
  cumError = 0;
  lastError = 0;
  input = 0;
  output = 0;
  rateError = 0;
  error = 0;
  goalTemp = 90;
  prevTime = millis();
  hardwareInit();
  // manualControlMotor();
}

void computePID() {
  elapsedTime = (double)(curTime - prevTime);  // compute time elapsed from previous computation

  error = curTemp - goalTemp;                     // determine error
  cumError += error * elapsedTime;                // compute integral
  rateError = (error - lastError) / elapsedTime;  // compute derivative

  output = (kp * error) + (ki * cumError) + (kd * rateError);  // PID output

  lastError = error;   // remember current error
  prevTime = curTime;  // remember current time
}

void loop() {
  curTime = millis();  // get current time
  if (curTime > lastSampleTime + SAMPLE_PERIOD) {
    goalTemp = getGoalTemp();
    curTemp = getTemp();
    computePID();
    goalFaucet = constrain(output + curFaucet, 0, MAX_FAUCET);
    updateScreen(curTemp, goalTemp);
    if (abs(goalFaucet - curFaucet) > OUPUT_NOISE_FILTER) {
      curFaucet = goalFaucet;
      setFaucet(goalFaucet);
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
