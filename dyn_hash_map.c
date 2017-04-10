#include "dyn_hash_map.h"

// Allowing it to be externally set
#ifndef MAX_HASH_CAPACITY
#define MAX_HASH_CAPACITY (1 << 10)
#endif
/* static function declarations */
// MAX_INT on error
int find_next_prime(int start);
bool is_prime(int n);

dyn_hash_map_t * dyn_hash_map_create(const size_t capacity, const data_type_size, void (*destructor_func)(void *))
{
    if(data_type_size && capacity <= MAX_HASH_CAPACITY)
    {
        dyn_hash_map_t * dyn_hash_map = (dyn_hash_map_t *)malloc(sizeof(dyn_hash_map));
        if(dyn_hash_map)
        {
            // Prime will  have a good performance
            size_t actual_capacity = 17;
            while(actual_capacity < capacity)
                actual_capacity = find_next_prime(actual_capacity);
            
            if(actual_capacity > MAX_HASH_CAPACITY)
            {
                free(dyn_hash_map);
                return NULL;
            }

            
            
            memcpy(dyn_hash_map,
                        &(dyn_hash_map_t){actual_capacity, 0, data_type_size, (dyn_array_t **)malloc(actual_capacity * sizeof(dyn_array_t *)), destructor_func}, 
                        sizeof(dyn_hash_map_t));
            if(dyn_hash_map->table)
            {
                size_t i;
                bool success = true;
                for(i = 0;i < dyn_hash_map->capacity;i++)
                {
                    dyn_hash_map->table[i] = dyn_array_create(0, data_type_size, destructor_func); 
                    if(!dyn_hash_map->table[i])
                        success = false;
                }
                if(success)
                    return dyn_hash_map;
                
                // free all 
                for(i = 0;i < dyn_hash_map->capacity;i++)
                {
                    if(dyn_hash_map->table[i])
                        dyn_array_destroy(dyn_hash_map->table[i]);
                }
                free(dyn_hash_map->table);
                free(dyn_hash_map);
            }
            free(dyn_hash_map);
        }
    }
    return NULL;
}

void dyn_hash_map_destroy(dyn_hash_map_t * const dyn_hash_map)
{
	if(dyn_hash_map)
	{
		for(size_t i = 0;i < dyn_hash_map->capacity;i++)
		{
			dyn_array_destroy(dyn_hash_map->table[i]);
		}
		free(dyn_hash_map->table);
		free(dyn_hash_map);
	}
}

/* static function definitions */
int find_next_prime(int start)
{
    int i;
    for(i = start + 1;i < MAX_INT;i++)
    {
        if(is_prime(i))
            return i;
    }
    return MAX_INT;
}

bool is_prime(int n)
{
    if(n < 2)
        return false;
    int i;
    for(i = 2;i * i <= n;i++)
    {
        if(n % i  == 0)
            return false;
    }
    return true;
}