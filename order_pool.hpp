#pragma once

#include "enums.hpp"
#include "order.hpp"
#include <memory>
#include <mutex>
#include <vector>

namespace hft {

// Memory pool for efficient order allocation
class OrderPool {
private:
  std::vector<std::unique_ptr<Order>> orders_;
  std::vector<Order *> free_orders_;
  std::mutex mutex_;

public:
  OrderPool(size_t initial_size = 10000);

  Order *allocate(uint64_t id, uint64_t price, uint32_t quantity,
                  uint32_t timestamp, Side side, std::string_view symbol);

  void deallocate(Order *order);
};

} // namespace hft
