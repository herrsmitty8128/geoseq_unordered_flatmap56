# geoseq_unordered_flatmap56

*geoseq_unordered_flatmap56* is a hash table that uses linear probing based on a geometric sequence of numbers. Its lookup times are faster than the [bytell](https://github.com/skarupke/flat_hash_map) hash table by [skarupke](https://github.com/skarupke). However, its key size is limited to 56 bits.

|Function|Description|
|--------|-----------|
|FlatMap56* FlatMap56_create(const uint64_t initial_capacity);|Allocates and initializes a new instance of a FlatMap56 object on the heap. Returns a pointer to the new object or NULL upon failure.|
|void FlatMap56_destroy(FlatMap56* map);|Deallocates the instance of a FlatMap56 object pointed to by *map*.|
|float FlatMap56_load_factor(const FlatMap56* map);|Calculates and returns the current hash table load factor.|
|uint64_t FlatMap56_bucket_count(const FlatMap56* map);|Returns the total number of buckets currently in the hash table.|
|uint64_t FlatMap56_max_bucket_count();|Returns the maximum capacity of the hash table.|
|uint64_t FlatMap56_size(const FlatMap56* map);|Returns the current number of elements in the hash table.|
|uint64_t FlatMap56_lookup(const FlatMap56* map, const uint64_t key);|Attempts to find the bucket in the hash table that is associated with *key*. Returns the corresponding *value* if successful, otherwise zero is returned upon failure.|
|uint64_t FlatMap56_remove(FlatMap56* map, const uint64_t key);|Removes the key-value pair associated with *key* and returns its corresponding *value*.|
|bool FlatMap56_insert(FlatMap56* map, const uint64_t key, const uint64_t value);|Adds a new key-value pair to the hash table. If the hash table already contains the key, then the current value is replaced with the new value. Returns true on success or false on failure.|

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