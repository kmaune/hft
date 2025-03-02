#include "order_book.hpp"
#include "order_pool.hpp"
#include "gtest/gtest.h"

namespace hft {

class SimpleOrderBookTest : public ::testing::Test {
protected:
    OrderPool pool;
    OrderBook book;

    SimpleOrderBookTest() : book("AAPL", pool) {}
};

TEST_F(SimpleOrderBookTest, AddOrder) {
    EXPECT_TRUE(book.add_order(1, 150'00, 100, 1, Side::Buy));
    EXPECT_TRUE(book.add_order(2, 151'00, 50, 2, Side::Buy));
    EXPECT_TRUE(book.add_order(3, 152'00, 200, 3, Side::Sell));

    EXPECT_EQ(book.get_depth().first, 2); // 2 buy levels
    EXPECT_EQ(book.get_depth().second, 1); // 1 sell level
}

TEST_F(SimpleOrderBookTest, CancelOrder) {
    EXPECT_EQ(book.get_depth().first, 0);
    book.add_order(1, 150'00, 100, 1, Side::Buy);
    EXPECT_EQ(book.get_depth().first, 1);
    EXPECT_TRUE(book.cancel_order(1));
    EXPECT_EQ(book.get_depth().first, 0); // No buy levels left
}

TEST_F(SimpleOrderBookTest, MarketOrder) {
    book.add_order(1, 150'00, 100, 1, Side::Buy);
    book.add_order(2, 151'00, 50, 2, Side::Buy);
    book.add_order(3, 152'00, 200, 3, Side::Sell);

    auto [filled, cost] = book.process_market_order(75, Side::Buy);
    EXPECT_EQ(filled, 75);
    EXPECT_EQ(cost, 75 * 152'00);
}

TEST_F(SimpleOrderBookTest, PriceLevelSorting) {
    book.add_order(1, 150'00, 100, 1, Side::Buy);
    book.add_order(2, 151'00, 50, 2, Side::Buy);
    book.add_order(3, 152'00, 200, 3, Side::Sell);
    book.add_order(4, 153'00, 100, 4, Side::Sell);

    EXPECT_EQ(book.get_best_bid(), 151'00); // Highest bid first
    EXPECT_EQ(book.get_best_ask(), 152'00); // Lowest ask first
}

TEST_F(SimpleOrderBookTest, OrderPool) {
    auto* order1 = pool.allocate(1, 150'00, 100, 1, Side::Buy, "AAPL");
    auto* order2 = pool.allocate(2, 151'00, 50, 2, Side::Buy, "AAPL");

    EXPECT_NE(order1, nullptr);
    EXPECT_NE(order2, nullptr);

    pool.deallocate(order1);
    pool.deallocate(order2);
}

} //namespace hft

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
