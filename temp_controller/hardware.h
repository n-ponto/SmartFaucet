#ifndef HARDWARE_H_
#define HARDWARE_H_

void refreshServos();
void hardwareInit();
float getTemp();
uint8_t getGoalTemp();
void setFaucet(uint8_t value);
void manualControlMotor();
void setMotorEnable(bool enable);
void updateScreen(uint8_t temp, uint8_t goal);

#endif  // HARDWARE_H_
