#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define MIC_PIN A0

#define BTN_NEXT 8
#define BTN_PREV 9

const char* stringNames[] = {"Low E", "A", "D", "G", "B", "High E"};
float stringFreqs[] = {82.41, 110.00, 146.83, 196.00, 246.94, 329.63};

int currentIndex = 0;

unsigned long lastPressTime = 0;
const int debounceDelay = 200;

bool showMessage = false;
unsigned long messageStart = 0;
const int messageDuration = 800;

unsigned long lastPeakTime = 0;
float frequency = 0;
float smoothedFreq = 0;

int threshold = 540;
int minAmplitude = 30;

int prevSignal = 0;
bool wasRising = false;

int minSignal = 1023;
int maxSignal = 0;

void setup() {
  lcd.begin(16, 2);

  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);

  lcd.setCursor(0, 0);
  lcd.print("Guitar Tuner");
  delay(1000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(stringNames[currentIndex]);
}

void loop() {
  readButtons();

  float targetFreq = stringFreqs[currentIndex];

  int signal = analogRead(MIC_PIN);

  if (signal < minSignal) minSignal = signal;
  if (signal > maxSignal) maxSignal = signal;

  int amplitude = maxSignal - minSignal;

  bool isRising = signal > prevSignal;

  if (amplitude > minAmplitude) {

    if (!isRising && wasRising && signal > threshold) {

      unsigned long currentTime = micros();

      if (lastPeakTime > 0) {
        unsigned long period = currentTime - lastPeakTime;

        if (period < 800) {
          lastPeakTime = currentTime;
          prevSignal = signal;
          wasRising = isRising;
          return;
        }

        frequency = 1000000.0 / period;

        while (frequency > targetFreq * 1.2) frequency /= 2;
        while (frequency < targetFreq * 0.8) frequency *= 2;

        smoothedFreq = (smoothedFreq * 0.85) + (frequency * 0.15);

        updateLCD(smoothedFreq, targetFreq);
      }

      lastPeakTime = currentTime;
    }
  }

  static unsigned long lastReset = 0;
  if (millis() - lastReset > 50) {
    minSignal = 1023;
    maxSignal = 0;
    lastReset = millis();
  }

  prevSignal = signal;
  wasRising = isRising;
}

void readButtons() {
  if (millis() - lastPressTime < debounceDelay) return;

  if (digitalRead(BTN_NEXT) == LOW) {
    currentIndex++;
    if (currentIndex > 5) currentIndex = 0;

    showSelectionMessage();
    lastPressTime = millis();
  }
  else if (digitalRead(BTN_PREV) == LOW) {
    currentIndex--;
    if (currentIndex < 0) currentIndex = 5;

    showSelectionMessage();
    lastPressTime = millis();
  }
}

void showSelectionMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Selected:");
  lcd.setCursor(0, 1);
  lcd.print(stringNames[currentIndex]);

  showMessage = true;
  messageStart = millis();
}

void updateLCD(float freq, float targetFreq) {

  if (showMessage) {
    if (millis() - messageStart > messageDuration) {
      showMessage = false;
      lcd.clear();
    } else {
      return;
    }
  }

  lcd.setCursor(0, 0);
  lcd.print(stringNames[currentIndex]);
  lcd.print("      "); 

  lcd.setCursor(0, 1);
  lcd.print(freq, 1);
  lcd.print("Hz ");

  float tolerance = targetFreq * 0.05;

  if (freq < targetFreq - tolerance) {
    lcd.print("UP  ");
  }
  else if (freq > targetFreq + tolerance) {
    lcd.print("DOWN");
  }
  else {
    lcd.print("OK  ");
  }
}