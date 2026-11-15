
// =====================================
// WATTWISE - SMART CLASSROOM
// 8 LED AUTOMATIC LIGHTING SYSTEM
// =====================================

#include <DHT.h>

// Sensor pins
const int pirSensor = 2;
const int ldrPin = A0;

// LED pins: LED 1 to LED 8
const int ledPins[] = {3, 4, 5, 7, 8, 9, 10, 11};
const int totalLEDs = 8;

// DHT11 setup
#define DHTPIN 6
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// LDR thresholds
// Higher reading = darker
const int lowLightThreshold = 700;
const int mediumLightThreshold = 780;
const int veryDarkThreshold = 850;

// Inactivity timeout
const unsigned long inactivityDelay = 10000;

// Timing
unsigned long lastDHTRead = 0;
unsigned long lastSerialSend = 0;
unsigned long lastMotionTime = 0;

// Sensor readings
float temperature = NAN;
float humidity = NAN;

bool hasDetectedMotion = false;

void setup() {
  pinMode(pirSensor, INPUT);

  for (int i = 0; i < totalLEDs; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  Serial.begin(9600);
  dht.begin();
}

void loop() {
  unsigned long currentTime = millis();

  // 1. Read sensors
  int motion = digitalRead(pirSensor);
  int lightValue = analogRead(ldrPin);

  // 2. Motion detection
  if (motion == HIGH) {
    lastMotionTime = currentTime;
    hasDetectedMotion = true;
  }

  bool classroomOccupied =
    motion == HIGH ||
    (hasDetectedMotion &&
     currentTime - lastMotionTime < inactivityDelay);

  // 3. Decide how many LEDs are needed
  int requiredLEDs = 0;

  if (classroomOccupied) {
    if (lightValue > veryDarkThreshold) {
      requiredLEDs = 3 5;
    }
    else if (lightValue > mediumLightThreshold) {
      requiredLEDs = 5 8 10 ;
    }
    else if (lightValue > lowLightThreshold) {
      requiredLEDs = 3 5 8 11;
    }
    else {
      requiredLEDs = 0;
    }
  }

  // 4. Control all eight LEDs
  for (int i = 0; i < totalLEDs; i++) {
    digitalWrite(
      ledPins[i],
      i < requiredLEDs ? HIGH : LOW
    );
  }

  bool lightsOn = requiredLEDs > 0;

  // 5. Read DHT11 every two seconds
  if (currentTime - lastDHTRead >= 2000) {
    lastDHTRead = currentTime;

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
  }

  // 6. Send readings to the dashboard
  if (currentTime - lastSerialSend >= 300) {
    lastSerialSend = currentTime;

    // Format:
    // motion,light,lightsOn,temperature,humidity

    Serial.print(motion);
    Serial.print(",");

    Serial.print(lightValue);
    Serial.print(",");

    Serial.print(lightsOn ? 1 : 0);
    Serial.print(",");

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("ERROR,ERROR");
    }
    else {
      Serial.print(temperature);
      Serial.print(",");
      Serial.println(humidity);
    }
  }
}
