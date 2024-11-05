#include <gtest/gtest.h>
#include "../src/utils/murmur_hash2.h"

namespace smallkv
{
    TEST(MurmurHash, basic)
    {
        EXPECT_EQ(lsmkv::utils::murmur_hash2("123", 3), 1023762986);
    }
}