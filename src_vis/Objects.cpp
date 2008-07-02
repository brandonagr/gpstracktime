#include "Objects.h"
#include <iostream>
#include <fstream>

using namespace std;


// Constructor
//------------------------------------------------------------------
RunData::RunData(std::string filename)
:last_val_(0)
{ 
  color_[0]=(double)rand()/(double)RAND_MAX;
  color_[1]=(double)rand()/(double)RAND_MAX;
  color_[2]=(double)rand()/(double)RAND_MAX;



  ifstream datafile(filename.c_str());

  if (!datafile.is_open())
    throw AppError("Unable to open rundata file!");


  //Assume file is in the format:
  /*
    160631.9	3037.9406	9628.7604
    160632.2	3037.9406	9628.7604
    160632.3	3037.9406	9628.7605
    160632.6	3037.9407	9628.7605
  */
 
  double first_t;

  double t;
  double lat,lon;
  double degrees,minutes,seconds;

  while(datafile.good())
  {
    datafile>>t>>lat>>lon;

    if (datafile.eof())
      break;

    minutes=floor(t/100.0);
    seconds=t-(minutes*100.0);
    t=seconds+minutes*60.0;

    if ((int)data_.size()==0)
      first_t=t;

    //convert degrees minutes into degrees
    degrees=floor(lat/100.0);
    minutes=lat-(degrees*100.0);
    lat=degrees+minutes/60.0;

    degrees=floor(lon/100.0);
    minutes=lon-(degrees*100.0);
    lon=-(degrees+minutes/60.0);

    data_.push_back(RunDataPoint(t-first_t,convert_lat_lon_to_feet(lat,lon)));       
  }

  /*
  for(int i=0; i<(int)data_.size(); i++)
    cout<<i<<", "<<data_[i].time_<<", ["<<data_[i].pos_[0]<<", "<<data_[i].pos_[1]<<"]"<<endl;
    */

  derive_data();

  /*
  for(int i=0; i<(int)data_.size(); i++)
    cout<<i<<", "<<data_[i].time_<<", ["<<data_[i].pos_[0]<<", "<<data_[i].pos_[1]<<"]"<<endl;
    */

  datafile.close();
}

// utility code to convert lat/lon into feet
//------------------------------------------------------------------
Vec2 RunData::convert_lat_lon_to_feet(double lat, double lon)
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
  static double p0 = 29.6666666666666666666666666666667 * angRad; 	 	//latitude of origin
  static double p1 = 30.1166666666666666666666666666667 * angRad;  		//latitude of first standard parallel
  static double p2 = 31.8833333333333333333333333333333 * angRad;  	//latitude of second standard parallel
  static double m0 = -100.333333333333333333333333333333 * angRad;  	//central meridian
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
  double latd = lat * angRad;
  double lond = lon * angRad;
  double t = tan(pi4 - (latd / 2));
  //alert(t);
  t = t / pow(((1 - (ec * (sin(latd)))) / (1 + (ec * (sin(latd))))),ec/2);
  double rho = a * f * pow(t,n);
  double theta = n * (lond - m0);
  
  Vec2 p;

  p[0]=(rho * sin(theta)) + x0;
  p[1]=rho0 - (rho * cos(theta)) + y0;

  p[1]=-p[1];

  return p;
}

// derive data
//------------------------------------------------------------------
void RunData::derive_data()
{
  max_vel_=0.0;
  max_t_=0.0;
  for(int i=1; i<(int)data_.size()-1; i++)
  {
    Vec2 vel(0);
    
    vel+=(data_[i].pos_-data_[i-1].pos_)/(data_[i].time_-data_[i-1].time_);
    vel+=(data_[i+1].pos_-data_[i].pos_)/(data_[i+1].time_-data_[i].time_);

    data_[i].vel_=vel*0.5f;

    if (len(data_[i].vel_)>max_vel_)
      max_vel_=len(data_[i].vel_);
    if (data_[i].time_>max_t_)
      max_t_=data_[i].time_;
  }

  for(int i=1; i<(int)data_.size()-1; i++)
  {
    Vec2 acc(0);
    
    acc+=(data_[i].vel_-data_[i-1].vel_)/(data_[i].time_-data_[i-1].time_);
    acc+=(data_[i+1].vel_-data_[i].vel_)/(data_[i+1].time_-data_[i].time_);

    data_[i].acc_=acc*0.5f;
  }
}

// center on average position
//------------------------------------------------------------------
Vec2 RunData::get_average()
{
  double x=0.0;
  double y=0.0;
  for(int i=0; i<(int)data_.size(); i++)
  {
    x+=data_[i].pos_[0];
    y+=data_[i].pos_[1];
  }
  x/=(double)data_.size();
  y/=(double)data_.size();

  return Vec2(x,y);
}
void RunData::center_on(Vec2& center)
{
  for(int i=0; i<(int)data_.size(); i++)
    data_[i].pos_-=center;
}

// find waypoint 
//------------------------------------------------------------------
int RunData::find_datapoint_preceding(double t)
{
  int previous=last_val_;  


  if (previous-1>0 && data_[previous-1].time_>=t)
    previous=0;


  bool done=false;
  for(int i=0, s=last_val_; !done && i<(int)data_.size(); s++,i++)
  {
    if (s>=(int)data_.size())
      s=0;

    if (data_[s].time_>=t)    
      done=true;
    else
      previous=s;
  }

  last_val_=previous;

  if (!done) //didn't find one, return last position
  {
    last_val_=0;
    return 0;
  }


  return previous;
}


// Get where the car is at a certain time
//------------------------------------------------------------------
Vec2 RunData::get_pos_at_t(double t)
{
  int prev=find_datapoint_preceding(t);
  
  Vec2 dir=data_[prev+1].pos_-data_[prev].pos_;
  double len=data_[prev+1].time_-data_[prev].time_;
  double dif=t-data_[prev].time_;

  return data_[prev].pos_+(dif/len)*dir;
}
Vec2 RunData::get_vel_at_t(double t)
{
  int prev=find_datapoint_preceding(t);
  
  Vec2 dir=data_[prev+1].vel_-data_[prev].vel_;
  double len=data_[prev+1].time_-data_[prev].time_;
  double dif=t-data_[prev].time_;

  return data_[prev].vel_+(dif/len)*dir;
}
Vec2 RunData::get_acc_at_t(double t)
{
  int prev=find_datapoint_preceding(t);
  
  Vec2 dir=data_[prev+1].acc_-data_[prev].acc_;
  double len=data_[prev+1].time_-data_[prev].time_;
  double dif=t-data_[prev].time_;

  return data_[prev].acc_+(dif/len)*dir;
}

// Get when the car is closest to a given point
//------------------------------------------------------------------
double RunData::get_t_near_position(Vec2& pos)
{
  int close_index=0;
  double close_dist=9999999.0;

  for(int i=0; i<(int)data_.size(); i++)
  {
    if (len(data_[i].pos_-pos)<close_dist)
    {
      close_dist=len(data_[i].pos_-pos);
      close_index=i;
    }
  }

  //now know closest node, need to find closest point on line for previous two sections

  int prev_index=close_index-1;
  int next_index=close_index+1;
  if (prev_index<0)prev_index=0;
  if (next_index>=(int)data_.size())next_index=close_index;

  Vec2 close_prev,close_next;
  double u_prev, u_next;
  Vec2 p0,p1;
  double l;


  //do prev_index to close_index
  p0=data_[prev_index].pos_;
  p1=data_[close_index].pos_;

  l=len(p1-p0);
  if (l==0.0)
  {
    u_prev=1.0;
    close_prev=data_[close_index].pos_;
  }
  else
  {
    u_prev=((pos[0]-p0[0])*(p1[0]-p0[0])+(pos[1]-p0[1])*(p1[1]-p0[1]))/(l*l);
    if (u_prev<0.0)u_prev=0.0;
    if (u_prev>1.0)u_prev=1.0;

    close_prev=p0+u_prev*(p1-p0);
  }

  //do prev_index to close_index
  p0=data_[close_index].pos_;
  p1=data_[next_index].pos_;

  l=len(p1-p0);
  if (l==0.0)
  {
    u_next=0.0;
    close_next=data_[close_index].pos_;
  }
  else
  {
    u_next=((pos[0]-p0[0])*(p1[0]-p0[0])+(pos[1]-p0[1])*(p1[1]-p0[1]))/(l*l);
    if (u_next<0.0)u_next=0.0;
    if (u_next>1.0)u_next=1.0;

    close_next=p0+u_next*(p1-p0);
  }

  if (len(close_prev-pos)<len(close_next-pos)) //use close_prev
    return data_[prev_index].time_+u_prev*(data_[close_index].time_-data_[prev_index].time_);
  else //use close_next
    return data_[close_index].time_+u_next*(data_[next_index].time_-data_[close_index].time_);
}

// Render the data
//------------------------------------------------------------------
void RunData::render()
{
  glBegin(GL_LINE_STRIP);

  Vec3 t;
  
  for(int i=0; i<(int)data_.size(); i++)
  {
    double color_percent=1.0-(max_vel_-len(data_[i].vel_))/max_vel_;
    glColor3f(color_percent,color_percent,color_percent);
    //cout<<color_percent<<endl;


    t=Vec3(data_[i].pos_[0],0.0,data_[i].pos_[1]);
    glVertex3dv(t.Ref());
  }

  glEnd();
}

