#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#ifdef ARDUINO
#include "pvc/i2c_arduino.hpp"
using I2C = arduino::I2C; // Use the Arduino I²C implementation
#else
#include "pvc/i2c_espidf.hpp"
using I2C = espidf::I2C; // Use the ESP-IDF I²C implementation
#endif

#include "ina260.hpp"

template <typename I = I2C>
class pvc {
public:
  using interface = I;

  pvc(interface *i2c,
    const std::uint8_t   addr = ina260::default_addr_id,
    const std::uint32_t  freq = ina260::default_freq_hz,
    const ina260::config &config = ina260::config(),
    const ina260::masken &masken = ina260::masken(),
    const ina260::alimit &alimit = ina260::alimit())
    : _i2c(i2c),
      _addr(ina260::dev_addr_id(addr)),
      _freq(ina260::min_freq_hz(freq)),
      _config(config),
      _masken(masken),
      _alimit(alimit) {}

  ~pvc() = default;

  ina260::config &config() { return _config; }
  ina260::masken &masken() { return _masken; }
  ina260::alimit &alimit() { return _alimit; }

  // Initialize the I²C bus and I/O pins.
  bool init(void) {
    return _i2c->init(_addr, _freq);
  }

  // Check if the sensor is responding over I²C as expected.
  bool ready() {
    std::uint8_t u[2] = { 0 };
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::device_id), u, sizeof(u)
    );
    if (nr != sizeof(u)) {
      return false;
    }
    return ina260::device() == u; // overloaded operator==
  }

  bool read_config(ina260::config &config) {
    std::uint8_t u[2] = { 0 };
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::configuration), u, sizeof(u)
    );
    if (nr != sizeof(u)) {
      return false;
    }
    std::memcpy(&config.u16, u, sizeof(u));
    return true;
  }

  bool write_config(const ina260::config &config) {
    std::uint8_t u[sizeof(config.u16)] = { 0 };
    std::memcpy(u, &config.u16, sizeof(u));
    auto nw = _i2c->write(
      static_cast<std::uint8_t>(ina260::reg::configuration), u, sizeof(u)
    );
    if (nw != sizeof(config.u16)) {
      return false;
    }
    _config.u16 = config.u16;
    return true;
  }

  bool read_masken(ina260::masken &masken) {
    std::uint8_t u[2] = { 0 };
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::mask_enable), u, sizeof(u)
    );
    if (nr != sizeof(u)) {
      return false;
    }
    std::memcpy(&masken.u16, u, sizeof(u));
    return true;
  }

  bool write_masken(const ina260::masken &masken) {
    std::uint8_t u[sizeof(masken.u16)] = { 0 };
    std::memcpy(u, &masken.u16, sizeof(u));
    auto nw = _i2c->write(
      static_cast<std::uint8_t>(ina260::reg::mask_enable), u, sizeof(u)
    );
    if (nw != sizeof(masken.u16)) {
      return false;
    }
    _masken.u16 = masken.u16;
    return true;
  }

  bool read_alimit(ina260::alimit &alimit) {
    std::uint8_t u[2] = { 0 };
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::alert_limit), u, sizeof(u)
    );
    if (nr != sizeof(u)) {
      return false;
    }
    std::memcpy(&alimit.u16, u, sizeof(u));
    return true;
  }

  bool write_alimit(const ina260::alimit &alimit) {
    std::uint8_t u[sizeof(alimit.u16)] = { 0 };
    std::memcpy(u, &alimit.u16, sizeof(u));
    auto nw = _i2c->write(
      static_cast<std::uint8_t>(ina260::reg::alert_limit), u, sizeof(u)
    );
    if (nw != sizeof(alimit.u16)) {
      return false;
    }
    _alimit.u16 = alimit.u16;
    return true;
  }

  template <typename T,
    typename std::enable_if_t<std::is_arithmetic_v<T>>* = nullptr>
  bool voltage(T &v) {
    std::uint8_t u[2] = { 0 };
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::voltage), u, sizeof(u)
    );
    if (nr != sizeof(u)) {
      return false;
    }
    std::uint16_t u16 = 0;
    std::memcpy(&u16, u, sizeof(u));
    v = ina260::lsb_voltage * u16;
    return true;
  }

  template <typename T,
    typename std::enable_if_t<std::is_arithmetic_v<T>>* = nullptr>
  bool current(T &i) {
    std::uint8_t u[2] = { 0 };
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::current), u, sizeof(u)
    );
    if (nr != sizeof(u)) {
      return false;
    }
    std::uint16_t u16 = 0;
    std::memcpy(&u16, u, sizeof(u));
    i = ina260::lsb_current * u16;
    return true;
  }

  template <typename T,
    typename std::enable_if_t<std::is_arithmetic_v<T>>* = nullptr>
  bool power(T &p) {
    std::uint8_t u[2] = { 0 };
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::power), u, sizeof(u)
    );
    if (nr != sizeof(u)) {
      return false;
    }
    std::uint16_t u16 = 0;
    std::memcpy(&u16, u, sizeof(u));
    p = ina260::lsb_power * u16;
    return true;
  }

private:
  interface     *_i2c;
  std::uint8_t   _addr;
  std::uint32_t  _freq;

  ina260::config _config;
  ina260::masken _masken;
  ina260::alimit _alimit;

}; // class pvc
