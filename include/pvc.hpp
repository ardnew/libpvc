#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include <bytes.hpp>

#ifdef ARDUINO
#include "pvc/arduino.hpp"
using i2c = arduino::i2c; // Use the Arduino I²C implementation
#else
#error "Unsupported platform"
#endif

#include "ina260.hpp"

template <typename I = i2c>
class pvc {
private:
  using interface = I;

  interface     *_i2c;
  std::uint8_t   _addr;
  std::uint32_t  _freq;

  ina260::config _config;
  ina260::masken _masken;
  ina260::alimit _alimit;

public:
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
    uint16_t u = 0;
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::device_id), &u, sizeof(u));
    if (nr != sizeof(u)) {
      return false;
    }
    return ina260::device().u16 == bytes::reorder(u);
  }

  bool read_config(ina260::config &config) {
    uint16_t u = 0;
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::configuration), &u, sizeof(u));
    if (nr != sizeof(u)) {
      return false;
    }
    config.u16 = bytes::reorder(u);
    return true;
  }

  bool write_config(const ina260::config &config) {
    uint16_t u = bytes::reorder(config.u16);
    auto nw = _i2c->write(
      static_cast<std::uint8_t>(ina260::reg::configuration), &u, sizeof(u));
    if (nw != sizeof(u)) {
      return false;
    }
    _config.u16 = config.u16;
    return true;
  }

  bool read_masken(ina260::masken &masken) {
    uint16_t u = 0;
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::mask_enable), &u, sizeof(u));
    if (nr != sizeof(u)) {
      return false;
    }
    masken.u16 = bytes::reorder(u);
    return true;
  }

  bool write_masken(const ina260::masken &masken) {
    uint16_t u = bytes::reorder(masken.u16);
    auto nw = _i2c->write(
      static_cast<std::uint8_t>(ina260::reg::mask_enable), &u, sizeof(u));
    if (nw != sizeof(u)) {
      return false;
    }
    _masken.u16 = masken.u16;
    return true;
  }

  bool read_alimit(ina260::alimit &alimit) {
    uint16_t u = 0;
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::alert_limit), &u, sizeof(u));
    if (nr != sizeof(u)) {
      return false;
    }
    alimit.u16 = bytes::reorder(u);
    return true;
  }

  bool write_alimit(const ina260::alimit &alimit) {
    uint16_t u = bytes::reorder(alimit.u16);
    auto nw = _i2c->write(
      static_cast<std::uint8_t>(ina260::reg::alert_limit), &u, sizeof(u));
    if (nw != sizeof(u)) {
      return false;
    }
    _alimit.u16 = alimit.u16;
    return true;
  }

  bool voltage(double &v) {
    uint16_t u = 0;
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::voltage), &u, sizeof(u));
    if (nr != sizeof(u)) {
      return false;
    }
    v = ina260::lsb_voltage * bytes::reorder(u);
    return true;
  }

  bool current(double &i) {
    uint16_t u = 0;
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::current), &u, sizeof(u));
    if (nr != sizeof(u)) {
      return false;
    }
    i = ina260::lsb_current * bytes::reorder(u);
    return true;
  }

  bool power(double &p) {
    uint16_t u = 0;
    auto nr = _i2c->read(
      static_cast<std::uint8_t>(ina260::reg::power), &u, sizeof(u));
    if (nr != sizeof(u)) {
      return false;
    }
    p = ina260::lsb_power * bytes::reorder(u);
    return true;
  }

  //int getVoltage(double *V_);
  //int getCurrent(double *I_);
  //int getPower(double *P_);
  //int setConfig(unsigned short val);
  //int setAlert(unsigned short val1);
  //void readAlert(void);
  //int setLim(unsigned short val2);

}; // class pvc
