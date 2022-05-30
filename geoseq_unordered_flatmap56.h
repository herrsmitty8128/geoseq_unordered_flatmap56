
//          Copyright Christopher Smith 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOSEQ_UNORDERED_FLAT_MAP_56_C_
#define _GEOSEQ_UNORDERED_FLAT_MAP_56_C_

#ifndef __cplusplus
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#endif

#ifdef __cplusplus
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdint>
extern "C" {
#endif

typedef struct UNORDERED_FLAT_MAP56 UnorderedFlatMap56;
UnorderedFlatMap56* create(const uint64_t initial_capacity);
void destroy(UnorderedFlatMap56* map);
float load_factor(const UnorderedFlatMap56* map);
uint64_t bucket_count(const UnorderedFlatMap56* map);
uint64_t max_bucket_count();
uint64_t size(const UnorderedFlatMap56* map);
uint64_t lookup(const UnorderedFlatMap56* map, const uint64_t key);
uint64_t extract(UnorderedFlatMap56* map, const uint64_t key);
bool insert(UnorderedFlatMap56* map, const uint64_t key, const uint64_t value);

#ifdef __cplusplus
};
#endif

#endif