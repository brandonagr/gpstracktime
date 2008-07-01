#include <iostream>
#include "Logger.h"

using namespace std;


void main()
{
  GPSLogger logger("logger_settings.txt");

  logger.run_logging();
}