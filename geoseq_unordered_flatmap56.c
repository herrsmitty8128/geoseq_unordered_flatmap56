
//          Copyright Christopher Smith 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "geoseq_unordered_flatmap56.h"

#define MAX_PROBES          128  // length of unordered_flatmap56::probes[]
#define NO_MORE_PROBES      127  // last index of unordered_flatmap56::probes[] is reserved
#define EMPTY_SLOT          0    // first index of unordered_flatmap56::probes[] is reserved
#define MAX_CAPACITY        (UINT64_MAX / sizeof(bucket_t))
#define MIN_CAPACITY        128
#define MIN(A,B)            ((A) < (B) ? (A) : (B))
#define MAX(A,B)            ((A) > (B) ? (A) : (B))
#define CALC_INDEX(MAP,H,P) ((H + MAP->probes[P]) & MAP->table_mask)
#define HASH(MAP,KEY)       ((KEY * 11400714819323198103ul) >> MAP->hash_shift)
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

typedef struct {
    struct {
        uint64_t next_probe : 7;  // next index into unordered_flatmap56::probes[]
        uint64_t direct_hit : 1;  // direct hit or not?
        uint64_t unique_key : 56; // unique key value
    };
    uint64_t value;
}bucket_t;

struct UNORDERED_FLATMAP56 {
    uint64_t  hash_shift;
    uint64_t  num_entries;
    uint64_t  num_bucket_ts;
    uint64_t  table_mask;
    uint64_t  probes[MAX_PROBES]; // first and last element are reserved
    bucket_t* buckets;
};

static inline bool initialize(flatmap56_t* map, const uint64_t capacity) {
    
    // determine how many bits we need for the requested capacity
    unsigned int bits = (unsigned int)(ceil(log2(MIN(MAX(capacity, MIN_CAPACITY), MAX_CAPACITY))));

    // calculate the probes using a geometric sequence
    //float growth_ratio = common_ratios[bits - 1];
    float growth_ratio = common_ratios[bits - 7];
    double p = 1.0f;
    map->probes[EMPTY_SLOT] = 0;     // reserved for empty
    for(size_t i = 1; i < MAX_PROBES - 1; i++){
        map->probes[i] = (uint64_t)p;
        p = ceil(p * growth_ratio);
    }
    map->probes[NO_MORE_PROBES] = 0; // reserved for NO_MORE_PROBES

    // calculate the other members; DO NOT touch map->num_entries //
    map->num_entries = 0;
    map->hash_shift = 64 - bits;
    map->num_bucket_ts = 1ul << bits;
    map->table_mask = map->num_bucket_ts - 1;
    
    // try to allocate an array of bucket_ts
    map->buckets = (bucket_t*)calloc(map->num_bucket_ts, sizeof(bucket_t));

    // if we failed to allocate an array of bucket_ts, then put the old map back and return
    if(map->buckets == NULL) return false;

    // return to the caller
    return true;
}

inline flatmap56_t* flatmap56_create(const uint64_t initial_capacity) {
    flatmap56_t* map = (flatmap56_t*)calloc(1, sizeof(flatmap56_t));
    if(!map) return NULL;
    if(!initialize(map, initial_capacity)){
        flatmap56_destroy(map);
        return NULL;
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
    return map->num_bucket_ts == 0 ? 0.0f : (float)map->num_entries / (float)map->num_bucket_ts;
}

inline uint64_t flatmap56_size(const flatmap56_t* map) {
    return map->num_entries;
}

inline uint64_t flatmap56_bucket_count(const flatmap56_t* map) {
    return map->num_bucket_ts;
}

inline uint64_t flatmap56_max_bucket_count() {
    return MAX_CAPACITY;
}

inline uint64_t flatmap56_lookup(const flatmap56_t* map, const uint64_t key) {
    uint64_t h = HASH(map,key);
    bucket_t*  b = &map->buckets[h];
    uint64_t p;
    if(b->direct_hit){
        for(;;){
            if(b->unique_key == key) return b->value;
            p = b->next_probe;
            if(p == NO_MORE_PROBES) break;
            p = CALC_INDEX(map,h,p);
            b = &map->buckets[p];
        }
    }
    return 0;
}

inline uint64_t flatmap56_remove(flatmap56_t* map, const uint64_t key) {
        
    uint64_t h = HASH(map,key);
    bucket_t*  b = &map->buckets[h];
    bucket_t*  b2 = NULL;
    uint64_t p,v;
    
    if(b->direct_hit){
        for(;;){
            if(b->unique_key == key){
                v = b->value;             // save the value so we can return it later
                p = b->next_probe;        // get the next probe index
                if(p == NO_MORE_PROBES){  // if there aren't any subsequent bucket_ts
                    // b2 points to the previous bucket_t if it's not the first iteration of the loop
                    if(b2) b2->next_probe = NO_MORE_PROBES;
                    memset(b, 0, sizeof(bucket_t));
                }
                else{
                    // set b2 to point to the next bucket_t
                    p = CALC_INDEX(map,h,p);
                    b2 = &map->buckets[p];
                    b->next_probe = b2->next_probe;
                    b->unique_key = b2->unique_key;
                    b->value = b2->value;
                    memset(b2, 0, sizeof(bucket_t));
                }
                return v;       // return the value
            }
            b2 = b;             // remember the previous bucket_t
            p = b->next_probe;  // goto the next bucket_t...
            if(p == NO_MORE_PROBES) break;
            p = CALC_INDEX(map,h,p);
            b = &map->buckets[p];
        }
    }

    return 0;
}

static inline bool emplace_new_direct_hit(flatmap56_t* map, const uint64_t key, const uint64_t value, const uint64_t h){

    bucket_t*  temp = NULL;
    bucket_t*  empty = NULL;
    bucket_t*  predecessor = NULL;
    uint8_t  x, y, z; 

    for(x = 0; x < 127; x = z){
            
        temp = &map->buckets[CALC_INDEX(map,h,x)];
        
        if(temp->unique_key == key){
            temp->value = value;
            return true;
        }
        
        z = temp->next_probe;

        if(!empty){
            for(y = x + 1; y < z; y++){
                if(map->buckets[CALC_INDEX(map,h,y)].next_probe == EMPTY_SLOT){
                    predecessor = temp;
                    empty = &map->buckets[CALC_INDEX(map,h,y)];
                    break;
                }
            }
        }
    }

    if(empty){
        empty->direct_hit = 0;
        empty->next_probe = NO_MORE_PROBES;
        empty->unique_key = key;
        empty->value = value;
        predecessor->next_probe = y;
        map->num_entries++;
        return true;
    }

    return false;
}


static inline bool emplace_new_indirect_hit(flatmap56_t* map, const uint64_t key, const uint64_t value, bucket_t* b){

    bucket_t*  temp = NULL;
    bucket_t*  empty = NULL;
    bucket_t*  predecessor = NULL;
    uint8_t  x, y, z;
    
    uint64_t h2 = HASH(map,b->unique_key);

    for(x = 0; x < 127; x = z){
        
        temp = &map->buckets[CALC_INDEX(map,h2,x)];
        z = temp->next_probe;
        
        if(!predecessor){
            if(b == &map->buckets[CALC_INDEX(map,h2,z)]){
                predecessor = temp;
            }
        }
                        
        if(!empty){
            for(y = x + 1; y < z; y++){
                if(map->buckets[CALC_INDEX(map,h2,y)].next_probe == EMPTY_SLOT){
                    // add the empty slot to the list
                    empty = &(map->buckets[CALC_INDEX(map,h2,y)]);
                    empty->direct_hit = 0;
                    empty->next_probe = z;
                    empty->unique_key = b->unique_key;
                    empty->value = b->value;
                    temp->next_probe = y;
                    break;
                }
            }
        }

        if(predecessor && empty){
            predecessor->next_probe = b->next_probe;
            b->direct_hit = 1;
            b->next_probe = NO_MORE_PROBES;
            b->unique_key = key;
            b->value = value;
            map->num_entries++;
            return true;
        }
    }

    return false;
}

static inline bool emplace(flatmap56_t* map, const uint64_t key, const uint64_t value) {
    uint64_t h = HASH(map,key);
    bucket_t*  b = &map->buckets[h];
    if(b->next_probe == EMPTY_SLOT){  // DONE
        b->unique_key = key;
        b->next_probe = NO_MORE_PROBES;
        b->direct_hit = 1;
        b->value = value;
        map->num_entries++;
        return true;
    }
    if(b->direct_hit) return emplace_new_direct_hit(map,key,value,h);

    return emplace_new_indirect_hit(map,key,value,b);
}

static inline bool grow(flatmap56_t* map){
    flatmap56_t old_map = *map;
    if(!initialize(map, old_map.num_bucket_ts * 2)){
        *map = old_map;
        return false;
    }
    for(uint64_t i = 0; i < old_map.num_bucket_ts; i++){
        if(old_map.buckets[i].next_probe != EMPTY_SLOT){
            if(!emplace(map, old_map.buckets[i].unique_key, old_map.buckets[i].value)){
                flatmap56_destroy(map);
                *map = old_map;
                return false;
            }
        }
    }
    if(old_map.buckets) free(old_map.buckets);
    return true;
}

inline bool flatmap56_insert(flatmap56_t* map, const uint64_t key, const uint64_t value) {
    return (!emplace(map,key,value) && (!grow(map) || !emplace(map,key,value))) ? false : true;
}

