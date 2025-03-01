#include "price_level.hpp"
#include "order.hpp"
#include <shared_mutex>

namespace hft {

PriceLevel::PriceLevel(uint64_t price) : price_(price) {}

bool PriceLevel::add_order(Order *order) {
  std::unique_lock lock(mutex_);
  if (order_count_ >= MAX_ORDERS_PER_LEVEL) {
    return false;
  }

  orders_[order_count_++] = order;
  total_quantity_ += order->quantity;
  return true;
}

bool PriceLevel::remove_order(uint64_t order_id) {
  std::unique_lock lock(mutex_);
  for (size_t i = 0; i < order_count_; ++i) {
    if (orders_[i]->id == order_id) {
      total_quantity_ -= orders_[i]->quantity;
      // Move the last order to fill the gap
      if (i < order_count_ - 1) {
        orders_[i] = orders_[order_count_ - 1];
      }
      --order_count_;
      return true;
    }
  }
  return false;
}

uint64_t PriceLevel::price() const { return price_; }

uint64_t PriceLevel::total_quantity() const { return total_quantity_; }

size_t PriceLevel::order_count() const {
  std::shared_lock lock(mutex_);
  return order_count_;
}

Order *PriceLevel::get_order(size_t index) const {
  std::shared_lock lock(mutex_);
  if (index < order_count_) {
    return orders_[index];
  }
  return nullptr;
}

} // namespace hft
