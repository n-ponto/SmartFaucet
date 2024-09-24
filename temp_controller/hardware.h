#ifndef HARDWARE_H_
#define HARDWARE_H_

#define SERVO_REFRESH_DELAY 20  // ms

void refreshServos();
void hardwareInit();
uint8_t getTemp();
uint8_t getGoalTemp();
void setFaucet(uint8_t value);
void manualControlMotor();
void setMotorEnable(bool enable);
void updateScreen(uint8_t temp, uint8_t goal);

#endif  // HARDWARE_H_
