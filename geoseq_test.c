
//          Copyright Christopher Smith 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <stdio.h>
#include <time.h>
#include "geoseq_unordered_flatmap56.h"

#define SAMPLE_SIZE 10000
uint64_t samples[SAMPLE_SIZE];

int main(){

    srand(time(0));
    for(int i = 0; i < SAMPLE_SIZE; i++){
        samples[i] = rand();
    }

    UnorderedFlatMap56* map = create(0);

    for(uint64_t i = 0; i < SAMPLE_SIZE; i++){
        if(!insert(map, samples[i], i)){
            printf("Failed to insert [%ld] %ld\n", i, samples[i]);
            break;
        }
        if(size(map) != (uint64_t)(i+1)){
            printf("num_entries incorrect [%ld] [%ld]\n", i+1, size(map));
            break;
        }
        for(uint64_t j = 0; j < i; j++){
            if(lookup(map,samples[j]) != j){
                printf("Failed to lookup [%ld] [%ld] %ld\n", i, j, samples[j]);
                goto end_test;
            }
        }
    }

    end_test:

    printf("buckets = %ld, size = %ld, load factor = %f\n", bucket_count(map), size(map), load_factor(map));

    destroy(map);
    return EXIT_SUCCESS;
}