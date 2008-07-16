#ifndef OBJECTS_H_
#define OBJECTS_H_

#include <string>
#include <vector>
#include "Util.h"
#include "GlUtil.h"

//need to load all of the lap data
//split into laps
//save lap data into files
//should each lap be inside a class


// l - load lap data
// s - load a session, split into laps, auto align laps, save summary data

//check for crossing finish line:
// if point is within 300 feet of center of finish


// different class to hold session data and to hold laps loaded from file?




// series of datapoints making up a line strip
//===================================================================
class LineStrip
{
private:
  std::vector<Vec2> data_;
  Vec3 color_;
  bool loop_;

public:
  LineStrip(std::string filename, Vec3& color, bool loop=true);

  void print_closest(Vec2& pos);

  bool on_inside(Vec2& p);
  inline double is_left(Vec2& p0, Vec2& p1, Vec2& p2){return ((p1[0] - p0[0]) * (p2[1] - p0[1]) - (p2[0] - p0[0]) * (p1[1] - p0[1]));}
  inline Vec2 project_onto(Vec2& p0, Vec2& p1, Vec2& p);

  Vec2 project_onto_linestrip(Vec2& p);
  double distance_from_segment(Vec2& p0, Vec2& p1, Vec2& p);

  Vec2 get_average();
  void center_on(const Vec2& pos);
  void flip_y();

  void render();
};


//A waypoint, an absolute time and position the car was located at
//===================================================================
struct LapDataPoint
{
  bool valid_;

  PrettyTime time_;
  Vec2 pos_;
  double speed_;
  double bearing_;

  //derived value
  Vec2 derived_vel_;

  LapDataPoint(std::string& line);
  LapDataPoint(double time, Vec2& pos, double& speed, double& bearing, Vec2& dvel)
    :valid_(true),
     time_(time),
     pos_(pos),
     speed_(speed),
     bearing_(bearing),
     derived_vel_(dvel)
  {}
  LapDataPoint()
    :valid_(false)
  {}

};


//Holds all of the data for a given run
//===================================================================
class LapData
{
private:
  std::vector<LapDataPoint> data_;
  Vec3 color_;

public:
  LapData(std::string filename, Vec3& color);

  void print_closest(Vec2& pos);

  Vec2 get_average();

  void move(Vec2& dx);

  void render();
};


//Holds all of the data for a session and then splits it into laps
//===================================================================
class TWSData
{
private:
  std::vector<LapDataPoint> sess_data_;
  std::vector<std::vector<LapDataPoint> > laps_data_;

  LineStrip left_;
  LineStrip right_;
  LineStrip island_;
  LineStrip start_;

  bool is_outside_right(Vec2& p);
  bool is_inside_left(Vec2& p); 

  

  //load and setup the texture to use somewhere in here as well

public:
  TWSData(std::string filename);
  ~TWSData();

  void test(Vec2& p)
  {
    //std::cout<<"on_inside left: "<<left_.on_inside(p)<<std::endl;
    //std::cout<<"on_inside right: "<<right_.on_inside(p)<<std::endl;
    Vec2 track_p(right_.project_onto_linestrip(p));

    std::cout<<"old p "<<p<<" new p "<<track_p<<" dist "<<len(track_p-p)<<std::endl;
  }

  void load_data(std::string& filename);

  void load_session_data_from_file(std::string& filename);  
  void split_into_laps();
  bool crosses_finish_line(Vec2& a1, Vec2& a2);

  void load_lap_data_from_file(std::string& filename);

  void render();
};


#endif