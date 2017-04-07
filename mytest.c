#include <stdio.h>
#include "dyn_array.h"

const int array_size = 100;

int main()
{
	dyn_array_t * dyn_arr = dyn_array_create(0, sizeof(int), NULL);
	int i;
	for(i = 0;i < array_size;i++)
	{
		dyn_array_push_back(dyn_arr, &i);
	}
	return 0;
}
