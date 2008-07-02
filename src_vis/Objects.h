#ifndef OBJECTS_H_
#define OBJECTS_H_

#include <string>
#include <vector>
#include "GlUtil.h"


//A waypoint, an absolute time and position the car was located at
//===================================================================
struct RunDataPoint
{
  double time_;
  Vec2 pos_;

  //derived value
  Vec2 vel_;
  Vec2 acc_;

  RunDataPoint()
    :time_(0)
  {}
  RunDataPoint(double time, Vec2& pos)
    :time_(time),
     pos_(pos),
     vel_(0),
     acc_(0)
  {}
  RunDataPoint(double time, Vec2& pos, Vec2& vel, Vec2& acc)
    :time_(time),
     pos_(pos),
     vel_(vel),
     acc_(acc)
  {}
};


//Holds all of the data for a given run
//===================================================================
class RunData
{
private:
  std::vector<RunDataPoint> data_;
  double max_vel_;  
  double max_t_;

  Vec3 color_;


  Vec2 convert_lat_lon_to_feet(double lat, double lon);
  void derive_data();


  int find_datapoint_preceding(double t);
  int last_val_;
  


public:
  RunData(std::string filename);

  Vec2 get_average();  
  void center_on(Vec2& new_center);

  double max_t(){return max_t_;}
  const Vec3& color(){return color_;}

  void reset(){last_val_=0;}

  Vec2 get_pos_at_t(double t);
  Vec2 get_vel_at_t(double t);
  Vec2 get_acc_at_t(double t);

  double get_t_near_position(Vec2& pos);

  void render();
};


#endif