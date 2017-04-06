#include <gtest/gtest.h>
#include "dyn_array.h"

// Support 64-bit size_t
// Allowing it to be externally set
#ifndef DYN_MAX_CAPACITY // in bytes
#define DYN_MAX_CAPACITY (((size_t) 1) << ((sizeof(size_t) << 3) - 8))
#endif

// 0 capacity
TEST(dyn_arr_create, create_with_zero_capacity)
{
    size_t data_type_size = 4;
    dyn_array_t * dyn_arr = dyn_array_create(0, data_type_size, NULL);
    EXPECT_NE(nullptr, dyn_arr) << "dyn_arr_create failed with 0 capacity" << std::endl;
}

// high capacity
TEST(dyn_arr_create, create_with_high_capacity)
{
	size_t high_capacity = 1 << 10;
    size_t data_type_size = 4;
    dyn_array_t * dyn_arr = dyn_array_create(high_capacity, data_type_size, NULL);
    EXPECT_NE(nullptr, dyn_arr) << "dyn_arr_create failed with a high capacity" << std::endl;
}

int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
