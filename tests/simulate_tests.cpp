#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "..\temp_controller\constants.h"
#include "mocks.h"
#include "temp_controller.h"

extern uint8_t (*getTempFunction)(void);

#define ROOM_TEMP 68

const int tempDelayMs = 1000;   // Delay in ms before temp reflects faucet change
const int thermDelayMs = 3000;  // Delay in ms before thermometer reflects real temp value
double hotWaterTemp;

// The tempurature values
double theoreticalTemp = ROOM_TEMP;  // Theoretical immediate temp based on faucet
double realTemp = ROOM_TEMP;         // Moving average of theoretical temp
double thermTemp = ROOM_TEMP;        // Moving average of real temp

const int tempDelaySamples = tempDelayMs / SAMPLE_PERIOD;
const int thermDelaySamples = thermDelayMs / SAMPLE_PERIOD;

const double tempDelayWeight = 1.0 / tempDelaySamples;
const double thermDelayWeight = 1.0 / thermDelaySamples;

uint8_t simulateTemp() {
  // Calculate real water temp based on faucet
  double coldPercent = (double)(curFaucet) / 2 / MAX_FAUCET;
  double hotPercent = 1.0 - coldPercent;
  theoreticalTemp = (hotWaterTemp * hotPercent) + (COLD_WATER * coldPercent);
  realTemp = realTemp * (1 - tempDelayWeight) + theoreticalTemp * tempDelayWeight;

  // Repeat for thermometer temp
  thermTemp = thermTemp * (1 - thermDelayWeight) + realTemp * thermDelayWeight;
  return thermTemp;
}

void testHotDuration() {
  // Hot water starts hot
  hotWaterTemp = HOT_WATER;
  setup();

  // Loop for 10 secs
  int loopSamples = 20 * 1000 / SAMPLE_PERIOD;
  // Arrays to record the real temp and thermometer temp values
  std::vector<double> realTemps(loopSamples);
  std::vector<double> thermTemps(loopSamples);
  std::vector<double> theoryTemps(loopSamples);
  std::vector<int> faucets(loopSamples);
  std::vector<double> outputs(loopSamples);
  std::vector<double> errors(loopSamples);
  std::vector<double> cumErrors(loopSamples);
  std::vector<double> rateErrors(loopSamples);

  for (int i = 0; i < loopSamples; i++) {
    loop();
    realTemps[i] = realTemp;
    thermTemps[i] = thermTemp;
    theoryTemps[i] = theoreticalTemp;
    // printf("Faucet: %d\n", curFaucet);
    faucets[i] = (double)curFaucet * 100 / MAX_FAUCET;  // Convert to percentage
    outputs[i] = output;
    errors[i] = error * kp;
    cumErrors[i] = cumError * ki;
    rateErrors[i] = rateError * kd;
  }

  // Save the last temp and faucet values to CSV
  std::ofstream file;
  file.open("test3.csv");
  if (!file.is_open()) {
    assertionFailure("Could not open file");
  }
  file << "Time (ms), Theory Temp (F),Real temp (F),Therm temp (F),Faucet (%),Output,P,I,D" << std::endl;
  for (int i = 0; i < loopSamples; i++) {
    file << (i * SAMPLE_PERIOD) << "," << theoryTemps[i] << "," << realTemps[i] << "," << thermTemps[i] << "," << faucets[i] << "," << outputs[i] << "," << errors[i] << "," << cumErrors[i] << "," << rateErrors[i] << std::endl;
  }
  file.close();
}

int main(int argc, char *argv[]) {
  getTempFunction = &simulateTemp;
  testHotDuration();
  return 0;
}