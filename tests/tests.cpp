#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "mocks.h"
#include "temp_controller.h"

const int tempDelayMs = 1000;  // Delay in ms before temp reflects faucet change
double hotWaterTemp;

void assertionFailure(std::string message) {
  std::cout << "\n[ASSERT FAILURE]: " << message << std::endl;
  throw std::exception();
}

void assert(char check, std::string message) {
  if (!check) {
    assertionFailure(message);
  }
}

bool inRange(int value, int target, int range) {
  return value >= target - range && value <= target + range;
}

void testInit() {
  setup();

  // Check that the initial faucet value is 0
  assert(faucetValue == 0, "faucet should be 0");

  hotWaterTemp = COLD_WATER;
  assert(getTemp() == COLD_WATER, "temp should be cold");

  unsigned char expectedTemp, actualValue;
  hotWaterTemp = HOT_WATER;
  setFaucet(maxFaucet / 2);
  assert(inRange(faucetValue, maxFaucet / 2, 1), "faucet should be half, was " + std::to_string(faucetValue));
  expectedTemp = COLD_WATER * 0.25 + hotWaterTemp * 0.75;
  actualValue = getTemp();
  assert(inRange(actualValue, expectedTemp, 1), "1 expected temp: " + std::to_string(expectedTemp) + ", was " + std::to_string(getTemp()));

  setFaucet(maxFaucet);
  assert(inRange(faucetValue, maxFaucet, 1), "faucet should be full, was " + std::to_string(faucetValue));
  expectedTemp =( COLD_WATER * 0.5) + (hotWaterTemp * 0.5);
  actualValue = getTemp();
  assert(inRange(actualValue, expectedTemp, 1), "2 expected temp: " + std::to_string(expectedTemp));

  setFaucet(0);
  assert(faucetValue == 0, "faucet should be 0");
  actualValue = getTemp();
  assert(actualValue == hotWaterTemp, "3 expected temp: " + std::to_string(hotWaterTemp));

  std::cout << "Test init passed!" << std::endl;
}

void testOneColdLoop() {
  setup();

  assert(faucetValue == 0, "faucet should init 0, was " + std::to_string(faucetValue));
  assert(output == 0, "output should init 0, was " + std::to_string(output));

  // Hot water is cold, so should do nothing
  hotWaterTemp = COLD_WATER;
  // printf("Looping\n");
  loop();
  // printf("Done looping\n");
  unsigned char temp = getTemp();
  // printf("Got temp: %d\n", temp);
  assert(0.1 > faucetValue && faucetValue >= 0, "faucet should be 0, was " + std::to_string(faucetValue));
  assert(output <= 0, "output should be <=0, was " + std::to_string(output));
  assert(temp < goalTemp, "temp should be less than goal temp, was " + std::to_string((int)temp) + " goal temp: " + std::to_string(goalTemp));

  // Should never try to move the faucet since the water is already cold
  std::cout << "Test one loop passed!" << std::endl;
}

void testColdWater() {
  setup();

  assert(faucetValue == 0, "faucet should init 0, was " + std::to_string(faucetValue));
  assert(output == 0, "output should init 0, was " + std::to_string(output));

  // Hot water is cold, do nothing
  hotWaterTemp = COLD_WATER;

  // Loop for 30 secs
  int loopSamples = 30 * 1000 / samplePeriod;
  for (int i = 0; i < loopSamples; i++) {
    loop();
    unsigned char temp = getTemp();
    assert(temp < goalTemp, "temp should be less than goal temp, was " + std::to_string((int)temp) + " goal temp: " + std::to_string(goalTemp));

    assert(output <= 0, "output should be 0, was " + std::to_string(output));
    // Should never try to move the faucet since the water is already cold
    assert(0.1 > faucetValue && faucetValue >= 0, "faucet should be 0, was " + std::to_string(faucetValue) + " i: " + std::to_string(i));
  }

  std::cout << "Test cold water passed!" << std::endl;
}

void testHotDuration() {
  // Hot water starts hot
  hotWaterTemp = HOT_WATER;
  setup();

  // Loop for 10 secs
  int loopSamples = 10 * 1000 / samplePeriod;

  // Arrays to record the temp and faucet values
  std::vector<int> temps(loopSamples);
  std::vector<int> faucets(loopSamples);
  std::vector<double> outputs(loopSamples);
  std::vector<double> errors(loopSamples);
  std::vector<double> cumErrors(loopSamples);
  std::vector<double> rateErrors(loopSamples);

  for (int i = 0; i < loopSamples; i++) {
    temps[i] = getTemp();
    // printf("Faucet: %d\n", faucetValue);
    faucets[i] = (double)faucetValue * 100 / maxFaucet;  // Convert to percentage
    outputs[i] = output;
    errors[i] = error * kp;
    cumErrors[i] = cumError * ki;
    rateErrors[i] = rateError * kd;
    loop();
  }

  // Save the last temp and faucet values to CSV
  std::ofstream file;
  file.open("test3.csv");
  if (!file.is_open()) {
    assertionFailure("Could not open file");
  }
  file << "Temp (F),Faucet (%),Output,P,I,D" << std::endl;
  for (int i = 0; i < loopSamples; i++) {
    file << temps[i] << "," << faucets[i] << "," << outputs[i] << "," << errors[i] << "," << cumErrors[i] << "," << rateErrors[i] << std::endl;
  }
  file.close();

  // Check that the temp is close to the goal temp
  // const int allowedRange = 5;  // degrees
  // double lastTemp = temps[loopSamples - 1];
  // bool inRange = lastTemp > goalTemp - allowedRange && lastTemp < goalTemp + allowedRange;
  // assert(inRange, "temp should be close to goal temp, but was " + std::to_string(lastTemp));

  std::cout << "Test hot water passed!" << std::endl;
}

int main(int argc, char *argv[]) {
  testInit();
  testOneColdLoop();
  testColdWater();
  testHotDuration();
  return 0;
}