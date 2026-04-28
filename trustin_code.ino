/*
  Simple Guitar Tuner using KY-037
  Method: Zero-crossing frequency estimation
  Author: ChatGPT

  Wiring:
  KY-037 AO -> A0
  VCC -> 5V
  GND -> GND
*/

const int micPin = A0;

int lastValue = 0;
int threshold = 512;

unsigned long lastTime = 0;
unsigned long sampleStart = 0;

int crossings = 0;
float frequency = 0;

// Guitar standard tuning reference
const char* noteNames[] = {"E2", "A2", "D3", "G3", "B3", "E4"};
float noteFreqs[] = {82.41, 110.00, 146.83, 196.00, 246.94, 329.63};

String getClosestNote(float freq) {
  int closestIndex = 0;
  float minDiff = abs(freq - noteFreqs[0]);

  for (int i = 1; i < 6; i++) {
    float diff = abs(freq - noteFreqs[i]);
    if (diff < minDiff) {
      minDiff = diff;
      closestIndex = i;
    }
  }

  return String(noteNames[closestIndex]);
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("KY-037 Guitar Tuner Starting...");
}

void loop() {
  int sensorValue = analogRead(micPin);

  // detect zero crossing
  if ((lastValue < threshold && sensorValue >= threshold) ||
      (lastValue > threshold && sensorValue <= threshold)) {
    crossings++;
  }

  lastValue = sensorValue;

  // 1-second measurement window
  if (millis() - sampleStart >= 1000) {

    frequency = crossings / 2.0; // 2 crossings per wave cycle

    Serial.print("Freq: ");
    Serial.print(frequency);
    Serial.print(" Hz");

    if (frequency > 20 && frequency < 1000) {
      Serial.print(" | Closest Note: ");
      Serial.print(getClosestNote(frequency));
    }

    Serial.println();

    crossings = 0;
    sampleStart = millis();
  }
}