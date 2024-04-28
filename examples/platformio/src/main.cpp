#include <Arduino.h>

#include "pvc/arduino.hpp"
#include "pvc.hpp"

pvc<arduino::i2c> *sensor;

void setup() {
  Serial.begin(115200);
  Serial.print("{ start }\r\n");

  sensor = new pvc(new arduino::i2c(0));

  bool init, ready;
  while (true) {
    bool init = sensor->init();
    bool ready = sensor->ready();
    Serial.printf("{ init: %d, ready: %d }\r\n", init, ready);
    if (init && ready) {
      break;
    }
    delay(1000);
  }

  bool config = sensor->read_config(sensor->config());
  Serial.printf("{ config: { read: %d } }\r\n", config);
}

struct measure {
  bool   valid;
  double value;
};

void loop() {
  measure voltage, current, power;

  voltage.valid = sensor->voltage(voltage.value);
  current.valid = sensor->current(current.value);
  power.valid   = sensor->power(power.value);

  Serial.printf("{ V: { %s: %lf }, I: { %s: %lf }, P: { %s: %lf } }\r\n",
    voltage.valid ? "ok" : "ERR", voltage.value,
    current.valid ? "ok" : "ERR", current.value,
    power.valid   ? "ok" : "ERR", power.value);

  delay(1000);
}
