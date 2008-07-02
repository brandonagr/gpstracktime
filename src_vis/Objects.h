#ifndef OBJECTS_H_
#define OBJECTS_H_

#include <string>
#include <vector>
#include "GlUtil.h"




//A waypoint, an absolute time and position the car was located at
//===================================================================
/*struct TrackDataPoint
{
  double time_;
  Vec2 pos_;
  double vel_;
  double bearing_;

  //derived value
  Vec2 vel_;
  Vec2 acc_;

  LineStripPoint()
    :time_(0)
  {}
  LineStripPoint(double time, Vec2& pos)
    :time_(time),
     pos_(pos),
     vel_(0),
     acc_(0)
  {}
  LineStripPoint(double time, Vec2& pos, Vec2& vel, Vec2& acc)
    :time_(time),
     pos_(pos),
     vel_(vel),
     acc_(acc)
  {}
};
*/


//Holds all of the data for a given run
//===================================================================
class TrackData
{
private:
  std::vector<Vec2> data_;
  Vec3 color_;

public:
  TrackData(std::string filename, Vec3& color);

  void print_closest(Vec2& pos);

  Vec2 get_average();
  void center_on(Vec2& center);
  void flip_y();

  void move(Vec2& dx);

  void render();
};


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

  Vec2 get_average();
  void center_on(Vec2& center);
  void flip_y();

  void render();
};


#endif