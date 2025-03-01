#pragma once

#include "enums.hpp"
#include "order_book.hpp"
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace hft {

// Class to manage multiple order books

class OrderBookManager {
private:
  std::unordered_map<std::string, std::unique_ptr<OrderBook>> order_books_;
  OrderPool order_pool_;
  std::shared_mutex mutex_;

public:
  OrderBookManager();

  // Get or create an order book for a symbol
  OrderBook *get_order_book(const std::string &symbol);

  // Process a new order
  bool process_order(const std::string &symbol, uint64_t id, uint64_t price,
                     uint32_t quantity, uint32_t timestamp, OrderType type,
                     Side side);
};

} // namespace hft
