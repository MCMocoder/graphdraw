/**
 * @file manager.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-18
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include <cstddef>
#include <functional>
#include <memory>

#include "component/arrow.h"
#include "component/component.h"
#include "component/condblock.h"
#include "component/ioblock.h"
#include "component/process.h"
#include "component/startblock.h"
#include "component/subblock.h"
#include "include/core/SkColor.h"
#include "include/core/SkTypeface.h"
#include "utils/frame.h"
#include "utils/quadtree.h"

// GLFW
#include "GLFW/glfw3.h"

#define SK_GANESH
#define SK_GL
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"

namespace mocoder {

class UIManager {
 public:
  enum WorkStatus {
    SELECTION,
    PROCESSBLOCK,
    STARTBLOCK,
    IOBLOCK,
    SUBBLOCK,
    CONDBLOCK,
    ARROW
  };
  WorkStatus workstatus_ = SELECTION;
  vector<shared_ptr<Component>> components;

  Component* selected_ = nullptr;

  GrDirectContext* context = nullptr;

  SkCanvas* canvas = nullptr;
  SkSurface* surface = nullptr;
  sk_sp<SkTypeface> skface;
  SkFont font;
  hb_face_t* face = nullptr;
  hb_font_t* hb_font = nullptr;
  FrameCounter fc;

  QuadTreeNode tree_;

  bool leftdown;
  Vec2d cursorpos;

  void InitSkia(int w, int h) {
    auto interface = GrGLMakeNativeInterface();
    context = GrDirectContext::MakeGL(interface).release();

    GrGLFramebufferInfo framebufferInfo;
    framebufferInfo.fFBOID = 0;  // assume default framebuffer
    framebufferInfo.fFormat = GL_RGBA8;

    SkColorType colorType = kRGBA_8888_SkColorType;
    GrBackendRenderTarget backendRenderTarget(w, h,
                                              0,  // sample count
                                              0,  // stencil bits
                                              framebufferInfo);
    surface = SkSurface::MakeFromBackendRenderTarget(
                  context, backendRenderTarget, kBottomLeft_GrSurfaceOrigin,
                  colorType, nullptr, nullptr)
                  .release();
    if (surface == nullptr) {
      abort();
    }
    canvas = surface->getCanvas();
    InitFont();
  }

  void InitFont() {
    auto data = SkData::MakeFromFileName("font.ttf");
    skface = SkTypeface::MakeFromData(data, 0);
    auto destroy = [](void* d) { static_cast<SkData*>(d)->unref(); };
    const char* bytes = (const char*)data->data();
    unsigned int size = (unsigned int)data->size();
    hb_blob_t* blob = hb_blob_create(bytes, size, HB_MEMORY_MODE_READONLY,
                                     data.release(), destroy);
    hb_blob_make_immutable(blob);
    face = hb_face_create(blob, 0);
    hb_blob_destroy(blob);
    hb_font = hb_font_create(face);
    hb_font_set_scale(hb_font, FONT_SIZE * 64, FONT_SIZE * 64);
    font = SkFont(skface);
    font.setSize(FONT_SIZE);
  }

  void Close() {
    hb_font_destroy(hb_font);
    hb_face_destroy(face);
    delete surface;
    surface = nullptr;
    delete context;
    context = nullptr;
    canvas = nullptr;
  }

  void AddComponent(shared_ptr<Component> component) {
    components.push_back(component);
    component->depth_ = components.size();
    UpdateDepth();
  }

  void DelComponent(Component* c) {
    for (auto i = components.begin(); i != components.end();) {
      if ((*i)->IsArrow() && !(c->IsArrow())) {
        Arrow* arrow = (Arrow*)i->get();
        if (arrow->start_ == c || arrow->end_ == c) {
          i = components.erase(i);
        } else {
          ++i;
        }
      }
      if (i->get() == c) {
        i = components.erase(i);
      } else {
        ++i;
      }
    }
    UpdateDepth();
  }

  void UpdateDepth() {
    for (int i = 0; i < components.size(); ++i) {
      components[i]->depth_ = i;
    }
  }

  double width, height;

  UIManager(double w, double h)
      : tree_(Box(Vec2d(0, 0), Vec2d(w, h))), width(w), height(h) {
    // InitSkia(width, height);
  }

  void ProcessFrame(double w, double h) {
    tree_.Clear();
    tree_.bound_ = Box(Vec2d(0, 0), Vec2d(w, h));
    for (auto& i : components) {
      tree_.Insert(i.get());
    }

    if (w != width || h != height) {
      OnWindowSizeChange(w, h);
    }

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    canvas->drawPaint(paint);

    paint.setColor(SK_ColorBLACK);

    canvas->drawLine(100, 0, 100, h, paint);

    for (auto& i : components) {
      i->Render(&tree_, w, h);
    }

    if (cursorpos.x > 100) {
      if (workstatus_ == PROCESSBLOCK) {
        DrawProcessBlock(cursorpos.x, cursorpos.y);
      } else if (workstatus_ == STARTBLOCK) {
        DrawStartBlock(cursorpos.x, cursorpos.y);
      } else if (workstatus_ == IOBLOCK) {
        DrawIOBlock(cursorpos.x, cursorpos.y);
      } else if (workstatus_ == SUBBLOCK) {
        DrawSubBlock(cursorpos.x, cursorpos.y);
      } else if (workstatus_ == CONDBLOCK) {
        DrawCondBlock(cursorpos.x, cursorpos.y);
      }
    }

    DrawSidebar(w, h);

    context->flush();
    fc.RenderFrame();
  }

  void OnCursorEvent(double xpos, double ypos) {
    Vec2d velocity = (Vec2d(xpos, ypos) - cursorpos).Abs();
    cursorpos = Vec2d(xpos, ypos);
    if (cursorpos.x > 100) {
      if (workstatus_ == SELECTION) {
        if (selected_ != nullptr) {
          if (selected_->status == Component::Status::MOVING ||
              selected_->status == Component::Status::ZOOMING) {
            selected_->CursorEvent(&tree_, leftdown, xpos, ypos, velocity * 2);
            return;
          }
        }
        auto t = tree_.Retrieve(Box(cursorpos - velocity, velocity * 2));
        if (t.size() != 0) {
          vector<int> collided_index;
          for (int i = 0; i < t.size(); ++i) {
            Component* ti = (Component*)t[i];
            if (ti->IsCollided(Box(cursorpos, Vec2d(0, 0)))) {
              collided_index.push_back(i);
            }
          }
          UpdateDepth();
          if (!collided_index.empty()) {
            int m = -1;
            for (int i = 0; i < collided_index.size(); ++i) {
              Component* ti = (Component*)t[i];
              if (ti->depth_ > m) {
                m = i;
              }
            }
            Component* ti = (Component*)t[collided_index[m]];
            ti->CursorEvent(&tree_, leftdown, xpos, ypos, velocity);
          }
        }
      } else if (workstatus_ == ARROW) {
        if (selected_ != nullptr) {
          selected_->CursorEvent(&tree_, leftdown, xpos, ypos, velocity * 2);
        } else {
          if (leftdown) {
            auto arrow = make_shared<Arrow>(
                Arrow(&font, hb_font, &canvas, width, height));
            AddComponent(arrow);
            if (selected_ != nullptr && selected_ != arrow.get()) {
              selected_->Unselect();
            }
            selected_ = arrow.get();
            arrow->CursorEvent(&tree_, leftdown, xpos, ypos, velocity * 2);
          }
        }
      }
    }
  }

  void DrawProcessBlock(double xpos, double ypos) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(2);
    paint.setColor(SK_ColorBLACK);
    Box box = Box(Vec2d(xpos, ypos) - Vec2d(75, 50), Vec2d(150, 100));
    canvas->drawRect(box.GetEdge(), paint);
  }

  void DrawStartBlock(double xpos, double ypos) {
    Box box = Box(Vec2d(xpos, ypos) - Vec2d(75, 50), Vec2d(150, 100));
    SkPaint paint;

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(2);

    paint.setColor(SK_ColorBLACK);

    double r = sqrt(0.09 * box.size_.x * box.size_.x +
                    0.25 * box.size_.y * box.size_.y);
    double angle =
        atan((0.5 * box.size_.y) / (0.3 * box.size_.x)) * 180 / 3.14159;
    Vec2d o1(box.pos_.x + r, box.pos_.y + box.size_.y * 0.5);
    SkRect rect1 = SkRect::MakeXYWH(o1.x - r, o1.y - r, 2 * r, 2 * r);
    (*canvas).drawArc(rect1, 180.0 - angle, 2 * angle, false, paint);

    Vec2d o2(box.pos_.x + box.size_.x - r, box.pos_.y + box.size_.y * 0.5);
    SkRect rect2 = SkRect::MakeXYWH(o2.x - r, o2.y - r, 2 * r, 2 * r);
    (*canvas).drawArc(rect2, 360 - angle, 2 * angle, false, paint);

    (*canvas).drawLine(box.pos_.x - box.size_.x * 0.3 + r, box.pos_.y,
                       box.pos_.x + box.size_.x - r + box.size_.x * 0.3,
                       box.pos_.y, paint);
    (*canvas).drawLine(box.pos_.x - box.size_.x * 0.3 + r,
                       box.pos_.y + box.size_.y,
                       box.pos_.x + box.size_.x - r + box.size_.x * 0.3,
                       box.pos_.y + box.size_.y, paint);
  }

  void DrawIOBlock(double xpos, double ypos) {
    Box box_ = Box(Vec2d(xpos, ypos) - Vec2d(75, 50), Vec2d(150, 100));

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(2);

    (*canvas).drawLine(box_.pos_.x + box_.size_.x * 0.2, box_.pos_.y,
                       box_.pos_.x + box_.size_.x, box_.pos_.y, paint);

    (*canvas).drawLine(box_.pos_.x, box_.pos_.y + box_.size_.y,
                       box_.pos_.x + box_.size_.x * 0.8,
                       box_.pos_.y + box_.size_.y, paint);

    (*canvas).drawLine(box_.pos_.x + box_.size_.x * 0.2, box_.pos_.y,
                       box_.pos_.x, box_.pos_.y + box_.size_.y, paint);

    (*canvas).drawLine(box_.pos_.x + box_.size_.x, box_.pos_.y,
                       box_.pos_.x + box_.size_.x * 0.8,
                       box_.pos_.y + box_.size_.y, paint);
  }

  void DrawSubBlock(double xpos, double ypos) {
    Box box_ = Box(Vec2d(xpos, ypos) - Vec2d(75, 50), Vec2d(150, 100));
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(2);
    paint.setColor(SK_ColorBLACK);
    (*canvas).drawRect(box_.GetEdge(), paint);
    (*canvas).drawLine(box_.pos_.x + 15, box_.pos_.y, box_.pos_.x + 15,
                       box_.pos_.y + box_.size_.y, paint);
    (*canvas).drawLine(box_.pos_.x + box_.size_.x - 15, box_.pos_.y,
                       box_.pos_.x + box_.size_.x - 15,
                       box_.pos_.y + box_.size_.y, paint);
  }

  void DrawCondBlock(double xpos, double ypos) {
    Box box = Box(Vec2d(xpos, ypos) - Vec2d(75, 50), Vec2d(150, 100));
    SkPaint paint;

    Vec2d mid = box.Mid();

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(2);
    paint.setColor(SK_ColorBLACK);
    (*canvas).drawLine(mid.x, box.pos_.y, box.pos_.x, mid.y, paint);
    (*canvas).drawLine(box.pos_.x, mid.y, mid.x, box.pos_.y + box.size_.y,
                       paint);
    (*canvas).drawLine(mid.x, box.pos_.y + box.size_.y,
                       box.pos_.x + box.size_.x, mid.y, paint);
    (*canvas).drawLine(box.pos_.x + box.size_.x, mid.y, mid.x, box.pos_.y,
                       paint);
  }

  void OnButtonEvent(int button, int type) {
    if (button == 0) {
      if (type == 1) {
        leftdown = true;
      } else {
        leftdown = false;
      }
    }
    if (cursorpos.x <= 100) {
      if (button == 0 && type == 1) {
        if (Box(Vec2d(0, 0), Vec2d(100, 100))
                .IsCollided(Box(cursorpos, Vec2d())) &&
            workstatus_ != SELECTION) {
          workstatus_ = SELECTION;
        } else if (Box(Vec2d(0, 600), Vec2d(100, 100))
                       .IsCollided(Box(cursorpos, Vec2d())) &&
                   workstatus_ != ARROW) {
          workstatus_ = ARROW;
          if (selected_ != nullptr) {
            selected_->Unselect();
            selected_ = nullptr;
          }

        } else if (Box(Vec2d(0, 100), Vec2d(100, 100))
                       .IsCollided(Box(cursorpos, Vec2d())) &&
                   workstatus_ != PROCESSBLOCK) {
          workstatus_ = PROCESSBLOCK;
          if (selected_ != nullptr) {
            selected_->Unselect();
            selected_ = nullptr;
          }
        } else if (Box(Vec2d(0, 200), Vec2d(100, 100))
                       .IsCollided(Box(cursorpos, Vec2d())) &&
                   workstatus_ != STARTBLOCK) {
          workstatus_ = STARTBLOCK;
          if (selected_ != nullptr) {
            selected_->Unselect();
            selected_ = nullptr;
          }
        } else if (Box(Vec2d(0, 300), Vec2d(100, 100))
                       .IsCollided(Box(cursorpos, Vec2d())) &&
                   workstatus_ != IOBLOCK) {
          workstatus_ = IOBLOCK;
          if (selected_ != nullptr) {
            selected_->Unselect();
            selected_ = nullptr;
          }
        } else if (Box(Vec2d(0, 400), Vec2d(100, 100))
                       .IsCollided(Box(cursorpos, Vec2d())) &&
                   workstatus_ != SUBBLOCK) {
          workstatus_ = SUBBLOCK;
          if (selected_ != nullptr) {
            selected_->Unselect();
            selected_ = nullptr;
          }
        } else if (Box(Vec2d(0, 500), Vec2d(100, 100))
                       .IsCollided(Box(cursorpos, Vec2d())) &&
                   workstatus_ != CONDBLOCK) {
          workstatus_ = CONDBLOCK;
          if (selected_ != nullptr) {
            selected_->Unselect();
            selected_ = nullptr;
          }
        }
      }
    } else if (workstatus_ == SELECTION) {
      if (selected_ != nullptr) {
        if (selected_->status != Component::Status::SELECTED) {
          selected_->ButtonEvent(&tree_, button, type);
          return;
        }
      }
      auto t = tree_.Retrieve(Box(cursorpos, Vec2d(0, 0)));
      bool collided = false;
      if (t.size() != 0) {
        vector<int> collided_index;
        for (int i = 0; i < t.size(); ++i) {
          Component* ti = (Component*)t[i];
          if (ti->IsCollided(Box(cursorpos, Vec2d(0, 0)))) {
            collided_index.push_back(i);
          }
        }
        UpdateDepth();
        if (!collided_index.empty()) {
          int m = -1;
          for (int i = 0; i < collided_index.size(); ++i) {
            Component* ti = (Component*)t[i];
            if (ti->depth_ > m) {
              m = i;
            }
          }
          Component* ti = (Component*)t[collided_index[m]];
          if (ti->box_.IsCollided(Box(cursorpos, Vec2d(0, 0)))) {
            ti->ButtonEvent(&tree_, button, type);
            if (ti->status == Component::Status::SELECTED) {
              int index = ti->depth_;
              auto t = components[index];
              components.erase(components.begin() + index);
              components.push_back(t);
              UpdateDepth();
              if (selected_ != nullptr && selected_ != ti) {
                selected_->Unselect();
              }
              selected_ = ti;
              collided = true;
            }
          }
        }
      }
      if (button == 0 && !collided && selected_ != nullptr) {
        selected_->Unselect();
        selected_ = nullptr;
      }
    } else if (workstatus_ == ARROW) {
      if (selected_ != nullptr) {
        selected_->ButtonEvent(&tree_, button, type);
        Arrow* arrow = (Arrow*)selected_;
        if (arrow->astatus_ == Arrow::COMPLETED) {
          selected_ = nullptr;
        } else if (arrow->astatus_ == Arrow::FAIL) {
          DelComponent(selected_);
          selected_ = nullptr;
        }
      }
    } else if (workstatus_ == PROCESSBLOCK) {
      if (button == 0 && type == 1) {
        Box box = Box(cursorpos - Vec2d(75, 50), Vec2d(150, 100));
        AddComponent(make_shared<ProcessBlock>(
            ProcessBlock(&font, hb_font, &canvas, width, height, box)));
      }
    } else if (workstatus_ == STARTBLOCK) {
      if (button == 0 && type == 1) {
        Box box = Box(cursorpos - Vec2d(75, 50), Vec2d(150, 100));
        AddComponent(make_shared<StartBlock>(
            StartBlock(&font, hb_font, &canvas, width, height, box)));
      }
    } else if (workstatus_ == IOBLOCK) {
      if (button == 0 && type == 1) {
        Box box = Box(cursorpos - Vec2d(75, 50), Vec2d(150, 100));
        AddComponent(make_shared<IOBlock>(
            IOBlock(&font, hb_font, &canvas, width, height, box)));
      }
    } else if (workstatus_ == SUBBLOCK) {
      if (button == 0 && type == 1) {
        Box box = Box(cursorpos - Vec2d(75, 50), Vec2d(150, 100));
        AddComponent(make_shared<SubBlock>(
            SubBlock(&font, hb_font, &canvas, width, height, box)));
      }
    } else if (workstatus_ == CONDBLOCK) {
      if (button == 0 && type == 1) {
        Box box = Box(cursorpos - Vec2d(75, 50), Vec2d(150, 100));
        AddComponent(make_shared<CondBlock>(
            CondBlock(&font, hb_font, &canvas, width, height, box)));
      }
    }
  }

  void OnKeyboardEvent(GLFWwindow* window, int key, int action, int modifier) {
    if (workstatus_ == SELECTION) {
      if (selected_ != nullptr) {
        int oper = selected_->OnKeyboard(window, key, action, modifier);
        if (oper == -1) {
          DelComponent(selected_);
          selected_ = nullptr;
        } else if (oper == -2) {
          return;
        }
      }
    }
    if (key == GLFW_KEY_1 && workstatus_ != SELECTION) {
      workstatus_ = SELECTION;
    } else if (key == GLFW_KEY_2 && workstatus_ != ARROW) {
      workstatus_ = ARROW;
      if (selected_ != nullptr) {
        selected_->Unselect();
        selected_ = nullptr;
      }
    } else if (key == GLFW_KEY_3 && workstatus_ != PROCESSBLOCK) {
      workstatus_ = PROCESSBLOCK;
      if (selected_ != nullptr) {
        selected_->Unselect();
        selected_ = nullptr;
      }
    } else if (key == GLFW_KEY_4 && workstatus_ != STARTBLOCK) {
      workstatus_ = STARTBLOCK;
      if (selected_ != nullptr) {
        selected_->Unselect();
        selected_ = nullptr;
      }
    } else if (key == GLFW_KEY_5 && workstatus_ != IOBLOCK) {
      workstatus_ = IOBLOCK;
      if (selected_ != nullptr) {
        selected_->Unselect();
        selected_ = nullptr;
      }
    } else if (key == GLFW_KEY_6 && workstatus_ != SUBBLOCK) {
      workstatus_ = SUBBLOCK;
      if (selected_ != nullptr) {
        selected_->Unselect();
        selected_ = nullptr;
      }
    } else if (key == GLFW_KEY_7 && workstatus_ != CONDBLOCK) {
      workstatus_ = CONDBLOCK;
      if (selected_ != nullptr) {
        selected_->Unselect();
        selected_ = nullptr;
      }
    }
  }

  void OnCharEvent(unsigned codepoint) {
    if (selected_ != nullptr) {
      if (!selected_->IsArrow()) {
        selected_->OnCharEvent(codepoint);
      }
    }
  }

  void OnWindowSizeChange(double w, double h) {
    width = w;
    height = h;
    GrGLFramebufferInfo framebufferInfo;
    framebufferInfo.fFBOID = 0;  // assume default framebuffer
    framebufferInfo.fFormat = GL_RGBA8;
    GrBackendRenderTarget backendRenderTarget(w, h,
                                              0,  // sample count
                                              0,  // stencil bits
                                              framebufferInfo);
    SkColorType colorType = kRGBA_8888_SkColorType;
    delete surface;
    surface = SkSurface::MakeFromBackendRenderTarget(
                  context, backendRenderTarget, kBottomLeft_GrSurfaceOrigin,
                  colorType, nullptr, nullptr)
                  .release();
    if (surface == nullptr) {
      abort();
    }
    canvas = surface->getCanvas();
    UpdateDepth();
  }

  void DrawSidebar(double w, double h) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeN32(100, height, kOpaque_SkAlphaType));
    bitmap.allocPixels();
    bitmap.eraseColor(SK_ColorWHITE);
    SkCanvas offscr(bitmap);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(2);
    paint.setColor(SK_ColorBLACK);

    offscr.drawLine(10, 50, 90, 50, paint);
    offscr.drawLine(50, 10, 50, 90, paint);
    if (workstatus_ == SELECTION) {
      offscr.drawRect(SkRect::MakeXYWH(5, 5, 90, 90), paint);
    }

    offscr.drawRect(SkRect::MakeXYWH(10, 110, 80, 80), paint);
    if (workstatus_ == PROCESSBLOCK) {
      offscr.drawRect(SkRect::MakeXYWH(5, 105, 90, 90), paint);
    }

    {
      Box box = Box(Vec2d(10, 210), Vec2d(80, 80));

      double r = sqrt(0.09 * box.size_.x * box.size_.x +
                      0.25 * box.size_.y * box.size_.y);
      double angle =
          atan((0.5 * box.size_.y) / (0.3 * box.size_.x)) * 180 / 3.14159;
      Vec2d o1(box.pos_.x + r, box.pos_.y + box.size_.y * 0.5);
      SkRect rect1 = SkRect::MakeXYWH(o1.x - r, o1.y - r, 2 * r, 2 * r);
      offscr.drawArc(rect1, 180.0 - angle, 2 * angle, false, paint);

      Vec2d o2(box.pos_.x + box.size_.x - r, box.pos_.y + box.size_.y * 0.5);
      SkRect rect2 = SkRect::MakeXYWH(o2.x - r, o2.y - r, 2 * r, 2 * r);
      offscr.drawArc(rect2, 360 - angle, 2 * angle, false, paint);

      offscr.drawLine(box.pos_.x - box.size_.x * 0.3 + r, box.pos_.y,
                      box.pos_.x + box.size_.x - r + box.size_.x * 0.3,
                      box.pos_.y, paint);
      offscr.drawLine(box.pos_.x - box.size_.x * 0.3 + r,
                      box.pos_.y + box.size_.y,
                      box.pos_.x + box.size_.x - r + box.size_.x * 0.3,
                      box.pos_.y + box.size_.y, paint);
      if (workstatus_ == STARTBLOCK) {
        offscr.drawRect(SkRect::MakeXYWH(5, 205, 90, 90), paint);
      }
    }

    {
      Box box_ = Box(Vec2d(10, 310), Vec2d(80, 80));

      offscr.drawLine(box_.pos_.x + box_.size_.x * 0.2, box_.pos_.y,
                      box_.pos_.x + box_.size_.x, box_.pos_.y, paint);

      offscr.drawLine(box_.pos_.x, box_.pos_.y + box_.size_.y,
                      box_.pos_.x + box_.size_.x * 0.8,
                      box_.pos_.y + box_.size_.y, paint);

      offscr.drawLine(box_.pos_.x + box_.size_.x * 0.2, box_.pos_.y,
                      box_.pos_.x, box_.pos_.y + box_.size_.y, paint);

      offscr.drawLine(box_.pos_.x + box_.size_.x, box_.pos_.y,
                      box_.pos_.x + box_.size_.x * 0.8,
                      box_.pos_.y + box_.size_.y, paint);

      if (workstatus_ == IOBLOCK) {
        offscr.drawRect(SkRect::MakeXYWH(5, 305, 90, 90), paint);
      }
    }

    {
      Box box_ = Box(Vec2d(10, 410), Vec2d(80, 80));
      offscr.drawRect(box_.GetEdge(), paint);
      offscr.drawLine(box_.pos_.x + 15, box_.pos_.y, box_.pos_.x + 15,
                      box_.pos_.y + box_.size_.y, paint);
      offscr.drawLine(box_.pos_.x + box_.size_.x - 15, box_.pos_.y,
                      box_.pos_.x + box_.size_.x - 15,
                      box_.pos_.y + box_.size_.y, paint);
      if (workstatus_ == SUBBLOCK) {
        offscr.drawRect(SkRect::MakeXYWH(5, 405, 90, 90), paint);
      }
    }

    {
      Box box = Box(Vec2d(10, 510), Vec2d(80, 80));

      Vec2d mid = box.Mid();

      offscr.drawLine(mid.x, box.pos_.y, box.pos_.x, mid.y, paint);
      offscr.drawLine(box.pos_.x, mid.y, mid.x, box.pos_.y + box.size_.y,
                      paint);
      offscr.drawLine(mid.x, box.pos_.y + box.size_.y, box.pos_.x + box.size_.x,
                      mid.y, paint);
      offscr.drawLine(box.pos_.x + box.size_.x, mid.y, mid.x, box.pos_.y,
                      paint);

      if (workstatus_ == CONDBLOCK) {
        offscr.drawRect(SkRect::MakeXYWH(5, 505, 90, 90), paint);
      }
    }

    {
      offscr.drawLine(10, 690, 90, 610, paint);
      if (workstatus_ == ARROW) {
        offscr.drawRect(SkRect::MakeXYWH(5, 605, 90, 90), paint);
      }
    }

    canvas->writePixels(bitmap, 0, 0);
  }
};

}  // namespace mocoder
