
#include <benchmark/benchmark.h>
#include "ska/bytell_hash_map.hpp"
#include "geoseq_unordered_flatmap56.h"


#define MAX_COUNT 10000000
uint64_t myarray[MAX_COUNT];


static void geoseq_UnorderedFlatMap56_insert(benchmark::State& state) {
    size_t range = state.range(0);
    UnorderedFlatMap56* map = create(0);
    for (auto _ : state){
        for(size_t i = 0; i < range; i++){
            insert(map, myarray[i], i);
        }
    }
    state.counters["load_factor"] = load_factor(map);
    state.counters["ns_per_entry"] = benchmark::Counter((double)(range * state.iterations()) / (double)1000000000.0, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    destroy(map);
}

BENCHMARK(geoseq_UnorderedFlatMap56_insert)->Name("geoseq_UnorderedFlatMap56_insert")->DenseRange(10000, MAX_COUNT, 25000)->Unit(benchmark::kNanosecond);


static void geoseq_UnorderedFlatMap56_lookup(benchmark::State& state) {
    size_t range = state.range(0);
    UnorderedFlatMap56* map = create(0);
    for(size_t i = 0; i < range; i++) insert(map, myarray[i], i);
    for (auto _ : state){
        for(size_t i = 0; i < range; i++)
            benchmark::DoNotOptimize(lookup(map, myarray[i]));
    }
    state.counters["load_factor"] = load_factor(map);
    state.counters["ns_per_entry"] = benchmark::Counter((double)(range * state.iterations()) / (double)1000000000.0, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    destroy(map);
}

BENCHMARK(geoseq_UnorderedFlatMap56_lookup)->Name("geoseq_UnorderedFlatMap56_lookup")->DenseRange(10000, MAX_COUNT, 25000)->Unit(benchmark::kNanosecond);



static void ska_bytell_hash_map_lookup(benchmark::State& state) {
    size_t range = state.range(0);
    ska::bytell_hash_map<uint64_t,uint64_t> map;
    for(size_t i = 0; i < range; i++) map[myarray[i]] = i;
    for (auto _ : state){
        for(size_t i = 0; i < range; i++)
            //map.find(myarray[i]);
            map.at(myarray[i]);
    }
    state.counters["load_factor"] = benchmark::Counter(map.load_factor());
    state.counters["ns_per_entry"] = benchmark::Counter((double)(range * state.iterations()) / (double)1000000000.0, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}

BENCHMARK(ska_bytell_hash_map_lookup)->Name("ska_bytell_hash_map_lookup")->DenseRange(10000, MAX_COUNT, 25000)->Unit(benchmark::kNanosecond);



static void ska_bytell_hash_map_insert(benchmark::State& state) {
    size_t range = state.range(0);
    ska::bytell_hash_map<uint64_t,uint64_t> map;
    for (auto _ : state){
        for(size_t i = 0; i < range; i++) map[myarray[i]] = i;
    }
    state.counters["load_factor"] = benchmark::Counter(map.load_factor());
    state.counters["ns_per_entry"] = benchmark::Counter((double)(range * state.iterations()) / (double)1000000000.0, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}

BENCHMARK(ska_bytell_hash_map_insert)->Name("ska_bytell_hash_map_insert")->DenseRange(10000, MAX_COUNT, 25000)->Unit(benchmark::kNanosecond);




//BENCHMARK_MAIN();
int main(int argc, char** argv) {
    // other initialization code goes here
    srand(time(0));
    for(size_t i = 0; i < MAX_COUNT; i++) myarray[i] = rand();

    ::benchmark::Initialize(&argc, argv); 
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1; 
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}