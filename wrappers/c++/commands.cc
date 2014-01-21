#include "commands.hh"

using namespace std;
using namespace lothar;

void lothar::getinputvalues(Connection &connection,
                            input_port port,
                            bool *valid,
                            bool *calibrated,
                            sensor_type *type,
                            sensor_mode *mode,
                            uint16_t *rawvalue,
                            uint16_t *normvalue,
                            int16_t *scaledvalue,
                            int16_t *calibratedvalue)
{
  uint8_t validu;
  uint8_t calibratedu;
  check_return(lothar_getinputvalues(connection,
                                     port,
                                     valid ? &validu : NULL,
                                     calibrated ? & calibratedu : NULL,
                                     type,
                                     mode,
                                     rawvalue,
                                     normvalue,
                                     scaledvalue,
                                     calibratedvalue));

  if(valid)
    *valid = validu;
  if(calibrated)
    *calibrated = calibratedu;
}
