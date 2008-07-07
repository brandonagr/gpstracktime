#include "Objects.h"
#include "Util.h"
#include <iostream>
#include <fstream>

using namespace std;


//Global values
const Vec2 origin_offset_feet(52003804.092942, 36644507.955591);


//52004214.579549 36644546.649679 //original positions
//52004151.514680 36644575.508395
//410.486607, -38.694088 //positions aligned with y flipped
//347.421738, -67.552804
const Vec2 start_line_left(347.421738, -67.552804);
const Vec2 start_line_right(410.486607, -38.694088);
const Vec2 start_line_center(378.9541725, -53.123446);


//===================================================================

// Constructor
//------------------------------------------------------------------
LineStrip::LineStrip(std::string filename, Vec3& color, bool loop)
:color_(color),
 loop_(loop)
{ 
  ifstream datafile(filename.c_str());

  if (!datafile.is_open())
    throw AppError("Unable to open LineStrip file!");


  //Assume file is in the format:
  /*
    36644528.208180 52003935.431540 82919.000000
    36644528.208180 52003935.431540 82920.000000
  */
 
  double t,x,y;

  while(!datafile.eof())
  {
    datafile>>y>>x>>t;

    data_.push_back(Vec2(x,y));       
  }

  datafile.close();
}

// get index of closest point
//------------------------------------------------------------------
void LineStrip::print_closest(Vec2& pos)
{
  double best_dist=999999999.0;
  int best_index=-1;

  cout<<"find close to "<<pos<<endl;

  for(int i=0; i<(int)data_.size(); i++)
  {
    double d=len(data_[i]-pos);

    if (d<best_dist)
    {
      best_dist=d;
      best_index=i;
    }
  }

  cout<<best_index+1<<" at "<<best_dist<<endl;
}


// center on average position
//------------------------------------------------------------------
Vec2 LineStrip::get_average()
{
  double x=0.0;
  double y=0.0;
  for(int i=0; i<(int)data_.size(); i++)
  {
    x+=data_[i][0];
    y+=data_[i][1];
  }
  x/=(double)data_.size();
  y/=(double)data_.size();

  return Vec2(x,y);
}
void LineStrip::center_on(const Vec2& pos)
{
  for(int i=0; i<(int)data_.size(); i++)
    data_[i]-=pos;    
}
void LineStrip::flip_y()
{
  for(int i=0; i<(int)data_.size(); i++)
    data_[i][1]=-data_[i][1];
}

// Render the data
//------------------------------------------------------------------
void LineStrip::render()
{
  glBegin(GL_LINE_STRIP);

  Vec3 v;
  
  for(int i=0; i<(int)data_.size(); i++)
  {
    v=Vec3(data_[i][0],0.0,data_[i][1]);
    glVertex3dv(v.Ref());
  }

  if (loop_)
  {
    v=Vec3(data_[0][0],0.0,data_[0][1]);
    glVertex3dv(v.Ref());
  }

  glEnd();
}



// Constructor
//------------------------------------------------------------------
LapDataPoint::LapDataPoint(std::string &line)
:valid_(true)
{
  vector<string> parts(tokenize(line,"\t"));

  //Assume file is in the format:
  /*
    Y	36644806.218739	52004078.937939	63.511518	192.370000	 21:31:12.00
    Y	36644893.090255	52004117.597392	64.363095	193.140000	 21:31:13.00

    Y	36644893.090255	52004117.597392	64.363095	193.140000	 21:31:13.00 derived_vel
    Y	36644893.090255	52004117.597392	64.363095	193.140000	 21:31:13.00
  */
  
  if (parts.size()!=6)
  {
    cout<<"size is "<<parts.size()<<" on string "<<line<<endl;
    throw AppError("Invalid string passed to LapDataPoint constructor");
  }

  if (!parts[1].compare("Y"))
    throw AppError("Invalid data passed to LapDataPoint constructor, beginning of line must be Y");

  pos_=Vec2(convertTo<double>(parts[2]),convertTo<double>(parts[1]));
  pos_-=origin_offset_feet;
  pos_[1]=-pos_[1];

  speed_=convertTo<double>(parts[3]);
  bearing_=convertTo<double>(parts[4]);
  time_.set(parts[5]);
}



// Constructor
//------------------------------------------------------------------
LapData::LapData(std::string filename, Vec3& color)
:color_(color)
{ 
  ifstream datafile(filename.c_str());

  if (!datafile.is_open())
    throw AppError("Unable to open LapData file!");
 
  std::string line;

  while(!datafile.eof())
  {
    getline(datafile,line);
    data_.push_back(LapDataPoint(line));       
  }

  datafile.close();

  if (data_.size()<3)
    throw AppError("Not enough points loaded in LapData!");

  for(int i=1; i<(int)data_.size()-1; i++)
  {
    //derive
    data_[i].derived_vel_=Vec2(0,0);

    data_[i].derived_vel_+=data_[i+1].pos_-data_[i].pos_;
    data_[i].derived_vel_+=data_[i].pos_-data_[i-1].pos_;

    data_[i].derived_vel_*=0.5;
  }
  data_[0].derived_vel_=data_[1].derived_vel_;
  data_[data_.size()-1].derived_vel_=data_[data_.size()-2].derived_vel_;
}

// get index of closest point
//------------------------------------------------------------------
void LapData::print_closest(Vec2& pos)
{
  double best_dist=999999999.0;
  int best_index=-1;

  cout<<"find close to "<<pos<<endl;

  for(int i=0; i<(int)data_.size(); i++)
  {
    double d=len(data_[i].pos_-pos);

    if (d<best_dist)
    {
      best_dist=d;
      best_index=i;
    }
  }

  cout<<best_index+1<<" at "<<best_dist<<endl;
}


// center on average position
//------------------------------------------------------------------
Vec2 LapData::get_average()
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
void LapData::move(Vec2& dx)
{
  cout<<"move "<<dx<<endl;
  for(int i=0; i<(int)data_.size(); i++)
    data_[i].pos_+=dx;
}


// Render the data
//------------------------------------------------------------------
void LapData::render()
{
  glBegin(GL_POINTS);

  Vec3 t;
  glPointSize(3.0f);
  glColor3f(color_[0],color_[1],color_[2]);
  
  for(int i=0; i<(int)data_.size(); i++)
  {
    t=Vec3(data_[i].pos_[0],0.0,data_[i].pos_[1]);
    glVertex3dv(t.Ref());
  }

  glEnd();

  
  glBegin(GL_LINES);

  Vec3 t2;
  glColor3f(0,1,0);
  
  for(int i=0; i<(int)data_.size(); i++)
  {
    t=Vec3(data_[i].pos_[0],0.0,data_[i].pos_[1]);
    t2=Vec3(data_[i].pos_[0]+data_[i].derived_vel_[0],0.0,data_[i].pos_[1]+data_[i].derived_vel_[1]);
    glVertex3dv(t.Ref());
    glVertex3dv(t2.Ref());
  }

  glEnd();
  
}


//===================================================================

// Constructor
//------------------------------------------------------------------
TWSData::TWSData(std::string filename)
: left_("./vis_data/track_data/left_corrected.dat",Vec3(0,0,0)),
  right_("./vis_data/track_data/right_corrected.dat",Vec3(0,0,0)),
  island_("./vis_data/track_data/island_corrected.dat",Vec3(0,0,0)),
  start_("./vis_data/track_data/start_corrected.dat",Vec3(0,0.75f,0),false)
{
  left_.center_on(origin_offset_feet);
  left_.flip_y();
  right_.center_on(origin_offset_feet);
  right_.flip_y();
  island_.center_on(origin_offset_feet);
  island_.flip_y();
  start_.center_on(origin_offset_feet);
  start_.flip_y();

/*
  load_session_data_from_file(filename);
  split_into_laps();
*/
}
TWSData::~TWSData()
{
  
}

// load either entire session or individual lap data
//------------------------------------------------------------------
void TWSData::load_data(std::string &filename)
{
  if (filename.substr(-4)==".txt")
    load_session_data_from_file(filename);
  if (filename.substr(-5)==".laps")
    load_lap_data_from_file(filename);

  cout<<"Unknown file format, not loading anything..."<<endl;
}


// load session data from file
//------------------------------------------------------------------
void TWSData::load_session_data_from_file(std::string& filename)
{
  //load data from session data file
  ifstream datafile(filename.c_str());

  if (!datafile.is_open())
    throw AppError("Unable to open LapData file!");
 
  std::string line;

  while(!datafile.eof())
  {
    getline(datafile,line);
    sess_data_.push_back(LapDataPoint(line));       
  }

  datafile.close();

  if (sess_data_.size()<3)
    throw AppError("Not enough points loaded in LapData!");

  for(int i=1; i<(int)sess_data_.size()-1; i++)
  {
    //derive
    sess_data_[i].derived_vel_=Vec2(0,0);

    sess_data_[i].derived_vel_+=sess_data_[i+1].pos_-sess_data_[i].pos_;
    sess_data_[i].derived_vel_+=sess_data_[i].pos_-sess_data_[i-1].pos_;

    sess_data_[i].derived_vel_*=0.5;
  }
  sess_data_[0].derived_vel_=sess_data_[1].derived_vel_;
  sess_data_[sess_data_.size()-1].derived_vel_=sess_data_[sess_data_.size()-2].derived_vel_;
}

//split session data into laps
//------------------------------------------------------------------
void TWSData::split_into_laps()
{
  int begin_lap=-1;

  for(int i=0; i<(int)sess_data_.size()-1; i++)
  {
    if (crosses_finish_line(sess_data_[i].pos_, sess_data_[i+1].pos_))
    {
      if (begin_lap>0)
      {
        //there is a lap from begin_lap to i+1
        std::vector<LapDataPoint> lap;
        int end_lap=((i+1)>=(int)sess_data_.size()?(int)sess_data_.size()-1:i+1);
        for(int j=begin_lap; j<=end_lap; j++)
        {
          lap.push_back(sess_data_[j]);
        }
        laps_data_.push_back(lap);
      }
      else
        begin_lap=((i-1)>0?i-1:0);
    }
  }

  cout<<"Split into "<<laps_data_.size()<<" laps"<<endl;
}

//see if this segment crosses the start/finish line
//------------------------------------------------------------------
bool TWSData::crosses_finish_line(Vec2 &a1, Vec2 &a2)
{
  if (len(a1-start_line_center)>350.0) //data point has to be within 350 feet of center of track, that is more than any amount of gps drift possible
    return false;

  const Vec2& b1=start_line_left;
  const Vec2& b2=start_line_right;

  Vec2 s1(a2-a1);
  Vec2 s2(b2-b1);

  //float s=(-s1[1]*(a1[0]-b1[0])+s1[0]*(a1[1]-b1[1]))/(-s2[0]*s1[1]+s1[0]*s2[1]);
  float t=(s2[0]*(a1[1]-b1[1])-s2[1]*(a1[0]-b1[0]))/(-s2[0]*s1[1]+s1[0]*s2[1]);

  return (t>=0 && t<=1);
}


// load laps data from file
//------------------------------------------------------------------
void TWSData::load_lap_data_from_file(std::string &filename)
{
  cout<<"Not implemented!"<<endl;
}


// render all the data
//------------------------------------------------------------------
void TWSData::render()
{
  left_.render();
  right_.render();
  island_.render();
  start_.render();
}

