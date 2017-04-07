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

/// Create a new dynamic array capable of holding at least capacity number of
/// data_type_size-sized objects with optional destructor
/// \param capacity Minimum capacity request(0 is fine if you have no option)
/// \param data_type_size  Size of the object type to be stored in bytes
/// \param destruct_func Optional destructor to be applied on destruct operations(NULL to disable)
/// \return new dynamic array pointer, NULL on error
dyn_array_t * dyn_array_create(const size_t capacity, const size_t data_type_size, void (*destruct_func)(void *));

/// Create a new dynamic array from a given array (we only copy the data)
/// \param data The data to import
/// \param count Number of objects to import
/// \param data_type_size The size of each object
/// \param destruct_func Optional destructor (NULL to disable)
/// \return new dynamic array pointer, NULL on error
dyn_array_t * dyn_array_import(const void *const data, const size_t count, const size_t data_type_size,
                                                void (*destruct_func)(void *));
 
/// Returns an internal pointer to the data array for export
/// Since this pointer is internal, it may be invalidated by insertions that trigger reallocation
/// \param dyn_array The dynamic array to export
/// \return Pointer to dynamic array contents, NULL on error
const void *dyn_array_export(const dyn_array_t *const dyn_array);
 
/// Dynamic array destructor
/// Applies destructor to all remaining elements
/// \param byn_array The dynamic array to destruct
void dyn_array_destroy(dyn_array_t *const dyn_arr);



// the family of 'view' functions 

/// Returns a pointer to the object at the front of the array
/// \param dyn_array the dynamic array
/// \return Pointer to front object (NULL on error/empty array)
void * dyn_array_front(dyn_array_t * const dyn_arr);

/// Returns a pointer to the object at the end of the array
/// \param dyn_array the dynamic array
/// \return Pointer to last entry, NULL on error/empty array
void *dyn_array_back(const dyn_array_t *const dyn_arr);

/// Returns a pointer to the desired object in the array
/// Pointer may be invalidated if the container increases in size
/// \param dyn_array the dynamic array
/// \param index the index of the object to retrieve
/// \return pointer to the requested object, NULL on error
void *dyn_array_at(const dyn_array_t *const dyn_arr, const size_t index);



// the family of 'insert' functions

/// Copies the given object and places it at the front of the array, increasing container size by one
/// \param dyn_array the dynamic array
/// \param object the object to insert
/// \return bool representing success of the operation
bool dyn_array_push_front(dyn_array_t *const dyn_arr, const void *const object);

/// Copies the given object and places it at the back of the array, increasing container size by one
/// \param dyn_array the dynamic array
/// \param object the object to insert
/// \return bool representing success of the operation
bool dyn_array_push_back(dyn_array_t *const dyn_arr, const void *const object);

/// Inserts the given object at the given index in the array, increasing the container size by one
/// and moving any contents at index and beyond down one
/// \param dyn_array the dynamic array
/// \param index the position to insert the object at
/// \param object the object to insert
/// \return bool representing success of the operation
bool dyn_array_insert(dyn_array_t *const dyn_arr, const size_t index, const void *const object);



// the family of 'remove' functions

// MODE_ERASE

/// Removes and optionally destructs the object at the front of the array, decreasing the container size by one
/// Returns false only when array is empty or NULL was given
/// \param dyn_array the dynamic array
/// \return bool representing success of the operation
bool dyn_array_pop_front(dyn_array_t *const dyn_arr);

/// Removes and optionally destructs the object at the back of the array
/// \param dyn_array the dynamic array
/// \return bool representing success of the operation
bool dyn_array_pop_back(dyn_array_t *const dyn_arr);

/// Removes and optionally destructs the object at the given index
/// \param dyn_array the dynamic array
/// \param index index of the object to be erased
/// \return bool representing success of the operation
bool dyn_array_erase(dyn_array_t *const dyn_arr, const size_t index);

/// Removes and optionally destructs all array elements
/// \param dyn_array the dynamic array
void dyn_array_clear(dyn_array_t *const dyn_arr);

// MODE_EXTRACT

/// Removes the object in the front of the array and places it in the desired location, decreasing container size
/// Does not destruct since it was returned to the user
/// \param dyn_array the dynamic array
/// \param object destination for extracted object
/// \return bool representing success of the operation
bool dyn_array_extract_front(dyn_array_t *const dyn_arr, void *const object);

/// Removes the object in the back of the array and places it in the desired location
/// Does not destruct since it was returned to the user
/// \param dyn_array the dynamic array
/// \param object destination for extracted object
/// \return bool representing success of the operation
bool dyn_array_extract_back(dyn_array_t *const dyn_arr, void *const object);

/// Removes the object at the given index and places it at the desired location
/// Does not destruct the object since it is returned to the user
/// \param dyn_array the dynamic array
/// \param index the index of the object to extract
/// \param object destination for extracted object
/// \return bool representing success of the operation
bool dyn_array_extract(dyn_array_t *const dyn_arr, const size_t index, void *const object);



// the family of 'get' functions

/// Tests if array is empty
/// \param dyn_array the dynamic array
/// \return true if array is empty (or NULL was passed), false otherwise
bool dyn_array_empty(const dyn_array_t *const dyn_arr);

/// Returns size of array
/// \param dyn_array the dynamic array
/// \return the size of the array, 0 on error
size_t dyn_array_size(const dyn_array_t *const dyn_arr);

/// Returns the current capacity of the array
/// \param dyn_array the dynamic array
/// \return the capacity of the array, 0 on error
size_t dyn_array_capacity(const dyn_array_t *const dyn_arr);

/// Returns the size of the object stored in the array
/// \param dyn_array the dynamic array
/// \return the size of a stored object (bytes), 0 on error
size_t dyn_array_data_type_size(const dyn_array_t *const dyn_arr);


// 'sort' functions
/// Sorts the array according to the given comparator function
/// compare(x,y) < 0 iff x < y
/// compare(x,y) = 0 iff x == y
/// compare(x,y) > 0 iff y > x
/// Sort is not guaranteed to be stable
/// \param dyn_array the dynamic array
/// \param compare the comparison function
/// \return bool representing success of the operation
bool dyn_array_sort(dyn_array_t *const dyn_arr, int (*const compare)(const void *, const void *));

/// Inserts the given object into the correct sorted position
///  increasing the container size by one
/// and moving any contents beyond the sorted position down one
/// Note: calling this on an unsorted array will insert it... somewhere
/// \param dyn_array the dynamic array
/// \param object the object to insert
/// \param compare the comparison function
/// \return bool representing success of the operation
bool dyn_array_insert_sorted(dyn_array_t *const dyn_arr, const void *const object,
                             int (*const compare)(const void *const, const void *const));



// other functions
                             
/// Applies the given function to every object in the array
/// \param dyn_array the dynamic array
/// \param func the function to apply
/// \param arg argument that will be passed to the function (as parameter 2)
/// \return bool representing success of operation (really just pointer and size checks)
bool dyn_array_for_each(dyn_array_t *const dyn_arr, void (*const func)(void *const, void *), void *arg);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DYN_ARRAY_H_
