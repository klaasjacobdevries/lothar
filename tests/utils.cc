#include <gtest/gtest.h>
#include "utils.hh"

using namespace std;
using namespace lothar;

struct radian_and_degree_pair
{
  float radians;
  int   degrees;
};

class RadiansAndDegreesTest : public testing::TestWithParam<radian_and_degree_pair>
{};

TEST_P(RadiansAndDegreesTest, DegToRad)
{
  float const radians = GetParam().radians;
  int   const degrees = GetParam().degrees;

  float actual = degtorad<float, int>(degrees);
  EXPECT_FLOAT_EQ(radians, actual);
}

TEST_P(RadiansAndDegreesTest, RadToDeg)
{
  float const radians = GetParam().radians;
  int   const degrees = GetParam().degrees;

  int actual = radtodeg<int, float>(radians);
  EXPECT_FLOAT_EQ(degrees, actual);
}

radian_and_degree_pair RadiansAndDegreesTestCases[] = {
  {0,            0},
  {M_PI,       180},
  {M_PI / 2,    90},
  {-M_PI / 180, -1}};

INSTANTIATE_TEST_CASE_P(RadiansAndDegrees,
                        RadiansAndDegreesTest,
                        testing::ValuesIn(RadiansAndDegreesTestCases));

struct host_and_nxt_pair_16
{
  uint16_t host;

  uint8_t nxt0;
  uint8_t nxt1;
};

class Endian16Test : public testing::TestWithParam<host_and_nxt_pair_16>
{};

TEST_P(Endian16Test, HostToNXT)
{
  uint16_t const host = GetParam().host;
  uint8_t const nxt[] = {GetParam().nxt0, GetParam().nxt1};

  uint8_t buf[2];
  htonxts(host, buf);
  EXPECT_EQ(nxt[0], buf[0]);
  EXPECT_EQ(nxt[1], buf[1]);
}

TEST_P(Endian16Test, NXTToHost)
{
  uint16_t const host = GetParam().host;
  uint8_t const nxt[] = {GetParam().nxt0, GetParam().nxt1};

  EXPECT_EQ(host, nxttohs(nxt));
}

host_and_nxt_pair_16 Endian16TestCases[] = {
  {300, 44, 1}};

INSTANTIATE_TEST_CASE_P(Endian16,
                        Endian16Test,
                        testing::ValuesIn(Endian16TestCases));

struct host_and_nxt_pair_32
{
  uint32_t host;

  uint8_t nxt0;
  uint8_t nxt1;
  uint8_t nxt2;
  uint8_t nxt3;
};

class Endian32Test : public testing::TestWithParam<host_and_nxt_pair_32>
{};

TEST_P(Endian32Test, HostToNXT)
{
  uint32_t const host = GetParam().host;
  uint8_t const nxt[] = {GetParam().nxt0, GetParam().nxt1, GetParam().nxt2, GetParam().nxt3};

  uint8_t buf[4];
  htonxtl(host, buf);
  EXPECT_EQ(nxt[0], buf[0]);
  EXPECT_EQ(nxt[1], buf[1]);
  EXPECT_EQ(nxt[2], buf[2]);
  EXPECT_EQ(nxt[3], buf[3]);
}

TEST_P(Endian32Test, NXTToHost)
{
  uint32_t const host = GetParam().host;
  uint8_t const nxt[] = {GetParam().nxt0, GetParam().nxt1, GetParam().nxt2, GetParam().nxt3};

  EXPECT_EQ(host, nxttohl(nxt));
}

host_and_nxt_pair_32 Endian32TestCases[] = {
  {67504647, 7, 10, 6, 4}};

INSTANTIATE_TEST_CASE_P(Endian32,
                        Endian32Test,
                        testing::ValuesIn(Endian32TestCases));

TEST(UtilsTest, Time)
{
  lothar::time_t start = lothar::time();
  msleep(10);
  lothar_time_t end = lothar::time() - start;
  EXPECT_GE(end, 10u);
  EXPECT_LE(end, 20u) << "5 ms sleep took too long";
}

TEST(UtilsTest, Timer)
{
  lothar::time_t start = timer(NULL);
  msleep(10);
  lothar::time_t end = timer(&start);
  EXPECT_GE(end, 10u);
  EXPECT_LE(end, 20u) << "10 ms sleep took too long";
}
