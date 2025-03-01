#pragma once

#include "enums.hpp"
#include "order.hpp"
#include <array>
#include <atomic>
#include <shared_mutex>

namespace hft {

// Price level in the order book
class PriceLevel {
private:
  std::array<Order*, MAX_ORDERS_PER_LEVEL> orders_;
  size_t order_count_ = 0;
  std::atomic<uint64_t> total_quantity_{0};
  uint64_t price_;
  mutable std::shared_mutex mutex_; //Read-write lock for thread safety

public:
  explicit PriceLevel(uint64_t price);

  // Add order to this price level
  bool add_order(Order* order);

  // Remove order from this price level
  bool remove_order(uint64_t order_id);

  // Getters w/ appropriate synchronization
  uint64_t price() const;
  uint64_t total_quantity() const;
  size_t order_count() const;

  // Get order at specific index (for matching)
  Order* get_order(size_t index) const;

};

} // namespace hft
