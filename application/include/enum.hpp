#pragma once

#include <cstdint>
#include <type_traits>

// -- useful functions to eliminate need of static_cast

template <typename EnumType> constexpr auto Idx(EnumType const & v) {
  return static_cast<std::underlying_type_t<EnumType>>(v);
}

template <typename EnumType>
constexpr const auto & Idx(EnumType & v) {
  return reinterpret_cast<std::underlying_type_t<EnumType> &>(v);
}

namespace ld {
  template <typename T> int sgn(T val) {
      return (T(0) < val) - (val < T(0));
  }
}
