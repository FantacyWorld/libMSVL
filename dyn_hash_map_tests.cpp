#include <gtest/gtest.h>
#include "dyn_hash_map.h"
// dyn_hash_map_create
TEST(dyn_hash_map_create, zero_capacity_NULL_destructor)
{
	dyn_hash_map_t * dyn_hash_map = dyn_hash_map_create(0, )
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}