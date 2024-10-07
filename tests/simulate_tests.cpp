#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include "..\temp_controller\constants.h"
#include "mocks.h"
#include "temp_controller.h"

extern float (*getTempFunction)(void);

#define ROOM_TEMP 68
#define HIGH_TEMP_COST 100  // Multiply the error by this value if the temp is too high

extern uint8_t mockGoal;
extern uint8_t mockFaucet;

const int realDelayMs = 600;     // Delay in ms before temp reflects faucet change
const int thermDelayMs = 10000;  // Delay in ms before thermometer reflects real temp value
const int realDelaySamples = realDelayMs / SAMPLE_PERIOD;
const int thermDelaySamples = thermDelayMs / SAMPLE_PERIOD;
const double realDelayWeight = 1.0 / realDelaySamples;
const double thermDelayWeight = 1.0 / thermDelaySamples;

std::queue<float> thermometerQueue;

// The tempurature values
double hotWaterTemp;     // Temp coming from hot water
double theoreticalTemp;  // Theoretical immediate temp based on faucet
double realTemp;         // Moving average of theoretical temp
double thermTemp;        // Moving average of real temp

float simulateTemp() {
  // Calculate real water temp based on faucet
  double coldPercent = (double)(mockFaucet) / 2 / MAX_FAUCET;
  double hotPercent = 1.0 - coldPercent;
  theoreticalTemp = (hotWaterTemp * hotPercent) + (COLD_WATER * coldPercent);
  realTemp = realTemp * (1 - realDelayWeight) + theoreticalTemp * realDelayWeight;

  // Delay the thermometer reading
  // thermometerQueue.push(realTemp);
  // thermTemp = thermometerQueue.front();
  // thermometerQueue.pop();
  thermTemp = thermTemp * (1 - thermDelayWeight) + realTemp * thermDelayWeight;
  return thermTemp;
}

// Arrays to record the real temp and thermometer temp values
std::vector<double> realTemps, thermTemps, theoryTemps;
std::vector<double> outputs, ps, is, ds;
std::vector<int> faucets;

void initValues(int loopSamples) {
  realTemps = std::vector<double>(loopSamples);
  thermTemps = std::vector<double>(loopSamples);
  theoryTemps = std::vector<double>(loopSamples);
  outputs = std::vector<double>(loopSamples);
  ps = std::vector<double>(loopSamples);
  is = std::vector<double>(loopSamples);
  ds = std::vector<double>(loopSamples);
  faucets = std::vector<int>(loopSamples);
}

void recordValues(int i) {
  // Record values for CSV
  realTemps[i] = realTemp;
  thermTemps[i] = thermTemp;
  theoryTemps[i] = theoreticalTemp;
  faucets[i] = (double)mockFaucet * 100 / MAX_FAUCET;  // Convert to percentage
  outputs[i] = output;
  ps[i] = pTerm;
  is[i] = iTerm;
  ds[i] = dTerm;
}

void saveValuesToCSV(std::string filename, int loopSamples) {
  std::ofstream file;
  file.open(filename);
  if (!file.is_open()) {
    assertionFailure("Could not open file " + filename);
  }
  file << "Time (ms), Theory Temp (F),Real temp (F),Therm temp (F),Faucet (%),Output,P,I,D" << std::endl;
  for (int i = 0; i < loopSamples; i++) {
    file << (i * SAMPLE_PERIOD) << "," << theoryTemps[i] << "," << realTemps[i] << "," << thermTemps[i] << "," << faucets[i] << "," << outputs[i] << "," << ps[i] << "," << is[i] << "," << ds[i] << std::endl;
  }
  file.close();
}

void lowTempFluctuationSimulation(int runSecs, int goal) {
  const int maxTemp = COLD_WATER + 10;
  const double maxChangePerStep = 1.0;
  hotWaterTemp = COLD_WATER;
  theoreticalTemp = ROOM_TEMP;
  realTemp = ROOM_TEMP;
  thermTemp = ROOM_TEMP;
  mockGoal = goal;
  int loopSamples = runSecs * 1000 / SAMPLE_PERIOD;
  initValues(loopSamples);  // Initialize the values arrays for making the plots

  for (int i = 0; i < loopSamples; i++) {
    loop();
    recordValues(i);
    // Randomly change the hot water temp
    double change = ((double(rand() % 10) / 10.0) * maxChangePerStep * 2) - maxChangePerStep;
    change = std::min(change, maxTemp - hotWaterTemp);
    change = std::max(change, COLD_WATER - hotWaterTemp);
    hotWaterTemp += change;
  }

  char filename[64];
  sprintf(filename, "csv/lowflux_%d_%d_%0.7f_%0.7f_%0.7f.csv", runSecs, goal, kp, ki, kd);
  saveValuesToCSV(filename, loopSamples);
}

double runBasicSimulation(int runSecs, int warmUpMs, int goal, bool saveCSV = false) {
  // Initialize the thermometer queue
  // Hot water starts cold
  hotWaterTemp = COLD_WATER;
  theoreticalTemp = ROOM_TEMP;
  realTemp = ROOM_TEMP;
  thermTemp = ROOM_TEMP;
  mockGoal = goal;

  const int rangeReachedTemp = 3;  // Number of degrees to be within goal temp
  double meanSquaredError = 0;

  const int warmUpSamples = warmUpMs / SAMPLE_PERIOD;
  double warmUpStep;
  if (warmUpSamples == 0) {
    hotWaterTemp = HOT_WATER;
  } else {
    warmUpStep = ((double)HOT_WATER - COLD_WATER) / warmUpSamples;
  }
  setup();

  // Loop for 10 secs
  int loopSamples = runSecs * 1000 / SAMPLE_PERIOD;
  initValues(loopSamples);  // Initialize the values arrays for making the plots

  for (int i = 0; i < loopSamples; i++) {
    loop();
    recordValues(i);
    // Record mean squared error
    double tempDiff = std::abs(realTemp - mockGoal);
    if (realTemp > mockGoal + rangeReachedTemp) {
      tempDiff *= HIGH_TEMP_COST;  // Quadruple the error if the temp is too high
    }
    meanSquaredError += tempDiff * tempDiff;  // Record the mean squared error

    // Warm up the water
    if (i < warmUpSamples) {
      hotWaterTemp += warmUpStep;
    }
  }

  // Save the last temp and faucet values to CSV
  if (saveCSV) {
    char filename[64];
    sprintf(filename, "csv/%d_%d_%d_%0.7f_%0.7f_%0.7f.csv", runSecs, warmUpMs, goal, kp, ki, kd);
    saveValuesToCSV(filename, loopSamples);
  }
  return meanSquaredError / loopSamples;
}

double runMultipleSimulations(bool saveCSV = false) {
  const int runSecs = 40;
  const int tempuratures[] = {90, 100, 110};
  const int warmUpTimes[] = {0, 5000};
  double mse = 0;
  double error;
  for (int i = 0; i < 3; i++) {
    for (int k = 0; k < 2; k++) {
      error = runBasicSimulation(runSecs, warmUpTimes[k], tempuratures[i], saveCSV);
      mse += error * error;
    }
  }
  return mse;
}

void explorePID(double kpValues[], double kiValues[], double kdValues[], int kpSize, int kiSize, int kdSize, double& bestKp, double& bestKi, double& bestKd, double& lowestError) {
  double mse;
  for (int i = 0; i < kpSize; i++) {
    for (int j = 0; j < kiSize; j++) {
      for (int k = 0; k < kdSize; k++) {
        kp = kpValues[i];
        ki = kiValues[j];
        kd = kdValues[k];

        mse = runMultipleSimulations();

        // Check if this is the best PID values
        if (mse < lowestError || lowestError == -1) {
          lowestError = mse;
          bestKp = kp;
          bestKi = ki;
          bestKd = kd;
          // printf("New lowest %0.7f kp: %0.7f, ki: %0.7f, kd: %0.7f\n", mse, kp, ki, kd);
        }
      }
    }
  }

  printf("Best kp: %0.7f, ki: %0.7f, kd: %0.7f lowest error: %0.7f\n", bestKp, bestKi, bestKd, lowestError);
}

#define VALUES_SIZE 100  // Number of values to generate

void generateValues(double center, double arr[VALUES_SIZE], double range) {
  // Fill the values array
  const int steps = VALUES_SIZE / 2;
  const double stepvalue = range / steps;
  for (int i = 0; i < steps; i += 1) {
    arr[i * 2] = center * (1.0 - stepvalue * i);
    arr[i * 2 + 1] = center * (1.0 + stepvalue * i);
  }
}

void drillInPID() {
  const double startKp = 5, startKd = 5000;

  double kpValues[VALUES_SIZE];
  double kiValues[VALUES_SIZE];
  double kdValues[VALUES_SIZE];

  const double rangeDecay = 0.95;
  const int rounds = 5;
  double range = .9;  // Percent range to search on either side of the best value
  double bestKp, bestKi, bestKd;
  double lowestError = -1;
  std::vector<double> maxKps(rounds), minKps(rounds), bestKps(rounds), maxKds(rounds), minKds(rounds), bestKds(rounds), sumErrors(rounds);

  // Get error for initial values
  double initP[] = {startKp};
  double initD[] = {startKd};
  explorePID(initP, kiValues, initD, 1, 1, 1, bestKp, bestKi, bestKd, lowestError);

  // Drill in on the best values
  generateValues(bestKp, kpValues, range);
  generateValues(bestKd, kdValues, range);
  for (int i = 0; i < rounds; i++) {
    explorePID(kpValues, kiValues, kdValues, VALUES_SIZE, 1, VALUES_SIZE, bestKp, bestKi, bestKd, lowestError);

    // Record the results
    minKps[i] = kpValues[VALUES_SIZE - 2];
    maxKps[i] = kpValues[VALUES_SIZE - 1];
    bestKps[i] = bestKp;
    minKds[i] = kdValues[VALUES_SIZE - 2];
    maxKds[i] = kdValues[VALUES_SIZE - 1];
    bestKds[i] = bestKd;
    sumErrors[i] = lowestError;

    // Drill in on the best values
    generateValues(bestKp, kpValues, range);
    generateValues(bestKd, kdValues, range);
    range *= rangeDecay;  // Decrease the search range

    // Print the results
    printf("Round %d\t", i);
    printf("Best kp: %0.7f, ki: %0.7f, kd: %0.7f\n", bestKp, bestKi, bestKd);
  }

  // Save the results to a CSV
  std::ofstream file;
  file.open("csv/drill_results.csv");
  if (!file.is_open()) {
    assertionFailure("Could not open file");
  }
  file << "Min Kp,Max Kp,Best Kp,Min Kd,Max Kd,Best Kd,Sum Error" << std::endl;
  for (int i = 0; i < rounds; i++) {
    file << minKps[i] << "," << maxKps[i] << "," << bestKps[i] << "," << minKds[i] << "," << maxKds[i] << "," << bestKds[i] << "," << sumErrors[i] << std::endl;
  }
  file.close();

  // Plot the best values
  kp = bestKp;
  ki = bestKi;
  kd = bestKd;
  runMultipleSimulations(true);
}

int main(int argc, char* argv[]) {
  // check if the csv folder exists, and if not create it
  std::string folder = "csv";
  if (!std::filesystem::exists(folder)) {
    std::filesystem::create_directory(folder);
  }

  getTempFunction = &simulateTemp;
  // drillInPID();
  runMultipleSimulations(true);
  lowTempFluctuationSimulation(40, 90);
  lowTempFluctuationSimulation(40, 100);
  lowTempFluctuationSimulation(40, 110);
  return 0;
}