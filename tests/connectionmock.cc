#include "connectionmock.hh"
#include "error_handling.hh"
#include <algorithm>

using namespace std;
using namespace lothar;
using namespace testing;

// custom matcher to match writing arrays
MATCHER_P(ArrayEquals, expected, "")
{
  return std::equal(expected.begin(), expected.end(), arg);
}

void ConnectionMock::expect_read(vector<uint8_t> const &reply)
{
  EXPECT_CALL(*this, read(_, reply.size())).
    Times(1).
    WillOnce(DoAll(SetArrayArgument<0>(reply.begin(), reply.end()),
                   Return(reply.size())));
}

void ConnectionMock::expect_write(std::vector<uint8_t> const &request)
{
  // hmmm, may be unsafe if the argument 0 to write will be evalueted before argument 1
  EXPECT_CALL(*this, write(ArrayEquals(request), request.size())).
    Times(1).
    WillOnce(Return(request.size()));
}

void ConnectionMock::expect_close()
{
  EXPECT_CALL(*this, close()).
    Times(1).
    WillOnce(Return(0));
}
