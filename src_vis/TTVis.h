#ifndef TRACKTIME_H_
#define TRACKTIME_H_

#include <stdlib.h>
#include <GL/glut.h>
#include <string>
#include <vector>

#include "svl/SVL.h"
#include "GlUtil.h"
#include "Util.h"
#include "Objects.h"


class App
{
public:
  //Initialize Everything  
  App(int wx, int wy, bool stereo);
  ~App();
  
  void init();
  void reset();  
  void set_window(int width, int height);
  
  //Glut input functions  
  void click(int button, int state, int x, int y);
  void move(int x, int y);
  void keypress(unsigned char key);
  void keypress(int key);
  
  //run and draw one frame
  void frame(double dt);  
  void render_frame(double dt);  

private:

  TWSData trackdata_;
  LapDataArray laps_;
  std::vector<LapData>::iterator draw_lap_;

  Vec2 test_d;

  

  //utility vars  
  Params params_;

  //Camera and mouse vars
  Camera cam_;
  int window_width_, window_height_;
  bool stereo_rendering_;  

  bool left_mouse_down_, middle_mouse_down_, right_mouse_down_;
  Vec2 mouse_pos_,mouse_old_pos_,mouse_delta_;
  bool fresh_move_;

  //Utility class to draw text
  void drawText(const char *str, void *font, GLclampf r, GLclampf g, GLclampf b, GLdouble x, GLdouble y);
};

#endif

