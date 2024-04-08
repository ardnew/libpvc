#pragma once

#include <cstddef>
#include <type_traits>

namespace bits {

namespace bytes {

enum class Ord: bool {
  MSFirst = false,
  LSFirst = true,
  BE      = MSFirst,
  LE      = LSFirst,
  Network = MSFirst,
  Host    = LSFirst,
  // Most "native endianness" examples online either occur at runtime, are not
  // constexpr-compatible, or they mistakenly determine the endianness of the
  // compilation host.
  //
  // The following macro is defined by both gcc and clang, so it covers many of
  // the common cases (notably, MSVC and ICC are unverified.)
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  Native  = MSFirst,
  Reverse = LSFirst,
#elif   defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  Native  = LSFirst,
  Reverse = MSFirst,
#else
  #error "undefined: __BYTE_ORDER__ (of target architecture)"
#endif
};

template <typename V, Ord To = Ord::Reverse, Ord From = Ord::Native,
  typename U = typename std::make_unsigned<V>::type,
  typename std::enable_if_t<std::is_integral_v<V>>* = nullptr>
struct Order {
private:
  static constexpr auto _size = sizeof(V);
  using byte_index = std::make_index_sequence<_size>;
  const V &_v;

  // byte swap unsigned integer of arbitrary size (1..N bytes)
  template <std::size_t ...N>
  inline static constexpr U swap(U v, std::index_sequence<N...>) noexcept {
    return ((((v>>nbi(N)) & 0xFF) << nbi(_size-1-N)) | ...);
  };

public:
  using type = U;

  Order() = delete;
  inline constexpr Order(const V &value): _v(value) {}

  ~Order() = default;

  // Return the desired byte ordering by casting Order to Order::type.
  inline constexpr operator U() const noexcept {
    if constexpr (To == From) {
      return static_cast<U>(_v);
    } else {
      return swap(static_cast<U>(_v), byte_index{});
    }
  }
};

// Return the given integral value with a desired byte ordering.
//
// The returned value is unsigned and has the same size as the input value.
//
// The byte ordering is determined by template parameters `To` and `From`, which
// are platform-dependent Ord::Reverse and Ord::Native by default, respectively.
//
// This function is constexpr-compatible and is a convenience wrapper around the
// Order struct.
template <Ord To = Ord::Reverse, Ord From = Ord::Native,
  typename V, typename O = Order<V, To, From>>
inline constexpr auto reorder(const V &value) noexcept {
  return static_cast<typename O::type>(O(value));
}

} // namespace bytes

} // namespace bits
