/**
 * @file component.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-17
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include <iostream>
#include <vector>

#include "component/textinput.h"
#include "harfbuzz/hb.h"
#include "utils/box.h"
#include "utils/boxedobj.h"
#include "utils/quadtree.h"

// GLFW
#include "GLFW/glfw3.h"

#define SK_GANESH
#define SK_GL
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"

namespace mocoder {

using namespace std;

class Component : public BoxedObj {
 public:
  Component(SkFont* _font, hb_font_t* _hb_font, SkCanvas** _canvas, double _w,
            double _h, const Box& box)
      : text_(_font, _hb_font),
        canvas(_canvas),
        width(_w),
        height(_h),
        BoxedObj(box) {}

  class Port {
   public:
    Vec2d pos;
    Port(Vec2d _pos) : pos(_pos){};
  };

  vector<Port> ports_;
  SkCanvas** canvas;

  TextInput text_;

  enum class Status { UNSELECTED, SELECTED, MOVING, ZOOMING, EDITING };

  enum EditStatus { WAITING, PREEDIT, EDITING };
  EditStatus editstatus = WAITING;

  enum class ZoomStatus { NO, LU, RU, LD, RD };

  Status status = Status::UNSELECTED;
  ZoomStatus zstatus = ZoomStatus::NO;
  Vec2d relative_curpos_[4];
  FrameCounter fc;
  int prevframe = 0;

  int depth_ = 0;

  int width = 0, height = 0;

  virtual void Render(QuadTreeNode* node, double w, double h) = 0;

  bool Selected() {
    return status == Status::SELECTED || status == Status::MOVING ||
           status == Status::ZOOMING || status == Status::EDITING;
  }

  virtual void RenderPorts(double w, double h) {
    return;
    for (auto& i : ports_) {
      SkPaint paint;
    }
  }

  virtual void UpdateSize(double _w, double _h) {
    width = _w;
    height = _h;
    fc.RenderFrame();
  }

  Box GetInbox() {
    inbox_ = box_;
    inbox_.size_ = inbox_.size_ - Vec2d(30, 30);
    inbox_.pos_ = inbox_.pos_ + Vec2d(15, 15);
    return inbox_;
  }

  bool OutofWindow(Box box) {
    return !(box.pos_.x > 100 && box.pos_.x + box.size_.x < width &&
             box_.pos_.y >= 0 && box.pos_.y + box.size_.y < height);
  }

  virtual bool IsCollided(Box box) { return box_.IsCollided(box); }

  virtual void CursorEvent(QuadTreeNode* node, bool ldown, double xpos,
                           double ypos, Vec2d velocity) {
    // GetInbox();
    if (!Selected()) {
      return;
    }
    Box mousebox = Box(Vec2d(xpos, ypos) - velocity, velocity * 2);
    // Box mousebox2 = Box(Vec2d(xpos, ypos) - velocity * 2, velocity * 4);
    if (status == Status::SELECTED) {
      if (ldown && inbox_.IsCollided(mousebox)) {
        status = Status::MOVING;
        relative_curpos_[0] = box_.pos_ - Vec2d(xpos, ypos);
        relative_curpos_[1] =
            box_.pos_ + Vec2d(box_.size_.x, 0) - Vec2d(xpos, ypos);
        relative_curpos_[2] =
            box_.pos_ + Vec2d(0, box_.size_.y) - Vec2d(xpos, ypos);
        relative_curpos_[3] = box_.pos_ + box_.size_ - Vec2d(xpos, ypos);
      } else if (ldown) {
        status = Status::ZOOMING;
        relative_curpos_[0] = box_.pos_ - Vec2d(xpos, ypos);
        relative_curpos_[1] =
            box_.pos_ + Vec2d(box_.size_.x, 0) - Vec2d(xpos, ypos);
        relative_curpos_[2] =
            box_.pos_ + Vec2d(0, box_.size_.y) - Vec2d(xpos, ypos);
        relative_curpos_[3] = box_.pos_ + box_.size_ - Vec2d(xpos, ypos);
      }
    }
    if (status == Status::MOVING) {
      if (ldown) {
        if (box_.IsCollided(mousebox) || !box_.IsCollided(mousebox)) {
          Vec2d pos = Vec2d(xpos, ypos) + relative_curpos_[0];
          if (!OutofWindow(Box(pos, box_.size_))) {  // Need to be optimized
            SetBox(Box(pos, box_.size_));
          } else if (!OutofWindow(
                         Box(Vec2d(box_.pos_.x, ypos + relative_curpos_[0].y),
                             box_.size_))) {
            SetBox(Box(Vec2d(box_.pos_.x, ypos + relative_curpos_[0].y),
                       box_.size_));
            relative_curpos_[0] = box_.pos_ - Vec2d(xpos, ypos);
            if (-relative_curpos_[0].x < 20) {
              relative_curpos_[0].x = -20;
            }
            if (-relative_curpos_[0].x > box_.size_.x - 20) {
              relative_curpos_[0].x = -box_.size_.x + 20;
            }
            if (-relative_curpos_[0].y < 20) {
              relative_curpos_[0].y = -20;
            }
            if (-relative_curpos_[0].y > box_.size_.y - 20) {
              relative_curpos_[0].y = -box_.size_.y + 20;
            }
          } else if (!OutofWindow(
                         Box(Vec2d(xpos + relative_curpos_[0].x, box_.pos_.y),
                             box_.size_))) {
            SetBox(Box(Vec2d(xpos + relative_curpos_[0].x, box_.pos_.y),
                       box_.size_));
            relative_curpos_[0] = box_.pos_ - Vec2d(xpos, ypos);
            if (-relative_curpos_[0].x < 20) {
              relative_curpos_[0].x = -20;
            }
            if (-relative_curpos_[0].x > box_.size_.x - 20) {
              relative_curpos_[0].x = -box_.size_.x + 20;
            }
            if (-relative_curpos_[0].y < 20) {
              relative_curpos_[0].y = -20;
            }
            if (-relative_curpos_[0].y > box_.size_.y - 20) {
              relative_curpos_[0].y = -box_.size_.y + 20;
            }
          }
        }
      } else {
        status = Status::SELECTED;
      }
    }
    if (status == Status::ZOOMING) {
      if (ldown) {
        Box lubox = Box(box_.pos_, Vec2d(15, 15));
        Box rubox = Box(box_.pos_ + Vec2d(box_.size_.x - 15, 0), Vec2d(15, 15));
        Box ldbox = Box(box_.pos_ + Vec2d(0, box_.size_.y - 15), Vec2d(15, 15));
        Box rdbox = Box(box_.pos_ + box_.size_ - Vec2d(15, 15), Vec2d(15, 15));
        if (zstatus == ZoomStatus::NO) {
          if (lubox.IsCollided(mousebox)) {
            zstatus = ZoomStatus::LU;
          } else if (rubox.IsCollided(mousebox)) {
            zstatus = ZoomStatus::RU;
          } else if (rdbox.IsCollided(mousebox)) {
            zstatus = ZoomStatus::RD;
          } else if (ldbox.IsCollided(mousebox)) {
            zstatus = ZoomStatus::LD;
          }
        }
        if (zstatus == ZoomStatus::LU) {
          Vec2d static_point = box_.size_ + box_.pos_;
          Vec2d pos = Vec2d(xpos, ypos) + relative_curpos_[0];
          Vec2d size = static_point - pos;
          if (size.x < 50) {
            size.x = 50;
            pos.x = static_point.x - 50;
          }
          if (size.y < 50) {
            size.y = 50;
            pos.y = static_point.y - 50;
          }
          if (!OutofWindow(Box(pos, size))) {
            SetBox(Box(pos, size));
          }
        } else if (zstatus == ZoomStatus::RU) {
          Vec2d static_point = box_.pos_ + Vec2d(0, box_.size_.y);
          Vec2d pos =
              Vec2d(static_point.x, ypos) + Vec2d(0, relative_curpos_[1].y);
          Vec2d size = Vec2d(xpos + relative_curpos_[1].x - static_point.x,
                             static_point.y - pos.y);
          if (size.x < 50) {
            size.x = 50;
          }
          if (size.y < 50) {
            size.y = 50;
            pos.y = static_point.y - 50;
          }
          if (!OutofWindow(Box(pos, size))) {
            SetBox(Box(pos, size));
          }
        } else if (zstatus == ZoomStatus::LD) {
          Vec2d static_point = box_.pos_ + Vec2d(box_.size_.x, 0);
          Vec2d pos = Vec2d(xpos + relative_curpos_[2].x, static_point.y);
          Vec2d size = Vec2d(static_point.x - pos.x,
                             ypos + relative_curpos_[2].y - static_point.y);
          if (size.x < 50) {
            size.x = 50;
            pos.x = static_point.x - 50;
          }
          if (size.y < 50) {
            size.y = 50;
          }
          if (!OutofWindow(Box(pos, size))) {
            SetBox(Box(pos, size));
          }
        } else if (zstatus == ZoomStatus::RD) {
          Vec2d static_point = box_.pos_;
          Vec2d pos = static_point;
          Vec2d size = -static_point + relative_curpos_[3] + Vec2d(xpos, ypos);
          if (size.x < 50) {
            size.x = 50;
          }
          if (size.y < 50) {
            size.y = 50;
          }
          if (!OutofWindow(Box(pos, size))) {
            SetBox(Box(pos, size));
          }
        }
      } else {
        status = Status::SELECTED;
        zstatus = ZoomStatus::NO;
      }
    }
  }

  virtual void ButtonEvent(QuadTreeNode* node, int button, int type) {
    if (status == Status::UNSELECTED) {
      if (button == 0) {
        if (type == 1) {
          // status = Status::PRESELECT;
          status = Status::SELECTED;
        }
      }
    } else if (status == Status::MOVING) {
      if (button == 0 && type == 0) {
        status = Status::SELECTED;
      }
    } else if (status == Status::ZOOMING) {
      if (button == 0 && type == 0) {
        status = Status::SELECTED;
        zstatus = ZoomStatus::NO;
      }
    } else if (status == Status::SELECTED) {
      if (button == 0 && type == 1) {
        if (editstatus == WAITING) {
          editstatus = PREEDIT;
        }
      }
      if (button == 0 && type == 0) {
        if (editstatus == PREEDIT) {
          editstatus = EDITING;
          text_.status_ = TextInput::EDIT;
        }
      }
    }
  }

  virtual int OnKeyboard(GLFWwindow* window, int key, int action,
                         int modifier) {
    if (status == Status::SELECTED) {
      if (text_.status_ != TextInput::EDIT) {
        if ((key == GLFW_KEY_DELETE || key == GLFW_KEY_BACKSPACE) &&
            action == GLFW_PRESS) {
          return -1;
        }
      } else {
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
          editstatus = WAITING;
          text_.status_ = TextInput::SHOW;
        } else {
          text_.OnKeyboard(window, key, action, modifier);
          return -2;
        }
      }
    }
    return 0;
  }

  virtual vector<Vec2d> GetLineIntersection(Vec2d p1, Vec2d p2) = 0;

  void Unselect() {
    status = Status::UNSELECTED;
    text_.status_ = TextInput::SHOW;
    editstatus = WAITING;
  }

  virtual bool IsArrow() { return false; }

  virtual void OnCharEvent(unsigned codepoint) {
    if (editstatus == EDITING) {
      text_.OnChar(codepoint);
    }
  }

 protected:
};

}  // namespace mocoder
