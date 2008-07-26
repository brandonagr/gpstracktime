//Implement utility classes
// Brandon Green

#include "GlUtil.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#include "tga.h"

using namespace std;

GLuint LoadTexture(char *TexName)
{
  TGAImg Img;        // Image loader
  GLuint Texture;

  // Load our Texture
  if(Img.Load(TexName)!=IMG_OK)
  {
    cout<<"FAIL to load texture"<<endl;
    return 0;
  }

  //cout<<"Loaded image size to be "<<Img.GetWidth()<<", "<<Img.GetHeight()<<" bpp "<<Img.GetBPP()<<endl;

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1,&Texture);            // Allocate space for texture
  glBindTexture(GL_TEXTURE_2D,Texture); // Set our Tex handle as current

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);


  // Create the texture
  if(Img.GetBPP()==24)
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, Img.GetWidth(), Img.GetHeight(), GL_RGB, GL_UNSIGNED_BYTE, Img.GetImg());
  else if(Img.GetBPP()==32)
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, Img.GetWidth(), Img.GetHeight(), GL_RGB, GL_UNSIGNED_BYTE, Img.GetImg());
  else
  {
    cout<<"unknown format!"<<endl;
    return 0;
  }

  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

  //cout<<"returning "<<Texture<<endl;
  return Texture;
}

// MeshObject Loading
//-------------------------------------------------------------------------------
void MeshObject::load_meshobjects(std::string filename, std::vector<MeshObject>& obj_list)
{

  cout<<"Attempting to load MeshObjects from '"<<filename<<"'"<<endl;  
  
  ifstream input;
  input.open(filename.c_str());
  if (!input.is_open())
    throw AppError("Unable to open model file");

  string line;
  getline(input, line);

  if (line!="// MilkShape 3D ASCII")
    throw AppError("Incorrect file format in load_meshobjects");

  //burn through useless lines;
  getline(input, line);//cout<<line<<endl;
  getline(input, line);//cout<<line<<endl;
  getline(input, line);//cout<<line<<endl;
  getline(input, line);//cout<<line<<endl;
  
  int numb_meshes;
  input>>line>>numb_meshes;

  if (numb_meshes<0 || numb_meshes>100)
    throw AppError("numb_meshes sanity check failed in load_meshobjects");

  for(int i=0; i<numb_meshes; i++)
    obj_list.push_back(MeshObject(input));
}

// MeshObject Constructor
//-------------------------------------------------------------------------------
MeshObject::MeshObject(std::ifstream& input, double scale)
{
  load_from_filestream(input,scale);  
}

// load from Milkshape ASCII file
//-------------------------------------------------------------------------------
void MeshObject::load_from_filestream(std::ifstream& input, double load_scale)
{
  if (!input.is_open())
    throw AppError("WTF ARE YOU DOING?");

  std::string line;
  input>>line;
  name_=line.substr(1,line.length()-2);
  std::getline(input,line);

  if (name_.substr(0,6)=="Sphere")
    is_sphere_=true;
  else
    is_sphere_=false;

  int numb;
  input>>numb;
  for(int i=0; i<numb;i++) //read in vertex position information
  {
    int temp_numb;
    Vec3 temp_vec;

    input>>temp_numb>>temp_vec[0]>>temp_vec[1]>>temp_vec[2];
    std::getline(input,line);

    temp_vec*=load_scale; //scale the vertex data

    verts_.push_back(temp_vec);

    bbox_.expand_against(temp_vec);
  }
  input>>numb;
  for(int i=0; i<numb; i++) //read in normal data
  {
    Vec3 temp_vec;
    input>>temp_vec[0]>>temp_vec[1]>>temp_vec[2];
    temp_vec.Normalise();

    normals_.push_back(temp_vec);
  }
  input>>numb;
  for(int i=0; i<numb; i++)
  {
    int temp_numb;
    TriangleData temp_tri;
    input>>temp_numb;
    input>>temp_tri.vert_i_[0]>>temp_tri.vert_i_[1]>>temp_tri.vert_i_[2];
    input>>temp_tri.norm_i_[0]>>temp_tri.norm_i_[1]>>temp_tri.norm_i_[2];
    std::getline(input,line);

    Vec3 p0(verts_[temp_tri.vert_i_[0]]);
    Vec3 p1(verts_[temp_tri.vert_i_[1]]);
    Vec3 p2(verts_[temp_tri.vert_i_[2]]);
    Vec3 a=p1-p0;
    Vec3 b=p2-p0;
    temp_tri.normal_=cross(a,b).Normalise();

    if (temp_tri.vert_i_[0]==temp_tri.vert_i_[1] ||temp_tri.vert_i_[0]==temp_tri.vert_i_[2] ||temp_tri.vert_i_[1]==temp_tri.vert_i_[2])
    {
      //throw AppError("wtf");
    }
    else
      tridata_.push_back(temp_tri);
  }

  pos_=bbox_.get_mid();
  

  //std::cout<<"Loaded model - "<<name_<<std::endl;

  if (is_sphere_)
  {
    radius_=fabs(pos_[1]-bbox_.min_[1]);
    radius_sq_=radius_*radius_;
  }
  bbox_.grow(1.0f);
}


// MeshObject render
//-------------------------------------------------------------------------------
void MeshObject::render()
{	
  GLdouble vert[3];
  for(TriIter t=tridata_.begin(); t!=tridata_.end(); t++)
  {
    glNormal3dv(t->normal_.Ref());
    for(int i=0; i<3; i++) //for the 3 verts that define a triangle
    {
      /*
      vert[0]=normals_[t->norm_i_[i]][0];
      vert[1]=normals_[t->norm_i_[i]][1];
      vert[2]=normals_[t->norm_i_[i]][2];
      glNormal3fv(vert);
      */

      vert[0]=verts_[t->vert_i_[i]][0];
      vert[1]=verts_[t->vert_i_[i]][1];
      vert[2]=verts_[t->vert_i_[i]][2];

      glVertex3dv(vert);
    }
  }  
}
void MeshObject::render_bbox()
{	
  Vec3& min=bbox_.min_;
  Vec3& max=bbox_.max_;

  glVertex3f(min[0], min[1], min[2]);
  glVertex3f(min[0], max[1], min[2]);
  
  glVertex3f(min[0], min[1], min[2]);
  glVertex3f(max[0], min[1], min[2]);
  
  glVertex3f(min[0], min[1], min[2]);
  glVertex3f(min[0], min[1], max[2]);
  
  glVertex3f(min[0], min[1], max[2]);
  glVertex3f(max[0], min[1], max[2]);
  
  glVertex3f(max[0], min[1], max[2]);
  glVertex3f(max[0], min[1], min[2]);

  glVertex3f(min[0], max[1], min[2]);
  glVertex3f(min[0], max[1], max[2]);

  glVertex3f(min[0], max[1], max[2]);
  glVertex3f(max[0], max[1], max[2]);

  glVertex3f(max[0], max[1], max[2]);
  glVertex3f(max[0], max[1], min[2]);

  glVertex3f(min[0], max[1], max[2]);
  glVertex3f(min[0], min[1], max[2]);

  glVertex3f(max[0], max[1], max[2]);
  glVertex3f(max[0], min[1], max[2]);

  glVertex3f(max[0], max[1], min[2]);
  glVertex3f(max[0], min[1], min[2]);

  glVertex3f(max[0], max[1], min[2]);
  glVertex3f(min[0], max[1], min[2]);
}

// MeshObject collide with line segment
//-------------------------------------------------------------------------------
CollReport MeshObject::collide_lineseg(LineSeg& line)
{
  if (is_sphere_)
  {
    Vec3 dist(line.p1-pos_);
    if (sqrlen(dist)<radius_sq_)
    {
      CollReport c;

      c.collision_=true;
      c.norm_=dist.Normalise();
      c.pos_=pos_+c.norm_*radius_;
      c.t_=0.0f;

      return c;
    }
    else
      return CollReport();
  }

  
  Vec3 u, v; //triangle vectors
  Vec3 dir, w0, w; //ray vectors
  double a, b;
  CollReport c;

  Vec3 g1,g2,g3,g4;
  Vec3& p0(g1);
  Vec3& p1(g2);
  Vec3& p2(g3);
  Vec3& n(g4);

  for(TriIter t=tridata_.begin(); t!=tridata_.end(); t++)
  {
    p0=verts_[t->vert_i_[0]];
    p1=verts_[t->vert_i_[1]];
    p2=verts_[t->vert_i_[2]];
    n=t->normal_;

    if (dot(line.p0-p0,n)*dot(line.p1-p0,n)>=0.0f) //both on same side, skip tests
      continue;

    dir=line.p1-line.p0;
    w0=line.p0-p0;
    a=-dot(n,w0);
    b=dot(n,dir);

    if ( fabs(b) < SMALL_NUMBER )
      continue;
        
    c.t_=a/b;
    if (c.t_<0.0f || c.t_>1.0f)
      continue;

    c.collision_=true;
    c.pos_ = line.p0 + c.t_*dir;
    
    
    // is I inside T?
    u=p1-p0;
    v=p2-p0;
    double    uu, uv, vv, wu, wv, D;
    uu = dot(u,u);
    uv = dot(u,v);
    vv = dot(v,v);
    w = c.pos_ - p0;
    wu = dot(w,u);
    wv = dot(w,v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    double su, tu;
    su = (uv * wv - vv * wu) / D;
    if (su < 0.0 || su > 1.0)        // I is outside T
        continue;
    tu = (uv * wu - uu * wv) / D;
    if (tu < 0.0 || (su + tu) > 1.0)  // I is outside T
        continue;

    c.norm_=n;
    return c;
  }

  return CollReport();
}

// MeshObject move where it is located at
//-------------------------------------------------------------------------------
void MeshObject::move(Vec3& d)
{
  pos_+=d;
  bbox_.move(d);

  for(int i=0; i<(int)verts_.size(); i++)
    verts_[i]+=d;
}


// MeshObject mesh analysis functions
//-------------------------------------------------------------------------------
Vec3 MeshObject::get_cog()
{
  Vec3 avg(0);
  for(int i=0; i<(int)verts_.size();i++)
    avg+=verts_[i];
  return avg/(double)verts_.size();
}
Vec3 MeshObject::get_first_normal()
{
  return normals_[0];
}



//==========================================================

//-------------------------------------------------
// Constructor
//-------------------------------------------------
Camera::Camera()
{
  std::ifstream config("./vis_data/camera.txt");
  if (!config.is_open())
  {
    std::cout<<"Could not open ./vis_data/camera.txt, loading camera defaults"<<std::endl;

    aperture_=50.0f;
    focallength_=70.0f;
    eyesep_=focallength_/30.0f;
  }
  else
  {
    double temp;
    config>>aperture_>>focallength_>>temp;
    eyesep_=focallength_/temp;

    config.close();
  }

  setupCamPos(0.0f, 0.0f, 10.0f, Vec3(0,0,0));
}

//-------------------------------------------------
// Destructor
//-------------------------------------------------
Camera::~Camera()
{
  //nothing to do
}

void Camera::setupParams(int width, int height)
{
  window_width_=width;
  window_height_=height;
}

//-------------------------------------------------
// set up camera position
//-------------------------------------------------
void Camera::setupCamPos(double rx, double ry, double dist, Vec3 lookat)
{
  cam_lookat_=lookat;
  
  rx_=rx;
  ry_=ry;
  dist_=dist;

  calcCamPos();

  cam_up_=Vec3(0,1.0f,0);
}

//-------------------------------------------------
// set up camera position
//-------------------------------------------------
void Camera::calcCamPos()
{  
  double cX=cos(rx_)*dist_;
  cam_pos_[0]=cam_lookat_[0]+cX*sin(ry_);
  cam_pos_[1]=cam_lookat_[1]+sin(rx_)*dist_;
  cam_pos_[2]=cam_lookat_[2]+cX*-cos(ry_);

  cam_dir_=cam_lookat_-cam_pos_;
  cam_dir_.Normalise();
}

//-------------------------------------------------
// move lookat position
//-------------------------------------------------
void Camera::moveLookAt(Vec3 dmove)
{
  cam_lookat_+=dmove;
}
void Camera::setLookAt(Vec3 pos)
{
  cam_lookat_=pos;
}

//-------------------------------------------------
// rotate camera
//-------------------------------------------------
void Camera::rotateCam(double dry, double drx)
{
  ry_+=dry;
  rx_+=drx;

  if (rx_>(PI_/2.0f)-0.0001f)
    rx_=PI_/2.0f-0.0001f;
  if (rx_<0.0f)
    rx_=0.0f;
}

//-------------------------------------------------
// move distance of camera
//-------------------------------------------------
void Camera::moveCamDist(double ddist)
{

  dist_+=ddist*0.5*dist_;

  if (dist_<1)dist_=1;

  //cam_lookat_+=ddist*cam_dir_;
}

//-------------------------------------------------
// Drive the camera around
//-------------------------------------------------
void Camera::moveDrive(double dx, double dy)
{
  //drive along x/z plane

  double sY=sin(ry_);
  double cY=cos(ry_);
  
  if (dy)
  {
    cam_lookat_[0]-=sY*dy*0.2f*dist_; //x
    cam_lookat_[2]+=cY*dy*0.2f*dist_; //z
  }
  if (dx)
  {
    cam_lookat_[0]+=cY*dx*0.2f*dist_;
    cam_lookat_[2]+=sY*dx*0.2f*dist_;
  }
  
  /*
  //drive in screenspace
  Vec3 screen_right, screen_up;
  screen_right=cross(cam_dir_,cam_up_);
  screen_up=cross(cam_dir_, screen_right);

  screen_right.Normalise();
  screen_up.Normalise();

  cam_lookat_+=-dx*screen_right*0.2*dist_+-dy*screen_up*0.2*dist_;
  */
}

//-------------------------------------------------
// Drive the camera around
//-------------------------------------------------
void Camera::moveUp(double dy)
{
	cam_lookat_[1]+=dy;
}

//-------------------------------------------------
// Set the projection for non stereo, draw buffer
//-------------------------------------------------
void Camera::set_proj()
{
  calcCamPos();

  double ratio,radians,wd2;
  double left,right,top,bottom,near_cam=1.0,far_cam=50000;

  /* Misc stuff */
  ratio  = window_width_ / (double)window_height_;
  radians = DEG_TO_RAD * aperture_ / 2.0f;
  wd2     = near_cam * tan(radians);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  left  = - ratio * wd2;
  right =   ratio * wd2;
  top    =   wd2;
  bottom = - wd2;
  glFrustum(left,right,bottom,top,near_cam,far_cam);
  //glOrtho(0.0f,2048,2048,0.0f,near_cam,far_cam);

  glMatrixMode(GL_MODELVIEW);
  glDrawBuffer(GL_BACK);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  gluLookAt(cam_pos_[0], cam_pos_[1], cam_pos_[2],
            cam_lookat_[0], cam_lookat_[1], cam_lookat_[2],
            cam_up_[0], cam_up_[1], cam_up_[2]);  
}

//-------------------------------------------------
// Set the projection for right eye, draw buffer
//-------------------------------------------------
void Camera::set_proj_right()
{
  calcCamPos();

  Vec3 r;
  double ratio,radians,wd2,ndfl;
  double left,right,top,bottom,near_cam=0.1,far_cam=10000;

  /* Misc stuff */
  ratio  = window_width_ / (double)window_height_;
  radians = DEG_TO_RAD * aperture_ / 2.0f;
  wd2     = near_cam * tan(radians);
  ndfl    = near_cam / focallength_;

    /* Derive the two eye positions */
  r=cross(cam_dir_, cam_up_);
  r.Normalise();
  r*=eyesep_/2.0f;      

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  left  = - ratio * wd2 - 0.5 * eyesep_ * ndfl;
  right =   ratio * wd2 - 0.5 * eyesep_ * ndfl;
  top    =   wd2;
  bottom = - wd2;
  glFrustum(left,right,bottom,top,near_cam,far_cam);

  glMatrixMode(GL_MODELVIEW);
  glDrawBuffer(GL_BACK_RIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  gluLookAt(cam_pos_[0]+r[0], cam_pos_[1]+r[1], cam_pos_[2]+r[2],
            cam_lookat_[0]+r[0], cam_lookat_[1]+r[1], cam_lookat_[2]+r[2],
            cam_up_[0], cam_up_[1], cam_up_[2]);  

}

//-------------------------------------------------
// Set the projection for left eye, draw buffer
//-------------------------------------------------
void Camera::set_proj_left()
{
  calcCamPos();

  Vec3 r;
  double ratio,radians,wd2,ndfl;
  double left,right,top,bottom,near_cam=0.1,far_cam=10000;

  /* Misc stuff */
  ratio  = window_width_ / (double)window_height_;
  radians = DEG_TO_RAD * aperture_ / 2.0f;
  wd2     = near_cam * tan(radians);
  ndfl    = near_cam / focallength_;

    /* Derive the two eye positions */
  r=cross(cam_dir_, cam_up_);
  r.Normalise();
  r*=eyesep_/2.0f;      

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  left  = - ratio * wd2 + 0.5 * eyesep_ * ndfl;
  right =   ratio * wd2 + 0.5 * eyesep_ * ndfl;
  top    =   wd2;
  bottom = - wd2;
  glFrustum(left,right,bottom,top,near_cam,far_cam);

  glMatrixMode(GL_MODELVIEW);
  glDrawBuffer(GL_BACK_LEFT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  gluLookAt(cam_pos_[0]-r[0], cam_pos_[1]-r[1], cam_pos_[2]-r[2],
          cam_lookat_[0]-r[0], cam_lookat_[1]-r[1], cam_lookat_[2]-r[2],
          cam_up_[0], cam_up_[1], cam_up_[2]);  
}

//-------------------------------------------------
// find the ray from the camera into world space given a click on the screen
//-------------------------------------------------
Vec3 Camera::get_ray_from_click(int x, int y)
{
  //http://www.3dkingdoms.com/selection.html#point
  // This function will find 2 points in world space that are on the line into the screen defined by screen-space( ie. window-space ) point (x,y)
  double mvmatrix[16];
  double projmatrix[16];
  int viewport[4];
  double dX, dY, dZ, dClickY; // glUnProject uses doubles, but I'm using doubles for these 3D vectors

  glGetIntegerv(GL_VIEWPORT, viewport);	
  glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
  glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
  dClickY = double (window_height_ - y); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

  gluUnProject ((double) x, dClickY, 0.0, mvmatrix, projmatrix, viewport, &dX, &dY, &dZ);
  Vec3 origin( dX, dY, dZ );
  gluUnProject ((double) x, dClickY, 1.0, mvmatrix, projmatrix, viewport, &dX, &dY, &dZ);
  Vec3 click( dX, dY, dZ );

  return norm(click-origin);
}