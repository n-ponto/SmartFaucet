#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "..\temp_controller\constants.h"
#include "mocks.h"
#include "temp_controller.h"

const int tempDelayMs = 1000;  // Delay in ms before temp reflects faucet change
double hotWaterTemp;

void testHotDuration() {
  // Hot water starts hot
  hotWaterTemp = HOT_WATER;
  setup();

  // Loop for 10 secs
  int loopSamples = 10 * 1000 / SAMPLE_PERIOD;

  // Arrays to record the temp and faucet values
  std::vector<int> temps(loopSamples);
  std::vector<int> faucets(loopSamples);
  std::vector<double> outputs(loopSamples);
  std::vector<double> errors(loopSamples);
  std::vector<double> cumErrors(loopSamples);
  std::vector<double> rateErrors(loopSamples);

  for (int i = 0; i < loopSamples; i++) {
    loop();
    temps[i] = getTemp();
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
  file << "Temp (F),Faucet (%),Output,P,I,D" << std::endl;
  for (int i = 0; i < loopSamples; i++) {
    file << temps[i] << "," << faucets[i] << "," << outputs[i] << "," << errors[i] << "," << cumErrors[i] << "," << rateErrors[i] << std::endl;
  }
  file.close();
}

int main(int argc, char *argv[]) {
  testHotDuration();
  return 0;
}