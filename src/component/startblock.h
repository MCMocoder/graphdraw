/**
 * @file startblock.h
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

class StartBlock : public Component {
 public:
  virtual void Render(double w,double h) override {
    renderer_.AddVertex(box_.GetVertex());
    renderer_.Render(0,0,0);
  }
};

}
