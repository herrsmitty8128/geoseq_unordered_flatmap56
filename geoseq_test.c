
//          Copyright Christopher Smith 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "geoseq_unordered_flatmap56.h"

#define SAMPLE_SIZE 10000
int samples[SAMPLE_SIZE];

int main(){

    int i,j,buff,*value;
    int r = EXIT_SUCCESS;
    flatmap56_t* map = flatmap56_create(0, sizeof(int));

    srand(time(0));
    //srand(0);

    fprintf(stdout, "\n               Buckets\tCount\tLoad Factor\n");
    fprintf(stdout, "               -------\t-----\t-----------");

    for(int n = 1; n <= 3; n++){

        fprintf(stdout, "\nIteration #%d:\n",n);
        fprintf(stdout, "-------------\n");

        for(i = 0; i < SAMPLE_SIZE; i++){
            do{
                samples[i] = rand();
                for(j = 0; samples[j] != samples[i]; j++);
            }while(j < i);
        }

        fprintf(stdout, "Initial State: %ld\t%ld,\t%f\n", flatmap56_bucket_count(map), flatmap56_size(map), flatmap56_load_factor(map));
        
        for(i = 0; i < SAMPLE_SIZE; i++){
            value = flatmap56_insert(map, samples[i]);
            if(!value){
                fprintf(stderr, "Insertion failed [%d] %d\n", i, samples[i]);
                r = EXIT_FAILURE;
                goto end_test;
            }
            *value = samples[i];
            if(flatmap56_size(map) != (uint64_t)i+1){
                fprintf(stderr, "Size is incorrect [%d] [%ld]\n", i+1, flatmap56_size(map));
                r = EXIT_FAILURE;
                goto end_test;
            }
            for(j = 0; j < i; j++){
                value = flatmap56_lookup(map, samples[j]);
                if(!value){
                    fprintf(stderr, "Lookup failed [%d] [%d] %d\n", i, j, samples[j]);
                    goto end_test;
                }
                if(*value != samples[j]){
                    fprintf(stderr, "Lookup value value not correct [%d] %d [%d] %d\n", i, *value, j, samples[j]);
                    goto end_test;
                }
            }
        }

        fprintf(stdout, "After Insert:  %ld,\t%ld,\t%f\n", flatmap56_bucket_count(map), flatmap56_size(map), flatmap56_load_factor(map));
        
        for(i = 0; i < SAMPLE_SIZE; i++){
            if(!flatmap56_remove(map, samples[i], &buff)){
                fprintf(stderr, "Removal failed [%d] %d\n", i, samples[i]);
                r = EXIT_FAILURE;
                goto end_test;
            }
            if(buff != samples[i]){
                fprintf(stderr, "Removal value incorrect %d [%d] %d\n", buff, i, samples[i]);
                r = EXIT_FAILURE;
                goto end_test;
            }
            if(flatmap56_size(map) != (uint64_t)(SAMPLE_SIZE - (i+1))){
                fprintf(stderr, "Size incorrect after removal %d [%d] %d\n", buff, i, samples[i]);
                r = EXIT_FAILURE;
                goto end_test;
            }
            for(j = i + 1; j < SAMPLE_SIZE; j++){
                value = flatmap56_lookup(map,samples[j]);
                if(!value){
                    fprintf(stderr, "Lookup failed after removal [%d] %d\n", j, samples[j]);
                    r = EXIT_FAILURE;
                    goto end_test;
                }
                if(*value != samples[j]){
                    fprintf(stderr, "Lookup value incorrect after removal %d [%d] %d\n", *value, j, samples[j]);
                    r = EXIT_FAILURE;
                    goto end_test;
                }
            }
        }

        fprintf(stdout, "After Remove:  %ld,\t%ld,\t%f\n", flatmap56_bucket_count(map), flatmap56_size(map), flatmap56_load_factor(map));

    }
    
    end_test:

    fprintf(stdout, "\nEnd State:     %ld,\t%ld,\t%f\n\n", flatmap56_bucket_count(map), flatmap56_size(map), flatmap56_load_factor(map));

    flatmap56_destroy(map);
    
    return r;
}