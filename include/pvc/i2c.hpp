#pragma once

#include <cstddef>
#include <cstdint>

// Enclose the abstract class in namespace "proto" to prevent name clashes,
// and to make it clear that it must be implemented — not instantiated directly.
namespace proto {

// Pure abstract class that defines the interface for communicating register
// read/write operations over I²C.
//
// The I²C protocol itself does not define any concept of memory or registers.
// Conventionally, these are implemented using multi-message transactions where:
//  - the first message identifies the address or command; and
//  - subsequent messages read or write the data at that address.
//
// Using multi-message transactions introduces another problem: the byte order
// of message data is unspecified. In general, there is not a conventional byte
// order, so it must be specified per device relative to the derived class's
// native byte order. The INA260 byte order is big-endian (most-significant byte
// first).
//
// This class does not use fixed-width buffers for data transfer, which gives
// derived classes full control over memory allocation. However, this requires
// derived classes to also handle all byte order conversions.
template <typename Tx = const std::uint16_t, typename Rx = std::uint16_t &>
struct I2C {
  // (Re)Initialize the I²C controller interface.
  // The I²C hardware and I/O pins must already be inititalized.
  //
  // The given device address and bus frequency will be used for all subsequent
  // read/write operations.
  virtual bool init(const std::uint8_t addr, const std::uint32_t freq) = 0;

  // Write data with the given number of bytes to the specified memory address,
  // and return the number of bytes successfully written.
  virtual std::size_t write(const std::uint8_t addr, Tx data, const std::size_t size) = 0;

  // Read the given number of bytes from the specified memory address, and
  // return the number of bytes successfully read.
  virtual std::size_t read(const std::uint8_t addr, Rx data, const std::size_t size) = 0;
};

} // namespace proto
