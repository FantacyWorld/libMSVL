#ifndef _DYN_HASH_MAP_H_
#define _DYN_HASH_MAP_H_

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

/* headers */
#include <stddef.h>
#include "dyn_array.h"
#include <stdbool.h>
#include "pair.h"

/* structures */
typedef struct dyn_hash_map dyn_hash_map_t;

// Dynamic array helps dealing with hash conflict 
struct dyn_hash_map{
    size_t capacity; // Prime has a good perfomance
    size_t size;
    const size_t data_type_size; // pair in bytes
    dyn_array_t **table;
    void (*destructor)(void *); // for pair;
};
/// Create a new dynamic hash map of holding at least capacity number of 
/// data_type_size-sized objects with destructor
/// \param capacity Minimum hash capacity request (0 is fine if you have no option)
/// \param data_type_size Size of the pair to be stored in bytes
/// \param destructor_func Optional destructor to be applied to pair type
dyn_hash_map_t * dyn_hash_map_create(const size_t capacity, const data_type_size, void (*destructor_func)(void *));

/// dynamic hash map destructor
/// applying destructor to all remaining elements in dynamic arrays
/// \param dyn_hash_map the dynamic hash map to destructor
void dyn_hash_map_destroy(dyn_hash_map_t * const dyn_hash_map);

/// insert a pair into the map 
/// \return true if the key of the pair to be inserted is not in the map
bool dyn_hash_mao_insert(dyn_hash_map_t * const dyn_hash_map, ); 


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DYN_HASH_MAP_H_