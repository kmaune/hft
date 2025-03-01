#include "order.hpp"
#include "order_pool.hpp"
#include "price_level.hpp"
#include "order_book.hpp"
#include "order_book_manager.hpp"

int main() {
  hft::Order order;
  hft::OrderPool order_pool;
  hft::PriceLevel price_level(100);
  hft::OrderBook aapl_book("AAPL", order_pool);
  hft::OrderBookManager manager;

  return 0;
}
