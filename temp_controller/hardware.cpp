#include <Arduino.h>
#include <Stepper.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define THERM_PIN 2  // Pin for the thermometer

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
const int rolePerMinute = 15;         // Adjustable range of 28BYJ-48 stepper is 0~17 rpm

int faucetMax = stepsPerRevolution / 2;  // Stepper value when faucet is fully open
int currentStepper = 0;                  // Current position of the stepper motor

float currentTemp = 0;

OneWire oneWire(THERM_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress thermAddr;

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

/// Initialize the thermometer module
void thermInit() {
  sensors.begin();
  if (!sensors.getAddress(thermAddr, 0)) Serial.println("Unable to find address for Device 0");
  sensors.setResolution(thermAddr, 9);
}

/// Initialize all hardware
void hardwareInit() {
  // set the speed at 60 rpm:
  myStepper.setSpeed(rolePerMinute);
  thermInit();
}

/// Read the tempurature from the thermometer
unsigned char getTemp() {
  sensors.requestTemperatures();
  float temp = sensors.getTempF(thermAddr);
  if (temp == DEVICE_DISCONNECTED_F) {
    Serial.println("ERROR: Could not read tempurature data.");
  } else if (temp != currentTemp) {
    Serial.print(" Temperature: ");
    Serial.println(temp);
  }
  currentTemp = temp;
  return static_cast<unsigned char>(round(temp));
}

/// Set the faucet stepper motor to a certain value
void setFaucet(unsigned char value) {
  // Map value to stepper range
  int goalFaucet = map(value, 0, 255, 0, faucetMax);
  int diff = goalFaucet - currentStepper;
  myStepper.step(diff);
  currentStepper = goalFaucet;
}
