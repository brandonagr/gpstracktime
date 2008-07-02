#include "Objects.h"
#include <iostream>
#include <fstream>

using namespace std;


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

  center_on(Vec2(52003804.092942, 36644507.955591));
  flip_y();
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
void LineStrip::center_on(Vec2& center)
{
  for(int i=0; i<(int)data_.size(); i++)
    data_[i]-=center;
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

  Vec3 t;

  glColor3f(color_[0],color_[1],color_[2]);
  
  for(int i=0; i<(int)data_.size(); i++)
  {
    t=Vec3(data_[i][0],0.0,data_[i][1]);
    glVertex3dv(t.Ref());
  }

  if (loop_)
  {
    t=Vec3(data_[0][0],0.0,data_[0][1]);
    glVertex3dv(t.Ref());
  }

  glEnd();
}
