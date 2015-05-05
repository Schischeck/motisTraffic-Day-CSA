#include <functional>

#include "gtest/gtest.h"

#include "Array.h"

using namespace td;

uint32_t nextPowerOf2(uint32_t n)
{
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  return n;
}

TEST(ArrayTest, NextPowerOfTwoTest)
{
  for (int i = 0; i < 2049; ++i)
    EXPECT_EQ(nextPowerOf2(i), nextPowerOfTwo(i));
}

TEST(ArrayTest, SizeTest)
{
  // check size
  // (simple - for 0, ..., 3 elements)
  Array<int> arr;
  EXPECT_EQ(0, arr.size());

  arr.emplace_back(1);
  EXPECT_EQ(1, arr.size());

  arr.emplace_back(2);
  EXPECT_EQ(2, arr.size());

  arr.emplace_back(3);
  EXPECT_EQ(3, arr.size());

  // content should match the inserted elements
  EXPECT_EQ(1, arr[0]);
  EXPECT_EQ(2, arr[1]);
  EXPECT_EQ(3, arr[2]);

  // always allocate buffers for size=2^X elements
  arr.reserve(100);
  EXPECT_EQ(128, arr._allocatedSize);

  // won't shrink automatically
  arr.reserve(0);
  EXPECT_EQ(128, arr._allocatedSize);
}

TEST(ArrayTest, DefaultConstructedElements)
{
  struct TestStruct
  {
    TestStruct() : a(42) {}
    int a;
  };

  Array<TestStruct> arr(100);

  unsigned elementCount = 0;
  for (auto const& el : arr)
    EXPECT_EQ(42, el.a);
}

TEST(ArrayTest, DestructorTestDefaultConstructed)
{
  std::vector<bool> destructed(100);
  std::fill(std::begin(destructed), std::end(destructed), false);

  struct TestStruct
  {
    TestStruct() = default;

    TestStruct(std::function<void (int)> destFun, int a)
        : destFun(destFun),
          a(a)
    {}

    ~TestStruct() { if (destFun) destFun(a); }

    std::function<void (int)> destFun;
    int a;
  };

  {
    Array<TestStruct> arr(100);
    for (int i = 0; i < 100; ++i)
      arr[i] = { [&destructed](int i) {
        destructed[i] = true;
      }, i };
  }

  for (auto const& d : destructed)
    EXPECT_TRUE(d);
}

TEST(ArrayTest, DestructorTestWithEmplaceBack)
{
  struct TestStruct
  {
    TestStruct() = default;

    TestStruct(std::function<void (int)> destFun, int a)
        : destFun(destFun),
          a(a)
    {}

    ~TestStruct() { if (destFun) destFun(a); }

    std::function<void (int)> destFun;
    int a;
  };

  Array<TestStruct> arr;
  for (int i = 0; i < 100; ++i)
  {
    arr.emplace_back([](int i) {  }, i);
    EXPECT_EQ(i, arr[i].a);
    EXPECT_EQ(i + 1, arr.size());
  }
}

TEST(ArrayTest, DestructorTestWithPushBack)
{
  struct TestStruct
  {
    TestStruct() = default;

    TestStruct(std::function<void (int)> destFun, int a)
        : destFun(destFun),
          a(a)
    {}

    ~TestStruct() { if (destFun) destFun(a); }

    std::function<void (int)> destFun;
    int a;
  };

  {
    Array<TestStruct> arr;
    for (int i = 0; i < 100; ++i)
    {
      arr.push_back({ [](int i) {  }, i });
      EXPECT_EQ(i, arr[i].a);
      EXPECT_EQ(i + 1, arr.size());
    }
  }
}


TEST(ArrayTest, RangeInit)
{
  std::vector<bool> destructed(100);
  std::fill(std::begin(destructed), std::end(destructed), false);

  struct TestStruct
  {
    TestStruct() = default;

    TestStruct(std::function<void (int)> destFun, int a)
        : destFun(destFun),
          a(a)
    {}

    ~TestStruct() { if (destFun) destFun(a); }

    std::function<void (int)> destFun;
    int a;
  };

  {
    auto vec = std::vector<TestStruct>(100);
    for (int i = 0; i < 100; ++i)
      vec[i] = { [&destructed](int i) {
        destructed[i] = true;
      }, i };

    Array<TestStruct> arr(std::begin(vec), std::end(vec));
  }

  for (auto const& d : destructed)
    EXPECT_TRUE(d);
}

TEST(ArrayTest, ConstructorThrowsInEmplaceBack)
{
  struct TestStruct
  {
    TestStruct() = default;
    TestStruct(int i) { throw std::runtime_error(""); }
  };

  Array<TestStruct> arr;

  EXPECT_THROW({
    arr.emplace_back(33);
  }, std::runtime_error);

  EXPECT_EQ(0, arr.size());
}

TEST(ArrayTest, CopyConstructorThrowsInPushBack)
{
  struct TestStruct
  {
    TestStruct() = default;
    explicit TestStruct(int i) {}
    TestStruct(TestStruct const& other) { throw std::runtime_error(""); }
  };

  Array<TestStruct> arr;

  EXPECT_THROW({
    arr.push_back(TestStruct(33));
  }, std::runtime_error);

  EXPECT_EQ(0, arr.size());
}

TEST(ArrayTest, Copy)
{
  Array<int> a;
  a.emplace_back(1);
  a.emplace_back(2);
  a.emplace_back(3);

  Array<int> b = a;
  EXPECT_EQ(1, b[0]);
  EXPECT_EQ(2, b[1]);
  EXPECT_EQ(3, b[2]);
}

TEST(ArrayTest, Move)
{
  Array<int> a;
  a.emplace_back(1);
  a.emplace_back(2);
  a.emplace_back(3);

  Array<int> b = std::move(a);
  EXPECT_EQ(1, b[0]);
  EXPECT_EQ(2, b[1]);
  EXPECT_EQ(3, b[2]);

  EXPECT_EQ(0, a.size());
}

TEST(ArrayTest, CopyString)
{
  String a("hello world");
  String b = a;

  EXPECT_EQ(a, b);
}