#ifndef TESTING
#include <Arduino.h>

#include "hardware.h"
#else
#include "../tests/mocks.h"
#endif

const unsigned char maxFaucet = 255;
const unsigned long samplePeriod = 200;  // Delay in ms between updates

// PID constants
double kp = 5;
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
  currentTime = millis();                              // get current time
  elapsedTime = (double)(currentTime - previousTime);  // compute time elapsed from previous computation

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
  Serial.println("Started.");
}

void loop() {
  unsigned char currentTemp = getTemp();
  computePID(currentTemp);
  unsigned char goalFaucet = (unsigned char)constrain(output + currentFaucet, 0, maxFaucet);
  setFaucet(goalFaucet);
  currentFaucet = goalFaucet;
  delay(samplePeriod);
}
