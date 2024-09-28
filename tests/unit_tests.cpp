#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "..\temp_controller\constants.h"
#include "mocks.h"
#include "temp_controller.h"

double hotWaterTemp;

bool inRange(int value, int target, int range) {
  return value >= target - range && value <= target + range;
}

void testInit() {
  setup();

  // Check that the initial faucet value is 0
  assert(curFaucet == 0, "faucet should be 0");

  hotWaterTemp = COLD_WATER;
  assert(getTemp() == COLD_WATER, "temp should be cold");

  unsigned char expectedTemp, actualValue;
  hotWaterTemp = HOT_WATER;
  setFaucet(MAX_FAUCET / 2);
  assert(inRange(curFaucet, MAX_FAUCET / 2, 1), "faucet should be half, was " + std::to_string(curFaucet));
  expectedTemp = COLD_WATER * 0.25 + hotWaterTemp * 0.75;
  actualValue = getTemp();
  assert(inRange(actualValue, expectedTemp, 1), "1 expected temp: " + std::to_string(expectedTemp) + ", was " + std::to_string(getTemp()));

  setFaucet(MAX_FAUCET);
  assert(inRange(curFaucet, MAX_FAUCET, 1), "faucet should be full, was " + std::to_string(curFaucet));
  expectedTemp = (COLD_WATER * 0.5) + (hotWaterTemp * 0.5);
  actualValue = getTemp();
  assert(inRange(actualValue, expectedTemp, 1), "2 expected temp: " + std::to_string(expectedTemp));

  setFaucet(0);
  assert(curFaucet == 0, "faucet should be 0");
  actualValue = getTemp();
  assert(actualValue == hotWaterTemp, "3 expected temp: " + std::to_string(hotWaterTemp));

  std::cout << "Test init passed!" << std::endl;
}

void testOneColdLoop() {
  setup();

  assert(curFaucet == 0, "faucet should init 0, was " + std::to_string(curFaucet));
  assert(output == 0, "output should init 0, was " + std::to_string(output));

  // Hot water is cold, so should do nothing
  hotWaterTemp = COLD_WATER;
  // printf("Looping\n");
  loop();
  // printf("Done looping\n");
  unsigned char temp = getTemp();
  // printf("Got temp: %d\n", temp);
  assert(0.1 > curFaucet && curFaucet >= 0, "faucet should be 0, was " + std::to_string(curFaucet));
  assert(output <= 0, "output should be <=0, was " + std::to_string(output));
  assert(temp < goalTemp, "temp should be less than goal temp, was " + std::to_string((int)temp) + " goal temp: " + std::to_string(goalTemp));

  // Should never try to move the faucet since the water is already cold
  std::cout << "Test one loop passed!" << std::endl;
}

void testColdWater() {
  setup();

  assert(curFaucet == 0, "faucet should init 0, was " + std::to_string(curFaucet));
  assert(output == 0, "output should init 0, was " + std::to_string(output));

  // Hot water is cold, do nothing
  hotWaterTemp = COLD_WATER;

  // Loop for 30 secs
  int loopSamples = 30 * 1000 / SAMPLE_PERIOD;
  for (int i = 0; i < loopSamples; i++) {
    loop();
    unsigned char temp = getTemp();
    assert(temp < goalTemp, "temp should be less than goal temp, was " + std::to_string((int)temp) + " goal temp: " + std::to_string(goalTemp));

    assert(output <= 0, "output should be 0, was " + std::to_string(output));
    // Should never try to move the faucet since the water is already cold
    assert(0.1 > curFaucet && curFaucet >= 0, "faucet should be 0, was " + std::to_string(curFaucet) + " i: " + std::to_string(i));
  }

  std::cout << "Test cold water passed!" << std::endl;
}

int main(int argc, char *argv[]) {
  testInit();
  testOneColdLoop();
  testColdWater();
  return 0;
}