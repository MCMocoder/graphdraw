/**
 * @file vec2d.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-17
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include <cstdlib>

namespace mocoder {

class Vec2d {
 public:
  double x;
  double y;
  Vec2d(double _x, double _y) : x(_x), y(_y) {}
  Vec2d() : x(0.0), y(0.0) {}
  Vec2d operator+(const Vec2d& b) { return Vec2d(x + b.x, y + b.y); }
  Vec2d operator-(const Vec2d& b) { return Vec2d(x - b.x, y - b.y); }
  Vec2d operator-() { return Vec2d(-x, -y); }
  Vec2d operator*(const Vec2d& b) { return Vec2d(x * b.x, y * b.y); }
  Vec2d operator*(const double&& b) { return Vec2d(x * b, y * b); }
  Vec2d operator/(const Vec2d& b) { return Vec2d(x / b.x, y / b.y); }
  Vec2d operator/(const double b) { return Vec2d(x / b, y / b); }
  bool operator==(const Vec2d&& b) { return (b.x == x) && (b.y == y); }
  bool operator==(Vec2d& b) { return (b.x == x) && (b.y == y); }
  Vec2d Abs() {
    return Vec2d(abs(x),abs(y));
  }
};

}
