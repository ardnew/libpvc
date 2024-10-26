# libpvc
##### Platform-independent I²C driver for INA260 power/voltage/current sensor ([datasheet](docs/ina260.pdf))

![MAX POWER](docs/max-power.jpg)

---

#### API features:
- [x] C++17 header-only library
- [x] Abstract interface for all platform-specific I²C operations
- [x] Documentation provided directly in source code
#### Device driver features:
- [x] Configurable operating type
  - [x] Continuous or triggered "one-shot" conversions
- [x] Configurable ADC sample size and conversion time
  - [x] Independent configurations for bus voltage and current
- [x] Over/under voltage/current and conversion-ready ALERT interrupts
- [X] Native I²C adapters implemented for Arduino and ESP-IDF

## Design

This library is composed of C++17 header files that represent [various layers](https://en.wikipedia.org/wiki/Abstraction_layer) of [hardware abstraction](https://en.wikipedia.org/wiki/Hardware_abstraction).

|Header|Endpoint|Abstraction|Description|
|:-----|:------:|:---------:|:----------|
|[`pvc.hpp`](include/pvc.hpp)|Peripheral|Power sensor|General-purpose interface to a power/voltage/current sensor|
|[`ina260.hpp`](include/ina260.hpp)|Peripheral|TI INA260|INA260 programming interface (memory map, register addresses, etc.)|
|[`pvc/i2c.hpp`](include/pvc/i2c.hpp)|Controller|I²C communication|General-purpose I²C controller interface|
|[`pvc/i2c_arduino.hpp`](include/pvc/i2c_arduino.hpp)|Controller|I²C processor|Arduino reference implementation of I²C controller adapter|
|[`pvc/i2c_espidf.hpp`](include/pvc/i2c_espidf.hpp)|Controller|I²C processor|ESP-IDF reference implementation of I²C controller adapter|

#### Notes

This library uses C++ language features that are only available with C++17 or newer (`constexpr`, `auto`, etc.). Ensure your compiler and toolchain support this standard — **many do not**. With GCC, for example, you could use `-std=gnu++17` or `-std=c++17` or something newer.

## Reference Platform

The library uses a [platform-agnostic I²C interface](include/pvc/i2c.hpp) so that it can be easily integrated on any system with a user-provided adapter. There are three methods that must be implemented:

```c++
  virtual bool init(const std::uint8_t, const std::uint32_t);
  virtual std::size_t write(const std::uint8_t, const std::uint8_t * const &, const std::size_t);
  virtual std::size_t read(const std::uint8_t, std::uint8_t * const &, const std::size_t);
```

Example I²C adapters are included for:
 - [Arduino](include/pvc/i2c_arduino.hpp): uses [`Wire` from the Arduino core API](https://www.arduino.cc/reference/en/language/functions/communication/wire/).
 - [ESP-IDF](include/pvc/i2c_espidf.hpp): uses [`i2c_master` from Espressif's own driver component](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/i2c.html#api-reference).

Using the INA260 driver on Arduino could look as simple as the following. But, please, refer to [the example](examples/platformio/src/main.cpp) for a more complete reference with comments and sensor configuration.

```c++
#include <Arduino.h>
#include "pvc/i2c_arduino.hpp"
#include "pvc.hpp"

// Declare a pvc driver instance that uses the Arduino adapter
pvc<arduino::I2C> sensor(new arduino::I2C(0));

// Or, if using ESP-IDF, include "pvc/i2c_espidf.hpp" above and declare:
//pvc<espidf::I2C> sensor(new espidf::I2C(espidf::I2C::Config{...}));

// Pair a sensor measurement with its validity flag
struct measure { bool valid; float value; };

void setup() {
  while (!sensor.init() || !sensor.ready()) 
    { delay(200); }
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

```
