#include "gtest/gtest.h"

#include "execution/channel.h"

using namespace td;

TEST(ChannelTest, TestSimpleSingleThread)
{
  channel<int> ch;
  ch[ch.any] << 33;

  channel<int>::queue_element i;
  ch[ch.any] >> i;
  EXPECT_EQ(33, i.value);
}

TEST(ChannelTest, TestTopicSingleThread)
{
  channel<int> ch;
  ch[1] << 33;

  channel<int>::queue_element i;
  ch[1] >> i;
  EXPECT_EQ(33, i.value);
}