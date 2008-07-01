// Log sessions

#include "../src/GPSData.h"
#include "../src/Util.h"


// Log gps updates where speed is greater than 35
//=====================================================================
class GPSLogger
{
private:
  Params params_;
  GPSUtil gps_;
  std::vector<std::string> gps_lines_;

public:
  GPSLogger(std::string params_filename);

  void run_logging();
};