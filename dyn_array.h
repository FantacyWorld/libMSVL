#ifndef _DYN_ARRAY_H_
#define _DYN_ARRAY_H_

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include <stdbool.h> // for bool true false
#include <stddef.h> // for size_t
#include <string.h> // for memcpy
#include <stdlib.h> // for malloc
#include <stdint.h> // for uint8_t

typedef struct dyn_array dyn_array_t;

struct dyn_array {
    size_t capacity; // in objects
    size_t size; // current size in objects
    const size_t data_type_size;
    void *array;
    void (*destructor)(void *);
};

/*
    Destructor notes!

    This library accepts an optional destructor.

    Destructor settings are set at creation and cannot be changed afterwards.

    Your destructor should be a void function that accepts a single void pointer.
    ex: void my_destructor(void *object_to_destruct)

    Passing NULL as the destructor pointer disables destruction.

    Destruction is triggered whenever an object is erased.

    You can avoid destruction in a destruction-enabled dynamic array
      by using the extract family of functions.
*/

// Create a new dynamic array capable of holding at least capacity number of
// data_type_size-sized objects with optional destructor
// \param capacity Minimum capacity request(0 is fine if you have no option)
// \param data_type_size  Size of the object type to be stored in bytes
// \param destruct_func Optional destructor to be applied on destruct operations(NULL to disable)
// \return new dynamic array pointer, NULL on error
dyn_array_t * dyn_array_create(const size_t capacity, const size_t data_type_size, void (*destruct_func)(void *));

// Create a new dynamic array from a given array (we only copy the data)
// \param data The data to import
// \param count Number of objects to import
// \param data_type_size The size of each object
// \param destruct_func Optional destructor (NULL to disable)
// \return new dynamic array pointer, NULL on error
dyn_array_t * dyn_array_import(const void *const data, const size_t count, const size_t data_type_size,
                                                void (*destruct_func)(void *));

                                                
// Dynamic array destructor
// Applies destructor to all remaining elements
// \param byn_array The dynamic array to destruct
void dyn_array_destroy(dyn_array_t *const dyn_arr);
<<<<<<< HEAD

=======
// for destructor
void dyn_array_destroy_wrap(void *const dyn_arr);
>>>>>>> 8081a490136e26286bd84ceec32aa0557aa1238f

// Removes and optionally destructs all array elements
// \param dyn_array the dynamic array
void dyn_array_clear(dyn_array_t *const dyn_arr);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DYN_ARRAY_H_
