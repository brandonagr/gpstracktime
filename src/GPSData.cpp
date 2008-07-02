#include <iostream>
#include <fstream>
#include "GPSData.h"

using namespace std;

//----------------------------------------------------------------
GPSData::GPSData()
{
  valid_=false;
}

//----------------------------------------------------------------
GPSData::GPSData(string& gpsstring) //expects a valid $GPRMC NMEA string
  :valid_(false)
{    
  vector<string> parts=tokenize(gpsstring,",");

  if (parts[GPSData::RMC_SAT_FIX_STATUS]!="A")
  {
    valid_=false;
    return;
  }

  time_.set(parts[GPSData::RMC_TIME],false);

  lat_=convertTo<double>(parts[GPSData::RMC_LATITUDE].substr(0,2))+convertTo<double>(parts[GPSData::RMC_LATITUDE].substr(2))/60.0;  
  lon_=convertTo<double>(parts[GPSData::RMC_LONGITUDE].substr(0,3))+convertTo<double>(parts[GPSData::RMC_LONGITUDE].substr(3))/60.0;

  speed_=convertTo<double>(parts[GPSData::RMC_SPEED])*1.15077945; //convert knots to miles/hour  
  bearing_=convertTo<double>(parts[GPSData::RMC_BEARING]);

  calc_state();
}

//----------------------------------------------------------------
double GPSData::distance(const GPSData& opos) //returns distance in feet
{
  if (!valid_ || !opos.valid_)
  {
    std::cout<<"Position is not valid and trying to calc distance!"<<std::endl;
    return 0;
  }

  double dx=state_x_-opos.state_x_;
  double dy=state_y_-opos.state_y_;
  return sqrt(dx*dx+dy*dy);

  /* 
  //this formula has almost 5% error in distance calculation
  double ladif=(latitude_ - opos.latitude_) * 111300.0; //111300 m/deg
  double lodif=(longitude_ - opos.longitude_) * 85300.0; //85300 m/deg
  double aldif=(altitude_ - opos.altitude_);

  return sqrt(ladif*ladif + lodif*lodif + aldif*aldif);
  */
  
 
  /*
  //Basic arcsin halfangle distance formula
  return 6386847.0 * 2.0 * 
    asin( 
     sqrt(
      pow((sin((latitude_-opos.latitude_)/2)),2) + cos(latitude_)*cos(opos.latitude_)*pow(sin((longitude_-opos.longitude_)/2),2)
     ) 
    );
  */

  /*
  //Longer trig calculation is only about 0.000004 meters more accurate over 1000 measurements
  return 6380795.0 * 
   atan( 
    sqrt( pow(cos(opos.latitude_)*sin(longitude_-opos.longitude_),2) +
          pow( (cos(latitude_)*sin(opos.latitude_)) - (sin(latitude_)*cos(opos.latitude_)*cos(longitude_-opos.longitude_)) ,2)
         )
    /(
      (sin(latitude_)*sin(opos.latitude_)) +
      (cos(latitude_)*cos(opos.latitude_)*cos(longitude_-opos.longitude_))
      ));
  */
}  

//----------------------------------------------------------------
void GPSData::calc_state()
{
// Code to convert from decimal degrees to state plan.
// Code based on asp code from Gerry Daumiller, Montana State Library 8-23-02 email. ASP code on web at http://nris.state.mt.us/gis/projection/projection.html.  
//Translated to javascript by Jeff Miller, Jefferson County, WA and April Lafferty, Thurston County, WA.

// The formulae this program is based on are from "Map Projections,
// A Working Manual" by John P. Snyder, U.S. GeoLogical Survey
// Professional Paper 1395, 1987, pages 295-298

// Set up the coordinate system parameters.  This for WA South and US Survey Feet.
// Note!! this has been changed to Nad83 Texas Central in feet. (Christopher Graff 1/4/2007)
// Code found on forums at www.esri.com and heavily modified by Brett Blankner 1/16/2007.  It handles decimal degrees, degrees/minutes/seconds,
// and Texas State Plane Central Zone and sends URL to City of College Station's IMS services.

  static double a = 20925604.48;   		//major radius of ellipsoid, map units (NAD 83)
  static double ec = 0.08181922146;  		//eccentricity of ellipsoid (NAD 83)
  static double angRad = 0.017453292519943295769236907684886;  	//number of radians in a degree
  static double pi4 = 0.78539816339744830961566084581988;  		//Pi / 4
  static double p0 = 29.6666666666666666666666666666667 * 0.017453292519943295769236907684886; 	 	//latitude of origin
  static double p1 = 30.1166666666666666666666666666667 * 0.017453292519943295769236907684886;  		//latitude of first standard parallel
  static double p2 = 31.8833333333333333333333333333333 * 0.017453292519943295769236907684886;  	//latitude of second standard parallel
  static double m0 = -100.333333333333333333333333333333 * 0.017453292519943295769236907684886;  	//central meridian
  static double x0 = 2296583.33333333333333333333333333; 		//False easting of central meridian, map units
  static double y0 = 9842500;			//False northing of central meridian, map units

  // Calculate the coordinate system constants.
  double m1 = cos(p1) / sqrt(1 - (pow(ec,2)) * pow(sin(p1),2));  
  double m2 = cos(p2) / sqrt(1 - (pow(ec,2)) * pow(sin(p2),2));
  double t0 = tan(pi4 - (p0 / 2));
  double t1 = tan(pi4 - (p1 / 2));
  double t2 = tan(pi4 - (p2 / 2));
  t0 = t0 / pow(((1 - (ec * (sin(p0)))) / (1 + (ec * (sin(p0))))),ec/2);  
  t1 = t1 / pow(((1 - (ec * (sin(p1)))) / (1 + (ec * (sin(p1))))),ec/2);
  t2 = t2 / pow(((1 - (ec * (sin(p2)))) / (1 + (ec * (sin(p2))))),ec/2);
  double n = log(m1 / m2) / log(t1 / t2);
  double f = m1 / (n * pow(t1,n)); 
  double rho0 = a * f * pow(t0,n);


  //Convert the latitude/longitude to a coordinate.
  double latd = lat_ * angRad;
  double lond = lon_ * angRad;
  double t = tan(pi4 - (latd / 2));
  //alert(t);
  t = t / pow(((1 - (ec * (sin(latd)))) / (1 + (ec * (sin(latd))))),ec/2);
  double rho = a * f * pow(t,n);
  double theta = n * (lond - m0);
  
  state_x_ = (rho * sin(theta)) + x0;
  state_y_ = rho0 - (rho * cos(theta)) + y0;

  valid_=true;
}

//----------------------------------------------------------------
ostream& operator << (ostream& os, GPSData& pos)
{
  os.setf(ios_base::fixed);
  os<<(pos.valid_?"Y":"N")<<"\t"<<pos.state_x_<<"\t"<<pos.state_y_<<"\t"<<pos.speed_<<"\t"<<pos.bearing_<<"\t"<<pos.time_;  

  return os;
}




//================================================================
GPSUtil::GPSUtil(Params& params)
:replay_(false),
 cur_(0)
{
  cout<<"Initializing GPS Connection..."<<endl;


  fstream fin(params.get<std::string>("GPSReplayFile").c_str(),ios::in);
  if (fin.is_open()) //load replay
  {
    replay_=true;

    std::string line;
    while(!fin.eof())
    {    
      getline(fin,line);
    
      if (GPSData::is_GPRMC(line)) //skip blank lines
        replay_data_.push_back(line);
    }

    cout<<"Successfully loaded GPS replay of length "<<replay_data_.size()<<endl;

    fin.close();
  }
  else //use actual gps
  {
    try
    {
      if (!gpsport_.OpenPort(params.get<std::string>("GPSPort")))
        throw(exception("Failed to open port!"));
      if (!gpsport_.ConfigurePort(CBR_4800,8,true,NOPARITY,ONESTOPBIT))
        throw(exception("Failed to configure port!"));
      if (!gpsport_.SetCommunicationTimeouts(500,100,1,0,0))
        throw(exception("Failed to set timeouts on port!"));
    }
    catch(exception e)
    {
      std::cout<<"Failed to open com port: "<<e.what()<<std::endl;

    }

    
  }
}

//----------------------------------------------------------------
GPSUtil::~GPSUtil()
{
  if (!replay_)
    gpsport_.ClosePort();

}

//----------------------------------------------------------------
std::string GPSUtil::get_gps_line(double dt)
{
  string linedata="";

  if (replay_)
  {
    static double time_delay=0;

    time_delay+=dt;

    GPSData cur(replay_data_[cur_]);
    GPSData next(replay_data_[cur_+1]);    
    double delay=next.time_.get_seconds()-cur.time_.get_seconds();

    if (time_delay>delay)
    {
      time_delay-=delay;

      linedata=replay_data_[cur_++];
    }
  }
  else
  {
    BYTE data;
      
    if (gpsport_.ReadByte(data)) //as soon as one byte is ready assume whole string is ready
    {
      if (data!='\r' && data!='\n')
        linedata+=data;
      do
      {
        if(gpsport_.ReadByte(data))
        {
          if (data!='\r' && data!='\n')
           linedata+=data;
        }
        else
          data=0;
      }
      while(data!='\n');
    }
  }

  return linedata;
}


/*
Code to convert a gps log into a list of just the state plan coords
  ifstream left("raw_gps_data/start.txt");
  ofstream left_out("raw_gps_data/start state.dat");
  left_out.setf(ios::fixed);
  string line;
  while(!left.eof())
  {
    getline(left,line);

    if (GPSData::is_GPRMC(line))
    {
      GPSData d(line);
      left_out<<d.state_x_<<" "<<d.state_y_<<" "<<d.time_.get_seconds()<<endl;
    }
  }
  left_out.close();
  left.close();
*/