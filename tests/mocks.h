#ifndef TESTS_H_
#define TESTS_H_

#define COLD_WATER 55
#define HOT_WATER 120

// Variables from temp_controller.cpp
extern const unsigned long samplePeriod;
extern const unsigned char maxFaucet;
extern unsigned char faucetValue, goalTemp;

extern double error, cumError, rateError, output;
extern double kp, ki, kd;

class _Serial {
 public:
  void begin(int baud);
  bool operator!();
};

extern class _Serial Serial;

void hardwareInit();
unsigned char getTemp();
void setFaucet(double output);

double constrain(double value, double min, double max);
void delay(unsigned long ms);
unsigned long millis();

#endif  // TESTS_H_
