// Water Level Indicator (4-level) - Arduino
// Sensors are active LOW (connected to GND when water present).
// LEDs and buzzer driven with NPN transistors (Arduino sinks current).

const uint8_t levelPins[4] = {2, 3, 4, 5};    // sensor inputs (use INPUT_PULLUP)
const uint8_t ledPins[4]   = {8, 9, 10, 11};  // LED outputs (drive transistor base via 1k)
const uint8_t buzzerPin    = 12;              // buzzer output (drive transistor)
const unsigned long debounceDelay = 50;       // ms

// for simple debouncing
uint8_t levelState[4];        // stable state (0 = water present, 1 = no water because of pullup)
uint8_t lastReading[4];
unsigned long lastDebounceTime[4];

void setup() {
  Serial.begin(115200);
  // inputs with internal pullup
  for (uint8_t i = 0; i < 4; ++i) {
    pinMode(levelPins[i], INPUT_PULLUP);
    levelState[i] = digitalRead(levelPins[i]);
    lastReading[i] = levelState[i];
    lastDebounceTime[i] = 0;
  }
  // outputs
  for (uint8_t i = 0; i < 4; ++i) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); // transistor base low -> LEDs off
  }
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  Serial.println("Water Level Indicator Ready");
}

void loop() {
  // read and debounce sensors
  for (uint8_t i = 0; i < 4; ++i) {
    uint8_t reading = digitalRead(levelPins[i]);
    if (reading != lastReading[i]) {
      lastDebounceTime[i] = millis();
      lastReading[i] = reading;
    }
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != levelState[i]) {
        levelState[i] = reading; // stable value
        // report change
        Serial.print("Level ");
        Serial.print(i+1);
        Serial.print(levelState[i] == LOW ? " : WATER" : " : NO WATER");
        Serial.println();
      }
    }
  }

  // Control LEDs: show rising levels (if level 1 reached -> LED1 on, etc.)
  // Assuming levelPins[0] is lowest probe and [3] is highest.
  for (uint8_t i = 0; i < 4; ++i) {
    // levelState LOW means sensor is triggered (water)
    if (levelState[i] == LOW) {
      digitalWrite(ledPins[i], HIGH); // turn on LED (transistor base HIGH -> sink)
    } else {
      digitalWrite(ledPins[i], LOW);  // LED off
    }
  }

  // Buzzer logic examples:
  // - If highest level reached (top probe triggered) -> continuous buzzer
  // - If below lowest level (all sensors not triggered) -> short beep warning (low water)
  bool topLevel = (levelState[3] == LOW);
  bool anyLevel = (levelState[0] == LOW) || (levelState[1] == LOW) || (levelState[2] == LOW) || topLevel;

  if (topLevel) {
    // high water alarm
    digitalWrite(buzzerPin, HIGH);
  } else if (!anyLevel) {
    // low water -> intermittent beep
    unsigned long t = millis() / 300;
    digitalWrite(buzzerPin, (t % 2) ? HIGH : LOW);
  } else {
    digitalWrite(buzzerPin, LOW);
  }

  delay(50); // small loop delay
}
