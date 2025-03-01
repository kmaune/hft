#include "order.hpp"
#include "order_pool.hpp"
#include "price_level.hpp"
#include "order_book.hpp"

int main() {
  hft::Order order;
  hft::OrderPool order_pool;
  hft::PriceLevel price_level(100);
  hft::OrderBook aapl_book("AAPL", order_pool);

  return 0;
}
