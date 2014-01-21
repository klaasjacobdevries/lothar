#include <gtest/gtest.h>
#include "error_handling.hh"

using namespace std;
using namespace lothar;

TEST(ErrorHandlingTest, OkayIsZero)
{
  lothar_clear_error();
  EXPECT_EQ(LOTHAR_ERROR_OKAY, lothar_errno);
  EXPECT_EQ(0, LOTHAR_ERROR_OKAY) << "okay error is not 0";
}

TEST(ErrorHandlingTest, SetErrno)
{
  enum lothar_error error = LOTHAR_ERROR_ENTITY_CLOSED;
  LOTHAR_ERROR(error);
  EXPECT_EQ(error, lothar_errno);
  char const str[] = "operation attempted on a closed entity";
  
  EXPECT_STREQ(str, lothar_strerror(error));
  EXPECT_STREQ(str, lothar_strerror_global());
}

TEST(ErrorHandlingTest, ThrowError)
{
  enum lothar_error error = LOTHAR_ERROR_ENTITY_CLOSED;
  EXPECT_THROW(check_return(error), lothar::Error) << "check_return() did not throw an error";
  error = LOTHAR_ERROR_OKAY;
  EXPECT_NO_THROW(check_return(error)) << "check_return() throws an error on okay";
}
