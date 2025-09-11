#include <gtest/gtest.h>

TEST(basics, case_01_connect)
{}

int
main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
