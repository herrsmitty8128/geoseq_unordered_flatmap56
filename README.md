# geoseq_unordered_flatmap56

*geoseq_unordered_flatmap56* is a hash table that uses linear probing based on a geometric sequence of numbers. Its lookup times are faster than the [bytell](https://github.com/skarupke/flat_hash_map) hash table by [skarupke](https://github.com/skarupke). However, its key size is limited to 56 bits.

## License

*geoseq_unordered_flatmap56* is licensed uner the Boost Software License - Version 1.0 - August 17th, 2003.

## Building and running the files

The included makefile will build two executables. The first is called *geoseq_test*, which is used for testing and debugging. The second is named geoseq_benchmark, which performs a benchmark against bytell. [Google Benchmark](https://github.com/google/benchmark) is required to compile, build and run *geoseq_benchmark*.

    $ cd geoseq_unordered_flatmap56
    $ make
    $ make clean
    $ ./geoseq_test
    $ ./geoseq_benchmark

You can also tell Google Benchmark to output its data in CSV format with the following command:

    $ ./geoseq_benchmark --benchmark_format=csv > test_results.csv

## Performance

![Average lookup times](./images/lookup_chart.jpg)

![Average insertion/construction times](./images/insert_chart.jpg)