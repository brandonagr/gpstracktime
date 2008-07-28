#include "TTvis.h"
#include <time.h>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;



// Constructor
//-------------------------------------------------------------------------------
App::App(int wx, int wy, bool stereo)
: window_width_(wx),
  window_height_(wy),
  stereo_rendering_(stereo),
  left_mouse_down_(false),
  middle_mouse_down_(false),
  right_mouse_down_(false),
  mouse_pos_(0,0),
  mouse_old_pos_(0,0),
  mouse_delta_(0,0),
  fresh_move_(false),

  params_("./vis_data/settings.txt")
{ 
  set_window(window_width_, window_height_);

  reset();
}  
// Reset the app
//-------------------------------------------------------------------------------
void App::reset()
{  
  params_=Params("./vis_data/settings.txt"); //reload settings file
  
  laps_.clear();
  trackdata_.split_raw_session_into_laps(params_.get<std::string>("SessionData"), laps_);

  draw_lap_=laps_.begin();
}

// Destructor
//-------------------------------------------------------------------------------
App::~App()
{

}

// Setup rendering parameters
//-------------------------------------------------------------------------------
void App::init()
{  
  glClearColor(1.0,1.0,1.0,1.0);  
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  
  glEnable(GL_POINT_SMOOTH);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);   
  glPointSize(3.0f);

  glEnable(GL_LINE_SMOOTH);   
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glLineWidth(1.0f);

  /*
  glEnable(GL_POLYGON_SMOOTH); 
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);   
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);    
  */

  glDisable(GL_LIGHTING);

  glEnable(GL_TEXTURE_2D);						// Enable Texture Mapping ( NEW )
	glShadeModel(GL_SMOOTH);						// Enable Smooth Shading
	glDisable(GL_DEPTH_TEST);						// Enables Depth Testing	

  cam_.setupParams(window_width_, window_height_);
  cam_.setupCamPos(vl_pi/2.0f-0.0001f, vl_pi, 6500.0f, Vec3(0,0,0));
  
  //have to do this only after opengl is initialized
  trackdata_.load_textures();
}

// Reset the window
//-------------------------------------------------------------------------------
void App::set_window(int width, int height)
{
  window_width_=width;
  window_height_=height;

  cam_.setupParams(window_width_, window_height_);
}


// Process one frame
//-------------------------------------------------------------------------------
void App::frame(double dt)
{     
//Update Stuff
  //Move Camera
  if (fresh_move_ && len(mouse_delta_)>0.0f) //Look for camera updates
  {         
    if ((left_mouse_down_ && right_mouse_down_) || middle_mouse_down_)
      cam_.moveCamDist(mouse_delta_[1]/15.0f);
    else
    {
      if (left_mouse_down_)
        cam_.moveDrive(mouse_delta_[0]/50.0f,mouse_delta_[1]/50.0f);
      if (right_mouse_down_)
        cam_.rotateCam(mouse_delta_[0]/100.0f, mouse_delta_[1]/100.0f);    
    }

    fresh_move_=false;
  }  

/*
  if (left_mouse_down_) //process a selection click
  {
    //reset();

    Vec3 vdir=cam_.get_ray_from_click((int)mouse_pos_[0],(int)mouse_pos_[1]);
    Vec2 click_on_plane(0.0,0.0);

    if (cam_.pos()[1]>0) //camera is above the z=0 plane
    {
      if (vdir[1]<0) //ray is going towards ground
      {
        double t=-cam_.pos()[1]/vdir[1];
              
        click_on_plane[0]=cam_.pos()[0]+vdir[0]*t;
        click_on_plane[1]=cam_.pos()[2]+vdir[2]*t;
      }
    }
    else
    {
      if (vdir[1]>0) //ray is going towards ground
      {
        double t=-cam_.pos()[1]/vdir[1];
              
        click_on_plane[0]=cam_.pos()[0]+vdir[0]*t;
        click_on_plane[1]=cam_.pos()[2]+vdir[2]*t;
      }
    }     

    //Vec3 t(click_on_plane[0],0.0,click_on_plane[1]);
    //cam_.setLookAt(t);

    left_mouse_down_=false;
  }
*/

  //do update of stuff 



//Render
  if (stereo_rendering_)
  {
    cam_.set_proj_right();
    render_frame(dt);   

    cam_.set_proj_left();
    render_frame(dt);
  }
  else
  {
    cam_.set_proj();
    render_frame(dt);
  }

  glutSwapBuffers();
}

//Render an actual frame
//-------------------------------------------------------------------------------
void App::render_frame(double dt)
{
//Draw the origin
  glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(cam_.lookat()[0],cam_.lookat()[1],cam_.lookat()[2]);
    glVertex3f(cam_.lookat()[0]+2.0f,cam_.lookat()[1],cam_.lookat()[2]);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(cam_.lookat()[0],cam_.lookat()[1],cam_.lookat()[2]);
    glVertex3f(cam_.lookat()[0],cam_.lookat()[1]+2.0f,cam_.lookat()[2]);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(cam_.lookat()[0],cam_.lookat()[1],cam_.lookat()[2]);
    glVertex3f(cam_.lookat()[0],cam_.lookat()[1],cam_.lookat()[2]+2.0f);
  glEnd();
  

  //drawText("Click/Drag to move cam:",GLUT_BITMAP_HELVETICA_10, 0,0,0, 2,window_height_-12);  
  //drawText("Middle - Zoom",GLUT_BITMAP_HELVETICA_10, 0,0,0, 2,window_height_-24);
  //drawText("Right - Rotate",GLUT_BITMAP_HELVETICA_10, 0,0,0, 2,window_height_-36);

  //display fps
  {
    static double time=0.0f;
    static int frame_count=0;
    static double last_fps=0.0f;
    frame_count++;
    time+=dt;
    if (time>0.5f)
    {
      last_fps=(1/(time/frame_count));
      time=0.0f;
      frame_count=0;
    }
    std::stringstream fps;
    fps.precision(4);
    fps<<"FPS "<<last_fps;
    drawText(fps.str().c_str(), GLUT_BITMAP_HELVETICA_10, 0.5f,0.5f,0.5f, window_width_-60,window_height_-12);
  }


  //render track
  //trackdata_.render_texture();  
  trackdata_.render_edges();  
  draw_lap_->render();

  if (test_d[0]!=0.0 && test_d[1]!=0.0)
  {
    glBegin(GL_LINES);

    glColor3f(1.0, 0.0, 0.0);

    Vec3 i(test_d[0], 0.0, test_d[1]);
    Vec3 j(cam_.lookat()[0], 0.0, cam_.lookat()[2]);

    glVertex3dv(i.Ref());
    glVertex3dv(j.Ref());

    glEnd();
  }  

  //display current time
  /*
  {    
    std::stringstream time;
    time<<"Time: ";
    
    time.precision(5);
    time<<replay_time_;

    drawText(time.str().c_str(), GLUT_BITMAP_HELVETICA_18, 0.0f,0.0f,0.0f, window_width_-120,5);
  }
  */

  //render course
  /*
  glBegin(GL_LINE_STRIP);
  glColor3f(0.8f,0.0f,0.0f); 
  Vec3 t;
  for(int i=0; i<(int)course_.size(); i++)
  {
    t=Vec3(course_[i].pos_[0],0.0,course_[i].pos_[1]);
    glVertex3dv(t.Ref());
  }
  glEnd();
  

  //render runs
  for(int i=0; i<numb_runs_; i++) 
    LineStrip_[i].render();  


  glEnable(GL_LIGHTING);
  for(int i=0; i<numb_runs_; i++)
  {
    glColor3d(LineStrip_[i].color()[0],LineStrip_[i].color()[1],LineStrip_[i].color()[2]);
    glPushMatrix();
      
      glTranslated(markerdata_[i].pos_[0],0.0,markerdata_[i].pos_[1]);
            

      double angle=atan2(markerdata_[i].vel_[0],markerdata_[i].vel_[1])*180/PI_;
      angle+=90;
      glRotated(angle,0.0f,1.0f,0.0f);
      
      glBegin(GL_TRIANGLES); //render markers on the grid
      arrow_[0].render();
      glEnd();
    glPopMatrix();
  }
  */


}

// handle mouse click
//-------------------------------------------------------------------------------
void App::click(int button, int state, int x, int y)
{    
  if (button==GLUT_LEFT_BUTTON)
    left_mouse_down_=(state==GLUT_DOWN);
  if (button==GLUT_MIDDLE_BUTTON)
    middle_mouse_down_=(state==GLUT_DOWN);
  if (button==GLUT_RIGHT_BUTTON)
    right_mouse_down_=(state==GLUT_DOWN);

  if (state==GLUT_DOWN)
    mouse_pos_=Vec2(x,y);
}


// handle mouse movement
//-------------------------------------------------------------------------------
void App::move(int x, int y) 
{ 
  mouse_old_pos_=mouse_pos_;  
  mouse_pos_=Vec2(x,y);

  if (mouse_pos_!=mouse_old_pos_)
  {
    mouse_delta_=mouse_pos_-mouse_old_pos_;
    
    fresh_move_=true;
  }
}


// handle keypress
//-------------------------------------------------------------------------------
void App::keypress(unsigned char key) 
{ 
  switch(key)
  {  
  case ' ':
    draw_lap_++;
    if (draw_lap_==laps_.end())
      draw_lap_=laps_.begin();    
    break;

  case 'c':
  case 'C':
    test_d=0;
    break;
  case 'd':
  case 'D':
    {
      Vec2 cur(cam_.lookat()[0],cam_.lookat()[2]);

      if (test_d[0]!=0.0 && test_d[1]!=0.0)
      {
        cout<<"Distance: "<<len(test_d-cur)<<endl;
      }

      test_d=cur;
    }
    break;

  case 'R':
  case 'r':
      reset();      
    break;

  case 27:
    exit(0);
    break;
  default:
    std::cout<<"Unbound?"<<std::endl;
    break;
  }
}
void App::keypress(int key) 
{ 
  switch(key)
  {      
    
  case 27:
    exit(0);
    break;
  default:
    std::cout<<"Unbound?"<<std::endl;
    break;
  }
}



// Draw some text, found this code in a nehe tutorial
//-------------------------------------------------------------------------------
void App::drawText(const char *str, void *font, GLclampf r, GLclampf g, GLclampf b, GLdouble x, GLdouble y) 
{
  /* font: font to use, e.g., GLUT_BITMAP_HELVETICA_10
     r, g, b: text colour
     x, y: text position in window: range [0,0] (bottom left of window)
           to [width,height] (top right of window). */

  char *ch=(char*)str;
  GLint matrixMode;
  GLboolean lightingOn;

  lightingOn= glIsEnabled(GL_LIGHTING);        /* lighting on? */
  if (lightingOn) glDisable(GL_LIGHTING);

  glGetIntegerv(GL_MATRIX_MODE, &matrixMode);  /* matrix mode? */

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0.0, window_width_, 0.0, window_height_);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
     glLoadIdentity();
     glPushAttrib(GL_COLOR_BUFFER_BIT);       /* save current colour */
       glColor3f(r, g, b);
       glRasterPos3f(x, y, 0.0);
       for(; *ch; ch++) {
          glutBitmapCharacter(font, (int)*ch);
       }
     glPopAttrib();
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(matrixMode);
  if (lightingOn) glEnable(GL_LIGHTING);
}
  
