#include "uniquePtr.h"
#include "gtest/gtest.h"

TEST(DataStream, SerializeString) {
    std::string val = "hello";
    EXPECT_EQ(val, "hello");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

