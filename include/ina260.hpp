#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <array>
#include <string_view>

#include "pvc/internal/util.hpp"

using namespace std::literals;
using namespace std::literals::string_view_literals;

namespace ina260 {

  // Default I²C device address and bus frequency (Hz)
  constexpr std::uint8_t  default_addr_id = 0x40;
  constexpr std::uint32_t default_freq_hz = 100000;

  // Supported I²C bus frequencies (Hz)
  constexpr auto bus_freq_hz = util::make_array(
    100000U,  // 100.00 kHz · standard mode (Sm)
    400000U,  // 400.00 kHz · fast mode (Fm)
    1000000U, //   1.00 MHz · fast mode plus (Fm+)
    2940000U  //   2.94 MHz · high-speed mode (Hs)
  );

  // Units of measurement / values of LSB / units of least precision (ULP)
  constexpr auto lsb_current =  1.25;
  constexpr auto lsb_voltage =  1.25;
  constexpr auto lsb_power   = 10.00;

  // Configuration and data register addresses.
  enum class reg : std::uint8_t {
    configuration = 0x00,
    current       = 0x01,
    voltage       = 0x02,
    power         = 0x03,
    mask_enable   = 0x06,
    alert_limit   = 0x07,
    manufacturer  = 0xFE,
    device_id     = 0xFF,
  };

  // Contents of DEVICE_ID register (FFh)
  constexpr std::uint8_t  default_revision = 0x00;
  constexpr std::uint16_t default_deviceid = 0x227;

  template <typename K, typename V>
  using pair_type = std::pair<K, V>;

  template <typename K, typename V, std::size_t N>
  using pairs_type = std::array<pair_type<K, V>, N>;

  template <typename K, typename V, std::size_t N>
  constexpr K key_of_value(
    V value,
    const pairs_type<K, V, N> &mapping,
    const K &default_key
  ) {
    for (const auto &[key, val] : mapping) {
      if (val == value) {
        return key;
      }
    }
    return default_key;
  }

  template <typename K, typename V, std::size_t N>
  constexpr V const &value_of_key(
    K value,
    const pairs_type<K, V, N> &mapping,
    const V &default_value
  ) {
    for (const auto &[key, val] : mapping) {
      if (key == value) {
        return val;
      }
    }
    return default_value;
  }

  // Format of the CONFIGURATION register (00h)
  struct config {

    // Which measurements are performed for each conversion.
    enum class op_type : std::uint8_t {
      shutdown = 0x00, // = 0 (0b000)
      current  = 0x01, // = 1 (0b001)
      voltage  = 0x02, // = 2 (0b010)
      power    = 0x03, // = 3 (0b011) -- default
    };

    static constexpr pairs_type<op_type, std::string_view, 4> const op_type_mapping = {{
      {op_type::shutdown, std::string_view("shutdown")},
      {op_type::current, std::string_view("current")},
      {op_type::voltage, std::string_view("voltage")},
      {op_type::power, std::string_view("power")}
    }};

    template <typename T>
    static constexpr op_type to_op_type(T value) {
      using underlying = std::underlying_type_t<op_type>;
      return static_cast<op_type>(static_cast<underlying>(value));
    }

    static constexpr op_type to_op_type(std::string_view value) {
      return key_of_value(value, op_type_mapping, op_type::shutdown);
    }

    static constexpr const std::string_view value_of_key(const op_type &value) {
      return ina260::value_of_key(
        value, op_type_mapping, std::string_view("unknown")
      );
    }

    static constexpr const std::string_view to_base_units(op_type value) {
      static constexpr pairs_type<op_type, std::string_view, 3> const units_mapping = {{
        {op_type::current, "A"},
        {op_type::voltage, "V"},
        {op_type::power, "W"}
      }};
      return ina260::value_of_key(
        value, units_mapping, std::string_view("unknown")
      );
    }

    // Return a string representation of the default measurement units.
    // This returns the same units as to_base_units,
    // but with the sensor's native units prefix.
    static constexpr const std::string to_units(op_type value) {
      static constexpr std::string_view const &prefix = "m";
      auto const &units = to_base_units(value);
      if (units == "unknown") { return std::string(units); }
      return std::string(prefix) + std::string(units);
    }

    template <op_type enabled = op_type::power>
    static constexpr bool is_enabled(op_type value) {
      return static_cast<std::underlying_type_t<op_type>>(value) &
        static_cast<std::underlying_type_t<op_type>>(enabled);
    }

    // How measurements should be performed and updated in internal registers.
    enum class op_mode : std::uint8_t {
      triggered  = 0x00, // = 0 (0b000)
      continuous = 0x01, // = 1 (0b001) -- default
    };

    static constexpr pairs_type<op_mode, std::string_view, 2> const op_mode_mapping = {{
      {op_mode::triggered, std::string_view("triggered")},
      {op_mode::continuous, std::string_view("continuous")}
    }};

    template <typename T>
    static constexpr op_mode to_op_mode(T value) {
      using underlying = std::underlying_type_t<op_mode>;
      return static_cast<op_mode>(static_cast<underlying>(value));
    }

    static constexpr op_mode to_op_mode(std::string_view value) {
      return key_of_value(value, op_mode_mapping, op_mode::continuous);
    }

    static constexpr const std::string_view value_of_key(const op_mode &value) {
      return ina260::value_of_key(
        value, op_mode_mapping, std::string_view("unknown")
      );
    }

    // ADC conversion time for the voltage and current measurements.
    enum class adc_time : std::uint8_t {
      us140   = 0x00, // = 0 (0b000)
      us204   = 0x01, // = 1 (0b001)
      us332   = 0x02, // = 2 (0b010)
      us588   = 0x03, // = 3 (0b011)
      ms1p1   = 0x04, // = 4 (0b100) -- default (voltage, current)
      ms2p116 = 0x05, // = 5 (0b101)
      ms4p156 = 0x06, // = 6 (0b110)
      ms8p244 = 0x07, // = 7 (0b111)
    };

    static constexpr pairs_type<adc_time, std::string_view, 8> const adc_time_mapping = {{
      {adc_time::us140, std::string_view("140 µs")},
      {adc_time::us204, std::string_view("204 µs")},
      {adc_time::us332, std::string_view("332 µs")},
      {adc_time::us588, std::string_view("588 µs")},
      {adc_time::ms1p1, std::string_view("1.1 ms")},
      {adc_time::ms2p116, std::string_view("2.116 ms")},
      {adc_time::ms4p156, std::string_view("4.156 ms")},
      {adc_time::ms8p244, std::string_view("8.244 ms")}
    }};

    template <typename T>
    static constexpr adc_time to_adc_time(T value) {
      using underlying = std::underlying_type_t<adc_time>;
      return static_cast<adc_time>(static_cast<underlying>(value));
    }

    static constexpr adc_time to_adc_time(std::string_view value) {
      return key_of_value(value, adc_time_mapping, adc_time::us140);
    }

    static constexpr const std::string_view value_of_key(const adc_time &value) {
      return ina260::value_of_key(
        value, adc_time_mapping, std::string_view("unknown")
      );
    }

    // Number of samples that are collected and averaged.
    enum class adc_count : std::uint8_t {
      n1    = 0x00, // = 0 (0b000) -- default
      n4    = 0x01, // = 1 (0b001)
      n16   = 0x02, // = 2 (0b010)
      n64   = 0x03, // = 3 (0b011)
      n128  = 0x04, // = 4 (0b100)
      n256  = 0x05, // = 5 (0b101)
      n512  = 0x06, // = 6 (0b110)
      n1024 = 0x07, // = 7 (0b111)
    };

    static constexpr pairs_type<adc_count, std::string_view, 8> const adc_count_mapping = {{
      {adc_count::n1, std::string_view("1")},
      {adc_count::n4, std::string_view("4")},
      {adc_count::n16, std::string_view("16")},
      {adc_count::n64, std::string_view("64")},
      {adc_count::n128, std::string_view("128")},
      {adc_count::n256, std::string_view("256")},
      {adc_count::n512, std::string_view("512")},
      {adc_count::n1024, std::string_view("1024")}
    }};

    template <typename T>
    static constexpr adc_count to_adc_count(T value) {
      using underlying = std::underlying_type_t<adc_count>;
      return static_cast<adc_count>(static_cast<underlying>(value));
    }

    static constexpr adc_count to_adc_count(std::string_view value) {
      return key_of_value(value, adc_count_mapping, adc_count::n1);
    }

    static constexpr const std::string_view value_of_key(const adc_count &value) {
      return ina260::value_of_key(
        value, adc_count_mapping, std::string_view("unknown")
      );
    }

    union alignas(std::uint16_t) {
      std::uint16_t u16;
      #pragma pack(push, 1)
      struct {
        op_type       type : 2; //  0 —  1
        op_mode       mode : 1; //  2
        adc_time     ctime : 3; //  3 —  5
        adc_time     vtime : 3; //  6 —  8
        adc_count    count : 3; //  9 — 11
        std::uint8_t  resv : 3; // 12 — 14
        std::uint8_t reset : 1; // 15
      };
      #pragma pack(pop)
    };

    static constexpr std::uint16_t reserved_mask = 0x7000;

    constexpr config(
      const std::uint16_t value,
      const std::uint16_t mask = ~reserved_mask)
      : u16(value & mask) {}
    constexpr config(
      const op_type   type  = op_type::power,
      const op_mode   mode  = op_mode::continuous,
      const adc_time  ctime = adc_time::ms1p1,
      const adc_time  vtime = adc_time::ms1p1,
      const adc_count count = adc_count::n1,
      const bool      reset = false)
      : type(type),
        mode(mode),
        ctime(ctime),
        vtime(vtime),
        count(count),
        resv(0),
        reset(reset) {}

    virtual ~config() = default;

  }; // struct config

  // Format of the MASK/ENABLE register (06h)
  struct masken {
    union alignas(std::uint16_t) {
      std::uint16_t u16;
      #pragma pack(push, 1)
      struct {
        std::uint8_t  alert_latch_enable : 1; //  0
        std::uint8_t      alert_polarity : 1; //  1
        std::uint8_t       math_overflow : 1; //  2
        std::uint8_t    conversion_ready : 1; //  3
        std::uint8_t alert_function_flag : 1; //  4
        std::uint8_t                resv : 5; //  5 —  9
        std::uint8_t    alert_conversion : 1; // 10
        std::uint8_t    alert_over_power : 1; // 11
        std::uint8_t alert_under_voltage : 1; // 12
        std::uint8_t  alert_over_voltage : 1; // 13
        std::uint8_t alert_under_current : 1; // 14
        std::uint8_t  alert_over_current : 1; // 15
      };
      #pragma pack(pop)
    };

    static constexpr std::uint16_t reserved_mask = 0x03E0;

    constexpr masken(
      const std::uint16_t value,
      const std::uint16_t mask = ~reserved_mask)
      : u16(value & mask) {}
    constexpr masken(
      const bool  alert_latch_enable = false,
      const bool      alert_polarity = false,
      const bool       math_overflow = false,
      const bool    conversion_ready = false,
      const bool alert_function_flag = false,
      const bool    alert_conversion = false,
      const bool    alert_over_power = false,
      const bool alert_under_voltage = false,
      const bool  alert_over_voltage = false,
      const bool alert_under_current = false,
      const bool  alert_over_current = false)
      : alert_latch_enable(alert_latch_enable),
        alert_polarity(alert_polarity),
        math_overflow(math_overflow),
        conversion_ready(conversion_ready),
        alert_function_flag(alert_function_flag),
        resv(0),
        alert_conversion(alert_conversion),
        alert_over_power(alert_over_power),
        alert_under_voltage(alert_under_voltage),
        alert_over_voltage(alert_over_voltage),
        alert_under_current(alert_under_current),
        alert_over_current(alert_over_current) {}

    virtual ~masken() = default;

  }; // struct masken

  // Format of the ALERT_LIMIT register (07h)
  struct alimit {
    union alignas(std::uint16_t) {
      std::uint16_t u16;
      #pragma pack(push, 1)
      struct {
        std::uint16_t limit; //  0 — 15
      };
      #pragma pack(pop)
    };

    static constexpr std::uint16_t reserved_mask = 0x0;

    constexpr alimit(
      const std::uint16_t value,
      const std::uint16_t mask = ~reserved_mask)
      : u16(value & mask) {}
    constexpr alimit(
      const std::uint16_t limit = 0x0000)
      : limit(limit) {}

    virtual ~alimit() = default;

  }; // struct alimit

  // Format of the DEVICE_ID register (FFh)
  struct device {
    union alignas(std::uint16_t) {
      std::uint16_t u16;
      #pragma pack(push, 1)
      struct {
        std::uint8_t  revision :  4;
        std::uint16_t deviceid : 12;
      };
      #pragma pack(pop)
    };

    static constexpr std::uint16_t reserved_mask = 0x0;

    constexpr device(
      const std::uint16_t value,
      const std::uint16_t mask = ~reserved_mask)
      : u16(value & mask) {}
    constexpr device(
      const std::uint8_t revision = default_revision,
      const std::uint16_t deviceid = default_deviceid)
      : revision(revision),
        deviceid(deviceid) {}

    virtual ~device() = default;

    constexpr bool operator==(std::uint8_t (&p)[sizeof(u16)]) const {
      for (std::size_t i = 0; i < sizeof(u16); ++i) {
        if (p[i] != static_cast<std::uint8_t>(u16 >> (i << 3))) {
          return false;
        }
      }
      return true;
    }

  }; // struct device

  // Return the given I²C device address, masked to standard 7-bit addressing.
  constexpr auto dev_addr_id(std::uint8_t addr) {
    return addr & 0x7F;
  }

  // Return the minimum-supported bus frequency (Hz) that is greater than or
  // equal to the given frequency (Hz).
  // If no supported frequency is greater, return the maximum frequency.
  //
  // This way, requesting something beyond the supported range in either
  // direction will return the respective minimum or maximum supported.
  // E.g.: min_freq_hz(0)          == 100000
  //       min_freq_hz(UINT32_MAX) == 2940000
  constexpr auto min_freq_hz(std::uint32_t freq) {
    for (const auto &f : bus_freq_hz) {
      if (freq <= f) {
        return f;
      }
    }
    return bus_freq_hz.back();
  }

} // namespace ina260
