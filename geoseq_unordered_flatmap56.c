
//          Copyright Christopher Smith 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "geoseq_unordered_flatmap56.h"

#define EMPLACE_EMPTY(BUCKET,KEY,NEXT,DIRECT) \
    BUCKET->unique_key = KEY; \
    BUCKET->next_probe = NEXT; \
    BUCKET->direct_hit = DIRECT
#define NO_MORE_PROBES      (MAX_PROBES-1)
#define EMPTY_SLOT          0
#define MIN(A,B)            ((A) < (B) ? (A) : (B))
#define MAX(A,B)            ((A) > (B) ? (A) : (B))
#define CALC_INDEX(MAP,H,P) ((H + (MAP)->probes[P]) & (MAP)->table_mask)
#define HASH(MAP,KEY)       (((KEY) * 11400714819323198103ul) >> (MAP)->hash_shift)
#define BUCKET(MAP,INDEX)   ((bucket_t*)(&(MAP)->buckets[(INDEX) * (MAP)->bucket_size]))
#define NUM_COMMON_RATIOS   58
const float common_ratios[NUM_COMMON_RATIOS] = {
    1.007936, 1.017045, 1.02521 , 1.032786, 1.04    , 1.047058, 1.053763, 1.060397,
    1.067061, 1.073619, 1.080204, 1.086687, 1.093513, 1.1     , 1.106626, 1.112655,
    1.119395, 1.125382, 1.132877, 1.139668, 1.145878, 1.152606, 1.158852, 1.166347,
    1.172321, 1.179586, 1.186137, 1.193436, 1.2     , 1.206733, 1.21412 , 1.221013,
    1.227545, 1.234712, 1.242165, 1.25    , 1.25523 , 1.262626, 1.269855, 1.276308,
    1.283932, 1.291902, 1.299366, 1.306582, 1.314386, 1.32242 , 1.330434, 1.335375,
    1.343129, 1.350928, 1.358213, 1.366065, 1.374269, 1.382625, 1.390555, 1.398671,
    1.404869, 1.412407
};

static inline uint64_t flatmap56_restrict(const uint64_t n, const uint64_t min, const uint64_t max){
    return MIN(MAX(n, min), max);
}

static inline bool flatmap56_initialize(flatmap56_t* map, uint64_t capacity, const uint64_t value_size) {
    // determine how many bits we need for the requested capacity
    capacity = flatmap56_restrict(capacity, flatmap56_min_bucket_count(), flatmap56_max_bucket_count(map));
    unsigned int bits = (unsigned int)(ceil(log2(capacity)));
    // calculate the probes using a geometric sequence
    float growth_ratio = common_ratios[bits - 7];
    double p = 1.0f;
    map->probes[EMPTY_SLOT] = 0;
    for(size_t i = 1; i < NO_MORE_PROBES; i++){
        map->probes[i] = (uint64_t)p;
        p = ceil(p * growth_ratio);
    }
    map->probes[NO_MORE_PROBES] = 0;
    // initialize the member variables
    map->num_entries = 0;
    map->hash_shift = 64 - bits;
    map->num_buckets = 1ul << bits;
    map->table_mask = map->num_buckets - 1;
    map->value_size = value_size;
    map->bucket_size = sizeof(bucket_t) + value_size;
    if(map->bucket_size & 7) map->bucket_size = ((map->bucket_size >> 3) << 3) + 8; //round up to nearest multiple of 8
    map->buckets = (uint8_t*)calloc(map->num_buckets, map->bucket_size); // try to allocate an array
    if(map->buckets == NULL) return false; // return NULL if calloc failed
    return true; // return to the caller
}

inline flatmap56_t* flatmap56_create(const uint64_t initial_capacity, const uint64_t value_size) {
    flatmap56_t* map = (flatmap56_t*)calloc(1, sizeof(flatmap56_t));
    if(map){
        if(!flatmap56_initialize(map, initial_capacity, value_size)){
            flatmap56_destroy(map);
            return NULL;
        }
    }
    return map;
}

inline void flatmap56_destroy(flatmap56_t* map) {
    if(map){
        if(map->buckets) free(map->buckets);
        free(map);
    }
}

inline float flatmap56_load_factor(const flatmap56_t* map) {
    return map->num_buckets == 0 ? 0.0f : (float)map->num_entries / (float)map->num_buckets;
}

inline uint64_t flatmap56_size(const flatmap56_t* map) {
    return map->num_entries;
}

inline uint64_t flatmap56_bucket_count(const flatmap56_t* map) {
    return map->num_buckets;
}

inline uint64_t flatmap56_max_bucket_count(const flatmap56_t* map) {
    uint64_t max_keys = 1ul << 56;
    uint64_t max_buckets = map->bucket_size > 0 ? UINT64_MAX / map->bucket_size : UINT64_MAX;
    return MIN(max_keys, max_buckets);
}

inline uint64_t flatmap56_min_bucket_count() {
    return MAX_PROBES;
}

inline void* flatmap56_lookup(const flatmap56_t* map, const uint64_t key) {
    uint64_t  h = HASH(map,key);
    bucket_t* b = BUCKET(map,h);
    if(b->unique_key == key) return &b->value[0];
    if(b->direct_hit){
        while(b->next_probe != NO_MORE_PROBES){
            b = BUCKET(map,CALC_INDEX(map,h,b->next_probe));
            if(b->unique_key == key) return &b->value[0];
        }
    }
    return NULL;
}

/*
static inline void* flatmap56_emplace_empty(flatmap56_t* map, bucket_t* b, const uint64_t key, const uint8_t next, const uint8_t direct){
    b->unique_key = key;
    b->next_probe = next;
    b->direct_hit = direct;
    map->num_entries++;
    return b->value;
}
*/

static inline void* flatmap56_emplace_direct(flatmap56_t* map, const uint64_t key, const uint64_t h){

    bucket_t* temp;
    bucket_t* e;
    bucket_t* empty = NULL;
    bucket_t* predecessor = NULL;
    uint8_t   x, y, z; 

    for(x = 0; x < NO_MORE_PROBES; x = z){
        temp = BUCKET(map, CALC_INDEX(map,h,x));
        if(temp->unique_key == key) return temp->value;
        z = temp->next_probe;
        if(!empty){
            for(y = x + 1; y < z; y++){
                e = BUCKET(map, CALC_INDEX(map,h,y));
                if(e->next_probe == EMPTY_SLOT){
                    predecessor = temp;
                    empty = e;
                    break;
                }
            }
        }
    }

    if(empty){
        //x = predecessor->next_probe;
        EMPLACE_EMPTY(empty, key, x, 0);
        predecessor->next_probe = y;
        map->num_entries++;
        return empty->value;
        //return flatmap56_emplace_empty(map,empty,key,x,0);
    }

    return NULL;
}

static inline void* flatmap56_emplace_indirect(flatmap56_t* map, const uint64_t key, bucket_t* b){

    bucket_t* e;
    bucket_t* temp;
    bucket_t* empty = NULL;
    bucket_t* predecessor = NULL;
    uint8_t   x, y, z;
    
    uint64_t h2 = HASH(map, b->unique_key);

    for(x = 0; x < NO_MORE_PROBES; x = z){
        
        temp = BUCKET(map,CALC_INDEX(map,h2,x));
        z = temp->next_probe;
        
        if(!predecessor){
            if(b == BUCKET(map,CALC_INDEX(map,h2,z))){
                predecessor = temp;
                z = b->next_probe;
                predecessor->next_probe = z;
            }
        }
                        
        if(!empty){
            for(y = x + 1; y < z; y++){
                e = BUCKET(map,CALC_INDEX(map,h2,y));
                if(e->next_probe == EMPTY_SLOT){
                    empty = e;
                    /*empty->direct_hit = 0;
                    empty->next_probe = z;
                    empty->unique_key = b->unique_key;*/
                    EMPLACE_EMPTY(empty,b->unique_key,z,0);
                    memcpy(empty->value, b->value, map->value_size);
                    temp->next_probe = y;
                    break;
                }
            }
        }

        if(predecessor && empty){
            //return flatmap56_emplace_empty(map,b,key,NO_MORE_PROBES,1);
            EMPLACE_EMPTY(b, key, NO_MORE_PROBES, 1);
            map->num_entries++;
            return b->value;
        }
    }

    return NULL;
}

static inline void* flatmap56_emplace(flatmap56_t* map, const uint64_t key) {
    uint64_t  h = HASH(map,key);
    bucket_t* b = BUCKET(map,h);
    if(b->next_probe == EMPTY_SLOT){
        //return flatmap56_emplace_empty(map,b,key,NO_MORE_PROBES,1);
        EMPLACE_EMPTY(b,key,NO_MORE_PROBES,1);
        map->num_entries++;
        return b->value;
    }
    if(b->direct_hit) return flatmap56_emplace_direct(map,key,h);
    return flatmap56_emplace_indirect(map,key,b);
}

static inline bool flatmap56_resize(flatmap56_t* map, int action){
    flatmap56_t old_map = *map;
    uint64_t new_capacity;
    if(action > 0) new_capacity = old_map.num_buckets * 2;
    else if(action < 0) new_capacity = old_map.num_buckets / 2;
    else new_capacity = old_map.num_buckets;
    if(!flatmap56_initialize(map, new_capacity, old_map.value_size)){
        *map = old_map;
        return false;
    }
    for(uint64_t i = 0; i < old_map.num_buckets; i++){
        bucket_t* b = BUCKET(&old_map,i);
        if(b->next_probe != EMPTY_SLOT){
            void* value = flatmap56_emplace(map, b->unique_key);
            if(!value){
                free(map->buckets);
                *map = old_map;
                return false;
            }
            memcpy(value, b->value, map->value_size);
        }
    }
    free(old_map.buckets);
    return true;
}

inline void* flatmap56_insert(flatmap56_t* map, const uint64_t key) {
    bucket_t* value = flatmap56_emplace(map,key);
    if(!value){
        if(flatmap56_resize(map,1)){
            value = flatmap56_emplace(map,key);
        }
    }
    return value;
}

inline bool flatmap56_remove(flatmap56_t* map, const uint64_t key, void* value) {
        
    uint64_t  h = HASH(map,key);
    bucket_t* b = BUCKET(map,h);
    bucket_t* b2 = NULL;
    //uint64_t  p;

    if(b->direct_hit){
        for(;;){
            if(b->unique_key == key){
                if(value) memcpy(value, b->value, map->value_size);
                if(b2){ // not the head of the list
                    b2->next_probe = b->next_probe;
                }
                else if(b->next_probe != NO_MORE_PROBES){
                    b2 = BUCKET(map,CALC_INDEX(map,h,b->next_probe));
                    b->next_probe = b2->next_probe;
                    b->unique_key = b2->unique_key;
                    memcpy(b->value, b2->value, map->value_size);
                    b = b2;
                }
                //b->direct_hit = 0;
                //b->next_probe = EMPTY_SLOT;
                memset(b,0,map->bucket_size);
                map->num_entries--;
                // shrink the table if the load factor is less than 37.5%
                if(map->num_entries < (map->num_buckets >> 2) + (map->num_buckets >> 3)) flatmap56_resize(map,-1);
                return true;
            }
            b2 = b; // remember the previous bucket_t
            /*p = b->next_probe;
            if(p == NO_MORE_PROBES) break;
            p = CALC_INDEX(map,h,p);
            b = BUCKET(map,p);*/
            if(b->next_probe == NO_MORE_PROBES) break;
            b = BUCKET(map,CALC_INDEX(map,h,b->next_probe));
        }
    }

    return false;
}

