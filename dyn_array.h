#ifndef _DYN_ARRAY_H_
#define _DYN_ARRAY_H_

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include <stdbool.h> // for bool true false
#include <stddef.h> // for size_t
#include <string.h> // for memcpy
#include <stdlib.h> // for malloc

typedef struct dyn_array dyn_array_t;

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



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DYN_ARRAY_H_
