/**
 * @file quadtree.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-17
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include <memory>
#include <vector>

#include "utils/boxedobj.h"

namespace mocoder {

using namespace std;

class QuadTreeNode {
 public:
  Box bound_;
  shared_ptr<QuadTreeNode> nodes_[4];
  vector<BoxedObj*> obj_;

  QuadTreeNode(Box bound) : bound_(bound) {
    for (int i = 0; i < 4; ++i) {
      nodes_[i] = shared_ptr<QuadTreeNode>(nullptr);
    }
  }

  int GetBoxPos(BoxedObj* obj) {
    double xmid = bound_.pos_.x + bound_.size_.x / 2;
    double ymid = bound_.pos_.y + bound_.size_.y / 2;

    bool is_left = (obj->box_.pos_.x + obj->box_.size_.x < xmid);
    bool is_right = (obj->box_.pos_.x > xmid);
    bool is_down = (obj->box_.pos_.y + obj->box_.size_.y < ymid);
    bool is_up = (obj->box_.pos_.y > ymid);

    if (is_left) {
      if (is_up) {
        return 0;
      } else if (is_down) {
        return 2;
      }
    }

    if (is_right) {
      if (is_up) {
        return 1;
      } else if (is_down) {
        return 3;
      }
    }

    return -1;
  }

  int GetBoxPos(Box box) {
    double xmid = bound_.pos_.x + bound_.size_.x / 2;
    double ymid = bound_.pos_.y + bound_.size_.y / 2;

    bool is_left = (box.pos_.x + box.size_.x < xmid);
    bool is_right = (box.pos_.x > xmid);
    bool is_down = (box.pos_.y + box.size_.y < ymid);
    bool is_up = (box.pos_.y > ymid);

    if (is_left) {
      if (is_up) {
        return 0;
      } else if (is_down) {
        return 2;
      }
    }

    if (is_right) {
      if (is_up) {
        return 1;
      } else if (is_down) {
        return 3;
      }
    }

    return -1;
  }

  void Split() {
    nodes_[0] = make_shared<QuadTreeNode>(
        QuadTreeNode(Box(bound_.pos_, bound_.size_ / 2)));
    nodes_[1] = make_shared<QuadTreeNode>(QuadTreeNode(
        Box(bound_.pos_ + Vec2d(bound_.size_.x / 2, 0), bound_.size_ / 2)));
    nodes_[2] = make_shared<QuadTreeNode>(QuadTreeNode(
        Box(bound_.pos_ + Vec2d(0, bound_.size_.y / 2), bound_.size_ / 2)));
    nodes_[3] = make_shared<QuadTreeNode>(
        QuadTreeNode(Box(bound_.pos_ + bound_.size_ / 2, bound_.size_ / 2)));

    for (auto i = obj_.begin(); i != obj_.end();) {
      int pos = GetBoxPos(*i);
      if (pos != -1) {
        nodes_[pos]->Insert(*i);
        i = obj_.erase(i);
      } else {
        ++i;
      }
    }
  }

  void Clear() {
    obj_.clear();
    for (int i = 0; i < 4; ++i) {
      if (nodes_[i] != nullptr) {
        nodes_[i]->Clear();
        nodes_[i].reset();
      }
    }
  }

  void Insert(BoxedObj* obj) {
    if (nodes_[0] == nullptr) {
      obj_.push_back(obj);
      if (obj_.size() > 6) {
        Split();
      }
    } else {
      int pos = GetBoxPos(obj);
      if (pos != -1) {
        nodes_[pos]->Insert(obj);
      } else {
        obj_.push_back(obj);
      }
    }
  }

  vector<BoxedObj*> Retrieve(Box box) {
    vector<BoxedObj*> res = obj_;
    int pos = GetBoxPos(box);
    if (pos != -1 && nodes_[0] != nullptr) {
      auto t = nodes_[pos]->Retrieve(box);
      res.insert(res.end(), t.begin(), t.end());
    } else if (pos == -1 && nodes_[0] != nullptr) {
      double xmid = bound_.pos_.x + bound_.size_.x / 2;
      double ymid = bound_.pos_.y + bound_.size_.y / 2;

      bool is_left = (box.pos_.x + box.size_.x < xmid);
      bool is_right = (box.pos_.x > xmid);
      bool is_down = (box.pos_.y + box.size_.y < ymid);
      bool is_up = (box.pos_.y > ymid);

      if (is_up) {
        auto t1 = nodes_[0]->Retrieve(box);
        res.insert(res.end(), t1.begin(), t1.end());
        auto t2 = nodes_[1]->Retrieve(box);
        res.insert(res.end(), t2.begin(), t2.end());
      } else if (is_down) {
        auto t1 = nodes_[2]->Retrieve(box);
        res.insert(res.end(), t1.begin(), t1.end());
        auto t2 = nodes_[3]->Retrieve(box);
        res.insert(res.end(), t2.begin(), t2.end());
      } else if (is_left) {
        auto t1 = nodes_[0]->Retrieve(box);
        res.insert(res.end(), t1.begin(), t1.end());
        auto t2 = nodes_[2]->Retrieve(box);
        res.insert(res.end(), t2.begin(), t2.end());
      } else if (is_right) {
        auto t1 = nodes_[1]->Retrieve(box);
        res.insert(res.end(), t1.begin(), t1.end());
        auto t2 = nodes_[3]->Retrieve(box);
        res.insert(res.end(), t2.begin(), t2.end());
      } else {
        for (int i = 0; i < 4; ++i) {
          auto t = nodes_[i]->Retrieve(box);
          res.insert(res.end(), t.begin(), t.end());
        }
      }
    }
    return res;
  }

};

}  // namespace mocoder
