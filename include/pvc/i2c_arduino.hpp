#pragma once

#include <cstdint>

#include <Wire.h>

#include <bytes.hpp>

#include "pvc/i2c.hpp"

namespace arduino {

class I2C: public proto::I2C<>, public TwoWire {
public:
  // Construct a concrete I²C controller with the given bus and I/O pins.
  I2C(const std::uint8_t bus = 0, const std::int16_t sda = -1, const std::int16_t scl = -1)
    : TwoWire(bus), _enabled(TwoWire::begin(sda, scl)), _addr(0), _freq(0) {}

  virtual ~I2C() { TwoWire::end(); }

  // (Re)Initialize the I²C controller interface.
  // The I²C hardware and I/O pins must already be inititalized.
  //
  // The given device address and bus frequency will be used for all subsequent
  // read/write operations.
  bool init(const std::uint8_t addr, const std::uint32_t freq) override {
    if (did_init(addr, freq)) {
      return true; // already initialized
    }
    _addr = addr;
    _freq = freq;
    return _enabled && TwoWire::setClock(freq);
  }

  // Write data with the given number of bytes to the specified memory address,
  // and return the number of bytes successfully written.
  std::size_t write(const std::uint8_t addr, const std::uint16_t data, const std::size_t size) override {
    TwoWire::beginTransmission(_addr);
    (void)TwoWire::write(static_cast<const uint8_t *>(&addr), sizeof(addr));
    std::uint16_t u = bytes::reorder(data);
    std::size_t count = TwoWire::write(static_cast<const uint8_t *>(&u), size);
    if (TwoWire::endTransmission()) {
      return 0;
    }
    return count;
  }

  // Read the given number of bytes from the specified memory address, and
  // return the number of bytes successfully read.
  std::size_t read(const std::uint8_t addr, std::uint16_t &data, const std::size_t size) override {
    TwoWire::beginTransmission(_addr);
    (void)TwoWire::write(static_cast<const uint8_t *>(&addr), sizeof(addr));
    if (TwoWire::endTransmission(true)) {
      return 0;
    }
    (void)TwoWire::requestFrom(_addr, size, false);
    std::size_t count = 0;
    std::uint16_t u = 0;
    while (count < size && TwoWire::available()) {
      static_cast<uint8_t *>(&u)[count++] = TwoWire::read();
    }
    data = bytes::reorder(u);
    return count;
  }

protected:
  bool _enabled;

  std::uint8_t  _addr;
  std::uint32_t _freq;

  // Verify the controller was initialized with non-zero _addr and _freq.
  inline bool did_init() const { return _enabled && ((_addr | _freq) != 0); }

  // Verify the controller did_init with the given addr and freq.
  inline bool did_init(const std::uint8_t addr, const std::uint32_t freq) const {
    return did_init() && addr == _addr && freq == _freq;
  }
};

} // namespace arduino
