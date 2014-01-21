#ifndef CONNECTIONMOCK_HH
#define CONNECTIONMOCK_HH

#include "connection.hh"
#include <vector>
#include <gmock/gmock.h>

namespace lothar
{
  class ConnectionMock : public CustomConnection
  {
  public:
    MOCK_METHOD2(read, int(uint8_t *, size_t));
    MOCK_METHOD2(write, int(uint8_t const *, size_t));
    MOCK_METHOD0(close, int());

    // utilities for setting expectations
    void expect_read(std::vector<uint8_t> const &reply);
    void expect_write(std::vector<uint8_t> const &request);
    void expect_close();
  };
}

#endif // CONNECTIONMOCK_HH
