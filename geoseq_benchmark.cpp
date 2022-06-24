
//          Copyright Christopher Smith 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <benchmark/benchmark.h>
#include "ska/bytell_hash_map.hpp"
#include "geoseq_unordered_flatmap56.h"


#define MAX_COUNT 5000000
int myarray[MAX_COUNT];


static void geoseq_flatmap56_insert(benchmark::State& state) {
    size_t range = state.range(0);
    int *value;
    flatmap56_t* map = flatmap56_create(0,sizeof(int));
    for (auto _ : state){
        for(size_t i = 0; i < range; i++){
            value = (int*)flatmap56_insert(map, myarray[i]);
            *value = myarray[i];
        }
    }
    state.counters["load_factor"] = flatmap56_load_factor(map);
    state.counters["ns_per_entry"] = benchmark::Counter((double)(range * state.iterations()) / (double)1000000000.0, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    flatmap56_destroy(map);
}

BENCHMARK(geoseq_flatmap56_insert)->Name("geoseq_flatmap56_insert")->DenseRange(10000, MAX_COUNT, 25000)->Unit(benchmark::kNanosecond);


static void geoseq_flatmap56_lookup(benchmark::State& state) {
    size_t range = state.range(0);
    int *value;
    flatmap56_t* map = flatmap56_create(0,sizeof(int));
    for(size_t i = 0; i < range; i++){
        value = (int*)flatmap56_insert(map, myarray[i]);
        *value = myarray[i];
    }
    for (auto _ : state){
        for(size_t i = 0; i < range; i++)
            benchmark::DoNotOptimize(flatmap56_lookup(map, myarray[i]));
    }
    state.counters["load_factor"] = flatmap56_load_factor(map);
    state.counters["ns_per_entry"] = benchmark::Counter((double)(range * state.iterations()) / (double)1000000000.0, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    flatmap56_destroy(map);
}

BENCHMARK(geoseq_flatmap56_lookup)->Name("geoseq_flatmap56_lookup")->DenseRange(10000, MAX_COUNT, 25000)->Unit(benchmark::kNanosecond);



static void geoseq_flatmap56_remove(benchmark::State& state) {
    size_t range = state.range(0);
    int removed_value,*value;
    flatmap56_t* map = flatmap56_create(0,sizeof(int));
    for(size_t i = 0; i < range; i++){
        value = (int*)flatmap56_insert(map, myarray[i]);
        *value = myarray[i];
    }
    state.counters["load_factor"] = flatmap56_load_factor(map);
    for (auto _ : state){
        for(size_t i = 0; i < range; i++)
            benchmark::DoNotOptimize(flatmap56_remove(map, myarray[i], &removed_value));
    }
    state.counters["ns_per_entry"] = benchmark::Counter((double)(range * state.iterations()) / (double)1000000000.0, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    flatmap56_destroy(map);
}

BENCHMARK(geoseq_flatmap56_remove)->Name("geoseq_flatmap56_remove")->DenseRange(10000, MAX_COUNT, 25000)->Unit(benchmark::kNanosecond);



static void ska_bytell_lookup(benchmark::State& state) {
    size_t range = state.range(0);
    ska::bytell_hash_map<int,int> map;
    for(size_t i = 0; i < range; i++) map[myarray[i]] = i;
    for (auto _ : state){
        for(size_t i = 0; i < range; i++)
            benchmark::DoNotOptimize(map.at(myarray[i]));
    }
    state.counters["load_factor"] = benchmark::Counter(map.load_factor());
    state.counters["ns_per_entry"] = benchmark::Counter((double)(range * state.iterations()) / (double)1000000000.0, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}

BENCHMARK(ska_bytell_lookup)->Name("ska_bytell_lookup")->DenseRange(10000, MAX_COUNT, 25000)->Unit(benchmark::kNanosecond);


static void ska_bytell_insert(benchmark::State& state) {
    size_t range = state.range(0);
    ska::bytell_hash_map<int,int> map;
    for (auto _ : state){
        for(size_t i = 0; i < range; i++) map[myarray[i]] = i;
    }
    state.counters["load_factor"] = benchmark::Counter(map.load_factor());
    state.counters["ns_per_entry"] = benchmark::Counter((double)(range * state.iterations()) / (double)1000000000.0, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}

BENCHMARK(ska_bytell_insert)->Name("ska_bytell_insert")->DenseRange(10000, MAX_COUNT, 25000)->Unit(benchmark::kNanosecond);


static void ska_bytell_remove(benchmark::State& state) {
    size_t range = state.range(0);
    ska::bytell_hash_map<int,int> map;
    for(size_t i = 0; i < range; i++) map[myarray[i]] = i;
    state.counters["load_factor"] = benchmark::Counter(map.load_factor());
    for (auto _ : state){
        for(size_t i = 0; i < range; i++)
            benchmark::DoNotOptimize(map.erase(myarray[i]));
    }
    state.counters["ns_per_entry"] = benchmark::Counter((double)(range * state.iterations()) / (double)1000000000.0, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
}

BENCHMARK(ska_bytell_remove)->Name("ska_bytell_remove")->DenseRange(10000, MAX_COUNT, 25000)->Unit(benchmark::kNanosecond);



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