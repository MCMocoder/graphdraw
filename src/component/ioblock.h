/**
 * @file ioblock.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-06-07
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include "component/component.h"
#include "include/effects/SkDashPathEffect.h"

namespace mocoder {

class IOBlock : public Component {
 public:
  IOBlock(SkFont* _font, hb_font_t* _hb_font, SkCanvas** _canvas, double w,
          double h, const Box& box)
      : Component(_font, _hb_font, _canvas, w, h, box) {
    ports_ = {Vec2d(0.4, 1), Vec2d(0.6, 0), Vec2d(0.1, 0.5), Vec2d(0.9, 0.5)};
  }

  virtual void Render(QuadTreeNode* node, double w, double h) override {
    UpdateSize(w, h);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(2);
    if (Selected()) {
      paint.setColor(SK_ColorBLUE);
    } else {
      paint.setColor(SK_ColorBLACK);
    }

    (*canvas)->drawLine(box_.pos_.x + box_.size_.x * 0.2, box_.pos_.y,
                        box_.pos_.x + box_.size_.x, box_.pos_.y, paint);

    (*canvas)->drawLine(box_.pos_.x, box_.pos_.y + box_.size_.y,
                        box_.pos_.x + box_.size_.x * 0.8,
                        box_.pos_.y + box_.size_.y, paint);

    (*canvas)->drawLine(box_.pos_.x + box_.size_.x * 0.2, box_.pos_.y,
                        box_.pos_.x, box_.pos_.y + box_.size_.y, paint);

    (*canvas)->drawLine(box_.pos_.x + box_.size_.x, box_.pos_.y,
                        box_.pos_.x + box_.size_.x * 0.8,
                        box_.pos_.y + box_.size_.y, paint);

    if (Selected()) {
      float interval[] = {10, 20};
      paint.setPathEffect(SkDashPathEffect::Make(interval, 2, 0.0f));
      (*canvas)->drawRect(box_.GetEdge(), paint);
    }

    double textwidth = box_.size_.x - 30;
    double textheight = box_.size_.y - 30;
    Vec2d center = box_.Mid();
    text_.RerenderText(canvas, center - Vec2d(textwidth, textheight) / 2.0,box_.Mid(),
                       textwidth, textheight);
  }

  virtual vector<Vec2d> GetLineIntersection(Vec2d p1, Vec2d p2) override {
    vector<Vec2d> points = {
        Vec2d(box_.pos_.x + box_.size_.x * 0.2, box_.pos_.y),
        Vec2d(box_.pos_.x, box_.pos_.y + box_.size_.y),
        Vec2d(box_.pos_.x + box_.size_.x * 0.8, box_.pos_.y + box_.size_.y),
        Vec2d(box_.pos_.x + box_.size_.x, box_.pos_.y)};
    vector<Vec2d> res;
    auto a = GetTwoLineIntersection(p1, p2, points[0], points[1]);
    if (a.has_value()) {
      res.push_back(a.value());
    }
    auto b = GetTwoLineIntersection(p1, p2, points[1], points[2]);
    if (b.has_value()) {
      res.push_back(b.value());
    }
    auto c = GetTwoLineIntersection(p1, p2, points[2], points[3]);
    if (c.has_value()) {
      res.push_back(c.value());
    }
    auto d = GetTwoLineIntersection(p1, p2, points[3], points[0]);
    if (d.has_value()) {
      res.push_back(d.value());
    }
    return res;
  }

 private:
  // https://zhuanlan.zhihu.com/p/158533421
  virtual optional<Vec2d> GetTwoLineIntersection(Vec2d a, Vec2d b, Vec2d c,
                                                 Vec2d d) {
    Vec2d n2(d.y - c.y, c.x - d.x);
    double prjc2 = c.x * n2.x + c.y * n2.y;
    double prja2 = a.x * n2.x + a.y * n2.y;
    double prjb2 = b.x * n2.x + b.y * n2.y;
    if ((prja2 - prjc2) * (prjb2 - prjc2) >= 0) {
      return optional<Vec2d>();
    }
    Vec2d n1(b.y - a.y, a.x - b.x);
    double prja1 = a.x * n1.x + a.y * n1.y;
    double prjc1 = c.x * n1.x + c.y * n1.y;
    double prjd1 = d.x * n1.x + d.y * n1.y;
    if ((prjc1 - prja1) * (prjd1 - prja1) >= 0) {
      return optional<Vec2d>();
    }
    double denominator = n1.x * n2.y - n1.y * n2.x;
    double fraction = (prja2 - prjc2) / denominator;
    return Vec2d(a.x + fraction * n1.y, a.y - fraction * n1.x);
  }
};

}  // namespace mocoder
