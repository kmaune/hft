#include "order_book.hpp"
#include "order_pool.hpp"
#include <benchmark/benchmark.h>

static void BM_AddOrder(benchmark::State& state) {
    hft::OrderPool pool;
    hft::OrderBook book("AAPL", pool);

    for (auto _ : state) {
        book.add_order(state.range(0), 150'00, 100, 1, hft::Side::Buy);
    }
}
BENCHMARK(BM_AddOrder)->Arg(1)->Arg(1000);

BENCHMARK_MAIN();
