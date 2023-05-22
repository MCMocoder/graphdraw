/**
 * @file process.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-17
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include "component/component.h"

namespace mocoder {

class ProcessBlock : public Component {
 public:
  ProcessBlock(double w, double h, const Box& box) : Component(w, h, box) {
    ports_ = {Vec2d(0, 0.5), Vec2d(0.5, 0), Vec2d(1, 0.5), Vec2d(0.5, 1)};
  }
  
  virtual void Render(double w,double h) override {
    /// RenderSelected();
    RenderPorts(w,h);
    Box box = box_;

    renderer_.w = w;
    renderer_.h = h;

    renderer_.AddVertex(box_.GetVertex());
    if (status == Status::SELECTED || status == Status::MOVING ||
        status == Status::ZOOMING || status == Status::EDITING) {
      renderer_.Render(0,0,1);
    } else {
      renderer_.Render(0,0,0);
    }
  }
};

}
