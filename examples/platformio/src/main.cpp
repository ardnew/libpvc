#include <Arduino.h>
#include "pvc/i2c_arduino.hpp"
#include "pvc.hpp"

// Declare a pvc driver instance that uses the Arduino adapter.
// Bus 0 is the same as the default Wire object (e.g., use i2c(1) for Wire1).
pvc<arduino::I2C> sensor(new arduino::I2C(0));

// (optional) Initialize INA260 configuration settings:
//
// Enable continuous measurements over each power, voltage, and current.
// Each measurement is the average among 256 samples, and each sample requires
// 588µs to complete. Thus, each measurement requires 256 * 588µs = 150.528ms.
ina260::config config(
  ina260::config::op_type::power,      // operating type (power, voltage, current)
  ina260::config::op_mode::continuous, // operating mode (triggered, continuous)
  ina260::config::adc_time::us588,     // shunt current conversion time (ADC precision)
  ina260::config::adc_time::us588,     // bus voltage conversion time (ADC precision)
  ina260::config::adc_count::n256,     // averaging mode (number of ADC samples)
  false                                // power-on reset, clear all registers (false)
);

// Or, equivalently, use raw register values:
//ina260::config config(0x0ADF);

// Pair a sensor measurement with its validity flag.
//
// You can use any arithmetic type for the value field (e.g., int, float, etc.).
// The pvc class will automatically perform any necessary type conversions at
// compile-time.
struct measure { bool valid; float value; };

void setup() {
  Serial.begin(115200);

  // Initialize the pvc driver and Arduino adapter.
  while (!sensor.init() || !sensor.ready())
    { delay(200); }

  // Commit our configuration settings to the sensor.
  while (!sensor.write_config(config))
    { delay(200); }

  // You can use write_masken() and write_alimit() to configure how the ALERT
  // pin functions, and the threshold values for that function, respectively.
}

void loop() {
  static measure voltage, current, power;
  static char    output[256];

  // Read the sensor values continuously
  voltage.valid = sensor.voltage(voltage.value);
  current.valid = sensor.current(current.value);
  power.valid   = sensor.power(power.value);

  // Print each measurement (validity indicator: "==" vs. "!=")
  snprintf(output, sizeof(output) / sizeof(*output),
    "V %c= %-9.2f\tI %c= %-9.2f\tP %c= %-9.2f",
    ( voltage.valid ? '=' : '!' ), voltage.value,
    ( current.valid ? '=' : '!' ), current.value,
    ( power.valid   ? '=' : '!' ), power.value);

  Serial.println(output);
}
