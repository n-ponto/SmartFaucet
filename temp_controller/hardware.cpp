#include <Stepper.h>

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
const int rolePerMinute = 15;         // Adjustable range of 28BYJ-48 stepper is 0~17 rpm

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

void hardwareInit() {
  // set the speed at 60 rpm:
  myStepper.setSpeed(rolePerMinute);
}

unsigned char getTemp() {
  // Get temp from sensor
  return 0;
}

void setFaucet(unsigned char output) {
  // Update faucet based on output
  output = constrain(output, 0, 100);
  myStepper.step(output);
}