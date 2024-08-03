#include <Mouse.h>

const unsigned long TIMEOUT = 500000;  // max time to wait for flash in microseconds
const int BASELINE_SAMPLES = 1000;     // number of samples for calculating baseline illuminance
const int SENSOR_PIN = A1;             // the light sensor
const int RANDOM_PIN = A0;             // any unused analog pin to read a random value from

const long END = 1;
const long FAILED = 2;

enum Mode {
  Click = 0,
  Motion = 1
};

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  Mouse.begin();
}

void loop() {
  int threshold;     // sensitivity to changes in illuminance
  int trials;        // number of times to click/move mouse
  int time_between;  // min time between inputs in Click or Motion mode
  byte turn_amount;  // units to turn on the x axis when moving mouse, -128 to 127
  byte mode;

  // get settings
  while (true) {
    delay(200);

    byte available = Serial.available();
    if (available >= 8) {
      Serial.readBytes((byte*)&threshold, 2);
      Serial.readBytes((byte*)&trials, 2);
      Serial.readBytes((byte*)&time_between, 2);
      Serial.readBytes(&turn_amount, 1);
      Serial.readBytes(&mode, 1);

      Serial.flush();
      break;
    }
  }

  // for debugging with arduino ide serial monitor
  
  /* 
  
  delay(3000);

  for (int i = 0; i < 10; i++) {
    Serial.println(analogRead(SENSOR_PIN));
  }
  while (Serial.available()) {
    Serial.read();
  }
  return;

  */

  randomSeed(micros() + analogRead(RANDOM_PIN));  // micros() alone might not be random enough due to the delay(200) loop

  for (int i = 0; i < trials; i++) {
    // calibrate illuminance threshold
    int highest = 0;
    int lowest = 1023;
    for (int j = 0; j < BASELINE_SAMPLES; j++) {
      const int calibration_read = analogRead(SENSOR_PIN);

      if (calibration_read > highest) highest = calibration_read;
      if (calibration_read < lowest) lowest = calibration_read;
    }

    const int low_thres = lowest - threshold;
    const int high_thres = highest + threshold;

    // start trial
    unsigned long start = micros();

    if (mode == Click) {
      Mouse.click();
    } else {
      Mouse.move(turn_amount, 0);
    }

    while (true) {
      unsigned long time_diff = micros() - start;
      const int test_read = analogRead(SENSOR_PIN);

      if (test_read > high_thres || test_read < low_thres) {
        if (mode == Motion) {
          Mouse.move(-1 * turn_amount, 0);
        }

        Serial.write((byte*)&time_diff, 4);
        break;
      } else if (time_diff > TIMEOUT) {
        Serial.write((byte*)&FAILED, 4);
        return;
      }
    }

    delay(time_between + random(20));  // randomize to prevent phase locking
    delayMicroseconds(random(1000));
  }

  Serial.write((byte*)&END, 4);  // signal completion
}
