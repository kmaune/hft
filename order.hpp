#pragma once
#include "enums.hpp"
#include <cstddef>
#include <string_view>

namespace hft {

// Constants for order book sizing and optimization
constexpr size_t MAX_PRICE_LEVELS = 1024;
constexpr size_t MAX_ORDERS_PER_LEVEL = 256;
constexpr size_t MAX_SYMBOLS = 64;
constexpr size_t CACHE_LINE_SIZE = 64; // Typical cache line size

// Align order to cache line to prevent false sharing

struct alignas(CACHE_LINE_SIZE) Order {
  uint64_t id;
  uint64_t price;
  uint32_t quantity;
  uint32_t timestamp;
  Side side;
  std::string_view symbol;

  Order(uint64_t id_, uint64_t price_, uint32_t quantity_, uint32_t timestamp_,
        Side side_, std::string_view symbol_)
      : id(id_), price(price_), quantity(quantity_), timestamp(timestamp_),
        side(side_), symbol(symbol_) {}

  //TODO: probably can remove
  Order() : id(0), price(0), quantity(0), timestamp(0), side(Side::Buy), symbol("") {}
};

} // namespace hft
