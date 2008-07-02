#include <iostream>
#include "Logger.h"

using namespace std;


void main()
{
  /*
  GPSLogger logger("logger_settings.txt");

  logger.run_logging();
  */
  
  GPSData point1(std::string("$GPRMC,230119.000,A,3032.2971,N,09613.3961,W,0.00,,280608,,,A*61"));
  GPSData point2(std::string("$GPRMC,232337.000,A,3032.2960,N,09613.3967,W,0.00,,280608,,,A*6B"));

  //drift of .005510769 feet per second on avg, over 3 min that's 1 foot

  
  //GPSData point2(std::string("$GPRMC,133434.000,A,3032.2985,N,09613.3991,W,0.00,,290608,,,A*6E"));
  //GPSData point2(std::string("$GPRMC,175653.000,A,3032.2967,N,09613.3959,W,0.00,,290608,,,A*67"));
  //GPSData point2(std::string("$GPRMC,225029.000,A,3032.2982,N,09613.3989,W,0.00,,290608,,,A*6C"));
  

  cout<<"the distance between them is "<<point1.distance(point2)<<endl;  
  cout<<"the time between them is "<<(point2.time_-point1.time_).get_seconds()<<endl;
  cout<<point1<<endl<<point2<<endl;
  cout<<point1.state_x_<<" "<<point1.state_y_<<" "<<point1.time_.get_seconds()<<endl;
  cout<<point2.state_x_<<" "<<point2.state_y_<<" "<<point2.time_.get_seconds()<<endl;
}