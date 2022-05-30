
//          Copyright Christopher Smith 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "geoseq_unordered_flatmap56.h"

#define MAX_PROBES          128  // length of unordered_flatmap56::probes[]
#define NO_MORE_PROBES      127  // last index of unordered_flatmap56::probes[] is reserved
#define EMPTY_SLOT          0    // first index of unordered_flatmap56::probes[] is reserved
#define MAX_CAPACITY        (UINT64_MAX / sizeof(Bucket))
#define MIN_CAPACITY        128
#define MIN(A,B)            ((A) < (B) ? (A) : (B))
#define MAX(A,B)            ((A) > (B) ? (A) : (B))
#define CALC_INDEX(MAP,H,P) ((H + MAP->probes[P]) & MAP->table_mask)
#define HASH(MAP,KEY)       ((KEY * 11400714819323198103ul) >> MAP->hash_shift)
#define NUM_COMMON_RATIOS   64
const float common_ratios[NUM_COMMON_RATIOS] = {
    1.01,      1.01,      1.01,      1.01,      1.01,      1.01,      1.0079365, 1.0170454,
    1.02521,   1.0327868, 1.04,      1.0470588, 1.0537634, 1.0603975, 1.0670611, 1.0736196,
    1.0802047, 1.0866873, 1.0935134, 1.1,       1.1066263, 1.1126557, 1.1193954, 1.1253822,
    1.1328775, 1.1396684, 1.1458784, 1.1526062, 1.1588525, 1.1663471, 1.1723212, 1.1795865,
    1.186137,  1.1934365, 1.2,       1.2067335, 1.2141206, 1.2210136, 1.2275452, 1.2347129,
    1.2421652, 1.25,      1.2552301, 1.2626262, 1.2698558, 1.2763086, 1.2839322, 1.2919024,
    1.2993667, 1.306582,  1.3143861, 1.3224209, 1.3304347, 1.3353751, 1.3431294, 1.3509286,
    1.358213,  1.3660651, 1.374269,  1.3826252, 1.3905559, 1.398671,  1.4048699, 1.4124077
};

typedef struct {
    struct {
        uint64_t next_probe : 7;  // next index into unordered_flatmap56::probes[]
        uint64_t direct_hit : 1;  // direct hit or not?
        uint64_t unique_key : 56; // unique key value
    };
    uint64_t value;
}Bucket;

struct UNORDERED_FLAT_MAP56 {
    uint64_t hash_shift;
    uint64_t num_entries;
    uint64_t num_buckets;
    uint64_t table_mask;
    uint64_t probes[MAX_PROBES]; // first and last element are reserved
    Bucket*  buckets;
};

static inline bool initialize(FlatMap56* map, const uint64_t capacity) {
    
    // determine how many bits we need for the requested capacity
    unsigned int bits = (unsigned int)(ceil(log2(MIN(MAX(capacity, MIN_CAPACITY), MAX_CAPACITY))));

    // calculate the probes using a geometric sequence
    float growth_ratio = common_ratios[bits - 1];
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
    map->num_buckets = 1ul << bits;
    map->table_mask = map->num_buckets - 1;
    
    // try to allocate an array of buckets
    map->buckets = (Bucket*)calloc(map->num_buckets, sizeof(Bucket));

    // if we failed to allocate an array of buckets, then put the old map back and return
    if(map->buckets == NULL) return false;

    // return to the caller
    return true;
}

inline FlatMap56* FlatMap56_create(const uint64_t initial_capacity) {
    FlatMap56* map = (FlatMap56*)calloc(1, sizeof(FlatMap56));
    if(!map) return NULL;
    if(!initialize(map, initial_capacity)){
        FlatMap56_destroy(map);
        return NULL;
    }
    return map;
}

inline void FlatMap56_destroy(FlatMap56* map) {
    if(map){
        if(map->buckets) free(map->buckets);
        free(map);
    }
}

inline float FlatMap56_load_factor(const FlatMap56* map) {
    return map->num_buckets == 0 ? 0.0f : (float)map->num_entries / (float)map->num_buckets;
}

inline uint64_t FlatMap56_size(const FlatMap56* map) {
    return map->num_entries;
}

inline uint64_t FlatMap56_bucket_count(const FlatMap56* map) {
    return map->num_buckets;
}

inline uint64_t FlatMap56_max_bucket_count() {
    return MAX_CAPACITY;
}

inline uint64_t FlatMap56_lookup(const FlatMap56* map, const uint64_t key) {
    uint64_t h = HASH(map,key);
    Bucket*  b = &map->buckets[h];
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

inline uint64_t FlatMap56_remove(FlatMap56* map, const uint64_t key) {
        
    uint64_t h = HASH(map,key);
    Bucket*  b = &map->buckets[h];
    Bucket*  b2 = NULL;
    uint64_t p,v;
    
    if(b->direct_hit){
        for(;;){
            if(b->unique_key == key){
                v = b->value;             // save the value so we can return it later
                p = b->next_probe;        // get the next probe index
                if(p == NO_MORE_PROBES){  // if there aren't any subsequent buckets
                    // b2 points to the previous bucket if it's not the first iteration of the loop
                    if(b2) b2->next_probe = NO_MORE_PROBES;
                    memset(b, 0, sizeof(Bucket));
                }
                else{
                    // set b2 to point to the next bucket
                    p = CALC_INDEX(map,h,p);
                    b2 = &map->buckets[p];
                    b->next_probe = b2->next_probe;
                    b->unique_key = b2->unique_key;
                    b->value = b2->value;
                    memset(b2, 0, sizeof(Bucket));
                }
                return v;       // return the value
            }
            b2 = b;             // remember the previous bucket
            p = b->next_probe;  // goto the next bucket...
            if(p == NO_MORE_PROBES) break;
            p = CALC_INDEX(map,h,p);
            b = &map->buckets[p];
        }
    }

    return 0;
}

static inline bool emplace_new_direct_hit(FlatMap56* map, const uint64_t key, const uint64_t value, const uint64_t h){

    Bucket*  temp = NULL;
    Bucket*  empty = NULL;
    Bucket*  predecessor = NULL;
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


static inline bool emplace_new_indirect_hit(FlatMap56* map, const uint64_t key, const uint64_t value, Bucket* b){

    Bucket*  temp = NULL;
    Bucket*  empty = NULL;
    Bucket*  predecessor = NULL;
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

static inline bool emplace(FlatMap56* map, const uint64_t key, const uint64_t value) {
    uint64_t h = HASH(map,key);
    Bucket*  b = &map->buckets[h];
    if(b->next_probe == EMPTY_SLOT){  // DONE
        b->unique_key = key;
        b->next_probe = NO_MORE_PROBES;
        b->direct_hit = 1;
        b->value = value;
        map->num_entries++;
        return true;
    }
    return b->direct_hit ? emplace_new_direct_hit(map,key,value,h) : emplace_new_indirect_hit(map,key,value,b);
}

static inline bool grow(FlatMap56* map){
    FlatMap56 old_map = *map;
    if(!initialize(map, old_map.num_buckets * 2)){
        *map = old_map;
        return false;
    }
    for(uint64_t i = 0; i < old_map.num_buckets; i++){
        if(old_map.buckets[i].next_probe != EMPTY_SLOT){
            if(!emplace(map, old_map.buckets[i].unique_key, old_map.buckets[i].value)){
                FlatMap56_destroy(map);
                *map = old_map;
                return false;
            }
        }
    }
    if(old_map.buckets) free(old_map.buckets);
    return true;
}

inline bool FlatMap56_insert(FlatMap56* map, const uint64_t key, const uint64_t value) {
    return (!emplace(map,key,value) && (!grow(map) || !emplace(map,key,value))) ? false : true;
}

