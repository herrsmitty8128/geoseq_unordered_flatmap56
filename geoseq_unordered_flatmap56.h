
//          Copyright Christopher Smith 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOSEQ_UNORDERED_FLAT_MAP_56_C_
#define _GEOSEQ_UNORDERED_FLAT_MAP_56_C_

#ifndef __cplusplus
#include <stdint.h>
#include <stdbool.h>
#endif

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif

typedef struct UNORDERED_FLATMAP56 flatmap56_t;

/**
 * @brief Allocates and initializes a flatmap56_t object.
 * 
 * @param initial_capacity The minimum initial capacity of the table. This value is rounded up to
 *  the nearest power of 2 in the range max_bucket_count() to max_bucket_count().
 * @return flatmap56_t* on success or NULL on failure.
 */
flatmap56_t* flatmap56_create(const uint64_t initial_capacity);

/**
 * @brief Deallocates the instance of a flatmap56_t object pointed to by map.
 * 
 * @param map A pointer to the flatmap56_t object to deallocate.
 */
void flatmap56_destroy(flatmap56_t* map);

/**
 * @brief 
 * 
 * @param map 
 * @return float 
 */
float flatmap56_load_factor(const flatmap56_t* map);

/**
 * @brief Returns the current number of buckets in the hash table.
 * 
 * @param map A pointer to the map.
 * @return uint64_t
 */
uint64_t flatmap56_bucket_count(const flatmap56_t* map);

/**
 * @brief Returns the maximum number of buckets (table size) supported by this implementation.
 * 
 * @return uint64_t 
 */
uint64_t flatmap56_max_bucket_count();

/**
 * @brief Returns the minimum number of buckets (table size) supported by this implementation.
 * 
 * @return uint64_t 
 */
uint64_t flatmap56_min_bucket_count();

/**
 * @brief Returns the number of elements in the table.
 * 
 * @param map A pointer to the flatmap56_t object.
 * @return uint64_t 
 */
uint64_t flatmap56_size(const flatmap56_t* map);

/**
 * @brief Attempts to find the bucket in the hash table that is associated with key. Returns the corresponding value if successful, otherwise zero is returned upon failure.
 * 
 * @param map A pointer to the flatmap56_t object.
 * @param key The key to lookup.
 * @return uint64_t 
 */
uint64_t flatmap56_lookup(const flatmap56_t* map, const uint64_t key);

/**
 * @brief Removes the key-value pair associated with key and returns its corresponding value.
 * 
 * @param map A pointer to the flatmap56_t object.
 * @param key The key to lookup.
 * @return uint64_t 
 */
uint64_t flatmap56_remove(flatmap56_t* map, const uint64_t key);

/**
 * @brief Adds a new key-value pair to the hash table. If the hash table already contains the key, 
 * then the current value is replaced with the new value.
 * 
 * @param map The flatmap56_t object to insert the key-value pair into.
 * @param key The key.
 * @param value The value.
 * @return true on success
 * @return false on failure
 */
bool flatmap56_insert(flatmap56_t* map, const uint64_t key, const uint64_t value);

#ifdef __cplusplus
};
#endif

#endif