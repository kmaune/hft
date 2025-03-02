#include "gtest/gtest.h"
#include "order_book.hpp"

TEST(OrderBookTest, AddOrder) {
    hft::OrderPool pool(100);
    hft::OrderBook book("AAPL", pool);

    EXPECT_TRUE(book.add_order(1, 100'00, 10, 1, hft::Side::Buy));
    EXPECT_FALSE(book.add_order(1, 100'00, 10, 1, hft::Side::Buy)); // Duplicate ID
}

TEST(OrderBookTest, CancelOrder) {
    hft::OrderPool pool(100);
    hft::OrderBook book("AAPL", pool);

    book.add_order(1, 100'00, 10, 1, hft::Side::Buy);
    EXPECT_TRUE(book.cancel_order(1));
    EXPECT_FALSE(book.cancel_order(2)); // Non-existent order
}
