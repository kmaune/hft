#pragma once
#include <cstdint>

namespace hft {

enum class Side : uint8_t {
  Buy = 0,
  Sell = 1
};

enum class OrderType : uint8_t {
  Limit = 0,
  Market = 1, 
  Cancel = 2 // TODO: think I can probably get rid of this
};

} // namespace hft
