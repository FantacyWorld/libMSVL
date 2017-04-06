#include "dyn_array.h"

struct dyn_array {
    size_t capacity; // in bytes
    size_t size; // current size in bytes
    const size_t data_type_size;
    void *array;
    void (*destructor)(void *);
};

// Support 64-bit size_t
// Allowing it to be externally set
#ifndef DYN_MAX_CAPACITY // in bytes
#define DYN_MAX_CAPACITY (((size_t) 1) << ((sizeof(size_t) << 3) - 8))
#endif


dyn_array_t * dyn_array_create(const size_t capacity, const size_t data_type_size, void (*destruct_func)(void *))
{
    if(data_type_size && capacity <= DYN_MAX_CAPACITY)
    {
        dyn_array_t * dyn_arr = (dyn_array_t *)malloc(sizeof(dyn_array_t));
        if(dyn_arr)
        {
            // for 16 bytes alligned 
            size_t actual_capacity = 16;
            while(actual_capacity < capacity)
                actual_capacity <<= 1;
            
            // dyn_array->capacity = actual_capacity;
            // dyn_array->size = 0;
            // dyn_array->data_type_size = data_type_size;
            // dyn_array->destructor = destruct_func; 
            
            // Anonymous objects
            memcpy(dyn_arr, 
                  &(dyn_array_t){actual_capacity, 0, data_type_size, malloc(actual_capacity * data_type_size), destruct_func}, 
                  sizeof(dyn_array_t));
                        
            if(dyn_arr->array)
                return dyn_arr;
            
            free(dyn_arr);
        }
    }
    return NULL;
}
