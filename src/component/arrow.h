/**
 * @file arrow.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-21
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include "component/component.h"

namespace mocoder {

class Arrow : public Component {
 public:
  Component *start_, *end_;

  void Render() {}

  virtual bool IsCollided(Box box) override {
    
  }
};

}
