#include "spsc-queue.h"

#include <gtest/gtest.h>

TEST(SPSCQueue, EmptyAtStart) {
    SPSCQueue<int> queue(4);
    int out;
    EXPECT_FALSE(queue.try_pop(out));
}

TEST(SPSCQueue, PushThenPop) {
    SPSCQueue<int> queue(4);
    int out;

    EXPECT_TRUE(queue.try_push(42));
    EXPECT_TRUE(queue.try_pop(out));
    EXPECT_EQ(out, 42);
}

TEST(SPSCQueue, FullRejectsPush) {
    SPSCQueue<int> queue(4);
    EXPECT_TRUE(queue.try_push(1));
    EXPECT_TRUE(queue.try_push(2));
    EXPECT_TRUE(queue.try_push(3));
    EXPECT_FALSE(queue.try_push(4));
}

TEST(SPSCQueue, FIFOOrder) {
    SPSCQueue<int> queue(8);
    int out;

    EXPECT_TRUE(queue.try_push(1));
    EXPECT_TRUE(queue.try_push(2));
    EXPECT_TRUE(queue.try_push(3));

    EXPECT_TRUE(queue.try_pop(out));
    EXPECT_EQ(out, 1);
    EXPECT_TRUE(queue.try_pop(out));
    EXPECT_EQ(out, 2);
    EXPECT_TRUE(queue.try_pop(out));
    EXPECT_EQ(out, 3);
}
