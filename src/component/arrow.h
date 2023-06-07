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

#include <algorithm>
#include <ostream>
#include <set>

#include "component/component.h"
#include "include/core/SkColor.h"

namespace mocoder {

class Arrow : public Component {
 public:
  Component *start_ = nullptr, *end_ = nullptr;
  Component::Port *startport_ = nullptr, *endport_ = nullptr;
  Vec2d p1, p2;

  enum ArrowStatus { PREDRAW, DRAWING, COMPLETED, FAIL };

  Arrow(SkFont* _font, hb_font_t* _hb_font, SkCanvas** _canvas, double w,
        double h, Component* start, Component* end)
      : Component(_font, _hb_font, _canvas, w, h,
                  Box(start->box_.pos_, end->box_.pos_)),
        start_(start),
        end_(end) {}

  Arrow(SkFont* _font, hb_font_t* _hb_font, SkCanvas** _canvas, double w,
        double h, Box box)
      : Component(_font, _hb_font, _canvas, w, h, box) {}

  Arrow(SkFont* _font, hb_font_t* _hb_font, SkCanvas** _canvas, double w,
        double h)
      : Component(_font, _hb_font, _canvas, w, h, Box()) {}

  ArrowStatus astatus_ = PREDRAW;

  Box GetBox(Vec2d pos1, Vec2d pos2) {
    double minx = min(pos1.x, pos2.x);
    double miny = min(pos1.y, pos2.y);
    double maxx = max(pos1.x, pos2.x);
    double maxy = max(pos1.y, pos2.y);
    Vec2d a(minx, miny);
    Vec2d b(maxx, maxy);
    return Box(a, b - a);
  }

  void UpdatePos() {
    if (start_ != nullptr && startport_ != nullptr) {
      p1 = start_->box_.pos_ + start_->box_.size_ * startport_->pos;
    }
    if (end_ != nullptr && endport_ != nullptr) {
      p2 = end_->box_.pos_ + end_->box_.size_ * endport_->pos;
    }
  }

  void Render(QuadTreeNode* node, double w, double h) override {
    UpdateSize(w, h);

    UpdatePos();
    Vec2d pos1 = p1;
    Vec2d pos2 = p2;
    box_ = GetBox(pos1, pos2);
    if (astatus_ == COMPLETED && start_ != nullptr && end_ != nullptr) {
      vector<pair<Vec2d, Vec2d>> parts;

      struct InterPoint {
        Component* component;
        Vec2d point;
        InterPoint(Component* _component, Vec2d _point)
            : component(_component), point(_point) {}
      };

      vector<InterPoint> points;

      auto t = node->Retrieve(box_);
      vector<Component*> collided;
      for (auto i : t) {
        Component* ti = (Component*)i;
        if (ti != this && ti != start_ && ti != end_ && !ti->IsArrow() &&
            ti->IsCollided(box_)) {
          collided.push_back(ti);
        }
      }

      for (auto i : collided) {
        auto p = i->GetLineIntersection(pos1, pos2);
        if (!p.empty()) {
          for (auto j : p) {
            points.push_back(InterPoint(i, j));
          }
        }
      }

      bool start_collided = false;
      for (auto i : collided) {
        if (i->IsCollided(Box(pos1, Vec2d()))) {
          start_collided = true;
          points.push_back(InterPoint(i, pos1));
          // break;
        }
      }

      bool end_collided = false;
      for (auto i : collided) {
        if (i->IsCollided(Box(pos2, Vec2d()))) {
          end_collided = true;
          points.push_back(InterPoint(i, pos2));
          // break;
        }
      }

      sort(points.begin(), points.end(),
           [pos1](InterPoint a, InterPoint b) -> bool {
             return (a.point - pos1).SquareDist() <
                    (b.point - pos1).SquareDist();
           });

      set<Component*> start_on;
      bool start_under = false;
      int i = 0;
      if (start_collided) {
        for (i = 0; i < points.size(); ++i) {
          if (!start_on.empty()) {
            if (start_on.contains(points[i].component)) {
              start_on.erase(points[i].component);
              if (start_on.empty()) {
                break;
              }
              continue;
            }
          }
          if (points[i].component->depth_ > start_->depth_) {
            start_under = true;
            start_on.insert(points[i].component);
          }
        }
      }

      // i -= 1;
      Vec2d midstart =
          start_under && i < points.size() ? points[i].point : pos1;

      set<Component*> end_on;
      bool end_under = false;
      int j = points.size();
      if (end_collided) {
        for (j = points.size() - 1; j >= 0; --j) {
          if (!end_on.empty()) {
            if (end_on.contains(points[j].component)) {
              end_on.erase(points[j].component);
              if (end_on.empty()) {
                break;
              }
              continue;
            }
          }
          if (points[j].component->depth_ > end_->depth_) {
            end_under = true;
            end_on.insert(points[j].component);
          }
        }
      }

      // j += 1;
      Vec2d midend = end_under && j >= 0 ? points[j].point : pos2;

      if (start_under && end_under && i > j && !points.empty()) {
        return;
      }

      SkPaint paint;
      paint.setStyle(SkPaint::kStroke_Style);
      paint.setAntiAlias(true);
      paint.setStrokeWidth(2);
      if (status == Status::SELECTED || status == Status::MOVING ||
          status == Status::ZOOMING || status == Status::EDITING) {
        paint.setColor(SK_ColorBLUE);
        (*canvas)->drawLine(midstart.x, midstart.y, midend.x, midend.y, paint);
      } else {
        paint.setColor(SK_ColorBLACK);
        (*canvas)->drawLine(midstart.x, midstart.y, midend.x, midend.y, paint);
      }
    } else if (astatus_ == DRAWING) {
      SkPaint paint;
      paint.setStyle(SkPaint::kStroke_Style);
      paint.setAntiAlias(true);
      paint.setStrokeWidth(2);
      paint.setColor(SK_ColorBLUE);
      (*canvas)->drawLine(pos1.x, pos1.y, pos2.x, pos2.y, paint);
    }
  }

  virtual bool IsCollided(Box box) override {
    Vec2d point = box.pos_;
    // Ax1+By1=C
    // Ax2+By2=C
    double A, B, C;
    double t = p1.x * p2.y - p2.x * p1.y;
    if (t == 0.0) {
      C = 0.0;
      A = p1.y - p2.y;
      B = p1.x - p2.x;
    } else {
      C = 1.0;
      A = C * (p2.y - p1.y) / t;
      B = C * (p1.x - p2.x) / t;
    }

    // Bx-Ay=Bx0-Ay0=c1
    // Ax+By=C
    double c1 = B * point.x - A * point.y;
    double detd = A * A + B * B;
    double x = (c1 * B + C * A) / detd;
    if (x < min(p1.x, p2.x) || x > max(p1.x, p2.x)) {
      return false;
    }
    double d = abs(A * point.x + B * point.y - C) / sqrt(detd);
    if (d <= 5) {
      return true;
    }
    return false;
  }

  virtual vector<Vec2d> GetLineIntersection(Vec2d p1, Vec2d p2) override {
    return {};
  }

  virtual void CursorEvent(QuadTreeNode* node, bool ldown, double xpos,
                           double ypos, Vec2d velocity) override {
    if (astatus_ == PREDRAW) {
      if (ldown) {
        astatus_ = DRAWING;
        p1 = Vec2d(xpos, ypos);
        p2 = Vec2d(xpos, ypos);
      }
    } else if (astatus_ == DRAWING) {
      p2 = Vec2d(xpos, ypos);
    }
  }

  virtual void ButtonEvent(QuadTreeNode* node, int button, int type) override {
    Component::ButtonEvent(node, button, type);
    if (button == 0 && type == 0 && astatus_ == DRAWING) {
      astatus_ = COMPLETED;
      BindComponent(node);
    }
  }

  void BindComponent(QuadTreeNode* node) {
    auto t = node->Retrieve(Box(p1, Vec2d()));
    Component* startc = nullptr;
    for (auto i : t) {
      Component* c = (Component*)i;
      if (!c->IsArrow()) {
        if (c->IsCollided(Box(p1, Vec2d()))) {
          if (startc != nullptr) {
            if (startc->depth_ > c->depth_) {
              startc = c;
            }
          } else {
            startc = c;
          }
        }
      }
    }
    if (startc == nullptr) {
      astatus_ = FAIL;
      return;
    } else {
      start_ = startc;
    }
    t = node->Retrieve(Box(p2, Vec2d()));
    Component* endc = nullptr;
    for (auto i : t) {
      Component* c = (Component*)i;
      if (!c->IsArrow()) {
        if (c->IsCollided(Box(p2, Vec2d()))) {
          if (endc != nullptr) {
            if (endc->depth_ > c->depth_) {
              endc = c;
            }
          } else {
            endc = c;
          }
        }
      }
    }
    if (endc == nullptr) {
      astatus_ = FAIL;
      return;
    } else {
      end_ = endc;
    }
    if (startc == endc) {
      astatus_ = FAIL;
      return;
    }
    Vec2d startmid = start_->box_.Mid();
    Vec2d endmid = end_->box_.Mid();
    auto startinter = start_->GetLineIntersection(startmid, endmid)[0];
    auto endinter = end_->GetLineIntersection(startmid, endmid)[0];
    Port* startport = nullptr;
    for (auto& i : start_->ports_) {
      if (startport != nullptr) {
        if ((startport->pos * start_->box_.size_ + start_->box_.pos_ -
             startinter)
                .SquareDist() >
            (i.pos * start_->box_.size_ + start_->box_.pos_ - startinter)
                .SquareDist()) {
          startport = &i;
        }
      } else {
        startport = &i;
      }
    }
    startport_ = startport;
    Port* endport = nullptr;
    for (auto& i : end_->ports_) {
      if (endport != nullptr) {
        if ((endport->pos * end_->box_.size_ + end_->box_.pos_ - endinter)
                .SquareDist() >
            (i.pos * end_->box_.size_ + end_->box_.pos_ - endinter)
                .SquareDist()) {
          endport = &i;
        }
      } else {
        endport = &i;
      }
    }
    endport_ = endport;
  }

  virtual bool IsArrow() override { return true; }
};

}  // namespace mocoder
