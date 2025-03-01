#include "order_book.hpp"
#include "order.hpp"
#include <iostream>
#include <limits>
#include <shared_mutex>

namespace hft {

OrderBook::OrderBook(std::string symbol, OrderPool &order_pool)
    : symbol_(std::move(symbol)), order_pool_(order_pool) {}

OrderBook::~OrderBook() {
  // Clean up price levels
  for (size_t i = 0; i < buy_level_count_; ++i) {
    delete buy_levels_[i];
  }

  for (size_t i = 0; i < sell_level_count_; ++i) {
    delete sell_levels_[i];
  }
}

PriceLevel *OrderBook::add_price_level(Side side, uint64_t price) {
  auto level = new PriceLevel(price);

  if (side == Side::Buy) {
    // keep buy-levels sorted in descending order (best bid first)
    size_t i = 0;
    while (i < buy_level_count_ && buy_levels_[i]->price() > price) {
      ++i;
    }

    if (i < buy_level_count_) {
      // Shift levels to make room
      for (size_t j = buy_level_count_; j > i; --j) {
        buy_levels_[j] = buy_levels_[j - 1];
      }
    }

    buy_levels_[i] = level;
    ++buy_level_count_;
  } else {
    // Sell
    // Keep sell levels sorted in ascending order (best ask first)
    size_t i = 0;
    while (i < sell_level_count_ && sell_levels_[i]->price() < price) {
      ++i;
    }

    if (i < sell_level_count_) {
      // Shift levels to make room
      for (size_t j = sell_level_count_; j > i; --j) {
        sell_levels_[j] = sell_levels_[j - 1];
      }
    }

    sell_levels_[i] = level;
    ++sell_level_count_;
  }

  return level;
}

PriceLevel *OrderBook::find_price_level(Side side, uint64_t price) {
  if (side == Side::Buy) {
    for (size_t i = 0; i < buy_level_count_; ++i) {
      if (buy_levels_[i]->price() == price) {
        return buy_levels_[i];
      }
    }
  } else {
    for (size_t i = 0; i < sell_level_count_; ++i) {
      if (sell_levels_[i]->price() == price) {
        return sell_levels_[i];
      }
    }
  }
  return nullptr;
}

void OrderBook::remove_price_level(Side side, uint64_t price) {
  if (side == Side::Buy) {
    for (size_t i = 0; i < buy_level_count_; ++i) {
      if (buy_levels_[i]->price() == price) {
        delete buy_levels_[i];

        // Shift levels to close the gap
        for (size_t j = i; j < buy_level_count_ - 1; ++j) {
          buy_levels_[j] = buy_levels_[j + 1];
        }

        --buy_level_count_;
        break;
      }
    }
  } else {
    for (size_t i = 0; i < sell_level_count_; ++i) {
      if (sell_levels_[i]->price() == price) {
        delete sell_levels_[i];

        // Shift levels to close the gap
        for (size_t j = i; j < sell_level_count_ - 1; ++j) {
          sell_levels_[j] = sell_levels_[j + 1];
        }

        --sell_level_count_;
        break;
      }
    }
  }
}

bool OrderBook::add_order(uint64_t id, uint64_t price, uint32_t quantity,
                          uint32_t timestamp, Side side) {
  std::unique_lock lock(mutex_);

  // Check if order alread exists
  if (order_map_.find(id) != order_map_.end()) {
    return false;
  }

  // Allocate new order
  Order *order =
      order_pool_.allocate(id, price, quantity, timestamp, side, symbol_);

  // Add order to map for fast lookup
  order_map_[id] = order;

  // Find or create the price level
  PriceLevel *level = find_price_level(side, price);
  if (!level) {
    if ((side == Side::Buy && buy_level_count_ > MAX_PRICE_LEVELS) ||
        (side == Side::Sell && sell_level_count_ > MAX_PRICE_LEVELS)) {
      // "Reject" order
      order_map_.erase(id);
      order_pool_.deallocate(order);
      return false;
    }
    level = add_price_level(side, price);
  }

  // Add order to the price level
  if (!level->add_order(order)) {
    // "Reject" order, price level full
    order_map_.erase(id);
    order_pool_.deallocate(order);
    return false;
  }

  // Try to match orders immediately
  return true;
}

bool OrderBook::cancel_order(uint64_t order_id) {
  std::unique_lock lock(mutex_);

  auto it = order_map_.find(order_id);
  if (it == order_map_.end()) {
    return false;
  }

  Order *order = it->second;
  PriceLevel *level = find_price_level(order->side, order->price);

  if (!level) {
    return false;
  }

  if (level->remove_order(order_id)) {
    // If price level is now empty, remove it
    if (level->order_count()) {
      remove_price_level(order->side, order->price);
    }

    // Return order to the order pool
    order_pool_.deallocate(order);
    order_map_.erase(it);
    return true;
  }
  return false;
}

std::pair<uint32_t, uint64_t> OrderBook::process_market_order(uint32_t quantity,
                                                              Side side) {
  std::unique_lock lock(mutex_);

  uint32_t filled_quantity = 0;
  uint64_t total_cost = 0;

  while (quantity > 0) {
    // Get best price level on the opposite side
    PriceLevel *level = nullptr;

    if (side == Side::Buy && sell_level_count_ > 0) {
      level = sell_levels_[0]; // Best ask
    } else if (side == Side::Sell && buy_level_count_ > 0) {
      level = buy_levels_[0]; // Best bid
    }

    if (!level) {
      break; // No matching orders
    }

    // Match with orders at this level
    for (size_t i = 0; i < level->order_count() && quantity > 0; ++i) {
      Order *order = level->get_order(i);
      if (!order)
        continue;

      uint32_t match_quantity = std::min(quantity, order->quantity);
      filled_quantity += match_quantity;
      total_cost += match_quantity * level->price();

      // Update matched order
      order->quantity -= match_quantity;
      quantity -= match_quantity;

      // Record last trade
      last_trade_price_ = level->price();
      last_trade_quantity_ = match_quantity;

      if (order->quantity == 0) {
        // Remove fully matched order
        cancel_order(order->id);
        --i; // Adjust for removed order
      }

      if (quantity == 0) {
        break;
      }
    }
    // If the level is empty after matching, it will be removed in cancel_order
  }
  return {filled_quantity, total_cost};
}

void OrderBook::match_orders() {
  // While there are buy and sell orders that can match
  while (buy_level_count_ > 0 && sell_level_count_ > 0) {
    PriceLevel *best_bid = buy_levels_[0];
    PriceLevel *best_ask = sell_levels_[0];

    if (best_bid->price() >= best_ask->price()) {
      // Orders can match -- get OLDEST order from each side
      Order *buy_order = best_bid->get_order(0);
      Order *sell_order = best_ask->get_order(0);

      if (!buy_order || !sell_order) {
        break;
      }

      // Match the orders
      uint32_t match_quantity =
          std::min(buy_order->quantity, sell_order->quantity);

      // Record the trade
      last_trade_price_ = (best_bid->price() + best_ask->price()) /
                          2; // TODO: confirm this is correct
      last_trade_quantity_ = match_quantity;

      // Update the orders
      buy_order->quantity -= match_quantity;
      sell_order->quantity -= match_quantity;

      // Remove orders that are fully filled
      if (buy_order->quantity == 0) {
        cancel_order(buy_order->id);
      }
      if (sell_order->quantity == 0) {
        cancel_order(sell_order->id);
      }
    } else {
      break; // No more matches possible
    }
  }
}

uint64_t OrderBook::get_spread() const {
  std::shared_lock lock(mutex_);

  if (buy_level_count_ > 0 && sell_level_count_ > 0) {
    return sell_levels_[0]->price() - buy_levels_[0]->price();
  }
  return std::numeric_limits<uint64_t>::max();
}

uint64_t OrderBook::get_mid_price() const {
  std::shared_lock lock(mutex_);

  if (buy_level_count_ > 0 && sell_level_count_ > 0) {
    return (buy_levels_[0]->price() + sell_levels_[0]->price()) / 2;
  } else if (buy_level_count_ > 0) {
    return buy_levels_[0]->price();
  } else if (sell_level_count_ > 0) {
    return sell_levels_[0]->price();
  }
  return 0;
}

uint64_t OrderBook::get_best_bid() const {
  std::shared_lock lock(mutex_);

  if (buy_level_count_ > 0) {
    return buy_levels_[0]->price();
  }
  return 0;
}

uint64_t OrderBook::get_best_ask() const {
  std::shared_lock lock(mutex_);

  if (sell_level_count_ > 0) {
    return sell_levels_[0]->price();
  }
  return std::numeric_limits<uint64_t>::max();
}

std::pair<size_t, size_t> OrderBook::get_depth() const {
  std::shared_lock lock(mutex_);
  return {buy_level_count_, sell_level_count_};
}

std::string_view OrderBook::get_symbol() const { return symbol_; }

void OrderBook::print_book(size_t depth) const {
    std::shared_lock lock(mutex_);
    
    std::cout << "\nOrder Book for " << symbol_ << "\n";
    std::cout << "================================\n";
    
    // Print sells (in reverse order, highest first)
    for (int i = std::min(sell_level_count_, depth) - 1; i >= 0; --i) {
        std::cout << "SELL " << sell_levels_[i]->price() 
                  << " x " << sell_levels_[i]->total_quantity() 
                  << " (" << sell_levels_[i]->order_count() << " orders)\n";
    }
    
    std::cout << "--------------------------------\n";
    
    // Print buys
    for (size_t i = 0; i < std::min(buy_level_count_, depth); ++i) {
        std::cout << "BUY  " << buy_levels_[i]->price() 
                  << " x " << buy_levels_[i]->total_quantity() 
                  << " (" << buy_levels_[i]->order_count() << " orders)\n";
    }
    
    std::cout << "================================\n";
    std::cout << "Spread: " << get_spread() << " | Mid Price: " << get_mid_price() << "\n";
    std::cout << "Last Trade: " << last_trade_price_ << " x " << last_trade_quantity_ << "\n";
}

} // namespace hft
