//Brandon Green
//
// when using gcc link with -lglut -lGL -lGLU -lm 
//


#include <iostream>
#include <fstream>
#include <memory>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <GL/glut.h>

#include "TTVis.h"



std::auto_ptr<App> app;

//Callbacks have to be regular nonmember functions
void glutReshape(int width, int height)
{
  glViewport(0, 0, width, height);
  app->set_window(width, height);
}
void glutKeyboard(unsigned char key, int x, int y)
{
  app->keypress(key);
}
void glutKeyboardSpecial(int key, int x, int y)
{
  app->keypress(key);
}
void glutMotion(int x, int y)
{
  app->move(x, y);
}
void glutMouse(int button, int state, int x, int y)
{
  app->click(button, state, x, y);      
}
void Init()
{
  app->init();
}
void glutFrame(void)
{
  static clock_t last_t=clock();
  clock_t cur_t=clock();

  if (cur_t!=last_t)
  {
    app->frame((cur_t-last_t)/(float)CLOCKS_PER_SEC);
    last_t=cur_t;
  }
}


int main(int argc, char **argv)
{  
  srand(time(NULL));
	
//Load window config from file
  std::ifstream config("./vis_data/window.txt");

  int wx, wy;
  bool use_stereo;
  if (!config.is_open())
  {
    std::cout<<"Could not open './vis_data/window.txt', using defaults"<<std::endl;
    
    wx=800;
    wy=600;
    use_stereo=false;
  }
  else
  {
    int stereo;

    config>>wx>>wy>>stereo;
    config.close();

    use_stereo=(stereo!=0);
  }  

//create window app
  try
  {
    app.reset(new App(wx, wy, use_stereo));
  }
  catch(AppError err)
  {
    std::cout<<"Failed to create App: "<<err.what()<<std::endl;
  }
  catch(std::exception& e)
  {
    std::cout<<"FATAL ERROR!"<<std::endl<<std::endl;
    std::cout<<e.what()<<std::endl;
    return -1;
  }

//setup glut
  glutInit(&argc, argv);  

  GLenum type=0;
  if (use_stereo)
  {
    std::cout<<"Attempting to use stereo"<<std::endl;
    type = GLUT_STEREO;
  }
  else
  {
    std::cout<<"NOT using stereo"<<std::endl;
  }

  type |= GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH;// | GLUT_ALPHA;// | GLUT_MULTISAMPLE;


  glutInitDisplayMode(type);
  glutInitWindowSize(wx, wy);
  glutCreateWindow("GPS Track Time");

  Init();

  glutReshapeFunc(glutReshape);
  glutKeyboardFunc(glutKeyboard);
  glutSpecialFunc(glutKeyboardSpecial);
  glutMotionFunc(glutMotion);
  glutPassiveMotionFunc(glutMotion);
  glutMouseFunc(glutMouse);
  glutIdleFunc(glutFrame);
  glutDisplayFunc(glutFrame);
  glutMainLoop();

  return 0;
}



