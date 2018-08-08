// Pins
#include <Arduino.h>

const byte coinSig = A0;
const byte hvSolenoid = A1; // for turning on solenoid/coin counter
const byte battControl = 4;
const byte battRead = A3;
const byte resetButton = 3;

bool previousCoinSignal = false;
const float coinValue = 0.05;
float bankValue = 0.00;
unsigned long bankCount = 0;
const unsigned long pulseTimeOut = 75;

unsigned long armedForCount = 0;
unsigned long previousUpdateMillis = 0;
unsigned long updateInterval = 110;
bool printOnceFlag = false;

unsigned long previousEEPROMwrite = 0;  // delays writing to EEPROM
unsigned long updateEEPROMinterval = 1000;
//float EEPROMbankValue = 0.00;
unsigned long EEPROMbankCount;

unsigned long previousBatteryMillis = 0;
const unsigned long batteryInterval = 1000;
const float adcResolution = 5.00/1023.0;
bool enableCounter = true;
