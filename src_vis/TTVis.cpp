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

  trackdata_("./gps_record s4.txt"),//,Vec3(1,0,0)),

  offx_(-992),
  offy_(-535),
  angle_(2.58359),
  scale_(2.8349),

  params_("./vis_data/settings.txt")
{ 
  set_window(window_width_, window_height_);

  reset();
}  
// Reset the app
//-------------------------------------------------------------------------------
void App::reset()
{  

}

// Destructor
//-------------------------------------------------------------------------------
App::~App()
{
  glDeleteTextures(1,&tex_);
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
  glLineWidth(2.0f);

  /*
  glEnable(GL_POLYGON_SMOOTH); 
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);   
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);    
  */
   

  glDisable(GL_LIGHTING);

  glEnable(GL_TEXTURE_2D);						// Enable Texture Mapping ( NEW )
	glShadeModel(GL_SMOOTH);						// Enable Smooth Shading
	glDisable(GL_DEPTH_TEST);						// Enables Depth Testing	



  //GLfloat ambient[4] = {0.2f, 0.2f, 0.2f, 1.0};
  //GLfloat specular[4] = {0.4f, 0.4f, 0.4f, 1.0f};

  //GLfloat light_ambient[4]  = {0.6f, 0.2f, 0.2f, 1.0f};
  //GLfloat light_pos[4]  = {0,25,0,1};
  

  //glEnable(GL_LIGHTING);
  //glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);

  //glLightfv(GL_LIGHT0,GL_POSITION,light_pos);
  //glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
  //glEnable(GL_LIGHT0);
  
  //glEnable(GL_COLOR_MATERIAL);
  //glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  //glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
  //glMaterialf(GL_FRONT,GL_SHININESS,50);


  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);  


  cam_.setupParams(window_width_, window_height_);
  cam_.setupCamPos(vl_pi/2.0f-0.0001f, vl_pi, 6500.0f, Vec3(0,0,0));

  tex_=LoadTexture("track_1_2.tga");
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
void App::frame(float dt)
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
void App::render_frame(float dt)
{
//Draw the origin
  /*
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
  */

  //drawText("Click/Drag to move cam:",GLUT_BITMAP_HELVETICA_10, 0,0,0, 2,window_height_-12);  
  //drawText("Middle - Zoom",GLUT_BITMAP_HELVETICA_10, 0,0,0, 2,window_height_-24);
  //drawText("Right - Rotate",GLUT_BITMAP_HELVETICA_10, 0,0,0, 2,window_height_-36);
/*
  //display fps
  {
    static float time=0.0f;
    static int frame_count=0;
    static float last_fps=0.0f;
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
*/

  //Draw track stuff  
    glColor3f(1.0f,1.0f,1.0f);
  glBindTexture( GL_TEXTURE_2D, tex_ );

  cout<<offx_<<" "<<offy_<<" "<<angle_<<" "<<scale_<<endl;

  Vec2 p1(offx_+1024.0*scale_*cos(angle_), offy_+1024.0*scale_*sin(angle_));
  Vec2 p2(offx_+1024.0*scale_*cos(angle_-PI_/2.0), offy_+1024.0*scale_*sin(angle_-PI_/2.0));
  Vec2 p3(offx_+1024.0*scale_*cos(angle_-PI_), offy_+1024.0*scale_*sin(angle_-PI_));
  Vec2 p4(offx_+1024.0*scale_*cos(angle_-3.0*PI_/2.0), offy_+1024.0*scale_*sin(angle_-3.0*PI_/2.0));

  glBegin(GL_QUADS);
    glTexCoord2f(1.0f,0.0f); glVertex3f(p1[0], 0.0f, p1[1]);
    glTexCoord2f(0.0f,0.0f); glVertex3f(p2[0], 0.0f, p2[1]);
    glTexCoord2f(0.0f,1.0f); glVertex3f(p3[0], 0.0f, p3[1]);
    glTexCoord2f(1.0f,1.0f); glVertex3f(p4[0], 0.0f, p4[1]);
   glEnd();
   
/*
  glBegin(GL_QUADS);
    glTexCoord2f(1.0f,0.0f); glVertex3f(-1000.0+offx_, 0.0f, 1000.0+offy_);
    glTexCoord2f(0.0f,0.0f); glVertex3f( 1000.0+offx_, 0.0f, 1000.0+offy_);
    glTexCoord2f(0.0f,1.0f); glVertex3f( 1000.0+offx_, 0.0f, -1000.0+offy_);
    glTexCoord2f(1.0f,1.0f); glVertex3f(-1000.0+offx_, 0.0f, -1000.0+offy_);
   glEnd();*/


  glColor3f(1.0f,0.0f,0.0f);
  trackdata_.render();
  


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
  case 'R':
  case 'r':
      reset();      
    break;

  case 'D':
  case 'd':
    {
      static Vec2 last_pos(cam_.lookat()[0],cam_.lookat()[2]);

      Vec2 current(cam_.lookat()[0],cam_.lookat()[2]);

      cout<<len(last_pos-current)<<" Dist from "<<last_pos<<" to "<<current<<endl;

      last_pos=current;
    }
    break;

   case 'n':
    angle_+=0.001;
    break;
  case 'm':
    angle_-=0.001;
    break;
  case 'k':
    scale_*=0.999;
    break;
  case 'l':
    scale_*=1.001;
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
    
  case GLUT_KEY_UP:
    offx_+=1;
    break;
  case GLUT_KEY_DOWN:
    offx_-=1;
    break;
  case GLUT_KEY_RIGHT:
    offy_+=1;
    break;
  case GLUT_KEY_LEFT:
    offy_-=1;
    break;
    
    
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
void App::drawText(const char *str, void *font, GLclampf r, GLclampf g, GLclampf b, GLfloat x, GLfloat y) 
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
  
