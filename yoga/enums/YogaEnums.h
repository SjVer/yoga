/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iterator>
#include <type_traits>

#ifndef APPLY_FIXES_FOR_CPP17

namespace facebook::yoga {

/**
 * Concept for any enum/enum class
 */
template <typename EnumT>
concept Enumeration = std::is_enum_v<EnumT>;

/**
 * Count of ordinals in a Yoga enum which is sequential
 */
template <Enumeration EnumT>
constexpr int32_t ordinalCount();

/**
 * Concept for a yoga enum which is sequential
 */
template <typename EnumT>
concept HasOrdinality = (ordinalCount<EnumT>() > 0);

/**
 * Count of bits needed to represent every ordinal
 */
template <HasOrdinality EnumT>
constexpr int32_t bitCount() {
  return std::bit_width(
      static_cast<std::underlying_type_t<EnumT>>(ordinalCount<EnumT>() - 1));
}

/**
 * Polyfill of C++ 23 to_underlying()
 * https://en.cppreference.com/w/cpp/utility/to_underlying
 */
constexpr auto to_underlying(Enumeration auto e) noexcept {
  return static_cast<std::underlying_type_t<decltype(e)>>(e);
}

/**
 * Convenience function to iterate through every value in a Yoga enum as part of
 * a range-based for loop.
 */
template <HasOrdinality EnumT>
auto ordinals() {
  struct Iterator {
    EnumT e{};

    EnumT operator*() const {
      return e;
    }

    Iterator& operator++() {
      e = static_cast<EnumT>(to_underlying(e) + 1);
      return *this;
    }

    bool operator==(const Iterator& other) const = default;
    bool operator!=(const Iterator& other) const = default;
  };

  struct Range {
    Iterator begin() const {
      return Iterator{};
    }
    Iterator end() const {
      return Iterator{static_cast<EnumT>(ordinalCount<EnumT>())};
    }
  };

  return Range{};
}

} // namespace facebook::yoga

#else

namespace facebook::yoga {

/**
 * Concept for any enum/enum class
 */
template <typename EnumT>
constexpr bool IsEnumeration = std::is_enum_v<EnumT>;

/**
 * Count of ordinals in a Yoga enum which is sequential
 */
template <typename EnumT, typename = std::enable_if_t<IsEnumeration<EnumT>>>
constexpr int32_t ordinalCount() = delete;

/**
 * Concept for a yoga enum which is sequential
 */
template <typename EnumT, typename = std::enable_if_t<IsEnumeration<EnumT>>>
constexpr bool HasOrdinality =
    std::integral_constant<bool, (ordinalCount<EnumT>() > 0)>::value;

/**
 * Bit width implementation for C++17 (equivalent to std::bit_width)
 */
template <typename T>
constexpr int32_t cpp17_bit_width(T x) {
  if (x == 0)
    return 0;

#if defined(_MSC_VER)
  if (std::_Is_constant_evaluated()) {
    int32_t width = 0;
    while (x) {
      width++;
      x >>= 1;
    }
    return width;
  }

  unsigned long index;
  _BitScanReverse(&index, x);
  return index + 1;

#elif defined(__GNUC__)
  if (__builtin_is_constant_evaluated()) {
    int32_t width = 0;
    while (x) {
      width++;
      x >>= 1;
    }
    return width;
  }

  return x == 0 ? 0 : __builtin_clz(x);

#else
#error "cpp17_bit_width() not implemented for current compiler"
#endif
}

/**
 * Count of bits needed to represent every ordinal
 */
template <typename EnumT>
constexpr typename std::enable_if<HasOrdinality<EnumT>, int32_t>::type
bitCount() {
  return cpp17_bit_width(
      static_cast<std::underlying_type_t<EnumT>>(ordinalCount<EnumT>() - 1));
}

/**
 * Polyfill of C++ 23 to_underlying()
 * https://en.cppreference.com/w/cpp/utility/to_underlying
 */
template <typename EnumT, typename = std::enable_if_t<IsEnumeration<EnumT>>>
constexpr auto to_underlying(EnumT e) noexcept {
  return static_cast<std::underlying_type_t<decltype(e)>>(e);
}

/**
 * Convenience function to iterate through every value in a Yoga enum as part of
 * a range-based for loop.
 */
template <typename EnumT, typename = std::enable_if_t<IsEnumeration<EnumT>>>
auto ordinals() {
  struct Iterator {
    EnumT e{};

    EnumT operator*() const {
      return e;
    }

    Iterator& operator++() {
      e = static_cast<EnumT>(to_underlying(e) + 1);
      return *this;
    }

    bool operator==(const Iterator& other) const {
      return e == other.e;
    }
    bool operator!=(const Iterator& other) const {
      return e != other.e;
    }
  };

  struct Range {
    Iterator begin() const {
      return Iterator{};
    }
    Iterator end() const {
      return Iterator{static_cast<EnumT>(ordinalCount<EnumT>())};
    }
  };

  return Range{};
}

} // namespace facebook::yoga

#endif
