#include "order_pool.hpp"
#include <mutex>

// TODO: ask genai if the allocate and deallocate can cause vector resizing.
// Does pop back resize the vector?
namespace hft {

OrderPool::OrderPool(size_t initial_size) {
  orders_.reserve(initial_size);
  free_orders_.reserve(initial_size);

  // Pre-allocate orders
  for (size_t i = 0; i < initial_size; ++i) {
    orders_.push_back(std::make_unique<Order>());
    free_orders_.push_back(orders_.back().get());
  }
}

Order *OrderPool::allocate(uint64_t id, uint64_t price, uint32_t quantity,
                           uint32_t timestamp, Side side,
                           std::string_view symbol) {
  std::lock_guard<std::mutex> lock(mutex_);
  Order *order;

  if (!free_orders_.empty()) {
    order = free_orders_.back();
    free_orders_.pop_back();
  } else {
    orders_.push_back(std::make_unique<Order>());
    order = orders_.back().get();
  }

  order->id = id;
  order->price = price;
  order->quantity = quantity;
  order->timestamp = timestamp;
  order->side = side;
  order->symbol = symbol;

  return order;
}

void OrderPool::deallocate(Order *order) {
  std::lock_guard<std::mutex> lock(mutex_);
  free_orders_.push_back(order);
}

} // namespace hft
