#include <gtest/gtest.h>
#include "dyn_array.h"
#include <cstdlib>
#include <ctime>

// Support 64-bit size_t
// Allowing it to be externally set
#ifndef DYN_MAX_CAPACITY // in bytes
#define DYN_MAX_CAPACITY (((size_t) 1) << ((sizeof(size_t) << 3) - 8))
#endif

const int array_size = 100;

// dyn_array_t *
void destructor_dyn_array_t(void * dp)
{
	if(dp)
	{
		dyn_array_destroy(*(dyn_array_t **)dp);
	}
}

int compare_int(const void * x, const void * y)
{
	return (*(int *)x) == (*(int *)y) ? 0 : ((*(int *)x) > (*(int *)y) ? 1 : -1);
}

void print_int(void * const data, void *)
{
	std::cout << *(int *)data << "    ";
}

/*************************************************************************/
// dyn_array_create
TEST(dyn_array_create, zero_capacity)
{
    size_t data_type_size = 4;
    dyn_array_t * dyn_arr = dyn_array_create(0, data_type_size, NULL);
    EXPECT_NE(nullptr, dyn_arr) << "dyn_array_create failed with 0 capacity" << std::endl;

	dyn_array_destroy(dyn_arr);

}

TEST(dyn_array_create, high_capacity)
{
	size_t high_capacity = 1 << 10;
    size_t data_type_size = 4;
    dyn_array_t * dyn_arr = dyn_array_create(high_capacity, data_type_size, NULL);
    EXPECT_NE(nullptr, dyn_arr) << "dyn_array_create failed with a high capacity" << std::endl;
	dyn_array_destroy(dyn_arr);

}

// dyn_array_import
// int
TEST(dyn_array_import, int_type)
{
	int array[array_size];
	for(int i = 0;i < array_size;i++) array[i] = i;
	
	dyn_array_t * dyn_arr = dyn_array_import(array, array_size, sizeof(int), NULL);
    EXPECT_NE(nullptr, dyn_arr) << "dyn_array_import failed with an integer array" << std::endl;
	dyn_array_destroy(dyn_arr);
}

// dyn_array_t *
TEST(dyn_array_import, dyn_array_t_type)
{
    int array[array_size];
	for(int i = 0;i < array_size;i++) array[i] = i;
    
	dyn_array_t * dyn_array_array[array_size];

	for(int i = 0;i < array_size;i++)
	{
		dyn_array_t * dyn_arr = dyn_array_import(array, array_size, sizeof(int), NULL);
		ASSERT_NE(nullptr, dyn_arr) << "dyn_array_import failed with an array with int type" << std::endl;
		dyn_array_array[i] = dyn_arr;
	}
	dyn_array_t * dyn_arr = dyn_array_import(dyn_array_array, array_size, sizeof(dyn_array_t *), destructor_dyn_array_t);
    EXPECT_NE(nullptr, dyn_arr) << "dyn_array_import failed with an array with dyn_array_t type" << std::endl;
	dyn_array_destroy(dyn_arr);
}

// dyn_array_export 
// int
TEST(dyn_array_push_export, int_type)
{
    int array[array_size];
	for(int i = 0;i < array_size;i++) array[i] = i;

	dyn_array_t * dyn_arr = dyn_array_import(array, array_size, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_import failed with an integer array" << std::endl;

	
	const void * export_array = dyn_array_export(dyn_arr);
	for(int i = 0;i < array_size;i++)
	{
		ASSERT_EQ(*((int *)export_array + i), i);
	}
}

// dyn_array_front
// int
TEST(dyn_array_front, int_type)
{
    int array[array_size];
	for(int i = 0;i < array_size;i++) array[i] = i;

	dyn_array_t * dyn_arr = dyn_array_import(array, array_size, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_import failed with an integer array" << std::endl;

	ASSERT_EQ(*((int *)dyn_array_front(dyn_arr)), 0);
}

// dyn_array_back
// int
TEST(dyn_array_back, int_type)
{
    int array[array_size];
	for(int i = 0;i < array_size;i++) array[i] = i;

	dyn_array_t * dyn_arr = dyn_array_import(array, array_size, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_import failed with an integer array" << std::endl;

	ASSERT_EQ(*((int *)dyn_array_back(dyn_arr)), array_size - 1);
}

// dyn_array_at
// int
TEST(dyn_array_at, int_type)
{
    int array[array_size];
	for(int i = 0;i < array_size;i++) array[i] = i;

	dyn_array_t * dyn_arr = dyn_array_import(array, array_size, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_import failed with an integer array" << std::endl;
	
	srand(time(NULL));
	int index = rand() % array_size;
	ASSERT_EQ(*((int *)dyn_array_at(dyn_arr, index)), index);
}

// dyn_array_push_front
// int
TEST(dyn_array_push_front, int_type)
{
	dyn_array_t * dyn_arr = dyn_array_create(0, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_create failed with an integer array" << std::endl;

	/* compare with vector<int> */
	std::vector<int> vec;

	for(int i = 0;i < array_size;i++)
	{
		vec.push_back(i);
	}

	for(int i = 0;i < array_size;i++)
	{
		dyn_array_push_front(dyn_arr, &i);
	}

	for(int i = 0;i < array_size;i++)
	{
		ASSERT_EQ((vec[i] + *(int *)dyn_array_at(dyn_arr, i)), array_size - 1); 
	}


	dyn_array_destroy(dyn_arr);
}

// dyn_array_push_back
// int
TEST(dyn_array_push_back, int_type)
{
	dyn_array_t * dyn_arr = dyn_array_create(0, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_create failed with an integer array" << std::endl;

	/* compare with vector<int> */
	std::vector<int> vec;

	for(int i = 0;i < array_size;i++)
	{
		vec.push_back(i);
		dyn_array_push_back(dyn_arr, &i);
		ASSERT_EQ(vec[i], *(int *)dyn_array_at(dyn_arr, i)) << "vector: " << vec[i] << " != " << "dyn_array_t: " 
															<< *(int *)dyn_array_at(dyn_arr, i) << std::endl;
	}
	dyn_array_destroy(dyn_arr);
}

// dyn_array_insert
// int
TEST(dyn_array_insert, int_type)
{
	dyn_array_t * dyn_arr = dyn_array_create(0, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_create failed with an integer array" << std::endl;

	/* compare with vector<int> */
	std::vector<int> vec;

	for(int i = 0;i < array_size;i++)
	{
		vec.push_back(i);
		dyn_array_insert(dyn_arr, i, &i);
		ASSERT_EQ(vec[i], *(int *)dyn_array_at(dyn_arr, i)) << "vector: " << vec[i] << " != " << "dyn_array_t: " 
															<< *(int *)dyn_array_at(dyn_arr, i) << std::endl;
	}
	dyn_array_destroy(dyn_arr);
}


// dyn_array_pop_front
// int
TEST(dyn_array_pop_front, int_type)
{
	dyn_array_t * dyn_arr = dyn_array_create(0, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_create failed with an integer array" << std::endl;



	for(int i = 0;i < array_size;i++)
	{
		dyn_array_push_front(dyn_arr, &i);
	}

	for(int i = 0;i < array_size;i++)
	{
		if(dyn_array_pop_front(dyn_arr))
			ASSERT_EQ(dyn_array_size(dyn_arr), array_size - 1 -i);
	}
	dyn_array_destroy(dyn_arr);
}

// dyn_array_pop_back
// int
TEST(dyn_array_pop_back, int_type)
{
	dyn_array_t * dyn_arr = dyn_array_create(0, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_create failed with an integer array" << std::endl;



	for(int i = 0;i < array_size;i++)
	{
		dyn_array_push_back(dyn_arr, &i);
	}

	for(int i = 0;i < array_size;i++)
	{
		if(dyn_array_pop_back(dyn_arr))
			ASSERT_EQ(dyn_array_size(dyn_arr), array_size - 1 -i);
	}
	dyn_array_destroy(dyn_arr);
}

// dyn_array_erase
// int
TEST(dyn_array_erase, int_type)
{
	dyn_array_t * dyn_arr = dyn_array_create(0, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_create failed with an integer array" << std::endl;



	for(int i = 0;i < array_size;i++)
	{
		dyn_array_push_back(dyn_arr, &i);
	}

	for(int i = 0;i < array_size;i++)
	{
		int index = rand() % dyn_array_size(dyn_arr);
		if(dyn_array_erase(dyn_arr, index))
			ASSERT_EQ(dyn_array_size(dyn_arr), array_size - 1 -i);
	}
	dyn_array_destroy(dyn_arr);
}

// dyn_array_clear
// int
TEST(dyn_array_clear, int_type)
{
	dyn_array_t * dyn_arr = dyn_array_create(0, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_create failed with an integer array" << std::endl;


	for(int i = 0;i < array_size;i++)
	{
		dyn_array_push_back(dyn_arr, &i);
	}

	dyn_array_clear(dyn_arr);
	ASSERT_EQ(dyn_array_size(dyn_arr), 0);

	dyn_array_destroy(dyn_arr);
}

// dyn_array_extract_front
// dyn_array_extract_back
// dyn_array_extract

// dyn_array_sort
TEST(dyn_array_sort, int_type)
{
	dyn_array_t * dyn_arr = dyn_array_create(0, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_create failed with an integer array" << std::endl;


	for(int i = 0;i < array_size;i++)
	{
		int temp = rand() % array_size;
		dyn_array_push_back(dyn_arr, &temp);
	}

	std::cout << "before sorting" << std::endl;
	dyn_array_for_each(dyn_arr, print_int, NULL);
	std::cout << std::endl;

	dyn_array_sort(dyn_arr, compare_int);

	std::cout << "after sorting" << std::endl;	
	dyn_array_for_each(dyn_arr, print_int, NULL);
	std::cout << std::endl;

	dyn_array_destroy(dyn_arr);
}

// dyn_array_insert_sorted
TEST(dyn_array_insert_sorted, int_type)
{
	dyn_array_t * dyn_arr = dyn_array_create(0, sizeof(int), NULL);
	ASSERT_NE(nullptr, dyn_arr) << "dyn_array_create failed with an integer array" << std::endl;


	for(int i = 0;i < array_size;i++)
	{
		dyn_array_push_back(dyn_arr, &i);
	}

	std::cout << "before insert sorting" << std::endl;
	dyn_array_for_each(dyn_arr, print_int, NULL);
	std::cout << std::endl;

	int random = rand() % array_size;
	dyn_array_insert_sorted(dyn_arr, &random, compare_int);
	random = 100;
	dyn_array_insert_sorted(dyn_arr, &random, compare_int);
	random = 0;
	dyn_array_insert_sorted(dyn_arr, &random, compare_int);

	std::cout << "after insert sorting" << std::endl;	
	dyn_array_for_each(dyn_arr, print_int, NULL);
	std::cout << std::endl;

	dyn_array_destroy(dyn_arr);
}
int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
