#include "Quaternion.h"


Quaternion::Quaternion()
:w(1.0f)
{}
Quaternion::Quaternion(float ix, float iy, float iz, float iw)
:x(ix),
 y(iy),
 z(iz),
 w(iw)
{}
Quaternion::Quaternion(const Vec3 &v, float angle)
{
  FromAxis(v,angle);
}
Quaternion::Quaternion(float pitch, float yaw, float roll)
{
  FromEuler(pitch,yaw,roll);
}

inline Quaternion& Quaternion::operator = (const Quaternion& q)
{
  x=q.x;
  y=q.y;
  z=q.z;
  w=q.w;

  return(*this);
}

// normalising a quaternion works similar to a vector. This method will not do anything
// if the quaternion is close enough to being unit-length. define TOLERANCE as something
// small like 0.00001f to get accurate results
void Quaternion::normalise()
{
	// Don't normalize if we don't have to
	float mag2 = w * w + x * x + y * y + z * z;
	if (fabs(mag2 - 1.0f) > 0.00001f) {
		float mag = sqrt(mag2);
		w /= mag;
		x /= mag;
		y /= mag;
		z /= mag;
	}
}

// We need to get the inverse of a quaternion to properly apply a quaternion-rotation to a vector
// The conjugate of a quaternion is the same as the inverse, as long as the quaternion is unit-length
Quaternion Quaternion::get_conjugate() const
{
	return Quaternion(-x, -y, -z, w);
}

// Multiplying q1 with q2 applies the rotation q2 to q1
Quaternion Quaternion::operator* (const Quaternion &rq) const
{
	return Quaternion(w * rq.x + x * rq.w + y * rq.z - z * rq.y,
					  w * rq.y + y * rq.w + z * rq.x - x * rq.z,
					  w * rq.z + z * rq.w + x * rq.y - y * rq.x,
					  w * rq.w - x * rq.x - y * rq.y - z * rq.z);
}

// Multiplying a quaternion q with a vector v applies the q-rotation to v
Vec3 Quaternion::operator* (const Vec3 &vec) const
{
	Vec3 vn(vec);
  vn.Normalise();
 
	Quaternion vecQuat, resQuat;
	vecQuat.x = vn[0];
	vecQuat.y = vn[1];
	vecQuat.z = vn[2];
	vecQuat.w = 0.0f;
 
  //Quaternion conj=get_conjugate();
	resQuat = vecQuat * get_conjugate();
	resQuat = *this * resQuat;
 
	return (Vec3(resQuat.x, resQuat.y, resQuat.z));
}

// Convert from Axis Angle
void Quaternion::FromAxis(const Vec3& v, float angle)
{
	float sinAngle;
	angle *= 0.5f;
	Vec3 vn(v);
	vn.Normalise();
 
	sinAngle = sinf(angle);
 
	x = (vn[0] * sinAngle);
	y = (vn[1] * sinAngle);
	z = (vn[2] * sinAngle);
	w = cosf(angle);
}

// Convert from Euler Angles
void Quaternion::FromEuler(float pitch, float yaw, float roll)
{
	// Basically we create 3 Quaternions, one for pitch, one for yaw, one for roll
	// and multiply those together.
	// the calculation below does the same, just shorter
 
	float p = pitch * DEG_TO_RAD / 2.0;
	float y = yaw * DEG_TO_RAD / 2.0;
	float r = roll * DEG_TO_RAD / 2.0;
 
	float sinp = sinf(p);
	float siny = sinf(y);
	float sinr = sinf(r);
	float cosp = cosf(p);
	float cosy = cosf(y);
	float cosr = cosf(r);
 
	x = sinr * cosp * cosy - cosr * sinp * siny;
	y = cosr * sinp * cosy + sinr * cosp * siny;
	z = cosr * cosp * siny - sinr * sinp * cosy;
	w = cosr * cosp * cosy + sinr * sinp * siny;
 
	normalise();
}

// Convert to Matrix
Mat4 Quaternion::getMatrix() const
{
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;
	float xy = x * y;
	float xz = x * z;
	float yz = y * z;
	float wx = w * x;
	float wy = w * y;
	float wz = w * z;
 
	// This calculation would be a lot more complicated for non-unit length quaternions
	// Note: The constructor of Matrix4 expects the Matrix in column-major format like expected by
	//   OpenGL
	return Mat4( 1.0f - 2.0f * (y2 + z2), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
			         2.0f * (xy + wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz - wx), 0.0f,
			         2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
			         0.0f, 0.0f, 0.0f, 1.0f);
}

// Convert to Axis/Angles
void Quaternion::getAxisAngle(Vec3* axis, float* angle)
{
	float scale = sqrtf(x * x + y * y + z * z);
  (*axis)[0] = x / scale;
	(*axis)[1] = y / scale;
	(*axis)[2] = z / scale;
	*angle = acos(w) * 2.0f;
}


