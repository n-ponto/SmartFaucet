#define SIMULATE
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

#define WRITE_0 0      // 750
#define WRITE_180 180  // 2250

#define THERM_PIN 2          // Pin for the thermometer
#define MOTOR_ENABLE_PIN 8   // Relay to servos
#define MAIN_SERVO_PIN 9     // Servo to move faucet
#define ENGAGE_SERVO_PIN 10  // Servo to engage gears
// #define DIAL_PIN int(A0)     // Dial
#define MAX_DIAL 1023

#define ENGAGE_SERVO_TOP 10
#define ENGAGE_SERVO_BOTTOM 170

uint8_t motorPosition = 0;    // Current position of the stepper motor
float tempReading = 0;        // Current tempurature reading
int faucetMin, faucetMax;     // Min and max allowed values for the faucet
bool motorEnabled;            // Whether the motor is enabled or not
char tempStr[4], goalStr[4];  // Strings containing for displaying current and goal tempuratures

LiquidCrystal lcd(3, 4, 5, 6, 7, 12);

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
  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  motorEnabled = true;
  setMotorEnable(false);

  lcd.begin(16, 2);  // Init dimensions of LCD
  lcd.clear();
  lcd.print("Temp: ");
  lcd.setCursor(0, 1);
  lcd.print("Goal: ");

  // Set faucet limits to middle 80%
  int fullRange = WRITE_180 - WRITE_0;
  int remove = fullRange / 10;
  faucetMin = WRITE_0 + remove;
  faucetMax = WRITE_0 + remove + (fullRange / 2);  // Min value + 90 degrees
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
    value = analogRead(A0);
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
uint8_t getTemp() {
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
  currentTemp = temp;
#endif
#else
  // Instead of reading from the thermometer, read from the serial port
  if (Serial.available() > 0) {
    int val = Serial.parseInt();
    Serial.readString();
    Serial.print("Got value: ");
    Serial.println(val);
    currentTemp = val;
  }
#endif
#ifdef SIMULATE
  // Overwrite real value with simulated value from computer
  if (Serial.available() > 0) {
    tempReading = Serial.read();
  }
#endif
  // Simulation communicates with computer over serial
  return static_cast<uint8_t>(round(tempReading));
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
  if (enable == motorEnabled) return;
  motorEnabled = enable;
  digitalWrite(MOTOR_ENABLE_PIN, enable ? HIGH : LOW);
  delay(15);  // Wait for relay
  uint8_t servoGoal = enable ? ENGAGE_SERVO_BOTTOM : ENGAGE_SERVO_TOP;
  // lcd.clear();
  // lcd.print(servoGoal);
  engageServo.write(servoGoal);
  engageServo.refresh();
  // TODO: add delay somewhere that waits for the servo to raise
}

void refreshServos() {
  mainServo.refresh();
  engageServo.refresh();
}