#include <gtest/gtest.h>
#include <string>
#include "commands.hh"
#include "error_handling.hh"
#include "connectionmock.hh"

using namespace std;
using namespace lothar;
using namespace testing;

namespace 
{
  // the payload is as a std::string just for the convenience of being able to type string literals
  // be carefull with '\0' characters
  vector<uint8_t> create_request(char code, bool reply, string const &payload)
  {
    vector<uint8_t> result;
    result.push_back(reply ? '\0' : '\x80');
    result.push_back(code);
    
    for(string::const_iterator i = payload.begin(); i != payload.end(); ++i)
      result.push_back(*i);
    
    return result;
  }

  vector<uint8_t> create_reply(char code, string payload, char status = '\0')
  {
    vector<uint8_t> result;
    result.push_back('\x02');
    result.push_back(code);
    result.push_back(status);

    for(string::const_iterator i = payload.begin(); i != payload.end(); ++i)
      result.push_back(*i);
    
    return result;
  }
}

TEST(CommandsTest, StatusByte)
{
  uint8_t buf[2];
  vector<uint8_t> const request = create_request(11, true, "");
  vector<uint8_t> const reply = create_reply(11, string(buf, buf + 2));

  ConnectionMock mock;
  mock.expect_write(request);
  mock.expect_read(reply);

  EXPECT_NO_THROW(getbatterylevel(mock));
}

TEST(CommandsTest, StatusByteInsancePacket)
{
  uint8_t buf[2];
  vector<uint8_t> const request = create_request(11, true, "");
  vector<uint8_t> reply = create_reply(11, string(buf, buf + 2));
  reply[2] = LOTHAR_ERROR_INSANE_PACKET;

  ConnectionMock mock;
  mock.expect_write(request);
  mock.expect_read(reply);

  EXPECT_THROW(getbatterylevel(mock), lothar::Error);
}

TEST(CommandsTest, StartProgram)
{
  ConnectionMock mock;

  string filename = "12345678901234.123";
  vector<uint8_t> request = create_request(0, false, filename);
  request.push_back('\0');

  // happy path
  mock.expect_write(request);
  startprogram(mock, filename.c_str());
}

TEST(CommandsTest, StartProgramNullFilename)
{
  ConnectionMock mock;
  EXPECT_CALL(mock, write(_, _)).
    Times(0);

  EXPECT_THROW(startprogram(mock, NULL), lothar::Error);
}

TEST(CommandsTest, StartProgramFilenameTooLong)
{
  ConnectionMock mock;
  EXPECT_CALL(mock, write(_, _)).
    Times(0);

  EXPECT_THROW(startprogram(mock, "12345678901234.1234"), lothar::Error);
}

TEST(CommandsTest, StopProgram)
{
  ConnectionMock mock;

  // only happy path, this can only do one thing
  vector<uint8_t> const request = create_request(1, false, "");
  
  mock.expect_write(request);
  stopprogram(mock);
}

TEST(CommandsTest, PlaySoundFile)
{
  ConnectionMock mock;

  string filename = "12345678901234.123";
  vector<uint8_t> request = create_request(2, false, "\x01" + filename);
  request.push_back('\0');
  
  mock.expect_write(request);
  playsoundfile(mock, true, filename.c_str());
}

TEST(CommandsTest, PlaySoundFileNull)
{
  ConnectionMock mock;
  EXPECT_CALL(mock, write(_, _)).
    Times(0);

  // filename is null
  EXPECT_THROW(playsoundfile(mock, true, NULL), lothar::Error);
}

TEST(CommandsTest, PlaySoundFileNameTooLong)
{
  ConnectionMock mock;
  EXPECT_CALL(mock, write(_, _)).
    Times(0);
  // filename too long
  EXPECT_THROW(playsoundfile(mock, false, "12345678901234.1234"), lothar::Error);
}

TEST(CommandsTest, PlayTone)
{
  ConnectionMock mock;

  uint16_t const frequency = 500;
  uint16_t const duration = 40000;

  uint8_t freq[3] = {0, 0, 0};
  uint8_t dur[3] = {0, 0, 0};
  htonxts(frequency, freq);
  htonxts(duration, dur);
  vector<uint8_t> const request = create_request(3, false, string(freq, freq + 2) + string(dur, dur + 2));

  mock.expect_write(request);
  // happy
  playtone(mock, frequency, duration);
}

TEST(CommandsTest, PlayToneFequencyBelowRange)
{
  ConnectionMock mock;

  uint16_t const given_frequency = 199; // will be clamped to 200
  uint16_t const frequency = 200;
  uint16_t const duration = 40000;

  uint8_t freq[3] = {0, 0, 0};
  uint8_t dur[3] = {0, 0, 0};
  htonxts(frequency, freq);
  htonxts(duration, dur);
  vector<uint8_t> const request = create_request(3, false, string(freq, freq + 2) + string(dur, dur + 2));

  mock.expect_write(request);
  // happy
  playtone(mock, given_frequency, duration);
}

TEST(CommandsTest, PlayToneFequencyAboveRange)
{
  ConnectionMock mock;

  uint16_t const given_frequency = 14001; // will be clamped to 14000
  uint16_t const frequency = 14000;
  uint16_t const duration = 40000;

  uint8_t freq[3] = {0, 0, 0};
  uint8_t dur[3] = {0, 0, 0};
  htonxts(frequency, freq);
  htonxts(duration, dur);
  vector<uint8_t> const request = create_request(3, false, string(freq, freq + 2) + string(dur, dur + 2));

  mock.expect_write(request);
  // happy
  playtone(mock, given_frequency, duration);
}

TEST(CommandsTest, SetOutputState)
{
  ConnectionMock mock;

  vector<uint8_t> const request = create_request(4, false, 
                                                 string("\x01") +  // port
                                                 string("\x14") +  // power
                                                 string("\x01") +  // motor on
                                                 string("\x01") +  // requlation mode speed
                                                 string("\x14") +  // turn ratio
                                                 string("\x10") +  // run state (rampup)
                                                 string("\x01\x01\x01\x01")); // tacho limit (16843009)
  
  // happy
  mock.expect_write(request);
  setoutputstate(mock, 
                 OUTPUT_B, 
                 20, 
                 MOTOR_MODE_MOTORON, 
                 REGULATION_MODE_SPEED, 
                 20,
                 RUNSTATE_RAMPUP,
                 16843009);
}

TEST(CommandsTest, SetOutputStatePowerClamp)
{
  ConnectionMock mock;

  vector<uint8_t> const request = create_request(4, false, 
                                                 string("\x01") +  // port
                                                 string("\x64") +  // power
                                                 string("\x01") +  // motor on
                                                 string("\x01") +  // requlation mode speed
                                                 string("\x14") +  // turn ratio
                                                 string("\x10") +  // run state (rampup)
                                                 string("\x01\x01\x01\x01")); // tacho limit (16843009)
  
  // happy
  mock.expect_write(request);
  setoutputstate(mock, 
                 OUTPUT_B, 
                 101, 
                 MOTOR_MODE_MOTORON, 
                 REGULATION_MODE_SPEED, 
                 20,
                 RUNSTATE_RAMPUP,
                 16843009);
}

TEST(CommandsTest, SetOutputStateTurnRatioClamp)
{
  ConnectionMock mock;

  vector<uint8_t> const request = create_request(4, false, 
                                                 string("\x01") +  // port
                                                 string("\x14") +  // power
                                                 string("\x01") +  // motor on
                                                 string("\x01") +  // requlation mode speed
                                                 string("\x64") +  // turn ratio
                                                 string("\x10") +  // run state (rampup)
                                                 string("\x01\x01\x01\x01")); // tacho limit (16843009)
  
  // happy
  mock.expect_write(request);
  setoutputstate(mock, 
                 OUTPUT_B, 
                 20, 
                 MOTOR_MODE_MOTORON, 
                 REGULATION_MODE_SPEED, 
                 101,
                 RUNSTATE_RAMPUP,
                 16843009);
}

TEST(CommandsTest, SetInputMode)
{
  ConnectionMock mock;

  vector<uint8_t> const request = create_request(5, false,
                                                 string("\x01") + // port
                                                 string("\x01") + // type (switch)
                                                 string("\x20")); // mode (boolean)

  // happy
  mock.expect_write(request);
  setinputmode(mock, INPUT_2, SENSOR_SWITCH, SENSOR_MODE_BOOLEANMODE);
}

TEST(CommandsTest, GetOutputState)
{
  ConnectionMock mock;

  uint32_t val = (1 << 24) | (2 << 16) | (3 << 8) | 4;
  uint8_t buf[4];
  htonxtl(val, buf); 

  vector<uint8_t> const request = create_request(6, true, string("\x01"));// port (B)
  vector<uint8_t> const reply   = create_reply(6, 
                                               string("\x01") + // port (B)
                                               string("\x14") + // power (20)
                                               string("\x01") + // mode (motor on)
                                               string("\x01") + // regulation (speed)
                                               string("\x14") + // turn ratio (20)
                                               string("\x10") + // run state (rampup)
                                               string(buf, buf + 4) + // tacho limit
                                               string(buf, buf + 4) + // tacho count
                                               string(buf, buf + 4) + // block tacho limit
                                               string(buf, buf + 4)); // block tacho count
  mock.expect_write(request);
  mock.expect_read(reply);
  
  int8_t power;
  output_motor_mode motormode;
  output_regulation_mode regulationmode;
  uint8_t turnratio;
  output_runstate runstate;
  uint32_t tacholimit;
  int32_t tachocount;
  int32_t blocktachocount;
  int32_t rotationcount;

  getoutputstate(mock, OUTPUT_B, &power, &motormode, &regulationmode, &turnratio, &runstate, &tacholimit, &tachocount, &blocktachocount, &rotationcount);

  
  EXPECT_EQ(20, power);
  EXPECT_EQ(MOTOR_MODE_MOTORON, motormode);
  EXPECT_EQ(REGULATION_MODE_SPEED, regulationmode);
  EXPECT_EQ(20, turnratio);
  EXPECT_EQ(RUNSTATE_RAMPUP, runstate);
  EXPECT_EQ(val, tacholimit);
  EXPECT_EQ(static_cast<int32_t>(val), tachocount);
  EXPECT_EQ(static_cast<int32_t>(val), blocktachocount);
  EXPECT_EQ(static_cast<int32_t>(val), rotationcount);
}

TEST(CommandsTest, GetInputValues)
{
  ConnectionMock mock;

  uint16_t val = (1 << 8) | 2;
  uint8_t buf[2];
  htonxts(val, buf);

  vector<uint8_t> const request = create_request(7, true, string("\x01")); // INPUT_2
  vector<uint8_t> const reply   = create_reply(7, 
                                               string("\x01") + // port (2)
                                               string("\x01") + // valid (true)
                                               string("\x01") + // calibrated (true)
                                               string("\x01") + // type (switch)
                                               string("\x20") + // mode (boolean)
                                               string(buf, buf + 2) + // raw
                                               string(buf, buf + 2) + // normalized
                                               string(buf, buf + 2) + // scaled
                                               string(buf, buf + 2)); // calibrated

  mock.expect_write(request);
  mock.expect_read(reply);
  
  bool valid       = false; 
  bool calibrated  = false;
  sensor_type type = SENSOR_NO_SENSOR;
  sensor_mode mode = SENSOR_MODE_RAWMODE;
  uint16_t raw     = 0;
  uint16_t norm    = 0;
  int16_t scaled   = 0;
  int16_t cal      = 0;
  
  getinputvalues(mock, INPUT_2, &valid, &calibrated, &type, &mode, &raw, &norm, &scaled, &cal);
  
  EXPECT_TRUE(valid);
  EXPECT_TRUE(calibrated);
  EXPECT_EQ(SENSOR_SWITCH, type);
  EXPECT_EQ(SENSOR_MODE_BOOLEANMODE, mode);
  EXPECT_EQ(val, raw);
  EXPECT_EQ(val, norm);
  EXPECT_EQ(val, scaled);
  EXPECT_EQ(val, cal);
}

TEST(CommandsTest, ResetInputScaledValue)
{
  ConnectionMock mock;

  vector<uint8_t> const request = create_request(8, false, string("\x02")); // INPUT_3
  mock.expect_write(request);
  
  resetinputscaledvalue(mock, INPUT_3);
}

TEST(CommandsTest, MessageWrite)
{
  ConnectionMock mock;

  string const message = "1234567890";
  vector<uint8_t> const request = create_request(9, false, string("\x05\x0A") + message);

  mock.expect_write(request);
  messagewrite(mock, 5, reinterpret_cast<uint8_t const *>(message.data()), message.size());
}

TEST(CommandsTest, MessageWriteEmpty)
{
  ConnectionMock mock;

  vector<uint8_t> request = create_request(9, false, string("\x05"));
  request.push_back('\0');

  mock.expect_write(request);
  messagewrite(mock, 5, NULL, 0);
}

TEST(CommandsTest, MessageWriteMaximumSize)
{
  ConnectionMock mock;

  string const message = "12345678901234567890123456789012345678901234567890123456789";
  vector<uint8_t> const request = create_request(9, false, string("\x05\x3B") + message);

  mock.expect_write(request);
  messagewrite(mock, 5, reinterpret_cast<uint8_t const *>(message.data()), message.size());
}

TEST(CommandsTest, MessageWriteMoreThanMaximumSize)
{
  ConnectionMock mock;

  string const message = "123456789012345678901234567890123456789012345678901234567890";

  EXPECT_CALL(mock, write(_, _)).
    Times(0);

  EXPECT_THROW(
    messagewrite(mock, 9, reinterpret_cast<uint8_t const *>(message.data()), message.size()),
    lothar::Error);
}

TEST(CommandsTest, MessageWriteInvalidInbox)
{
  ConnectionMock mock;

  string const message = "1234567890";
  vector<uint8_t> const request = create_request(9, false, string("\x05\x0A") + message);

  EXPECT_CALL(mock, write(_, _)).
    Times(0);

  EXPECT_THROW(
    messagewrite(mock, 11, reinterpret_cast<uint8_t const *>(message.data()), message.size()),
    lothar::Error);
}

TEST(CommandsTest, ResetMotorPosition)
{
  ConnectionMock mock;

  vector<uint8_t> const request = create_request(10, false, string("\x02\x01"));

  mock.expect_write(request);
  resetmotorposition(mock, OUTPUT_C, true);
}

TEST(CommandsTest, GetBatteryLevel)
{
  ConnectionMock mock;

  uint16_t val = 8000;
  uint8_t buf[2];
  htonxts(val, buf);
  vector<uint8_t> const request = create_request(11, true, "");
  vector<uint8_t> const reply   = create_reply(11, string(buf, buf + 2));

  mock.expect_write(request);
  mock.expect_read(reply);
  
  uint16_t level = getbatterylevel(mock);
  EXPECT_EQ(val, level);
}

TEST(CommandsTest, StopSoundPlayback)
{
  ConnectionMock mock;

  vector<uint8_t> const request = create_request(12, false, "");
  mock.expect_write(request);

  stopsoundplayback(mock);
}

TEST(CommandsTest, KeepAlive)
{
  ConnectionMock mock;

  uint32_t val = (1 << 24) | (2 << 16) | (3 << 8) | 4;
  uint8_t buf[4];
  htonxtl(val, buf); 

  vector<uint8_t> const request = create_request(13, true, "");
  vector<uint8_t> const reply   = create_reply(13, string(buf, buf + 4));

  mock.expect_write(request);
  mock.expect_read(reply);

  uint32_t sleeptime = keepalive(mock);
  EXPECT_EQ(val, sleeptime);
}

TEST(CommandsTest, LSGetStatus)
{
  ConnectionMock mock;

  vector<uint8_t> const request = create_request(14, true, "\x01");
  vector<uint8_t> const reply   = create_reply(14, "\x14");

  mock.expect_write(request);
  mock.expect_read(reply);

  uint8_t ls = lsgetstatus(mock, INPUT_2);
  EXPECT_EQ(20, ls);
}

TEST(CommandsTest, LSWrite)
{
  ConnectionMock mock;

  string const txdata = "1234567890123456";
  vector<uint8_t> const request = create_request(15, false, string("\x01\x10\x10") + txdata);

  mock.expect_write(request);
  
  lswrite(mock, INPUT_2, reinterpret_cast<uint8_t const *>(txdata.data()), txdata.size(), 16);
}

TEST(CommandsTest, LSWriteDataTooBig)
{
  ConnectionMock mock;
  string const txdata = "12345678901234567";

  EXPECT_THROW(
    lswrite(mock, INPUT_2, reinterpret_cast<uint8_t const *>(txdata.data()), txdata.size(), 17), 
    lothar::Error);
}

TEST(CommandsTest, LSWriteEmpty)
{
  ConnectionMock mock;

  vector<uint8_t> request = create_request(15, false, string("\x01\x01\x10"));
  request[3] = '\0';

  mock.expect_write(request);
  lswrite(mock, INPUT_2, NULL, 0, 16);
}

TEST(CommandsTest, LSRead)
{
  ConnectionMock mock;

  string const data = "1234567890123456";
  vector<uint8_t> request = create_request(16, true, string("\x01"));
  vector<uint8_t> reply   = create_reply(16, string("\x10") + data);

  mock.expect_write(request);
  mock.expect_read(reply);

  uint8_t rxdata[16];
  uint8_t rxlen;
  lsread(mock, INPUT_2, rxdata, sizeof(rxdata), &rxlen);
}

TEST(CommandsTest, LSReadOverflow)
{
  ConnectionMock mock;

  string const data = "1234567890123456";
  vector<uint8_t> request = create_request(16, true, string("\x01"));
  vector<uint8_t> reply   = create_reply(16, string("\x10") + data);

  mock.expect_write(request);
  mock.expect_read(reply);

  uint8_t rxdata[10];
  uint8_t rxlen;
  EXPECT_THROW(lsread(mock, INPUT_2, rxdata, sizeof(rxdata), &rxlen),
               lothar::Error);
}

TEST(CommandsTest, GetCurrentProgramMame)
{
  ConnectionMock mock;

  string const filename = "12345678901234.123";
  vector<uint8_t> request = create_request(17, true, "");
  vector<uint8_t> reply   = create_reply(17, filename);
  reply.push_back('\0');
  
  mock.expect_write(request);
  mock.expect_read(reply);

  char cfilename[19];
  getcurrentprogramname(mock, cfilename);
  EXPECT_STREQ(filename.c_str(), cfilename);
}

TEST(CommandsTest, MessageRead)
{
  ConnectionMock mock;

  vector<uint8_t> const request = create_request(19, true, 
                                                 string("\x01") + // remote inbox (1)
                                                 string("\x02") + // local inbox (2)
                                                 string("\x01")); // remove (true)
  
  string const message = "12345678901234567890123456789012345678901234567890123456789";
  vector<uint8_t> const reply = create_reply(19, string("\x02\x3B") + message);
  
  mock.expect_write(request);
  mock.expect_read(reply);

  uint8_t data[59];
  uint8_t len = 0;
  messageread(mock, 1, 2, true, data, &len);

  EXPECT_EQ(message.size(), len);
  EXPECT_EQ(message, string(data, data + len));
}
