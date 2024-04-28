#pragma once

#include <array>

namespace util {

template <typename ...T>
using array = std::array<std::common_type_t<T...>, sizeof...(T)>;

template <typename ...T>
constexpr array<T...> make_array(T... args) { return { args... }; }

} // namespace util
