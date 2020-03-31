
#include "gtest/gtest.h"

TEST(Expected, theEq) {
  std::string theValue("echo");
  EXPECT_EQ(theValue, theValue);
}
