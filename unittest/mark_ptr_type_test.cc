//
// Created by grakra on 20-6-27.
//
#include <concurrent/mark_ptr_type.hh>
#include <gtest/gtest.h>
#include <atomic>
using std::unique_ptr;
namespace com {
namespace grakra {
namespace concurrent {
class TestMarkPtrType : public ::testing::Test {

};

TEST_F(TestMarkPtrType, fromPtr2MarkPtrType) {
  ASSERT_EQ(sizeof(MarkPtrType), 8);
  std::atomic<void*> atomic_ptr;
  ASSERT_TRUE(atomic_ptr.is_lock_free());
  void *hi_ptr = reinterpret_cast<void *>(0xffff876543210004);
  MarkPtrType markPtr(hi_ptr);
  ASSERT_EQ(markPtr.ptr, reinterpret_cast<void *>(0x876543210004));
  ASSERT_EQ(markPtr.get(), hi_ptr);
  GTEST_LOG_(INFO) << "ptr=" << markPtr.ptr << ", markPtr" << markPtr.ToString();
  markPtr.set_tag(markPtr.get_tag() + 1).mark_delele();
  ASSERT_EQ(markPtr.ptr, reinterpret_cast<void *>(0x1876543210005));
  ASSERT_EQ(markPtr.get(), hi_ptr);
  GTEST_LOG_(INFO) << "ptr=" << markPtr.ptr << ", markPtr" << markPtr.ToString();

  void *low_ptr = reinterpret_cast<void *>(0x476543210004);
  MarkPtrType markPtr2(low_ptr);
  ASSERT_EQ(markPtr2.ptr, reinterpret_cast<void *>(0x476543210004));
  ASSERT_EQ(markPtr2.get(), low_ptr);
  GTEST_LOG_(INFO) << "ptr=" << markPtr2.ptr << ", markPtr" << markPtr2.ToString();
  markPtr2.set_tag(markPtr2.get_tag() + 1).mark_delele();
  ASSERT_EQ(markPtr2.ptr, reinterpret_cast<void *>(0x1476543210005));
  ASSERT_EQ(markPtr2.get(), low_ptr);
  GTEST_LOG_(INFO) << "ptr=" << markPtr2.ptr << ", markPtr" << markPtr2.ToString();

  markPtr2.set_tag(0xffff);
  ASSERT_EQ(markPtr2.get_tag(), 0xffff);
  markPtr2.set_tag(markPtr2.get_tag()+1);
  ASSERT_EQ(markPtr2.get_tag(), 0);
  ASSERT_EQ(markPtr2.get(), low_ptr);
}

}
}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
