#include "msvl.h"



// global
// Global mapping from pthread_t to node_t *
// Defined in msvl.c
dyn_hash_map_t * g_thread_to_node; 

// The destructor will not free dynamic objects if it is true
bool g_is_copied;

node_t * g_thread_root;

// Mark if the current path is false
bool *g_false;

pthread_t g_turn_id;
pthread_t g_manager_id;

dyn_array_t * g_keep_childs;



// static
static dyn_array_t * g_$$prsnt;
static node_t * g_prj_node = NULL;
static node_t * g_changed_node = NULL;

// States output control
static void(*g_output)() = NULL;
static bool(*g_is_output)() = NULL;
static int * g_state_num = NULL;



// Node destructor wrapper
static void node_destroy_wrap(void * this){
	if(this){
		node_destroy(*(node_t **)this);
	}
}

// $$Element destructor wrapper
static void $$Element_destroy_wrap(void * this){
	if(this){
		$$Element_destroy(*($$Element **)this);
	}
}

// Initialize all global objects
static void initialize_all_global_objects(void){
	
	pthread_mutex_init(&g_stack_mutex, NULL);
		
	if(g_$$prsnt){
		g_$$prsnt = dyn_array_create(0, sizeof($$Element *), );
	}
	
}

static void UpdateNode(node_t * node){
	if(NULL == node || node_get_type(node) == node_get_new_type(node) || node == g_thread_root)
		return;
	
	dyn_array_t * childs = node_get_childs(node_get_father(node));
	
	size_t i;
	for(i = 0;*(node_t **)dyn_array_at(childs, i) != node;i++);
	
	node_t * temp = *(node_t **)dyn_array_at(childs, i);
	do{
		temp = GetNewNode(temp);
		
		node_t ** pos = (node_t **)dyn_array_at(node_get_childs(node_get_father(node)), i);
		*pos = temp;
		
		node_execute(temp);
	} while (node_get_type(temp) != node_get_new_type(node));
}

static void ExecuteKeepChilds(){
	size_t i = 0;
	
	while(i < dyn_array_size(g_keep_childs)){
		node_t * node = dyn_array_at(g_keep_childs, i);
		node_t * and_father = node;
		
		while(NULL != node_get_father(and_father) && AND == node_get_type(node_get_father(and_father))){
			and_father = node_get_father(and_father);
		}
		
		if(EMPTY == node_get_len(and_father)){
			node_destroy(node);
			node = NULL;
			i++;
			continue;
		}
		
		dyn_array_push_buck(node_get_childs(node_get_father(node)), &node);
		node_t * temp = node_get_father(node);
		
		while(NULL != temp && MORE != node_get_len(temp)){
			node_set_len(temp, MORE);
			temp = node_get_father(temp);
		}
		
		node_execute(node);
		UpdateNode(node);
		UpdateNode(node_get_father(node));
		i++;
	}
	dyn_array_clear(g_keep_childs);
}

static void* ManagerThread(void *para){
	while(g_turn_id != g_manager_id){
		sched_yield();
	}
	
	if(NULL == g_thread_root){
		printf("NULL == g_thread_root\n");
		return NULL;
	}
	
	size_t pre_size;
	pthread_t id;
	bool has_executed = false;
	
	do{
		if(node_get_new_type(g_thread_root) == LEAF || node_get_len(g_thread_root) == EMPTY)
			break;
		
		if(NULL != g_is_output && NULL != g_output && NULL != g_state_num){
			if(!g_is_output()) exit(0);
			printf("State[%d]", *g_state_num);
			g_output();
			putchar('\n');
			(*g_state_num)++;
		}
		
		node_execute(g_thread_root);
		g_changed_node = NULL;
		g_prj_node = NULL;
	
	SORT:
		if(NULL != g_prj_node){
			node_execute(g_prj_node);
			g_prj_node = NULL;
		}
	
		pthread_mutex_lock(&g_stack_mutex)；
		pre_size = dyn_array_size(g_$$prsnt);
		pthread_mutex_unlock(&g_stack_mutex);
		
		while(!dyn_array_empty(g_$$prsnt)){
			has_executed = true;
			
			pthread_mutex_lock(&g_stack_mutex);
			$$Element* ele;
			dyn_array_extract_back(g_$$prsnt, &ele)；
			pre_size = dyn_array_size(g_$$prsnt);
			pthread_mutex_unlock(&g_stack_mutex);
			
			id = ele->thread_id;

			pair_t * it = dyn_hash_map_find(g_thread_to_node, &id);
			
			if(NULL == it){
				assert(false);
			}
			
			node_t * temp = (node_t *)*(it->second);
			node_execute(tmp);
			
			UpdateNode(temp);
			
			if(NULL != g_changed_node){
				UpdateNode(g_changed_node);
				g_changed_node = NULL;
			}
			
			$$Element_destroy(ele);
			ele = NULL;
			
			if(pre_size < dyn_array_size(g_$$prsnt) || NULL != g_prj_node)
				goto SORT;
		}
		
		node_is_correct_and_exit(g_thread_root);
		
		if(!dyn_array_empty(g_keep_childs)){
			ExecuteKeepChilds();
			goto SORT;
		}
	}while(true);
	
	node_destroy(g_thread_root);
	g_thread_root = NULL;

	return NULL;
}




// global interface
// Collect data
void $$Push(char * left, char * right, int priority)
{
	$$Element *ele = ($$Element *)malloc(sizeof($$Element));
	if(ele)
	{
		if(left){
			int cap = strlen(left) + 1;
			ele->left = dyn_array_import(left, cap, sizeof(char), NULL);
		}else{
			ele->left = dyn_array_create(0, sizeof(char), NULL);			
		}
		
		if(right){
			ele->right = dyn_array_create(0, sizeof(dyn_array_t *), dyn_array_destroy_wrap);
			int i, j = 0;
			for(i = 0;right[i];i++){
				if(isspace(right[i])){
					dyn_array_t * temp = dyn_array_import(right + j, i - j, sizeof(char), NULL);
					dyn_array_push_buck(ele->right, &temp);
					
					while(right[i] && isspace(right[i]))i++;
					j = i;
					i--;
				}
			}
		}else{
			ele->right = dyn_array_create(0, sizeof(dyn_array_t *), dyn_array_destroy_wrap);
		}
	}
	
	ele->priority = priority;
	ele->thread_id = pthread_self();
	
	pthread_mutex_lock(&g_stack_mutex);
	if(g_$$prsnt){
		dyn_array_push_buck(g_$$prsnt, &ele);
	}else{
		g_$$prsnt = dyn_array_create(0, sizeof($$Element *), $$Element_destroy_wrap);
		dyn_array_push_buck($$prsnt, &ele);
	}
	pthread_mutex_unlock(&g_stack_mutex);
	
}

// When a parallel statement start, initialize the scheduling thread, initialize the root node of the thread tree
void Init(int type){
	
	if( 0 != pthread_create(&g_manager_id, 0, ManagerThread, NULL)){
		printf("ERROR: ManagerThread fail! at void Init(int type) !\n");
		exit(1)；
	}
	
	pthread_t thread = pthread_self();
	pthread_mutex_t *mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex,NULL);
	pthread_cond_t *cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	pthread_cond_init(cond,NULL);
	
	switch (type)
	{
	case 0: {
		g_thread_root = (node_t *)and_node_create(thread, NULL, NULL, false, mutex, cond);
		if(!g_thread_root){
			printf("ERROR: and_node_create failed.\n");
			exit(1);
		}
		break;
	}
	case 1: {
		g_thread_root = (node_t *)pal_node_create(thread, NULL, NULL, false, mutex, cond);
		if(!g_thread_root){
			printf("ERROR: pal_node_create failed.\n");
			exit(1);
		}
		break;
	}
	case 2: {
		g_thread_root = (node_t *)prj_node_create(thread, NULL, NULL, false, mutex, cond);
		if(!g_thread_root){
			printf("ERROR: prj_node_create failed.\n");
			exit(1);
		}
		break;
	}
	default:
		printf("ERROR: In Init() , undefined fatherType.\n");
		exit(1);
	}
	
}

// And Pal Prj Keep Alw Manager
//  0   1   2    3   4     5
// Creates a thread based on the function entry address and parameters, and generates a Node * type node for this thread, joins the thread tree
void MyCreateThread(void* addr(void*), void* para, int type){
	if(5 == type)
		return;
	
	if(NULL == g_thread_root)
		Init(type);
	
	pthread_t id;
	node_t * father = NULL;
	father = *(node_t **)dyn_hash_map_at(g_thread_to_node, &id);
	
	if(!father)
		return ;
	
	node_set_new_type(father, NODETYPE(type));
	
	if(NULL == addr)
		return ;
	
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	
	pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
	
	if(0 != pthread_create(&thread, &attr, addr, para)){
		printf("ERROR: Can't create a thread.\n");
		exit(0);
	}
	
	pthread_mutex_t * mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	pthread_cond_t * cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	pthread_cond_int(cond, NULL);
	
	node_t * child = (node_t *)leaf_node_create(thread, father, NULL, false, mutex, cond);
	dyn_array_t * childs = node_get_childs(father);
	dyn_array_push_buck(childs, &child);
	
}

// Pause a thread, and then the scheduling thread will block it
void MyPauseThread(){
	pthread_t id = pthread_self();
	node_t * node = *(node_t **)dyn_hash_map_at(g_thread_to_node, &id);

	pthread_mutex_t * mutex = node_get_mutex(node);
	pthread_cond_t * cond = node_get_cond(node);

	pthread_mutex_lock(mutex);
	g_turn_id = g_manager_id;
	while(g_turn_id != id){
		pthread_cond_wait(cond,mutex);
	}
	pthread_mutex_unlock(mutex);	
	
}

void MyPauseThreadInit(){
	pthread_t id = pthread_self();

	node_t * node = NULL;
	while(true){

		node = *(node_t **)dyn_hash_map_at(g_thread_to_node, &id);

		if(NULL != node)
			break;

	}

	node_set_is_init(node, true);
	
	pthread_mutex_t * mutex = node_get_mutex(node);
	pthread_cond_t * cond = node_get_cond(node);

	pthread_mutex_lock(mutex);
	g_turn_id = g_manager_id;
	while(g_turn_id != id){
		pthread_cond_wait(cond,mutex);
	}
	pthread_mutex_unlock(mutex);
}

// This function only the main thread will eventually call
void MyWaitForObject(){
	static bool has_waited = false;

	if (has_waited){
		MyPauseThread();
		return;
	}

	hasWaited = true;

	g_turn_id = g_manager_id;

	// 等待Manger 线程结束
	pthread_join(g_manager_id,NULL);

	has_waited = false;
	g_manager_id = 0;
	return;	
}

// Obtain information about global variables in MSV
// Parameter 1: Number of states. Parameter 2: Global variables output function
void GetMsvVar(int * s_num, void(*output)(), bool (*is_output)()){
	g_state_num = s_num;
	g_output = output;
	g_is_output = is_output;
	g_false = (bool *)malloc(sizeof(bool));
	g_false = false;
}


void SetNodeLength(int type){

}