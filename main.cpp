#include "order.hpp"
#include "order_pool.hpp"
#include "price_level.hpp"
#include "order_book.hpp"
#include "order_book_manager.hpp"
#include <iostream>

int main() {
  hft::OrderBookManager manager;

  auto* book = manager.get_order_book("AAPL");

    // Add some limit orders
    manager.process_order("AAPL", 1, 150'00, 100, 1, hft::OrderType::Limit, hft::Side::Buy);
    manager.process_order("AAPL", 2, 151'00, 50, 2, hft::OrderType::Limit, hft::Side::Buy);
    manager.process_order("AAPL", 3, 152'00, 200, 3, hft::OrderType::Limit, hft::Side::Sell);
    manager.process_order("AAPL", 4, 153'00, 100, 4, hft::OrderType::Limit, hft::Side::Sell);
    
    // Print the order book
    book->print_book();
    
    // Process a market order
    std::cout << "\nProcessing market order to buy 75 shares\n";
    manager.process_order("AAPL", 0, 0, 75, 5, hft::OrderType::Market, hft::Side::Buy);
    
    // Print the updated order book
    book->print_book();
    
    // Cancel an order
    std::cout << "\nCancelling order #2\n";
    manager.process_order("AAPL", 2, 0, 0, 0, hft::OrderType::Cancel, hft::Side::Buy);
    
    // Print the final order book
    book->print_book();

  return 0;
}
