#include <Wire.h>
#include <TM1637Display.h>
#include <DS1307.h>

#define BTN_INC 2
#define BTN_DEC 3
#define BTN_CONFIRM 4
#define BTN_CANCEL 5

TM1637 tmDisplay(10, 9);
DS1307<TwoWire> rtc(Wire);

const int RELAY_PIN_1 = 6;
const int RELAY_PIN_2 = 7;

const int EEPROM_ADDRESS = 0;

int currentMode = 0;
int currentDuration = 0;
int currentDelay = 0;
int currentRelay = 0;

int relayStates[2] = {0, 0};

void setup() {
  Serial.begin(9600);
  Wire.begin();
  tmDisplay.init();
  tmDisplay.set(BRIGHT_TYPICAL);

  rtc.begin();
  rtc.adjust(DateTime(__DATE__, __TIME__));

  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);

  for (int i = 0; i < 2; i++) {
    relayStates[i] = EEPROM.read(EEPROM_ADDRESS + i);
  }

  currentMode = EEPROM.read(EEPROM_ADDRESS + 2);
  currentDuration = EEPROM.read(EEPROM_ADDRESS + 3);
  currentDelay = EEPROM.read(EEPROM_ADDRESS + 4);
  currentRelay = EEPROM.read(EEPROM_ADDRESS + 5);
}

void loop() {
  if (digitalRead(BTN_INC) == LOW) {
    delay(100);
    if (digitalRead(BTN_INC) == LOW) {
      enterSettingsMenu();
    }
  }

  if (digitalRead(BTN_DEC) == LOW) {
    delay(100);
    if (digitalRead(BTN_DEC) == LOW) {
      enterSettingsMenu();
    }
  }

  if (currentMode != 0) {
    manageTimer();
  }
}

void enterSettingsMenu() {
  int newMode = currentMode;
  int newDuration = currentDuration;
  int newDelay = currentDelay;
  int newRelay = currentRelay;

  tmDisplay.clearDisplay();
  tmDisplay.showNumberDec(newMode);
  tmDisplay.showNumberDec(newDuration);
  tmDisplay.showNumberDec(newDelay);
  tmDisplay.showNumberDec(newRelay);

  while (true) {
    if (digitalRead(BTN_INC) == LOW) {
      delay(100);
      if (digitalRead(BTN_INC) == LOW) {
        if (newMode < 2) {
          newMode++;
        } else {
          newMode = 0;
        }
        tmDisplay.clearDisplay();
        tmDisplay.showNumberDec(newMode);
        tmDisplay.showNumberDec(newDuration);
        tmDisplay.showNumberDec(newDelay);
        tmDisplay.showNumberDec(newRelay);
      }
    }

    if (digitalRead(BTN_DEC) == LOW) {
      delay(100);
      if (digitalRead(BTN_DEC) == LOW) {
        if (newMode > 0) {
          newMode--;
        } else {
          newMode = 2;
        }
        tmDisplay.clearDisplay();
        tmDisplay.showNumberDec(newMode);
        tmDisplay.showNumberDec(newDuration);
        tmDisplay.showNumberDec(newDelay);
        tmDisplay.showNumberDec(newRelay);
      }
    }

    if (digitalRead(BTN_CONFIRM) == LOW) {
      delay(100);
      if (digitalRead(BTN_CONFIRM) == LOW) {
        EEPROM.update(EEPROM_ADDRESS + 2, newMode);
        EEPROM.update(EEPROM_ADDRESS + 3, newDuration);
        EEPROM.update(EEPROM_ADDRESS + 4, newDelay);
        EEPROM.update(EEPROM_ADDRESS + 5, newRelay);

        for (int i = 0; i < 2; i++) {
          EEPROM.update(EEPROM_ADDRESS + i, relayStates[i]);
        }

        currentMode = newMode;
        currentDuration = newDuration;
        currentDelay = newDelay;
        currentRelay = newRelay;

        break;
      }
    }

    if (digitalRead(BTN_CANCEL) == LOW) {
      delay(100);
      if (digitalRead(BTN_CANCEL) == LOW) {
        break;
      }
    }
  }
}

void manageTimer() {
  DateTime now = rtc.now();
  DateTime endTime = now;
  DateTime delayTime = now;

  switch (currentMode) {
    case 0:
      break;
    case 1:
      endTime += currentDuration;
      break;
    case 2:
      endTime += currentDuration;
      delayTime += currentDelay;
      break;
  }

  if (now >= delayTime) {
    digitalWrite(RELAY_PIN_1, relayStates[currentRelay]);
    digitalWrite(RELAY_PIN_2, relayStates[currentRelay ^ 1]);
  }

  if (now >= endTime) {
    digitalWrite(RELAY_PIN_1, LOW);
    digitalWrite(RELAY_PIN_2, LOW);
  }

  tmDisplay.clearDisplay();
  tmDisplay.showNumberDec(currentDuration);
  tmDisplay.showNumberDec(currentDelay);
  tmDisplay.showNumberDec(currentMode);
  tmDisplay.showNumberDec(currentRelay);

  if (digitalRead(BTN_CONFIRM) == LOW) {
    delay(100);
    if (digitalRead(BTN_CONFIRM) == LOW) {
      currentMode = 0;
      currentDuration = 0;
      currentDelay = 0;
      currentRelay = 0;

      digitalWrite(RELAY_PIN_1, LOW);
      digitalWrite(RELAY_PIN_2, LOW);
    }
  }
}
		