/**
 * @file frame.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-06-03
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include <cstdint>
namespace mocoder {

class FrameCounter {
 public:
  unsigned frame = 0;

  void RenderFrame() { ++frame; }

  unsigned GetDeltaFrame(unsigned prev) {
    if (frame > prev) {
      return frame - prev;
    } else {
      return UINT32_MAX - frame + prev;
    }
  }
};

}  // namespace mocoder
