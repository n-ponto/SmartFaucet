#ifndef TESTING
#include <Arduino.h>

#include <cmath>

#include "hardware.h"
#else
#include "../tests/mocks.h"
#endif
#include <cstdio>

const unsigned char maxFaucet = 255;
const unsigned long samplePeriod = 100;  // Sample time in milliseconds

// PID constants
double kp = 0.5;
double ki = 0;
double kd = 0;

unsigned char goalTemp = 90;
unsigned char currentFaucet;

unsigned long currentTime, previousTime;
double elapsedTime;
double error;
double lastError;
double input, output;
double rateError, cumError;

void computePID(double currentTemp) {
  currentTime = millis();                                    // get current time
  elapsedTime = (double)(currentTime - previousTime) / 100;  // compute time elapsed from previous computation

  error = currentTemp - goalTemp;                 // determine error
  cumError += error * elapsedTime;                // compute integral
  rateError = (error - lastError) / elapsedTime;  // compute derivative

  output = (kp * error) + (ki * cumError) + (kd * rateError);  // PID output

  lastError = error;           // remember current error
  previousTime = currentTime;  // remember current time
}

void setup() {
  Serial.begin(9600);
  currentFaucet = 0;
  cumError = 0;
  lastError = 0;
  input = 0;
  output = 0;
  rateError = 0;
  error = 0;
  previousTime = millis();
  while (!Serial) {
    delay(500);
  }
  hardwareInit();
}

void loop() {
  unsigned char currentTemp = getTemp();
  computePID(currentTemp);
  // printf("Computed output: %f\n", output);
  // printf("Current faucet: %d\n", currentFaucet);
  double goalFaucet = output + currentFaucet;
  // printf("Goal faucet: %d\n", goalFaucet);

  goalFaucet = constrain(goalFaucet, 0, 255);
  // printf("Goal faucet: %d\n", goalFaucet);
  setFaucet(goalFaucet);
  currentFaucet = goalFaucet;
  delay(samplePeriod);
}