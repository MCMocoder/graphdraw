/**
 * @file boxedobj.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-17
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include "utils/box.h"

namespace mocoder {

class BoxedObj {
 public:
  Box box_;
  Box inbox_;
  BoxedObj(const Box& box) { SetBox(box); }
  virtual ~BoxedObj() {}
  void SetBox(Box box) {
    if (box.pos_.x < 0) {
      box.pos_.x = 0;
    }
    if (box.pos_.y < 0) {
      box.pos_.y = 0;
    }
    if (box.size_.x < 0) {
      box.size_.x = 0;
    }
    if (box.size_.y < 0) {
      box.size_.y = 0;
    }
    box_ = box;
    inbox_ = box_;
    inbox_.size_ = inbox_.size_ - Vec2d(30, 30);
    inbox_.pos_ = inbox_.pos_ + Vec2d(15, 15);
  }
};

}  // namespace mocoder
