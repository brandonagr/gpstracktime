#ifndef C_UTIL_H
#define C_UTIL_H

//Brandon Green
// series of utility classes used across various projects
// includes a camera class, parameter loading, aabb, loading meshes from file

#include <stdlib.h>
#include <GL/glut.h>
#include <map>
#include <string>
#include <vector>
#include "svl/SVL.h"
#include <sstream>


//Useful defines
#define SMALL_NUMBER 0.0000001f

GLuint LoadTexture(char *TexName);


// Line segment to render, used for debug output
//==========================================================
struct LineSeg
{
  double r,g,b;
  Vec3 p0,p1;

  LineSeg()
    :r(0.0f),
     g(0.0f),
     b(0.0f)
  {}
  LineSeg(Vec3 begin, Vec3 end)
    :r(0.0f),
     g(0.0f),
     b(0.0f),
     p0(begin),
     p1(end)
  {}
  LineSeg(Vec3 begin, Vec3 end, double nr, double ng, double nb)
    :r(nr),
     g(ng),
     b(nb),
     p0(begin),
     p1(end)
  {}
};


// Simple struct to hold a bounding box
//==========================================================
struct AABB
{
  Vec3 min_, max_;

  AABB()
  {
    min_[0]=min_[1]=min_[2]=99999999.9f;
    max_[0]=max_[1]=max_[2]=-99999999.9f;
  }
  AABB(Vec3& p)
  {
    min_[0]=max_[0]=p[0];
    min_[1]=max_[1]=p[1];
    min_[2]=max_[2]=p[2];
  }
  AABB(Vec3& min, Vec3& max)
    :min_(min),
     max_(max)
  { }

  inline void expand_against(Vec3& p)
  {
    if (p[0]<min_[0])
      min_[0]=p[0];
    if (p[1]<min_[1])
      min_[1]=p[1];
    if (p[2]<min_[2])
      min_[2]=p[2];

    if (p[0]>max_[0])
      max_[0]=p[0];
    if (p[1]>max_[1])
      max_[1]=p[1];
    if (p[2]>max_[2])
      max_[2]=p[2];
  }

  inline void grow(double d)
  {
    min_[0]-=d;
    min_[1]-=d;
    min_[2]-=d;
    max_[0]+=d;
    max_[1]+=d;
    max_[2]+=d;
  }

  inline void move(Vec3& m)
  {
    min_+=m;
    max_+=m;
  }

  inline bool collide(Vec3& p)
  {
    return (p[0]>=min_[0] && p[1]>=min_[1] && p[2]>=min_[2] &&
            p[0]<=max_[0] && p[1]<=max_[1] && p[2]<=max_[2]);
  }
  inline bool collide(AABB& bbox)
  {
    return (max_[0]>=bbox.min_[0] && min_[0]<=bbox.max_[0] && 
            max_[1]>=bbox.min_[1] && min_[1]<=bbox.max_[1] &&
            max_[2]>=bbox.min_[2] && min_[2]<=bbox.max_[2]);
  }
  inline Vec3 get_min()
  {
    return min_;
  }
  inline Vec3 get_max()
  {
    return max_;
  }
  inline Vec3 get_mid()
  {
    return (max_+min_)/2.0f;
  }
};

//Utility struct used in 
//-----------------------------------------------------
struct CollReport
{
  bool collision_;
  Vec3 pos_;
  Vec3 norm_;
  double t_;

  CollReport():collision_(false)
  {}
};

//Utility struct used in TriangleObject to hold data
//-----------------------------------------------------
struct TriangleData
{
  unsigned short vert_i_[3];
  unsigned short norm_i_[3];
  Vec3 normal_;
};

//Holds a single mesh object loaded from 
//-----------------------------------------------------
class MeshObject
{
private:
  std::string name_;
  std::vector<Vec3> verts_;
  std::vector<Vec3> normals_;
  std::vector<TriangleData> tridata_;
  typedef std::vector<TriangleData>::iterator TriIter;

  Vec3 pos_;
  AABB bbox_;

  bool is_sphere_;
  double radius_;
  double radius_sq_;


  void load_from_filestream(std::ifstream& input, double load_scale);

public:
  static void load_meshobjects(std::string filename, std::vector<MeshObject>& obj_list);

  MeshObject(std::ifstream& input, double scale=1.0f); //build by reading in data from ifstream

  void render();
  void render_bbox();

  AABB& bbox(){return bbox_;}
  CollReport collide_lineseg(LineSeg& line);
  
  void move(Vec3& d);

  Vec3 get_cog();
  Vec3 get_first_normal();

  const std::vector<Vec3>& verts(){return verts_;}
  const std::vector<TriangleData>& tris(){return tridata_;}
};

typedef std::vector<MeshObject> MeshObjList;
typedef std::vector<MeshObject>::iterator MeshObjListIter;



// Camera class
//==========================================================
class Camera
{
private:
  int window_width_, window_height_;

  Vec3 cam_pos_;
  Vec3 cam_lookat_;
  Vec3 cam_dir_;
  Vec3 cam_up_;

  double focallength_;
  double aperture_;
  double eyesep_;

  double ry_, rx_; //rotation around lookat position
  double dist_;  

  void calcCamPos(); //calculate cam_pos, cam_dir, from cam_lookat

public:
  Camera();
  ~Camera();

  void setupParams(int width, int height);
  void setupCamPos(double rx, double ry, double dist, Vec3 lookat);

  void moveLookAt(Vec3 dmove);
  void setLookAt(Vec3 pos);
  void rotateCam(double dry, double drx);
  void moveCamDist(double ddist);

  void moveDrive(double dx, double dy);
  void moveUp(double dy);

  void set_proj();

  void set_proj_left();
  void set_proj_right();

  const Vec3& pos(){return cam_pos_;}
  const Vec3& lookat(){return cam_lookat_;}

  Vec3 get_ray_from_click(int x, int y);
};

struct AppError
{
  std::string what_;
  
  AppError(std::string error)
  :what_(error)
  {}
  
  std::string what(){return what_;}

};

#endif

