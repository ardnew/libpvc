# libpvc
##### Platform-independent I²C driver for INA260 power/voltage/current sensor

Datasheet: http://www.ti.com/lit/ds/symlink/ina260.pdf

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
