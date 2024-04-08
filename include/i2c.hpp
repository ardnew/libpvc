#pragma once

#include <cstddef>
#include <cstdint>
#include <chrono>

#include "bits/bytes.hpp"

namespace ina260 {

namespace i2c {

struct Dev {
protected:
  std::uint8_t _addr;
public:
  Dev() = delete;
  constexpr Dev(const std::uint8_t &addr) noexcept: _addr(addr) {}
  ~Dev() = default;

  constexpr std::uint8_t addr() const noexcept { return _addr; }
};

struct Reg {
protected:
  std::uint8_t addr;
public:
  Reg() = delete;
  constexpr Reg(const std::uint8_t &addr) noexcept: addr(addr) {}
  ~Reg() = default;

  constexpr std::uint8_t addr() const noexcept { return addr; }
};

struct Data {
protected:
  const std::uint16_t &_u16;

public:
  Data() = delete;
  constexpr Data(const Data &dat) noexcept: _u16(dat._u16) {}
  constexpr Data(const std::uint16_t &u16) noexcept: _u16(u16) {}
  ~Data() = default;

  constexpr operator std::uint16_t() const noexcept { return _u16; }

  constexpr std::uint16_t le16() const noexcept {
    return bits::bytes::reorder<bits::bytes::Ord::LE>(_u16);
  }
  constexpr std::uint16_t be16() const noexcept {
    return bits::bytes::reorder<bits::bytes::Ord::BE>(_u16);
  }
};

struct I2C {
protected:
  const Dev &_dev;

public:
  I2C() = delete;
  constexpr I2C(const Dev &dev) noexcept: _dev(dev) {}
  constexpr I2C(const std::uint8_t &addr) noexcept: _dev(addr) {}
  virtual ~I2C() = default;

  virtual std::chrono::milliseconds ticks() const noexcept = 0;
  virtual bool write(const Reg &reg, const Data &dat) = 0;
  virtual bool read(const Reg &reg, const Data &dat) = 0;
};

} // namespace i2c

} // namespace ina260
