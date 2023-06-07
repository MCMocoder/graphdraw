/**
 * @file vec3d.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-17
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

namespace mocoder {

class Vec3d {
 public:
  double x;
  double y;
  double z;
  Vec3d(double _x, double _y,double _z) : x(_x), y(_y),z(_z) {}
  Vec3d() : x(0.0), y(0.0),z(0.0) {}
  Vec3d operator+(const Vec3d& b) { return Vec3d(x + b.x, y + b.y,z+b.z); }
  Vec3d operator-(const Vec3d& b) { return Vec3d(x - b.x, y - b.y,z-b.z); }
  Vec3d operator*(const Vec3d& b) { return Vec3d(x * b.x, y * b.y,z*b.z); }
  Vec3d operator/(const Vec3d& b) { return Vec3d(x / b.x, y / b.y,z/b.z); }
  Vec3d operator/(const double b) { return Vec3d(x / b, y / b, z / b); }
  Vec3d cross(const Vec3d& b){
    return Vec3d(y * b.z - z * b.y, -x * b.z + z * b.x, x * b.y - y * b.x);
  };
};

}
