#include "Objects.h"
#include "Util.h"
#include <iostream>
#include <fstream>

using namespace std;


//Global values for aligning data / landmarks
//===================================================================
const Vec2 origin_offset_feet(52003804.092942, 36644507.955591); //origin set in state plane feet

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

  /* Assume file is in the format:
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

  cout<<"Find close to "<<pos<<" - "<<endl;

  for(int i=0; i<(int)data_.size(); i++)
  {
    double d=len(data_[i]-pos);

    if (d<best_dist)
    {
      best_dist=d;
      best_index=i;
    }
  }

  cout<<"["<<best_index<<"] at "<<best_dist<<endl;
}

// test if a point is inside a closed loop
//------------------------------------------------------------------
bool LineStrip::on_inside(Vec2& p)
{
	if (!loop_)
    return false;
	  
  int wn=0;
  Vec2& p0=Vec2();
  Vec2& p1=Vec2();

  for(int i=0; i<(int)data_.size(); i++)
  {
    p0=data_[i];
    if ((i+1)==data_.size())
      p1=data_[0];
    else
      p1=data_[i+1];

    if (p0[1] <= p[1])
    {
      if (p1[1] > p[1])
        if (is_left( p0, p1, p) >= 0 )
          ++wn;
    }
    else
    {                       
      if (p1[1] <= p[1])
        if (is_left( p0, p1, p) <= 0)
          --wn;
    }
  }

  return (wn!=0);
}

// project a point onto this linestrip by finding closest line segment to it
//------------------------------------------------------------------
Vec2 LineStrip::project_onto_linestrip(Vec2& p)
{
  double best_d=999999999.0;
  int best_i=-1;
  int best_j=-1;

  for(int i=0; i<(int)data_.size(); i++)
  {
    int j=i+1;
    if (j==data_.size())
      j=0;

    double d=distance_from_segment(data_[i],data_[j],p);
    if (d<best_d)
    {
      best_d=d;
      best_i=i;
      best_j=j;
    }
  }

  return project_onto(data_[best_i],data_[best_j],p);
}

// get distance from segment to point
//------------------------------------------------------------------
double LineStrip::distance_from_segment(Vec2& p0, Vec2& p1, Vec2 &p)
{  
  return len(project_onto(p0,p1,p)-p);
}
Vec2 LineStrip::project_onto(Vec2 &p0, Vec2 &p1, Vec2 &p)
{
  double l=len(p1-p0);
  
  if (l==0.0f)
    return p0;
  
  double u=((p[0] - p0[0])*(p1[0] - p0[0])+(p[1] - p0[1])*(p1[1] - p0[1])) / (l*l);
  
  if (u<0.0)u=0.0;
  if (u>1.0)u=1.0;

  return p0+u*(p1-p0);  
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


//===================================================================

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
  */
  
  if (parts.size()!=6)
  {
    cout<<"Size is "<<parts.size()<<" on string "<<line<<endl;
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


//===================================================================

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

// set lap time
//------------------------------------------------------------------
void LapData::calc_lap_time()
{
  lap_time_=data_[data_.size()-1].time_-data_[0].time_;
}

// hermite interpolation from http://www.cubic.org/docs/hermite.htm
//------------------------------------------------------------------
void LapData::interpolate(int steps)
{
  std::vector<LapDataPoint> new_data;

  //precompute the basis function
  double* h1=new double[steps];
  double* h2=new double[steps];
  double* h3=new double[steps];
  double* h4=new double[steps];
  for(int t=0; t<steps; t++)
  {
    double s = (double)t / (double)steps;    // scale s to go from 0 to 1

    h1[t] =  2*pow(s,3) - 3*pow(s,2) + 1;          // calculate basis function 1
    h2[t] = -2*pow(s,3) + 3*pow(s,2);              // calculate basis function 2
    h3[t] =   pow(s,3) - 2*pow(s,2) + s;         // calculate basis function 3
    h4[t] =   pow(s,3) -  pow(s,2);              // calculate basis function 4
  }

  for(int i=0; i<(int)data_.size()-1; i++)
  {
    //interpolate from data_[i] up to but not including data_[i+1]

    for(int t=0; t<steps; t++)
    {
      double s = (double)t / (double)steps;    // scale s to go from 0 to 1

      Vec2 pos= data_[i].pos_*h1[t] +
                data_[i+1].pos_*h2[t] +
                data_[i].derived_vel_*h3[t] +
                data_[i+1].derived_vel_*h4[t];
      
      //linearly interpolate other things
      double time=data_[i].time_.get_seconds()+s*(data_[i+1].time_-data_[i].time_).get_seconds();
      double speed=data_[i].speed_+s*(data_[i+1].speed_-data_[i].speed_);
      double bearing=data_[i].bearing_+s*(data_[i+1].bearing_-data_[i].bearing_);

      new_data.push_back(LapDataPoint(time,pos,speed,bearing,Vec2(0,0)));
    }
  }

  delete[] h1;
  delete[] h2;
  delete[] h3;
  delete[] h4;

  new_data.push_back(data_.back()); //include the very last point

  cout<<"went from "<<data_.size()<<" datapoints to ";
  data_=new_data;
  cout<<data_.size()<<endl;

}

// Render the data
//------------------------------------------------------------------
void LapData::render()
{
/*
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
*/
  
  glBegin(GL_LINES);

  Vec3 t;
  Vec3 t2;
  glColor3f(0,1,0);
  
  for(int i=0; i<(int)data_.size()-1; i++)
  {
    t=Vec3(data_[i].pos_[0],0.0,data_[i].pos_[1]);
    //t2=Vec3(data_[i].pos_[0]+data_[i].derived_vel_[0],0.0,data_[i].pos_[1]+data_[i].derived_vel_[1]);
    t2=Vec3(data_[i+1].pos_[0],0.0,data_[i+1].pos_[1]);
    glVertex3dv(t.Ref());
    glVertex3dv(t2.Ref());
  }

  glEnd();  
}


//===================================================================

// Constructor
//------------------------------------------------------------------
TWSData::TWSData()
: left_("./vis_data/tws_data/left_corrected.dat",Vec3(0,0,0)),
  right_("./vis_data/tws_data/right_corrected.dat",Vec3(0,0,0)),
  island_("./vis_data/tws_data/island_corrected.dat",Vec3(0,0,0)),
  start_("./vis_data/tws_data/start_corrected.dat",Vec3(0,0.75f,0),false)
{
  left_.center_on(origin_offset_feet);
  left_.flip_y();
  right_.center_on(origin_offset_feet);
  right_.flip_y();
  island_.center_on(origin_offset_feet);
  island_.flip_y();
  start_.center_on(origin_offset_feet);
  start_.flip_y();

  for(int i=0; i<16; i++)
    tex[i]=0;
}
TWSData::~TWSData()
{
  for(int i=0; i<16; i++)
    glDeleteTextures(1,&tex[i]);
}

// load texture information
//------------------------------------------------------------------
void TWSData::load_textures()
{
  //most of this info was found by experiment and is now hardcoded
  for(int i=0; i<16; i++)
  {
    std::stringstream str;

    str<<"vis_data/tws_data/"<<i<<".tga";

    tex[i]=LoadTexture((char*)str.str().c_str());
  }

  origin_x_=-50.0;
  origin_y_=1240.0;
  angle_=3.37159;
  scale_=1024.0;
  for(int i=0; i<16; i++)
  {    
    angle[i]=0.0;
    scale[i]=2.0;
  }

  offset_x[0]=-7;
  offset_y[0]=3;
  offset_x[1]=6;
  offset_y[1]=-3;
  offset_x[2]=3;
  offset_y[2]=1;
  offset_x[3]=0;
  offset_y[3]=0;
  offset_x[4]=-2;
  offset_y[4]=-1;
  offset_x[5]=16;
  offset_y[5]=-6;
  offset_x[6]=11;
  offset_y[6]=-8;
  offset_x[7]=15;
  offset_y[7]=1;
  offset_x[8]=2;
  offset_y[8]=1;
  offset_x[9]=-4;
  offset_y[9]=-7;
  offset_x[10]=0;
  offset_y[10]=-8;
  offset_x[11]=-3;
  offset_y[11]=-9;
  offset_x[12]=18;
  offset_y[12]=-6;
  offset_x[13]=5;
  offset_y[13]=-23;
  offset_x[14]=1;
  offset_y[14]=-23;
  offset_x[15]=0;
  offset_y[15]=-21;

  angle[0]=0.004;
  angle[1]=-0.008;
  angle[2]=0.0;
  angle[3]=0.0;
  angle[4]=-0.001;
  angle[5]=0.0;
  angle[6]=0.006;
  angle[7]=-0.021;
  angle[8]=0.009;
  angle[9]=0.004;
  angle[10]=-0.014;
  angle[11]=-0.005;
  angle[12]=0.015;
  angle[13]=0.006;
  angle[14]=0.001;
  angle[15]=-0.003;

  scale[10]=1.99798;
  scale[12]=2.02615;
  scale[14]=2.00397;


  double dx_horiz=cos(angle_)*scale_;
  double dx_vert=sin(angle_)*scale_;
  double dy_horiz=cos(angle_+PI_/2.0)*scale_;
  double dy_vert=sin(angle_+PI_/2.0)*scale_;

  for(int i=0; i<16; i++)
  {
    int x_i=(i%4);
    int y_i=(i/4);    

    pos_x[i]=origin_x_+x_i*dx_horiz+y_i*dy_horiz;
    pos_y[i]=origin_y_+x_i*dx_vert+y_i*dy_vert;
  }
}

// load session data from file
//------------------------------------------------------------------
void TWSData::split_raw_session_into_laps(std::string filename, LapDataArray& aligned_laps)
{
  //load data from text file and get rough laps
  LapData raw_sess(filename, Vec3(1.0,0.0,0.0));
  LapDataArray laps_data(split_into_rough_laps(raw_sess));

  //perform alignment on a per lap basis  
  for(int lap=0; lap<(int)laps_data.size(); lap++)
  {
    double error;
    int iter=0;
    Vec2 final_correction(0);

    std::vector<LapDataPoint>& lap_data(laps_data[lap].data_);

    do
    {
      Vec2 correction(0.0,0.0);
      int count=0;
      
      for(int i=0; i<(int)lap_data.size(); i++)
      {
        if (is_outside_right(lap_data[i].pos_)) //calc vector needed to move this point to inside the track
        {
          Vec2 track_p(right_.project_onto_linestrip(lap_data[i].pos_));
          correction+=track_p-lap_data[i].pos_;
          count++;
        }
        else if (is_inside_left(lap_data[i].pos_)) //calc vector needed to move this point to inside the track
        {
          Vec2 track_p(left_.project_onto_linestrip(lap_data[i].pos_));
          correction+=track_p-lap_data[i].pos_;
          count++;
        }
      }
      correction/=count;
      error=len(correction);
      correction*=0.66; //move only most of the way to avoid jumping over the actual solution
      iter++;

      final_correction+=correction;
      
      for(int i=0; i<(int)lap_data.size(); i++)
        lap_data[i].pos_+=correction;
    }
    while(error>0.05 && iter<50);

    cout<<"Final correction was "<<final_correction<<endl;
  }
  
  //now cut each aligned lap to the start/finish line precisely
  for(int lap=0; lap<(int)laps_data.size(); lap++)
  {    
    double t;
    bool done=false;
    std::vector<LapDataPoint>& lap_data(laps_data[lap].data_);

    while(!done) //cut the beginning of the lap
    {
      LapDataPoint& p0=lap_data[0];
      LapDataPoint& p1=lap_data[1];
      if (crosses_finish_line(p0.pos_,p1.pos_,&t))
      {
        //new point is p0+t*(p1-p0) //linearly interpolate everything, TODO: if worrted about less than .1 second accuracy this needs to be changed!
        LapDataPoint np;

        np.time_=PrettyTime( p0.time_.get_seconds()+t*(p1.time_.get_seconds()-p0.time_.get_seconds()) );
        np.pos_=p0.pos_+t*(p1.pos_-p0.pos_);
        np.speed_=p0.speed_+t*(p1.speed_-p0.speed_);
        np.bearing_=p0.bearing_+t*(p1.bearing_-p0.bearing_);
        np.derived_vel_=p0.derived_vel_+t*(p1.derived_vel_-p0.derived_vel_);
        np.valid_=true;

        lap_data[0]=np;

        done=true;
      }
      else
        lap_data.erase(lap_data.begin());
    }
    done=false;
    while(!done) //cut the end of the lap
    {
      LapDataPoint& p0=lap_data[lap_data.size()-2];
      LapDataPoint& p1=lap_data[lap_data.size()-1];
      if (crosses_finish_line(p0.pos_,p1.pos_,&t))
      {
        //new point is p0+t*(p1-p0) //linearly interpolate everything, TODO: if worrted about less than .1 second accuracy this needs to be changed!
        LapDataPoint np;

        np.time_=PrettyTime( p0.time_.get_seconds()+t*(p1.time_.get_seconds()-p0.time_.get_seconds()) );
        np.pos_=p0.pos_+t*(p1.pos_-p0.pos_);
        np.speed_=p0.speed_+t*(p1.speed_-p0.speed_);
        np.bearing_=p0.bearing_+t*(p1.bearing_-p0.bearing_);
        np.derived_vel_=p0.derived_vel_+t*(p1.derived_vel_-p0.derived_vel_);
        np.valid_=true;

        lap_data[lap_data.size()-1]=np;

        done=true;
      }
      else
        lap_data.pop_back();
    }
  }

  for(int lap=0; lap<(int)laps_data.size(); lap++)
  {
    laps_data[lap].calc_lap_time();
    laps_data[lap].interpolate(4); //make the data look smoother at least

    cout<<"Lap "<<lap<<": "<<laps_data[lap].lap_time_<<endl;

    aligned_laps.push_back(laps_data[lap]);
  }
}

//split session data into laps
//------------------------------------------------------------------
LapDataArray TWSData::split_into_rough_laps(LapData& raw_sess)
{
  LapDataArray raw_laps;

  int begin_lap=-1;
  double t;
  std::vector<LapDataPoint>& sdata(raw_sess.data_); //alias to make code more readable

  for(int i=0; i<(int)sdata.size()-1; i++)
  {
    if (crosses_finish_line(sdata[i].pos_, sdata[i+1].pos_, &t))
    {
      if (begin_lap>0)
      {
        raw_laps.push_back(LapData());
        LapData& lap(raw_laps.back());

        //there is a lap from begin_lap to i+1
        int end_lap=((i+1)>=(int)sdata.size()?(int)sdata.size()-1:i+1);
        for(int j=begin_lap; j<=end_lap; j++)
        {
          lap.data_.push_back(sdata[j]);
        }
        
        begin_lap=end_lap-1;
      }
      else
        begin_lap=((i-1)>0?i-1:0);
    }
  }

  cout<<"Split into "<<raw_laps.size()<<" laps"<<endl;

  return raw_laps;
}

//see if this segment crosses the start/finish line
//------------------------------------------------------------------
bool TWSData::crosses_finish_line(Vec2& a1, Vec2& a2, double* t)
{
  if (len(a1-start_line_center)>350.0) //data point has to be within 350 feet of center of track, that is more than any amount of gps drift possible
    return false;

  const Vec2& b1=start_line_left;
  const Vec2& b2=start_line_right;

  Vec2 s1(a2-a1);
  Vec2 s2(b2-b1);

  //double s=(-s1[1]*(a1[0]-b1[0])+s1[0]*(a1[1]-b1[1]))/(-s2[0]*s1[1]+s1[0]*s2[1]);
  (*t)=(s2[0]*(a1[1]-b1[1])-s2[1]*(a1[0]-b1[0]))/(-s2[0]*s1[1]+s1[0]*s2[1]);

  //cout<<"cross finish line got value of "<<*t<<endl;
  return (*t>=0 && *t<=1);
}

// test where a point is located
//------------------------------------------------------------------
bool TWSData::is_inside_left(Vec2& p)
{
  return left_.on_inside(p);
}
bool TWSData::is_outside_right(Vec2& p)
{
  return !right_.on_inside(p);
}

// render
//------------------------------------------------------------------
void TWSData::render_edges()
{
  left_.render();
  right_.render();
  island_.render();
  start_.render();

  /*
  //render just the laps
  glLineWidth(1.0f);  

  Vec3 t;
  

  srand(101);
  
  for(int lap=0; lap<(int)laps_data_.size(); lap++)
  {
    float r=(float)rand()/RAND_MAX;
    float g=(float)rand()/RAND_MAX;
    float b=(float)rand()/RAND_MAX;

    glColor3f(r,g,b);
    glBegin(GL_LINE_STRIP);
  
    for(int i=0; i<(int)laps_data_[lap].size(); i++)
    {
      t=Vec3(laps_data_[lap][i].pos_[0],0.0,laps_data_[lap][i].pos_[1]);
      glVertex3dv(t.Ref());
    }
    glEnd();
  }
  */  
}
void TWSData::render_texture()
{
  glColor3f(1.0f,1.0f,1.0f);

  for(int i=0; i<16; i++)
  {
    glBindTexture(GL_TEXTURE_2D, tex[i]);

    double ang=angle_+angle[i]-PI_/4.0;//-PI_/2.0;
    
    Vec2 p1(pos_x[i]+offset_x[i]+256.0*1.414213562*scale[i]*cos(ang), 
            pos_y[i]+offset_y[i]+256.0*1.414213562*scale[i]*sin(ang));
    ang+=PI_/2.0;
    Vec2 p2(pos_x[i]+offset_x[i]+256.0*1.414213562*scale[i]*cos(ang), 
            pos_y[i]+offset_y[i]+256.0*1.414213562*scale[i]*sin(ang));
    ang+=PI_/2.0;
    Vec2 p3(pos_x[i]+offset_x[i]+256.0*1.414213562*scale[i]*cos(ang), 
            pos_y[i]+offset_y[i]+256.0*1.414213562*scale[i]*sin(ang));
    ang+=PI_/2.0;
    Vec2 p4(pos_x[i]+offset_x[i]+256.0*1.414213562*scale[i]*cos(ang), 
            pos_y[i]+offset_y[i]+256.0*1.414213562*scale[i]*sin(ang));
    ang+=PI_/2.0;    

    glBegin(GL_QUADS);
      glTexCoord2f(1.0f,0.0f); glVertex3f(p1[0], 0.0f, p1[1]);
      glTexCoord2f(1.0f,1.0f); glVertex3f(p2[0], 0.0f, p2[1]);
      glTexCoord2f(0.0f,1.0f); glVertex3f(p3[0], 0.0f, p3[1]);
      glTexCoord2f(0.0f,0.0f); glVertex3f(p4[0], 0.0f, p4[1]);
     glEnd();
  }
}



// 
//------------------------------------------------------------------

