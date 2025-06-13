#include "order_book_manager.hpp"
#include <memory>

namespace hft {

OrderBookManager::OrderBookManager() : order_pool_(100000) {}

OrderBook *OrderBookManager::get_order_book(const std::string &symbol) {
  std::unique_lock lock(mutex_);

  auto it = order_books_.find(symbol);
  if (it != order_books_.end()) {
    return it->second.get();
  }

  // Create new order book
  // TODO: review performance benefits of emplace
  auto [new_it, success] = order_books_.emplace(
      symbol, std::make_unique<OrderBook>(symbol, order_pool_));
  return new_it->second.get();
}

bool OrderBookManager::process_order(const std::string &symbol, uint64_t id,
                                     uint64_t price, uint32_t quantity,
                                     uint32_t timestamp, OrderType type,
                                     Side side) {
  auto *book = get_order_book(symbol);

  switch (type) {
  case OrderType::Limit:
    return book->add_order(id, price, quantity, timestamp, side);

  case OrderType::Market: {
    auto [filled, cost] = book->process_market_order(quantity, side);
    return filled > 0;
  }

  case OrderType::Cancel:
    return book->cancel_order(id);

  default:
    return false;
  }
}

}; // namespace hft
