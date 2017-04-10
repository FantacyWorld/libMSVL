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

// static functions
// The core of any insert/remove operation
static bool dyn_shift_insert(dyn_array_t *const dyn_arr, const size_t position, const size_t count,
                      const DYN_SHIFT_MODE mode, const void *const data_src);
                      
static bool dyn_shift_remove(dyn_array_t *const dyn_arr, const size_t position, const size_t count,
                      const DYN_SHIFT_MODE mode, void *const data_dst);    


static bool dyn_request_size_increase(dyn_array_t *const dyn_arr, const size_t increment);

                      
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
            
            if(actual_capacity > DYN_MAX_CAPACITY)
                free(dyn_arr);
            
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

const void *dyn_array_export(const dyn_array_t *const dyn_arr)
{
    if(dyn_arr)
    {
        return dyn_arr->array;
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



// the family of 'view' functions 
void *dyn_array_front(const dyn_array_t *const dyn_arr)
{
    if(dyn_arr && dyn_arr->size)
    {
        return dyn_arr->array;
    }
    return NULL;
}

void *dyn_array_back(const dyn_array_t *const dyn_arr)
{
    if(dyn_arr && dyn_arr->size)
    {
        return DYN_ARRAY_POSITION(dyn_arr, dyn_arr->size - 1);
    }
    return NULL;
}

void *dyn_array_at(const dyn_array_t *const dyn_arr, const size_t index)
{
    if(dyn_arr && index < dyn_arr->size)
    {
        return DYN_ARRAY_POSITION(dyn_arr, index);
    }
    return NULL;
}



// the family of 'insert' functions
bool dyn_array_push_front(dyn_array_t *const dyn_arr, const void *const object)
{
    return dyn_shift_insert(dyn_arr, 0, 1, MODE_INSERT, object);
}

bool dyn_array_push_back(dyn_array_t *const dyn_arr, const void *const object)
{
    return dyn_shift_insert(dyn_arr, dyn_arr->size, 1, MODE_INSERT, (void * const)object);
}

bool dyn_array_insert(dyn_array_t *const dyn_arr, const size_t index, const void *const object)
{
    return dyn_shift_insert(dyn_arr, index, 1, MODE_INSERT, object);
}



// the family of 'remove' functions

// MODE_ERASE
bool dyn_array_pop_front(dyn_array_t *const dyn_arr)
{
    return dyn_shift_remove(dyn_arr, 0, 1, MODE_ERASE, NULL);
}

bool dyn_array_pop_back(dyn_array_t *const dyn_arr)
{
   return dyn_shift_remove(dyn_arr, dyn_arr->size - 1, 1, MODE_ERASE, NULL); 
}

bool dyn_array_erase(dyn_array_t *const dyn_arr, const size_t index)
{
       return dyn_shift_remove(dyn_arr, index, 1, MODE_ERASE, NULL); 
}

void dyn_array_clear(dyn_array_t *const dyn_arr)
{
    if(dyn_arr && dyn_arr->size)
    {
        dyn_shift_remove(dyn_arr, 0, dyn_arr->size, MODE_ERASE, NULL);
    }
}

// MODE_EXTRACT
bool dyn_array_extract_front(dyn_array_t *const dyn_arr, void *const object)
{
    return dyn_shift_remove(dyn_arr, 0, 1, MODE_EXTRACT, object);
}

bool dyn_array_extract_back(dyn_array_t *const dyn_arr, void *const object)
{
    return dyn_shift_remove(dyn_arr, dyn_arr->size - 1, 1, MODE_EXTRACT, object);
}

bool dyn_array_extract(dyn_array_t *const dyn_arr, const size_t index, void *const object)
{
    return dyn_shift_remove(dyn_arr, index, 1, MODE_EXTRACT, object);    
}



// the family of 'get' functions
bool dyn_array_empty(const dyn_array_t *const dyn_arr)
{
    return dyn_array_size(dyn_arr) == 0;
}

size_t dyn_array_size(const dyn_array_t *const dyn_arr)
{
    if(dyn_arr)
        return dyn_arr->size;
    return 0;
}

size_t dyn_array_capacity(const dyn_array_t *const dyn_arr)
{
    if(dyn_arr)
        return dyn_arr->capacity;
    return 0;
}

size_t dyn_array_data_type_size(const dyn_array_t *const dyn_arr)
{
    if(dyn_arr)
        return dyn_arr->data_type_size;
    return 0;
}

// 'sort' functions
bool dyn_array_sort(dyn_array_t *const dyn_arr, int (*const compare)(const void *, const void *))
{
    if(dyn_arr && dyn_arr->size && compare)
    {  
        // qsort in stdlib.h
        qsort(dyn_arr->array, dyn_arr->size, dyn_arr->data_type_size, compare);
        return true;
       
		/*
        // insert sort
        int i, j;
		dyn_array_t *temp = (dyn_array_t *)malloc(sizeof(dyn_array_t));
        for(i = 1;i < (int)dyn_arr->size;i++)
        {
            j = i - 1;
			if(temp == NULL)
				return false;
			memcpy(temp, DYN_ARRAY_POSITION(dyn_arr, i), dyn_arr->data_type_size);
            while(j >= 0 &&compare(DYN_ARRAY_POSITION(dyn_arr, i), DYN_ARRAY_POSITION(dyn_arr, j)) < 0) j--;
			j++;
            memmove(DYN_ARRAY_POSITION(dyn_arr, j + 1), DYN_ARRAY_POSITION(dyn_arr, j), DYN_SIZE_N_ELEMS(dyn_arr, i - j));
            memcpy(DYN_ARRAY_POSITION(dyn_arr, j), temp, dyn_arr->data_type_size);               
        }
		free(temp);
        return true;
		*/
        
    }
    return false;
}

bool dyn_array_insert_sorted(dyn_array_t *const dyn_arr, const void *const object,
                             int (*const compare)(const void *const, const void *const))
{
    if(dyn_arr && compare && object)
    {
        size_t insert_position = 0;
        if(dyn_arr->size)
        {
            while(insert_position < dyn_arr->size && 
            compare(DYN_ARRAY_POSITION(dyn_arr, insert_position), object) < 0)
            insert_position ++;
        }
        
        return dyn_shift_insert(dyn_arr, insert_position, 1, MODE_INSERT, object);
    }
    return false;
}



// other functions
bool dyn_array_for_each(dyn_array_t *const dyn_arr, void (*const func)(void *const, void *), void *arg)
{
    if(dyn_arr && dyn_arr->array && func)
    {
        // So I just noticed we never check the data array ever
        // Which is both unsafe and potentially undefined behavior
        // Although we're the only ones that touch the pointer and we always validate it.
        // So it's questionable. We'll check it here.
        // I'm considering taking these out. Anything under our control that touches this pointer is safe
        // Not checking it will segfault, which is good for debugging, but not so much for the end user
        // but good for the tester. But the tester may not trigger this if it's a crazy edge case.
        uint8_t * data_walker = (uint8_t *)dyn_arr->array;
        for(size_t idx = 0;idx < dyn_arr->size;idx++, data_walker += dyn_arr->data_type_size)
            func((void *)data_walker, arg);
        
        return true;
    }
    return false;
}


// core functions for insert and remove
// static function definitions
static bool dyn_shift_insert(dyn_array_t *const dyn_arr, const size_t position, const size_t count,
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


static bool dyn_shift_remove(dyn_array_t *const dyn_arr, const size_t position, const size_t count,
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

static bool dyn_request_size_increase(dyn_array_t *const dyn_arr, const size_t increment)
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
            void *new_array = realloc(dyn_arr->array, new_capacity * dyn_arr->data_type_size);
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
