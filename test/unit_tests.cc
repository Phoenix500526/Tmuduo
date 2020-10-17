#include <string>
#include "base/Palindrome.h"

#include <gtest/gtest.h>

class ReverseTests : public ::testing::Test {};

TEST_F(ReverseTests, is_palindrome) {
  std::string pal = "mom";
  Palindrome pally;

  EXPECT_TRUE(pally.isPalindrome(pal));
}
