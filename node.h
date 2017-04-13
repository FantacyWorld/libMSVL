#ifndef _NODE_H_
#define _NODE_H_

#include <pthread.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <signal.h>

#include "dyn_array.h"
#include "dyn_hash_map.h"
#include "pair.h"

typedef enum {
	MORE,
	EMPTY,
	NONE
}NODELENGTH;

typedef enum{
	AND,
	PAL,
	PRJ,
	KEEP,
	ALW,
	LEAF,
	ROOT
} NODETYPE;

typedef struct node node_t;
typedef struct node{
	pthread_t thread;
	node_t * father;
	dyn_array_t * childs;
    
	NODETYPE type;
	NODETYPE new_type;
	NODELENGTH len;
    
 	bool is_init;   
	bool has_empty;
	pthread_mutex_t * mutex = NULL;
	pthread_cond_t * cond = NULL;
};

typedef struct and_node and_node_t;
typedef struct and_node{
	pthread_t thread;
	node_t * father;
	dyn_array_t * childs;
    
	NODETYPE type;
	NODETYPE new_type;
	NODELENGTH len;
    
 	bool is_init;   
	bool has_empty;
	pthread_mutex_t * mutex = NULL;
	pthread_cond_t * cond = NULL;
};

typedef struct pal_node pal_node_t;
typedef struct pal_node{
	pthread_t thread;
	node_t * father;
	dyn_array_t * childs;
    
	NODETYPE type;
	NODETYPE new_type;
	NODELENGTH len;
    
 	bool is_init;   
	bool has_empty;
	pthread_mutex_t * mutex = NULL;
	pthread_cond_t * cond = NULL;
};

typedef struct prj_node prj_node_t;
typedef struct prj_node{
	pthread_t thread;
	node_t * father;
	dyn_array_t * childs;
    
	NODETYPE type;
	NODETYPE new_type;
	NODELENGTH len;
    
 	bool is_init;   
	bool has_empty;
	pthread_mutex_t * mutex = NULL;
	pthread_cond_t * cond = NULL;
	
	bool ext_Q;
	bool has_ext_Q;
};

typedef struct keep_node keep_node_t;
typedef struct keep_node{
	pthread_t thread;
	node_t * father;
	dyn_array_t * childs;
    
	NODETYPE type;
	NODETYPE new_type;
	NODELENGTH len;
    
 	bool is_init;   
	bool has_empty;
	pthread_mutex_t * mutex = NULL;
	pthread_cond_t * cond = NULL;
};

typedef struct alw_node alw_node_t;
typedef struct alw_node{
	pthread_t thread;
	node_t * father;
	dyn_array_t * childs;
    
	NODETYPE type;
	NODETYPE new_type;
	NODELENGTH len;
    
 	bool is_init;   
	bool has_empty;
	pthread_mutex_t * mutex = NULL;
	pthread_cond_t * cond = NULL;
};

typedef struct leaf_node leaf_node_t;
typedef struct leaf_node{
	pthread_t thread;
	node_t * father;
	dyn_array_t * childs;
    
	NODETYPE type;
	NODETYPE new_type;
	NODELENGTH len;
    
 	bool is_init;   
	bool has_empty;
	pthread_mutex_t * mutex = NULL;
	pthread_cond_t * cond = NULL;
};



// Constructor and Destructor
// node_t
node_t * node_create(pthread_t thread, node_t * father, dyn_array_t * childs, 
								NODETYPE type, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond);
node_t * node_import(node_t * node);
void node_destroy(node_t * this);

// and_node_t
and_node_t * and_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond);
and_node_t * and_node_import(and_node_t * and_node);
void and_node_destroy(and_node_t * this);

// pal_node_t
pal_node_t * pal_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond);
pal_node_t * pal_node_import(pal_node_t * pal_node);
void pal_node_destroy(pal_node_t * this);

// prj_node_t
prj_node_t * prj_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond);
prj_node_t * prj_node_import(prj_node_t * prj_node);
void prj_node_destroy(prj_node_t * this);

// keep_node_t
keep_node_t * keep_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond);
keep_node_t * keep_node_import(keep_node_t * keep_node);
void keep_node_destroy(keep_node_t * this);

// alw_node_t
alw_node_t * alw_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond);
alw_node_t * alw_node_import(alw_node_t * alw_node);
void alw_node_destroy(alw_node_t * this);

// leaf_node_t
leaf_node_t * leaf_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond);
leaf_node_t * leaf_node_import(leaf_node_t * leaf_node)
void leaf_node_destroy(leaf_node_t * this);



// 'Get' functions
pthread_t node_get_thread(node_t * this);
node_t * node_get_father(node_t * this);
dyn_array_t * node_get_childs(node_t * this);

NODETYPE node_get_type(node_t * this);
NODETYPE node_get_new_type(node_t * this);
NODELENGTH node_get_len(node_t * this);

bool node_get_is_init(node_t * this);
bool node_get_has_empty(node_t * this);

pthread_mutex_t * node_get_mutex(node_t * this);
pthread_cond_t * node_get_cond(node_t * this);



// 'Set' functions
void node_set_thread(pthread_t thread, node_t * this);
void node_set_father(node_t * father, node_t * this);
void node_add_child(node_t * child, node_t * this);

void node_set_type(NODETYPE type, node_t * this);
void node_set_new_type(NODETYPE new_type, node_t * this);
void node_set_len(NODELENGTH len, node_t * this);

void node_set_is_init(bool is_init, node_t * this);
void node_set_has_empty(bool has_empty, node_t * this);

void node_set_mutex(pthread_mutex_t * mutex, node_t * this);
void node_set_cond(pthread_cond_t * cond, node_t * this);



// justify if a node exited
bool node_is_exit(node_t * this);


// virtual 
// *_is_correct_and_exit
bool node_is_correct_and_exit(node_t * this);
bool and_node_is_correct_and_exit(and_node_t * this);
bool pal_node_is_correct_and_exit(pal_node_t * this);
bool prj_node_is_correct_and_exit(prj_node_t * this);
bool keep_node_is_correct_and_exit(keep_node_t * this);
bool alw_node_is_correct_and_exit(alw_node_t * this);
bool leaf_node_is_correct_and_exit(leaf_node_t * this);



// *_node_execute
void node_execute(node_t * this);
void and_node_execute(and_node_t * this);
void pal_node_execute(pal_node_t * this);
void prj_node_execute(prj_node_t * this);
void keep_node_execute(keep_node_t * this);
void alw_node_execute(alw_node_t * this);
void leaf_node_execute(leaf_node_t * this);



// special
void and_node_insert(node_t * child);



// others
node_t * GetNewNode(node_t * node);

#endif // _NODE_H_