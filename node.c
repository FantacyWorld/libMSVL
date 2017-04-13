#include "node.h"


// Global mapping from pthread_t to node_t *
// Defined in msvl.c
extern dyn_hash_map_t * g_thread_to_node; 

// The destructor will not free dynamic objects if it is true
extern bool g_is_copied;

extern node_t * g_thread_root;

// Mark if the current path is false
extern bool *g_false;

extern pthread_t g_turn_id;
extern pthread_t g_manager_id;

extern dyn_array_t * g_keep_childs;



// static function declaration
static size_t and_node_get_insert_pos(and_node_t * this);




// Constructor and Destructor
// node_t 
node_t * node_create(pthread_t thread, node_t * father, dyn_array_t * childs, 
								NODETYPE type, bool is_init, 
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond)
{	
	node_t * node = NULL;
	switch(type){
		case AND:{
			node = (node_t *)malloc(sizeof(and_node_t));
			break;
		}
		case PAL:{
			node = (node_t *)malloc(sizeof(pal_node_t));
			break;
		}
		case PRJ:{
			node = (node_t *)malloc(sizeof(prj_node_t));
			break;
		}
		case KEEP:{
			node = (node_t *)malloc(sizeof(keep_node_t));
			break;
		}
		case ALW:{
			node = (node_t *)malloc(sizeof(alw_node_t));
			break;
		}
		case LEAF:{
			node = (node_t *)malloc(sizeof(leaf_node_t));
			break;
		}
		default:
		return NULL;
	}
	
    if(!node) return NULL;
    
	node->thread = thread;
	node->father = father;
	if(childs)
        node->childs = dyn_array_import(dyn_array_front(childs), dyn_array_size(childs), sizeof(node_t *), node_destroy_wrap);
    else
        node->childs = dyn_array_create(0, sizeof(node_t *), node_destroy_wrap);
    
	node->type = type;
	node->new_type = type;
	node->len = MORE;
    
 	node->is_init = is_init;   
	node->has_empty = false;
	node->mutex = mutex;
	node->cond = cond;
	
	pair_t * p = pair_create(sizeof(pthread_t), sizeof(node_t *)); // pair_t * pair_create(const size_t key_type_size, const size_t value_type_size);
    if(NULL == g_thread_to_node){
		g_thread_to_node = dyn_hash_map_create(sizeof(pthread_t), sizeof(node_t *)); // dyn_hash_map_t * dyn_hash_map_create(const size_t key_type_size, const value_type_size_size);
		if(NULL == g_thread_to_node)
			return NULL;
	}
	// bool dyn_hash_map_insert(dyn_hash_map_t * map, const void * const p);
	assert(true == dyn_hash_map_insert(g_thread_to_node, p));
	pair_destroy(p);
	
	size_t i;
	for(i = 0;i < dyn_array_size(childs);i++){
		node_set_father((node_t *)dyn_array_at(childs, i), node);
	}
	
    return node;
}
node_t * node_import(node_t * node){
	if(node){
		node_t * new_node = node_create(node->thread, node->father, node->childs, node->is_init, node->type, node->mutex, node->cond);
		new_node->new_type = node->new_type;
		new_node->len = node->len;
		new_node->has_empty = node->has_empty;
		node_set_mutex(node, NULL);
		node_set_cond(node, NULL);
		return new_node;
	}
	return NULL;
}

void node_destroy(node_t * this)
{
    if(this){
		if(!g_is_copied){
			this->father = NULL;
			
			size_t i;
			node_t * temp;
			for(i = 0;i < dyn_array_size(this->childs);i++)
			{
				temp = *(node_t **)dyn_array_at(this->childs, i);
				free(temp);
				temp = NULL;
			}
			dyn_array_destroy(this->childs);
			
			assert(dyn_hash_map_erase(g_thread_to_node, &this->thread) == 1);
			
			if(!node_is_exit(this) && this != g_thread_root){
				pthread_kill(thread, SIGUSR2)
			}
			
			pthread_join(this->thread, NULL);
			
			this->thread = 0;
			
			if(NULL != this->mutex){
				pthread_mutex_destroy(this->mutex);
				free(this->mutex);
				this->mutex = NULL;
			}
			if(NULL != this->cond){
				pthread_cond_destroy(this->cond);
				free(this->cond);
				this->cond = NULL;
			}
		}
		free(this);
	}
}

// and_node_t
and_node_t * and_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond){
	return (and_node_t *)node_create(thread, father, childs, is_init, AND, mutex, cond);
}

and_node_t * and_node_import(and_node_t * and_node){
	return (and_node_t *)node_import((node_t *)node);
}

void and_node_destroy(and_node_t * this){
	node_destroy((node_t *)this);
}

// pal_node_t
pal_node_t * pal_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond){
	return (pal_node_t *)node_create(thread, father, childs, is_init, PAL, mutex, cond);									
}

pal_node_t * pal_node_import(pal_node_t * pal_node){
	return (pal_node_t *)node_import((node_t *)node);	
}
void pal_node_destroy(pal_node_t * this){
	node_destroy((node_t *)this);	
}


// prj_node_t
prj_node_t * prj_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond){
	prj_node_t * ret = (prj_node_t *)node_create(thread, father, childs, is_init, PRJ, mutex, cond);	
	ret->ext_Q = true;
	return ret;
}

prj_node_t * prj_node_import(prj_node_t * prj_node){
	prj_node_t * ret = (prj_node_t *)node_import((node_t *)node);
	ret->ext_Q = true;
	return ret;
}

void prj_node_destroy(prj_node_t * this){
	node_destroy((node_t *)this);
}


// keep_node_t
keep_node_t * keep_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond){
	keep_node_t * ret = (keep_node_t *)node_create(thread, father, childs, is_init, KEEP, mutex, cond);									
	ret->len = NONE;
	node_set_len(ret->father, NONE);
	return ret;
}
keep_node_t * keep_node_import(keep_node_t * keep_node){
	keep_node_t * ret = (keep_node_t *)node_import((node_t *)node);
	ret->len = NONE;
	node_set_len(ret->father, NONE);
	return ret;	
}
void keep_node_destroy(keep_node_t * this){
	node_destroy((node_t *)this);	
}

// alw_node_t
alw_node_t * alw_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond){
	alw_node_t * ret = (alw_node_t *)node_create(thread, father, childs, is_init, ALW, mutex, cond);									
	ret->len = NONE;
	node_set_len(ret->father, NONE);
	return ret;
}

alw_node_t * alw_node_import(alw_node_t * alw_node){
	alw_node_t * ret = (alw_node_t *)node_import((node_t *)node);	
	ret->len = NONE;
	node_set_len(ret->father, NONE);
	return ret;	
}

void alw_node_destroy(alw_node_t * this){
	node_destroy((node_t *)this);	
}

// leaf_node_t
leaf_node_t * leaf_node_create(pthread_t thread, node_t * father, dyn_array_t * childs, bool is_init
                                pthread_mutex_t * mutex,
                                pthread_cond_t * cond){
	return (leaf_node_t *)node_create(thread, father, childs, is_init, LEAF, mutex, cond);									
}

leaf_node_t * leaf_node_import(leaf_node_t * leaf_node){
	return (leaf_node_t *)node_import((node_t *)node);	
}

void leaf_node_destroy(leaf_node_t * this){
	node_destroy((node_t *)this);	
}



// 'Get' functions
pthread_t node_get_thread(node_t * this){
    return this->thread;
}

node_t * node_get_father(node_t * this){
   return this->father;
}

dyn_array_t * node_get_childs(node_t * this){
   return this->childs;
}

NODETYPE node_get_type(node_t * this){
   return this->type;
}

NODETYPE node_get_new_type(node_t * this){
   return this->new_type;
}

NODELENGTH node_get_len(node_t * this){
   return this->len;
}

bool node_get_is_init(node_t * this){
   return this->is_init; 
}

bool node_get_has_empty(node_t * this){
   return this->has_empty;
}

pthread_mutex_t * node_get_mutex(node_t * this){
   return this->mutex;
}

pthread_cond_t * node_get_cond(node_t * this){
   return this->cond; 
}



// 'Set' functions
void node_set_thread(pthread_t thread, node_t * this){
    this->thread = thread;
}

void node_set_father(node_t * father, node_t * this){
    this->father = father;
}

void node_add_child(node_t * child, node_t * this){
    dyn_array_push_buck(this->childs, &child);
}

void node_set_type(NODETYPE type, node_t * this){
    this->type = type;
}

void node_set_new_type(NODETYPE new_type, node_t * this){
    this->new_type = new_type;
}

void node_set_len(NODELENGTH len, node_t * this){
    this->len = len;
}

void node_set_is_init(bool is_init, node_t * this){
    this->is_init = is_init;
}

bool node_set_has_empty(bool has_empty, node_t * this){
    this->has_empty = has_empty;
}

void node_set_mutex(pthread_mutex_t * mutex, node_t * this){
    this->mutex = mutex;
}

void node_set_cond(pthread_cond_t * cond, node_t * this){
    this->cond = cond;
}

// justify if a node exited
bool node_is_exit(node_t * this){
	if(this->thread == 0)
		return true;
	
	int exit_cond = pthread_kill(this->thread, 0);
	bool rtn = (code == ESRCH || code == EINVAL);
	return rtn;
}



// virtual 
// *_is_correct_and_exit
bool node_is_correct_and_exit(node_t * this){
	if(this){
		switch(this->type){
			case AND:{
				return and_node_is_correct_and_exit((and_node_t *)this);
			}
			case PAL:{
				return pal_node_is_correct_and_exit((pal_node_t *)this);
			}
			case PRJ:{
				return prj_node_is_correct_and_exit((prj_node_t *)this);
			}
			case KEEP:{
				return keep_node_is_correct_and_exit((keep_node_t *)this);
			}
			case ALW:{
				return alw_node_is_correct_and_exit((alw_node_t *)this);
			}
			case LEAF:{
				return leaf_node_is_correct_and_exit((leaf_node_t *)this);
			}
			default:
			printf("ERROR: Undefined node type.\n");
			exit(1);
		}
	}
	exit(1);
}

bool and_node_is_correct_and_exit(and_node_t * this){
	
	size_t empty_num = 0, none_num = 0;
	size_t size = dyn_array_size(this->childs);
	
	node_t ** it = (node_t **)dyn_array_front(this->childs);
	
	while(it != (node_t **)dyn_array_back(this->childs) + 1){
		node_t * node = *it;
		bool exited = node_is_correct_and_exit(node);
		
		if(node_get_len(node) == EMPTY && node_get_type(node) == LEAF){
			empty_num++;
			this->has_empty = true;
		}else if(node_get_len(node) == LEN){
			none_num++;
		}
		
		if(exited && node_get_len(node) == EMPTY && node_get_type(LEAF)){
			node_destroy(node);
			node = NULL;
			*it = NULL;
			dyn_array_erase(this->childs, it);
			continue;
		}
		it++;
	}
	
	if(!this->has_empty){
		if(none_num == size){
			len = NONE;
		}else{
			len = MORE;
		}
		
		return false;
	}
	
	if(empty_num + none_num == size){
		len = EMPTY;
		return true;
	}
	
	return false;
}

bool pal_node_is_correct_and_exit(pal_node_t * this){
	node_t ** it = dyn_array_front(this->childs);
	int empty_num = 0;
	size_t i = 0;
	size_t size = dyn_array_size(this->childs);
	
	while( it != (node_t **)dyn_array_back(this->childs) + 1){
		node_t * node = *it;
		if(node_is_correct_and_exit(node) && node_get_type(node) == LEAF){
			if(node_get_len(node) == EMPTY){
				empty_num++;
				this->has_empty = true;
			}
			free(node);
			node = NULL;
			dyn_array_erase(this->childs, i);
			continue;
		}
		++it;
		++i;
	}
	
	if(node_get_len((node_t *)this) == EMPTY)
		return true;
	
	if(empty_num == size){
		node_set_len((node_t *)this, EMPTY);
		return true;
	}
	
	return false;
}

bool prj_node_is_correct_and_exit(prj_node_t * this){
	int exit_num = 0;
	int size = dyn_array_size(this->childs);
	
	if(node_is_exit(dyn_array_at(this->childs, 0))
		exit_num ++;
	
	node_t ** it = (node_t **)dyn_array_at(this->childs, 1);
	for(;it != dyn_array_back(this->childs) + 1;it++){
		if(node_get_len(*it) == EMPTY && node_get_type(*it) == LEAF){
			exit_num ++;
		}
	}
	
	if(node_get_len((node_t *)this) == EMPTY)
		return true;
	
	if(exit_num == size){
		node_set_len((node_t *)this, EMPTY);
		return true;
	}
	return false;
}

bool keep_node_is_correct_and_exit(keep_node_t * this){
	return node_is_exit((node_t *)this);
}

bool alw_node_is_correct_and_exit(alw_node_t * this){
	return node_is_exit((node_t *)this)£»
}

bool leaf_node_is_correct_and_exit(leaf_node_t * this){
	return node_is_exit((node_t *)this) || this->len;
}


// *_node_execute
void node_execute(node_t * this){
	if(this){
		switch(this->type){
			case AND:{
				return and_node_execute((and_node_t *)this);
			}
			case PAL:{
				return pal_node_execute((pal_node_t *)this);
			}
			case PRJ:{
				return prj_node_execute((prj_node_t *)this);
			}
			case KEEP:{
				return keep_node_execute((keep_node_t *)this);
			}
			case ALW:{
				return alw_node_execute((alw_node_t *)this);
			}
			case LEAF:{
				return leaf_node_execute((leaf_node_t *)this);
			}
			default:
			printf("ERROR: Undefined node type.\n");
			exit(1);
		}
	}
	exit(1);
}

void and_node_execute(and_node_t * this){
	if(and_node_is_correct_and_exit(this) || *g_false){
		this->new_type = LEAF;
		return ;
	}
	
	size_t i = 0;
	while(i < dyn_array_size(this->childs)){
		node_t * child = *(node_t **)dyn_array_at(this->childs, i);
		node_execute(node);
		
		if(node_get_type(child) != node_get_new_type(child)){
			node_t ** temp = (node_t **)dyn_array_at(this->childs, i);
			*temp = GetNewNode(child);
			continue;
		}
		++i;
	}
}

void pal_node_execute(pal_node_t * this){
	if(pal_node_is_correct_and_exit(this) || *g_false){
		this->new_type = LEAF;
		return ;
	}
	
	node_t ** it = (node_t **)dyn_array_front(this->childs);
	
	while(it != (node_t **)dyn_array_back() + 1){
		node_t * child = *it;
		node_execute(child);
		if(node_get_type(child) != node_get_new_type(child)){
			*it = GetNewNode(child);
			continue;
		}
		it++;
	}
}

void prj_node_execute(prj_node_t * this){
	if(prj_node_is_correct_and_exit(this) || *g_false){
		this->new_type = LEAF;
		return ;
	}	
	
	if(dyn_array_size(this->childs) <= 1){
		this->ext_Q = true;
	}else{
		node_t *n1 = *(node_t **)dyn_array_at(this->childs, 1);
		node_execute(n1);
		
		while(node_get_type(n1) != node_get_new_type(n1)){
			n1 = GetNewNode(n1);
			node_execute(n1);
		}
		
		while(node_get_len(n1) == EMPTY){
			this->ext_Q = true;
			free(n1);
			dyn_array_erase(this->childs, 1);
			
			if(dyn_array_size(this->childs) == 1)
				break;
			
			n1 = *(node_t **)dyn_array_at(this->childs, 1);
			node_execute(n1);
			while(node_get_type(n1) != node_get_new_type(n1)){
				n1 = GetNewNode(n1);
				node_execute(n1);
			}
		}
	}
	
	this->has_ext_Q = this->ext_Q;
	
	node_t *n0 = *(node_t **)dyn_array_at(this->childs, 0);
	if(this->ext_Q && node_is_correct_and_exit(n0)){
		node_execute(n0);
		while(node_get_type(n0) != node_get_new_type(n0)){
			n0 = GetNewNode(n0);
			node_execute(n0);
		}
		this->ext_Q = false;
	}
}

void keep_node_execute(keep_node_t * this){
	if(node_is_exit((node_t *)this) || *g_false){
		return ;
	}
	
	while(!this->is_init){
		sched_yield();
	}
	
	pthread_mutex_lock(node_get_mutex((node_t *)this));
	g_turn_id = this->thread;
	pthread_mutex_unlock(node_get_mutex((node_t *)this));
	pthread_cond_signal(node_get_cond((node_t *)this));	
	
	while(g_turn_id != g_manager_id){
		sched_yield();
		if(node_is_exit((node_t *)this)){
			g_turn_id = g_manager_id;
			break;
		}
	}
	
	size_t i = 0;
	for(; i < dyn_array_size(this->childs);i++){
		node_set_father(*(node_t **)dyn_array_at(this->childs, i), this->father);
	}
	

	for(i = 0;i < dyn_array_size(this->childs);i++){
		size_t n = dyn_array_size(g_keep_childs);
		dyn_array_insert(g_keep_childs, n, dyn_array_at(this->childs, i));
	}
	
	dyn_array_destroy(this->childs);
	this->childs = NULL;
}

void alw_node_execute(alw_node_t * this){
	if(node_is_exit((node_t *)this) || *g_false){
		return ;
	}
	
	while(!this->is_init){
		sched_yield();
	}
	
	pthread_mutex_lock(node_get_mutex((node_t *)this));
	g_turn_id = this->thread;
	pthread_mutex_unlock(node_get_mutex((node_t *)this));
	pthread_cond_signal(node_get_cond((node_t *)this));	
	
	while(g_turn_id != g_manager_id){
		sched_yield();
		if(node_is_exit((node_t *)this)){
			g_turn_id = g_manager_id;
			break;
		}
	}	
	
	size_t i = 0;
	for(; i < dyn_array_size(this->childs);i++){
		node_set_father(*(node_t **)dyn_array_at(this->childs, i), this->father);
	}	
	
	for(i = 0;i < dyn_array_size(this->childs);i++){
		size_t n = dyn_array_size(this->father->childs);
		dyn_array_insert(this->father->childs, n, dyn_array_at(this->childs, i));
	}
	
	if(dyn_array_size(this->childs) > 0){
		node_t * temp = this->father;
		while(temp != NULL && node_get_len(temp) != MORE){
			node_set_len(temp, MORE);
			temp = node_get_father(temp);
		}
	}
	
	node_destroy(this->childs);
	this->childs = NULL;
}

void leaf_node_execute(leaf_node_t * this){
	if(node_is_exit(this) || *g_false || this->len)
		return ;
	
	while(!this->is_init){
		sched_yield();
	}
	
	pthread_mutex_lock(node_get_mutex((node_t *)this));
	g_turn_id = this->thread;
	pthread_mutex_unlock(node_get_mutex((node_t *)this));
	pthread_cond_signal(node_get_cond((node_t *)this));	
	
	while(g_turn_id != g_manager_id){
		sched_yield();
		if(node_is_exit((node_t *)this)){
			g_turn_id = g_manager_id;
			break;
		}
	}	
}


// special
// and_node_t 
void and_node_insert_child(and_node_t * this, node_t * child){
	dyn_array_insert(this->childs, and_node_get_insert_pos(this), &child);
}

void and_node_insert_childs(and_node_t * this, dyn_array_t * childs){
	size_t index = and_node_get_insert_pos(this);
	size_t i = 0;
	for(;i < dyn_array_size(childs);i++){
		dyn_array_insert(this->childs, index, dyn_array_at(childs, i));
	}
}

static size_t and_node_get_insert_pos(and_node_t * this){
	int i = dyn_array_size(this->childs) - 1;
	for(;i >= 0;i--){
		node_t ** pos = (node_t **)dyn_array_at(this->childs, i);
		if(node_get_type(*pos) != KEEP){
			return (size_t)i;
		}
	}
	
	return 0;
}

// pri_node_t
bool prj_node_get_has_ext_Q(prj_node_t * this){
	return this->has_ext_Q;
}


// others
node_t * GetNewNode(node_t * node)
{
	
	node_t * new_node = NULL;
	pthread_t temp = node_get_thread(node);
	assert(dyn_hash_map_erase(g_thread_to_node, &temp) == true);
	
	switch(node_get_new_type(node)){
		case LEAF:{
			dyn_array_clear(node_get_childs(node));
			new_node = leaf_node_import((leaf_node_t *)node);
			break;
		}
		case AND:{
			new_node = and_node_import((and_node_t *)node);
			break;
		}
		case PRJ:{
			new_node = prj_node_import((prj_node_t *)node);
			break;
		}
		case KEEP:{
			new_node = keep_node_import((keep_node_t *)node);
			break;
		}
		case ALW:{
			new_node = alw_node_import((alw_node_t *)node);
			break;
		}
		default:{
			printf("ERROR: Undefined node type.\n");
			exit(1);
		}
	}
	
	g_is_copied = true;
	node = (node _t *)new_node;
	g_is_copied = false;
	
	return node;	
	
}


