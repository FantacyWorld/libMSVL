#include "dyn_array.h"

// Support 64-bit size_t
// Allowing it to be externally set
#ifndef DYN_MAX_CAPACITY // in bytes
#define DYN_MAX_CAPACITY (((size_t) 1) << ((sizeof(size_t) << 3) - 8))
#endif

// casts pointer and does arithmetic to get index of element
#define DYN_ARRAY_POSITION(dyn_array_ptr, idx) \
    (((uint8_t *) (dyn_array_ptr)->array) + ((idx) * (dyn_array_ptr)->data_type_size))
// Gets the size (in bytes) of n dyn_array elements
#define DYN_SIZE_N_ELEMS(dyn_array_ptr, n) ((dyn_array_ptr)->data_type_size * (n))

// Modes of operation for dyn_shift
typedef enum 
{ 
    MODE_INSERT = 0x01, 
    MODE_EXTRACT = 0x02, 
    MODE_ERASE = 0x06, 
    TYPE_REMOVE = 0x02
} DYN_SHIFT_MODE;

// The core of any insert/remove operation
bool dyn_shift_insert(dyn_array_t *const dyn_arr, const size_t position, const size_t count,
                      const DYN_SHIFT_MODE mode, const void *const data_src);
                      
bool dyn_shift_remove(dyn_array_t *const dyn_arr, const size_t position, const size_t count,
                      const DYN_SHIFT_MODE mode, void *const data_dst);    

                      
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

dyn_array_t * dyn_array_import(const void *const data, const size_t count, const size_t data_type_size,
                                                void (*destruct_func)(void *))
{
    if(data && count > 0)
    {
        dyn_array_t * dyn_arr = dyn_array_create(count, data_type_size, destruct_func);
        if(dyn_arr)
        {
            //memcpy(dyn_arr->array, data, count * data_type_size);
            //return dyn_arr;
            
            // dyn_shift_insert
            if(dyn_shift_insert(dyn_arr, 0, count, MODE_INSERT, data))
                return dyn_arr;
            
            dyn_array_destroy(dyn_arr);
        }
    }
    return NULL;
}


void dyn_array_destroy(dyn_array_t *const dyn_arr)
{
    if(dyn_arr)
    {
        dyn_array_clear(dyn_arr);
        free(dyn_arr->array);
        free(dyn_arr);
    }
}

void dyn_array_clear(dyn_array_t *const dyn_arr)
{
    if(dyn_arr && dyn_arr->size)
    {
        dyn_shift_remove(dyn_arr, 0, dyn_arr->size, MODE_ERASE, NULL);
    }
}


bool dyn_request_size_increase(dyn_array_t *const dyn_arr, const size_t increment);

bool dyn_shift_insert(dyn_array_t *const dyn_arr, const size_t position, const size_t count,
                      const DYN_SHIFT_MODE mode, const void *const data_src)
{
    if(dyn_arr && count && mode == MODE_INSERT && data_src)
    {
        // insert will lead to memory grow, so we need to reallocate if not enough
        if(position <= dyn_arr->size && dyn_request_size_increase(dyn_arr, count))
        {
            if(position != dyn_arr->size) // Not inserted at the end  of the array and need to move data
            {
                memmove(DYN_ARRAY_POSITION(dyn_arr, position + count),
                                DYN_ARRAY_POSITION(dyn_arr, position), 
                                DYN_SIZE_N_ELEMS(dyn_arr, dyn_arr->size - position));
            }
            memcpy(DYN_ARRAY_POSITION(dyn_arr, position), data_src, 
                            DYN_SIZE_N_ELEMS(dyn_arr, count));
            dyn_arr->size += count;
            return true;
        }
    }
    return false;
}


bool dyn_shift_remove(dyn_array_t *const dyn_arr, const size_t position, const size_t count,
                      const DYN_SHIFT_MODE mode, void *const data_dst)
{
    if(dyn_arr && count && dyn_arr->size && (mode & TYPE_REMOVE) // MODE_ERASE || MODE_EXTRACT = TYPE_REMOVE
         && position + count <= dyn_arr->size)     
    {
        if(mode == MODE_ERASE) 
         {
             if(dyn_arr->destructor)
             {
                 uint8_t * destruct_pos = DYN_ARRAY_POSITION(dyn_arr, position);
                 for(size_t total = count;total;total--, destruct_pos += dyn_arr->data_type_size)
                     dyn_arr->destructor(destruct_pos);
             }
         }
        else // extracting data
        {
            if(data_dst)
            {
                memcpy(data_dst, DYN_ARRAY_POSITION(dyn_arr, position), dyn_arr->data_type_size * count);
                return true;
            }
            else // extracting data without data_dst
                return false;
        }
        
        if(position + count < dyn_arr->size)
        {
            memmove(DYN_ARRAY_POSITION(dyn_arr, position),
                            DYN_ARRAY_POSITION(dyn_arr, position + count), 
                            DYN_SIZE_N_ELEMS(dyn_arr, dyn_arr->size - (position + count)));
        }
        dyn_arr->size -= count;
        return true;
    } 
    return false;
}

bool dyn_request_size_increase(dyn_array_t *const dyn_arr, const size_t increment)
{
    if(dyn_arr)
    {
        if(dyn_arr->size + increment <= dyn_arr->capacity)
            return true;
        
        // reallocate memory
        // the size needed memory
        size_t needed_size = dyn_arr->size + increment;
        if(needed_size <= DYN_MAX_CAPACITY)
        {
            size_t new_capacity = dyn_arr->capacity << 1;
            while(new_capacity < needed_size)
                new_capacity <<= 1;
            
            void *new_array = realloc(dyn_arr->array, new_capacity);
            if(new_array)
            {
                dyn_arr->capacity = new_capacity;
                dyn_arr->array = new_array;
                return true;
            }
        }
    }
    return false;
}               
