#ifndef GPS_POS_H_
#define GPS_POS_H_

#include <math.h>
#include <vector>
#include <string>
using namespace std;

#include "Util.h"
#include "SerialPort.h"


// Holds individual GPS update
//=====================================================================
struct GPSData
{
public:  
  bool valid_; //flag that says if this is a valid position

  double lat_; //both stored in degrees
  double lon_;
  double state_x_, state_y_; //stored in feet, derived from lat/long

  PrettyTime time_;
  double speed_;
  double bearing_;

  /*
  enum GPGGA {TIME=1,
              LATITUDE,
              LAT_DIR,
              LONGITUDE,
              LONG_DIR,
              VALID,
              NUMB_SATS,
              HDOP,
              ALTITUDE,
              ALT_UNITS,
              GEOID,
              GEOID_UNITS,
              BLANK,
              CHECKSUM};
  */

  enum GPRMC {TIME=1,
              SAT_FIX_STATUS,
              LATITUDE,
              LAT_DIR,
              LONGITUDE,
              LON_DIR,
              SPEED,
              BEARING,
              DATE,
              CHECKSUM};

public:
  GPSData();
  GPSData(string& gpsstring);

  double distance(const GPSData& opos);
  void calc_state();

  static inline bool is_GPRMC(string& gpsstring)
  {
    return (gpsstring.substr(0,6)=="$GPRMC");
  }
  static inline bool is_GPGGA(string& gpsstring)
  {
    return (gpsstring.substr(0,6)=="$GPGGA");
  }
};
ostream& operator << (ostream& os, GPSData& pos);

// Gets GPS updates or plays replay
//=====================================================================
class GPSUtil
{
private:
  bool replay_;
  int cur_;
  std::vector<std::string> replay_data_;

  CSerialPort gpsport_;


public:
  GPSUtil(Params& params);
  ~GPSUtil();

  std::string get_gps_line(double dt);
};


#endif
