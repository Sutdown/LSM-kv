#include <benchmark/benchmark.h>
#include <memory>
#include <string>
#include "../db/options.h"
#include "../db/db.h"
#include "../db/db_impl.h"

static void BM_DBInsert(benchmark::State& state) {
    auto test_options = MakeOptionsForDebugging();
    auto db_holder = std::make_unique<DB>(test_options);
    WriteOptions wOp;

    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            std::string key = "key_" + std::to_string(i);
            std::string value = "val_" + std::to_string(i);
            db_holder->Put(wOp, key, value);
        }
    }
}
BENCHMARK(BM_DBInsert)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();