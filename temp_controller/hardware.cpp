// #define SIMULATE
#include <Arduino.h>
// #include <ServoTimer2.h>
#include <Adafruit_SoftServo.h>
// #include <Servo.h>
#include <LiquidCrystal.h>
#ifndef MOCK_TEMP
#include <DallasTemperature.h>
#include <OneWire.h>
#endif
#include "hardware.h"

#define ENGAGE_SERVO_PIN 8  // Servo to engage gears
#define MAIN_SERVO_PIN 9     // Servo to move faucet
#define RELAY_PIN 10          // Relay to servos
#define THERM_PIN 11         // Pin for the thermometer
#define DIAL_PIN A0          // Dial
#define MAX_DIAL 1023        // Max analog read value

#define ENGAGE_SERVO_TOP 10      // Top angle for the engage servo
#define ENGAGE_SERVO_BOTTOM 170  // Bottom angle for the engage servo

#define ENGAGE_SERVO_DELAY 300  // Time in ms to wait for the servo to engage/disengage
#define RELAY_DELAY 15          // Time in ms to wait for the relay to engage/disengage

#define MIN_GOAL 80
#define MAX_GOAL 110

const uint8_t engageServoDelaySteps = ENGAGE_SERVO_DELAY / SERVO_REFRESH_DELAY;  // Number of steps to wait for the servo to engage/disengage

uint8_t motorPosition = 0;    // Current position of the stepper motor
float tempReading = 0;        // Current tempurature reading
int faucetMin, faucetMax;     // Min and max allowed values for the faucet
bool motorEnabled;            // Whether the motor is enabled or not
char tempStr[4], goalStr[4];  // Strings containing for displaying current and goal tempuratures

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

#ifndef MOCK_TEMP
OneWire oneWire(THERM_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress thermAddr;
#endif

Adafruit_SoftServo mainServo;
Adafruit_SoftServo engageServo;

/// Initialize the thermometer module
#ifndef MOCK_TEMP
void thermInit() {
  sensors.begin();
  if (!sensors.getAddress(thermAddr, 0)) {
    lcd.clear();
    lcd.print("ERROR: Couldn't");
    lcd.setCursor(0, 1);
    lcd.print("connect therm");
    while (1 == 1) delay(10000);
    // Serial.println("Unable to find address for Device 0");
  }
  sensors.setResolution(thermAddr, 9);
}
#endif

/// Initialize all hardware
void hardwareInit() {
  mainServo.attach(MAIN_SERVO_PIN);
  engageServo.attach(ENGAGE_SERVO_PIN);
  pinMode(RELAY_PIN, OUTPUT);
  motorEnabled = true;
  setMotorEnable(false);

  lcd.begin(16, 2);  // Init dimensions of LCD
  lcd.clear();
  lcd.print("Temp: ");
  lcd.setCursor(0, 1);
  lcd.print("Goal: ");

  // Set faucet limits to middle 80%
  int fullRange = 180;
  int remove = fullRange / 10;
  faucetMin = remove;
  faucetMax = remove + (fullRange / 2);  // Min value + 90 degrees
#ifndef MOCK_TEMP
  thermInit();
#endif
}

void manualControlMotor() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stepper:");
  char goalStr[4];
  int value;
  uint8_t goalPosition;
  while (1 == 1) {
    value = analogRead(DIAL_PIN);
    goalPosition = map(value, 0, MAX_DIAL, 0, 179);
    engageServo.write(goalPosition);
    sprintf(goalStr, "%3d", goalPosition);
    lcd.setCursor(0, 1);
    lcd.print(goalStr);
    motorPosition = goalPosition;
    delay(20);
    engageServo.refresh();
  }
}

void updateScreen(uint8_t temp, uint8_t goal) {
  sprintf(tempStr, "%3d", temp);
  sprintf(goalStr, "%3d", goal);
  lcd.setCursor(6, 0);
  lcd.print(tempStr);
  lcd.setCursor(6, 1);
  lcd.print(goalStr);
}

/// Read the tempurature from the thermometer
float getTemp() {
#ifndef MOCK_TEMP
  // Get real tempurature reading from thermometer
  sensors.requestTemperatures();
  float temp = sensors.getTempF(thermAddr);
  if (temp == DEVICE_DISCONNECTED_F) {
    // Serial.println("ERROR: Could not read tempurature data.");
    lcd.clear();
    lcd.print("ERROR: Could not");
    lcd.setCursor(0, 1);
    lcd.print("read temp data.");
    while (1 == 1) {
      // Do nothing
      delay(10000);
    }
  }
#ifndef SIMULATE
  tempReading = temp;
#endif
#else
  // Instead of reading from the thermometer, read from the serial port
  if (Serial.available() > 0) {
    int val = Serial.parseInt();
    Serial.readString();
    Serial.print("Got value: ");
    Serial.println(val);
    tempReading = val;
  }
#endif
#ifdef SIMULATE
  // Overwrite real value with simulated value from computer
  if (Serial.available() > 0) {
    tempReading = Serial.read();
  }
#endif
  // Simulation communicates with computer over serial
  return tempReading;
}

/// Set the faucet stepper motor to a certain value
void setFaucet(uint8_t value) {
// Map value to stepper range
#ifdef SIMULATE
  Serial.write(value);
#endif
  if (!motorEnabled) {
    setMotorEnable(true);
  }
  uint8_t goalFaucet = map(value, 0, 255, faucetMin, faucetMax);
  mainServo.write(goalFaucet);
  motorPosition = goalFaucet;
}

void setMotorEnable(bool enable) {
  if (motorEnabled == enable) return;
  motorEnabled = enable;
  if (enable) {
    // Enable motors before engaging
    digitalWrite(RELAY_PIN, HIGH);
    delay(RELAY_DELAY);
  }
  // Move servo
  engageServo.write(enable ? ENGAGE_SERVO_BOTTOM : ENGAGE_SERVO_TOP);
  for (uint8_t i = 0; i < engageServoDelaySteps; i++) {
    engageServo.refresh();
    delay(SERVO_REFRESH_DELAY);
  }
  if (!enable) {
    // Disable motors after disengaging
    digitalWrite(RELAY_PIN, LOW);
    delay(RELAY_DELAY);
  }
}

void refreshServos() {
  mainServo.refresh();
  engageServo.refresh();
}

uint8_t getGoalTemp() {
  int value = analogRead(DIAL_PIN);
  return static_cast<uint8_t>(map(value, 0, MAX_DIAL, MIN_GOAL, MAX_GOAL));
}