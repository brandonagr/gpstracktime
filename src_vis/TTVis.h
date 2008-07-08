#ifndef AUTOXTIME_H_
#define AUTOXTIME_H_

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
  void frame(float dt);  
  void render_frame(float dt);  

private:

  TWSData trackdata_;

  GLuint tex_;

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

  void calc_reset_whole();
  void output_state(int i=-1);

  int active_chunk;
  


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
  void drawText(const char *str, void *font, GLclampf r, GLclampf g, GLclampf b, GLfloat x, GLfloat y);
};

#endif

