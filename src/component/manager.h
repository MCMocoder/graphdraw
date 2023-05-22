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

#include <memory>

#include "component/component.h"
#include "utils/quadtree.h"

namespace mocoder {

class UIManager {
 public:
  enum WorkStatus { SELECTION, PROCESSBLOCK, STARTBLOCK, ARROW };
  WorkStatus workstatus_ = SELECTION;
  vector<shared_ptr<Component>> components;

  Component* selected_ = nullptr;
  QuadTreeNode tree_;

  bool leftdown;
  Vec2d cursorpos;

  void AddComponent(shared_ptr<Component> component) {
    components.push_back(component);
    component->depth_ = components.size();
  }

  void UpdateDepth() {
    for (int i = 0; i < components.size(); ++i) {
      components[i]->depth_ = i;
    }
  }

  UIManager(double w, double h) : tree_(Box(Vec2d(0, 0), Vec2d(w, h))) {}

  void ProcessFrame(double w, double h) {
    tree_.Clear();
    tree_.bound_ = Box(Vec2d(0, 0), Vec2d(w, h));
    for (auto& i : components) {
      tree_.Insert(i.get());
    }

    for (auto& i : components) {
      i->Render(w, h);
    }
  }

  void OnCursorEvent(double xpos, double ypos) {
    Vec2d velocity = (Vec2d(xpos, ypos) - cursorpos).Abs();
    cursorpos = Vec2d(xpos, ypos);
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
    }
  }

  void OnButtonEvent(int button, int type) {
    if (button == 0) {
      if (type == 1) {
        leftdown = true;
      } else {
        leftdown = false;
      }
    }
    if (selected_ != nullptr) {
      if (selected_->status != Component::Status::SELECTED) {
        selected_->ButtonEvent(button, type);
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
          ti->ButtonEvent(button, type);
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
  }
};

}  // namespace mocoder
