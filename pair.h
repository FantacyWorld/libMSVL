#ifndef _PAIR_H_
#define _PAIR_H
#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include <pthread.h>

typedef struct pair pair_t;
struct pair{
	void * key;
	void * value;
}

/// Create a pair with the specified key type size and value type size
/// Optionally destruct for key and value
/// \param key hold key
/// \param value hold value
/// \return get a pointer to pair on success(NULL on error)
pair_t * pair_create(const void * const key, const void * const value);

/// pair_t destructor
/// don't destruct the key and value 
/// \param pair the pair to be destruct
void pair_destroy(pair_r * const pair);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PAIR_H_