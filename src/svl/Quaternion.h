#ifndef QUATERNION_H_
#define QUATERNION_H_

#include "SVL.h"

//mainly put together from stuff on web and http://gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation


#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.0174532925f
#endif



class Quaternion
{
public:
  Quaternion();
  Quaternion(float x, float y, float z, float w);
  Quaternion(const Vec3 &v, float angle);
  Quaternion(float pitch, float yaw, float roll);

  Quaternion& operator =  (const Quaternion& a);

  void normalise();
  Quaternion get_conjugate() const;

  Quaternion Quaternion::operator* (const Quaternion& rq) const;
  Vec3 Quaternion::operator* (const Vec3& vec) const;

  void FromAxis(const Vec3& v, float angle);
  void FromEuler(float pitch, float yaw, float roll);

  Mat4 getMatrix() const;
  void getAxisAngle(Vec3* axis, float* angle);

private:
  float x,y,z,w;
};


#endif


