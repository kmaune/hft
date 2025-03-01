#pragma once

#include "enums.hpp"
#include "order.hpp"
#include "order_pool.hpp"
#include "price_level.hpp"
#include <array>
#include <limits>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>

namespace hft {
 
// Order book for single financial instrument/symbol
class OrderBook {
private:
  std::string symbol_;
  std::array<PriceLevel*, MAX_PRICE_LEVELS> buy_levels_;
  std::array<PriceLevel*, MAX_PRICE_LEVELS> sell_levels_;
  size_t buy_level_count_ = 0;
  size_t sell_level_count_ = 0;
  uint64_t last_trade_price;
  uint32_t last_trade_quantity_ = 0;
  std::shared_mutex mutex_; // Read-write lock for thread safety
  std::unordered_map<uint64_t, Order*> order_map; //For fast order lookup by id
  OrderPool& order_pool_;

  // Internal methods
  PriceLevel* add_price_level(Side side, uint64_t price);
  PriceLevel* find_price_level(Side side, uint64_t price);
  void remove_price_level(Side side, uint64_t price);

public:
  OrderBook(std::string symbol, OrderPool& order_pool);
  ~OrderBook();

  // TODO -- copy/move ctors/assignment
 
  bool add_order(uint64_t id, uint64_t price, uint32_t quantity, uint32_t timestamp, Side side);
  bool cancel_order(uint64_t order_id);
  std::pair<uint32_t, uint64_t> process_market_order(uint32_t quantity, Side side);

  // Cross-orders
  void match_orders();

  // Get spread (difference between best bid and ask)
  uint64_t get_spread() const;
  
  // Get mid price (average of best bid and ask)
  uint64_t get_mid_price() const;

  uint64_t get_best_bid() const;
  uint64_t get_best_ask() const;

  // Get order book depth (numbers of bids and asks)
  std::pair<size_t, size_t> get_depth() const;

  std::string_view get_symbol() const;

  // Print the order book, for debugging
  void print_book(size_t depth =5) const;

};

} // namespace hft
