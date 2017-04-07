#include <gtest/gtest.h>
#include "dyn_array.h"

// Support 64-bit size_t
// Allowing it to be externally set
#ifndef DYN_MAX_CAPACITY // in bytes
#define DYN_MAX_CAPACITY (((size_t) 1) << ((sizeof(size_t) << 3) - 8))
#endif

const int array_size = 100;


void destructor_dyn_array_t(void * dyn_arr_dp)
{
    if(dyn_arr_dp)
    {
        dyn_array_destroy((dyn_array_t *)*dyn_arr_dp);
    }
}

// 0 capacity
TEST(dyn_array_create, create_with_zero_capacity)
{
    size_t data_type_size = 4;
    dyn_array_t * dyn_arr = dyn_array_create(0, data_type_size, NULL);
    EXPECT_NE(nullptr, dyn_arr) << "dyn_array_create failed with 0 capacity" << std::endl;

	dyn_array_destroy(dyn_arr);

}

// high capacity
TEST(dyn_array_create, create_with_high_capacity)
{
	size_t high_capacity = 1 << 10;
    size_t data_type_size = 4;
    dyn_array_t * dyn_arr = dyn_array_create(high_capacity, data_type_size, NULL);
    EXPECT_NE(nullptr, dyn_arr) << "dyn_array_create failed with a high capacity" << std::endl;
	dyn_array_destroy(dyn_arr);

}

// import from an array with int
TEST(dyn_array_import, import_array_with_int_type)
{

	int array[array_size];
	for(int i = 0;i < array_size;i++) array[i] = i;
	
	dyn_array_t * dyn_arr = dyn_array_import(array, array_size, sizeof(int), NULL);
    EXPECT_NE(nullptr, dyn_arr) << "dyn_array_import failed with an array with int type" << std::endl;
	dyn_array_destroy(dyn_arr);
}

// import from an array with custom type
// eg: dyn_array_t
TEST(dyn_array_import, import_array_with_custom_type)
{
    int array[array_size];
	for(int i = 0;i < array_size;i++) array[i] = i;
    
	dyn_array_t * dyn_array_array[array_size];

	for(int i = 0;i < array_size;i++)
	{
		dyn_array_t * dyn_arr = dyn_array_import(array, array_size, sizeof(int), NULL);
		ASSERT_NE(nullptr, dyn_arr) << "dyn_array_import failed with an array with int type" << std::endl;
		array[i] = dyn_arr;
	}
	dyn_array_t * dyn_arr = dyn_array_import(array, array_size, sizeof(dyn_array_t *), destructor_dyn_array_t);
    EXPECT_NE(nullptr, dyn_arr) << "dyn_array_import failed with an array with dyn_array_t type" << std::endl;
	dyn_array_destroy(dyn_arr);
}



int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
