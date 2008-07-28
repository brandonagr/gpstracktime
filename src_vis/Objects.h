#ifndef OBJECTS_H_
#define OBJECTS_H_

#include <string>
#include <vector>
#include "Util.h"
#include "GlUtil.h"


// series of datapoints making up a line strip
//===================================================================
class LineStrip
{
private:
  std::vector<Vec2> data_;
  Vec3 color_;
  bool loop_;

  inline double is_left(Vec2& p0, Vec2& p1, Vec2& p2){return ((p1[0] - p0[0]) * (p2[1] - p0[1]) - (p2[0] - p0[0]) * (p1[1] - p0[1]));}
  inline Vec2 project_onto(Vec2& p0, Vec2& p1, Vec2& p);
  double distance_from_segment(Vec2& p0, Vec2& p1, Vec2& p);

public:
  LineStrip(std::string filename, Vec3& color, bool loop=true);  

  bool on_inside(Vec2& p);  
  Vec2 project_onto_linestrip(Vec2& p);
  
  void print_closest(Vec2& pos);

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


//Holds all of the data for a single lap
//===================================================================
class LapData
{
protected:
  std::vector<LapDataPoint> data_;
  PrettyTime lap_time_;
  Vec3 color_;

  //utility functions used by TWSData when constructing LapDatas
  void calc_lap_time();
  void interpolate(int steps);

public:
  LapData(std::string filename, Vec3& color);
  LapData()
    :color_(1.0, 0.0, 0.0)
  {};
 
  void render();


  friend class TWSData;
};

//Hold a bunch of laps in just an array
//===================================================================
typedef std::vector<LapData> LapDataArray;

//Holds all data for TWS and is used to split up session data into laps
//===================================================================
class TWSData
{
private:
  LineStrip left_;
  LineStrip right_;
  LineStrip island_;
  LineStrip start_;

// Texture information, for rendering track
  GLuint tex[16];
  double origin_x_;
  double origin_y_;
  double angle_;
  double scale_;

  double pos_x[16];
  double pos_y[16];

  double offset_x[16];
  double offset_y[16];
  double angle[16];
  double scale[16];
//-------------------------------  

  //utility functions for testing if a point is inside the track
  bool is_outside_right(Vec2& p);
  bool is_inside_left(Vec2& p); 
  
  //utility functions used to split raw session into laps
  LapDataArray split_into_rough_laps(LapData& raw_sess);
  bool crosses_finish_line(Vec2& a1, Vec2& a2, double* t);

public:
  TWSData();
  ~TWSData();

  void load_textures();

  void render_texture();
  void render_edges(); //draw edge of track

  void split_raw_session_into_laps(std::string filename, LapDataArray& aligned_laps); //load session data from filename and append cleaned/aligned laps to LapDataArray
};


#endif