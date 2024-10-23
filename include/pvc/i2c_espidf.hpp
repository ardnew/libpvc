#pragma once

#include <cstdint>
#include <cstring>

#include <driver/i2c_master.h>

#include <bytes.hpp>

#include "pvc/i2c.hpp"

namespace espidf {

class I2C: public proto::I2C<> {
public:
  struct Config {
    i2c_master_bus_config_t bus;
    i2c_device_config_t     dev;
  };

  struct Handle {
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t dev;
  };

  // Construct a concrete I²C controller with the given bus and I/O pins.
  I2C(const Config &config)
    : _hdl({}), _cfg(config), _init(ESP_ERR_NOT_FINISHED), _mount(ESP_ERR_NOT_FINISHED)
  {}

  virtual ~I2C() {
    if (did_init()) {
      if (did_mount(_cfg.dev.device_address, _cfg.dev.scl_speed_hz)) {
        i2c_master_bus_rm_device(_hdl.dev);
      }
      i2c_del_master_bus(_hdl.bus);
    }
  }

  // (Re)Initialize the I²C controller interface.
  // The I²C hardware and I/O pins must already be inititalized.
  //
  // The given device address and bus frequency will be used for all subsequent
  // read/write operations.
  bool init(const std::uint8_t addr, const std::uint32_t freq) override {
    bool result = true;
    if (!did_mount(addr, freq)) {
      if (!did_init()) {
        _init = i2c_new_master_bus(&_cfg.bus, &_hdl.bus);
        result = ESP_OK == _init;
      }
      _cfg.dev.device_address = addr;
      _cfg.dev.scl_speed_hz = freq;
      _mount = i2c_master_bus_add_device(_hdl.bus, &_cfg.dev, &_hdl.dev);
      result = result && ESP_OK == _mount;
    }
    return result;
  }

  // Write data with the given number of bytes to the specified memory address,
  // and return the number of bytes successfully written.
  std::size_t write(const std::uint8_t addr, const std::uint16_t data, const std::size_t size) override {
    std::uint8_t buf[size + sizeof(1)] = { addr };
    std::uint16_t u = bytes::reorder(data);
    std::memcpy(&buf[1], &u, size);
    if (ESP_OK == i2c_master_transmit(_hdl.dev, buf, size + 1, -1)) {
      return size;
    }
    return 0;
  }

  // Read the given number of bytes from the specified memory address, and
  // return the number of bytes successfully read.
  std::size_t read(const std::uint8_t addr, std::uint16_t &data, const std::size_t size) override {
    std::uint8_t buf[size] = { 0 };
    if (ESP_OK == i2c_master_transmit_receive(_hdl.dev, &addr, sizeof(addr), buf, size, -1)) {
      std::uint16_t u = 0;
      std::memcpy(&u, buf, size);
      data = bytes::reorder(u);
      return size;
    }
    return 0;
  }

protected:
  Handle _hdl;
  Config _cfg;

  esp_err_t _init;  // ESP_OK if the controller was initialized.
  esp_err_t _mount; // ESP_OK if the device was mounted.

  // Verify the controller was initialized.
  inline bool did_init() const { return ESP_OK == _init; }
  // Verify the device was mounted.
  inline bool did_mount(std::uint16_t addr, std::uint32_t freq) const {
    return ESP_OK == _mount &&
      addr == _cfg.dev.device_address &&
      freq == _cfg.dev.scl_speed_hz;
  }
};

} // namespace espidf
