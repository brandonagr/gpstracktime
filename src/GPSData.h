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

  
  enum GPGGA {GGA_TIME=1,
              GGA_LATITUDE,
              GGA_LAT_DIR,
              GGA_LONGITUDE,
              GGA_LONG_DIR,
              GGA_VALID,
              GGA_NUMB_SATS,
              GGA_HDOP,
              GGA_ALTITUDE,
              GGA_ALT_UNITS,
              GGA_GEOID,
              GGA_GEOID_UNITS,
              GGA_BLANK,
              GGA_CHECKSUM};
  
  enum GPRMC {RMC_TIME=1,
              RMC_SAT_FIX_STATUS,
              RMC_LATITUDE,
              RMC_LAT_DIR,
              RMC_LONGITUDE,
              RMC_LON_DIR,
              RMC_SPEED,
              RMC_BEARING,
              RMC_DATE,
              RMC_CHECKSUM};

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
