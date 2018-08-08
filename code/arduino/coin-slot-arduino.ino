#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "coin-slot-arduino.h"


// u8g2 constructor
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

void loadEEPROM() {
	EEPROM.get(0, bankCount);
	EEPROMbankCount = bankCount;
}

byte getValueWidth(float displayValue) {
  // average character width for u8g2_font_logisoso24_tn is 14
  const byte charWidth = u8g2.getMaxCharWidth();
  byte theWidth = 4 * charWidth; //1.00
  if (displayValue >= 10.00)
    theWidth = 5 * charWidth;
  if (displayValue >= 100.00)
    theWidth = 6 * charWidth; //1.00
  if (displayValue >= 1000.00)
    theWidth = 7 * charWidth; //1.00
return ((u8g2.getDisplayWidth()-theWidth)/2);
}


void printOLED() {
	u8g2.clearBuffer();					    // clear the internal memory
	u8g2.setFont(u8g2_font_logisoso34_tn);	// choose a suitable font
	byte centerHeight = (u8g2.getDisplayHeight()/2) + (u8g2.getMaxCharHeight()/2);
	u8g2.setCursor(getValueWidth(bankValue),centerHeight);
	u8g2.print(bankValue);
	u8g2.sendBuffer();					    // transfer internal memory to the display
}

void printBank() {
	bankValue = bankCount * coinValue;
	printOLED();
	Serial.print(F("Bank Value: "));
	Serial.println(bankValue);
}

void printPulses() {
	Serial.print(F("Pulse coutn: "));
	Serial.println(bankCount);
}


void handleSerial() {
	char incomingChar = Serial.read();

	switch(incomingChar) {
		case '!':
			Serial.println(F("Clearing..."));
			bankCount = 0;
      bankValue = bankCount * coinValue;
      printBank();
      break;
    case '.':
      printPulses();
      printBank();
      break;
	}
}

void checkBattery() {

	// hceck battery voltage
	digitalWrite(battControl, HIGH);
	delay(10);
	int battReading = analogRead(battRead);
	digitalWrite(battControl, LOW);
//	Serial.print(F("Battery: "));
//	Serial.println(battReading);
	if (battReading < 700) {
		// battery is too low, turn off solenoid
		enableCounter = false;
		// turn off enableCounter
		digitalWrite(hvSolenoid, LOW);
		//print message
//		Serial.println(F("Low Battery Disable"));
		u8g2.clearBuffer();					// clear the internal memory
		u8g2.setFont(u8g2_font_logisoso34_tr);	// choose a suitable font
		u8g2.drawStr(0, 50, "lo batt");
		u8g2.sendBuffer();					// transfer internal memory to the display
		while(1);
	}

	return;
}

void handleBankReset() {
	unsigned long resetCounter = millis();
	bool waiting = true;
	while((digitalRead(resetButton) == HIGH) && waiting) {
		if (millis() - resetCounter < 1000) {
			u8g2.clearBuffer();					// clear the internal memory
			u8g2.setFont(u8g2_font_logisoso34_tr);	// choose a suitable font
			u8g2.drawStr(0, 50, "3");
			u8g2.sendBuffer();					// transfer internal memory to the display
		}

		if ((millis() - resetCounter > 1000) && (millis() - resetCounter <= 2000)) {
			u8g2.clearBuffer();					// clear the internal memory
			u8g2.setFont(u8g2_font_logisoso34_tr);	// choose a suitable font
			u8g2.drawStr(0, 50, "2");
			u8g2.sendBuffer();					// transfer internal memory to the display
		}

		if ((millis() - resetCounter > 2000) && (millis() - resetCounter <= 3000)) {
			u8g2.clearBuffer();					// clear the internal memory
			u8g2.setFont(u8g2_font_logisoso34_tr);	// choose a suitable font
			u8g2.drawStr(0, 50, "1");
			u8g2.sendBuffer();					// transfer internal memory to the display
		}

		if ((millis() - resetCounter > 3000) && (millis() - resetCounter <= 3500)) {
			u8g2.clearBuffer();					// clear the internal memory
			u8g2.setFont(u8g2_font_logisoso34_tr);	// choose a suitable font
			u8g2.drawStr(0, 50, "0");
			u8g2.sendBuffer();					// transfer internal memory to the display
		}

		if ((millis() - resetCounter > 3500) && (millis() - resetCounter <= 4000)) {
			bankCount = 0;
		}

		if ((millis() - resetCounter > 4000)) {
			waiting = false;
		}

	}
	printBank();
}

void setup() {
	pinMode(battControl, OUTPUT);   // battery transistor
	digitalWrite(battControl, LOW);
	u8g2.begin();
	checkBattery();
	// init coin counter
	pinMode(hvSolenoid, OUTPUT);
	//Serial.print(F("Turning on counter..."));
  digitalWrite(hvSolenoid, HIGH);
	pinMode(coinSig, INPUT_PULLUP);
	u8g2.clearBuffer();					// clear the internal memory
	u8g2.setFont(u8g2_font_logisoso34_tr);	// choose a suitable font
	u8g2.drawStr(0, 50, "Wait");
	u8g2.sendBuffer();					// transfer internal memory to the display
	Serial.begin(115200);
  //delay(100); // TODO is there a way to tell when coin acceptor is ready
  while(digitalRead(coinSig));  // add timeout and error.
	previousCoinSignal = digitalRead(coinSig);
  //Serial.println(F("done!"));

	pinMode(resetButton, INPUT);

  // init screen
  //pinMode(hvOLED, OUTPUT);
	loadEEPROM();
	//Serial.print(F("After Setup: "));
	//Serial.println(freeMemory());
	printBank();
}

void loop() {
	handleSerial();

	if (digitalRead(resetButton) == HIGH)
		handleBankReset();

	if (millis() - previousBatteryMillis >= batteryInterval) {
		previousBatteryMillis = millis();
		checkBattery();
	}


	byte currentCoinSignal = digitalRead(coinSig);

	if (currentCoinSignal == HIGH) {
		previousUpdateMillis = millis(); // print after pulse train is done
    previousEEPROMwrite = millis(); // don't write until pulse train is over
		printOnceFlag = true;
	}

  // only print after pulses stop
	if (printOnceFlag && (millis() - previousUpdateMillis >= updateInterval)) {
		printBank();
		printOnceFlag = false;
	}

	// is time to update the EEPROM?

	if (millis() - previousEEPROMwrite >= updateEEPROMinterval) {
		// reset the clock
		previousEEPROMwrite = millis();

		// only write to EEPROM when a new value
		if (bankCount != EEPROMbankCount) {
			EEPROM.put(0,bankCount);  // writes a float to EEPROM
			Serial.println(F("!!! EEPROM WRITE !!!"));
			EEPROMbankCount = bankCount;
		}
	}

	if (currentCoinSignal != previousCoinSignal) {
		// Save the state for next iteration
		previousCoinSignal = currentCoinSignal;

		if (currentCoinSignal == HIGH) {
      // we see an edge, but is it a coin pulse?
      armedForCount = millis();
		} else {
      // short timeout to make sure it was a full pulse
      if (millis() - armedForCount < pulseTimeOut) {
        bankCount++;
      }
    }
	}
}
