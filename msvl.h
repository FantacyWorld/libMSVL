#ifndef _MSVL_H_
#define _MSVL_H_

#include <pthread.h> // For pthread_*
#include <stdbool.h> // For bool, true and false
#include <stdio.h> // For printf
#include <assert.h> // For assert
#include <ctype.h> // For isspace

#include "dyn_array.h" // For dyn_array_*
#include "pair.h" // For pair_*
#include "dyn_hash_map.h" // For dyn_hash_map_*
#include "node.h"


typedef struct {
	pthread_t thread_id = -1;
	dyn_array_t * left; // char 
	dyn_array_t * right; // dyn_array_t *
	int priority; // The smaller the value, the higher the priority
}$$Element;

$$Element * $$Element_create(pthread_t, char * left, char * right, int priority); // For $$Push

void $$Element_destroy($$Element * e);



// external interfaces

// Collect data
void $$Push(char * left, char * right, int priority);

// When a parallel statement start, initialize the scheduling thread, initialize the root node of the thread tree
void Init(int type);

// And Pal Prj Keep Alw Manager
//   0     1    2     3     4         5
// Creates a thread based on the function entry address and parameters, and generates a Node * type node for this thread, joins the thread tree
void MyCreateThread(void* addr(void*), void* para, int type);

// Pause a thread, and then the scheduling thread will block it
void MyPauseThread();
void MyPauseThreadInit();

// This function only the main thread will eventually call
void MyWaitForObject();

// Obtain information about global variables in MSV
// Parameter 1: Number of states. Parameter 2: Global variables output function
void GetMsvVar(int * sNum, void(*output)(), bool (*isoutput)());


void SetNodeLength(int type);
#endif // _MSVL_H_
