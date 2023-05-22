/**
 * @file box.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-17
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include <vector>

#include "utils/vec2d.h"

namespace mocoder {

using namespace std;

class Box {
 public:
  Vec2d pos_;
  Vec2d size_;
  Box(const Box &box):pos_(box.pos_),size_(box.size_){}
  Box(Vec2d pos, Vec2d size) : pos_(pos), size_(size) {}
  Box() : pos_(0, 0), size_(0, 0) {}
  bool IsCollided(Box b) {
    // https://heptaluan.github.io/2020/11/28/Essay/31/
    return (pos_.x < b.pos_.x + b.size_.x) && (pos_.x + size_.x > b.pos_.x) &&
           (pos_.y < b.pos_.y + b.size_.y) && (pos_.y + size_.y > b.pos_.y);
  }
  vector<Vec2d> GetVertex() {
    return {Vec2d(pos_.x, pos_.y), Vec2d(pos_.x, pos_.y + size_.y),
            Vec2d(pos_.x + size_.x, pos_.y + size_.y),
            Vec2d(pos_.x + size_.x, pos_.y)};
  }

  bool operator==(Box& b) {
    return pos_==b.pos_&&size_==b.size_;
  }
};

}  // namespace mocoder
