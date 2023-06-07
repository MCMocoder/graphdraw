/**
 * @file textinput.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-06-01
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include <unicode/unistr.h>

#include <queue>
#include <vector>

#include "component/textinput.h"
#include "ft2build.h"
#include "harfbuzz/hb-ft.h"
#include "harfbuzz/hb.h"
#include "unicode/brkiter.h"
#include "unicode/ubrk.h"
#include "unicode/utext.h"
#include "unicode/utf8.h"
#include "unicode/utypes.h"
#include "utils/frame.h"
#include "utils/vec2d.h"

// GLFW
#include "GLFW/glfw3.h"

#define SK_GANESH
#define SK_GL
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkDocument.h"
#include "include/core/SkFont.h"
#include "include/core/SkRasterHandleAllocator.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"

#define FONT_SIZE 24
#define SPACING_RATIO 1.5

namespace mocoder {

using namespace icu_72;

using namespace std;

class TextInput {
 public:
  /*UnicodeString ustr_ = UnicodeString(
      "我在法国Auvergne-Rhône-Alpes大区的Lyon市cos了一次纳西妲,"
      "同学则cos了提纳里",
      -1, US_INV);*/

  UnicodeString ustr_ = UnicodeString::fromUTF8("");

  Vec2d pos_;
  hb_font_t* hb_font = nullptr;
  hb_font_extents_t extents;
  SkFont* font;
  SkBitmap bitmap;
  FrameCounter fc;
  Vec2d cursor1, cursor2;
  bool allow_focus = true;
  double textw = 0, texth = 0;

  int w = 0, h = 0;
  int focuspoint = 0;

  bool should_rerender = true;

  TextInput(SkFont* _font, hb_font_t* _hb_font) : font(_font) {
    hb_font = _hb_font;
    hb_font_get_h_extents(hb_font, &extents);
  }

  // 此函数的返回包含0与ustr_.size()
  std::vector<int> GetPossibWrap() {
    UErrorCode status = U_ZERO_ERROR;
    BreakIterator* bi =
        BreakIterator::createLineInstance(Locale::getChina(), status);
    bi->setText(ustr_);

    std::vector<int> boundaries;

    int32_t p = bi->first();
    while (p != BreakIterator::DONE) {
      boundaries.push_back(p);
      p = bi->next();
    }
    delete bi;

    return boundaries;
  }

  void RerenderText(SkCanvas** canvas, Vec2d pos, Vec2d center, int width,
                    int height) noexcept {
    if (w != width || h != height) {
      should_rerender = true;
    }
    if (should_rerender) {
      w = width;
      h = height;
      auto possiblewrap = GetPossibWrap();

      int cursorpoint = 0;

      double midx = width / 2.0;

      struct UnitData {
        hb_buffer_t* buf = nullptr;
        int line = 0;
        double wcnt = 0.0, width = 0.0;
      };

      queue<UnitData> buffers;
      vector<double> linewidth;
      double width_cnt = 0.0;
      int line_cnt = 0;
      int glyph_cnt = 0;

      for (int i = 0; i < possiblewrap.size() - 1; ++i) {
        hb_buffer_t* buf = hb_buffer_create();
        std::string utf8;
        UnicodeString t(ustr_, possiblewrap[i],
                        possiblewrap[i + 1] - possiblewrap[i]);
        t.toUTF8String(utf8);

        hb_buffer_set_content_type(buf, HB_BUFFER_CONTENT_TYPE_UNICODE);

        // hb_buffer_add_utf8(buf, utf8.data(), -1, 0, -1);
        for (int j = possiblewrap[i]; j < possiblewrap[i + 1]; ++j) {
          hb_buffer_add(buf, ustr_.char32At(j), j);
        }
        // hb_buffer_guess_segment_properties(buf);
        hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
        hb_buffer_set_script(buf, HB_SCRIPT_HAN);
        hb_buffer_set_language(buf, hb_language_from_string("zh-cn", -1));
        hb_shape(hb_font, buf, NULL, 0);

        unsigned int glyph_count = hb_buffer_get_length(buf);
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buf, NULL);
        hb_glyph_position_t* glyph_pos =
            hb_buffer_get_glyph_positions(buf, NULL);

        double str_width = 0.0;

        for (int j = 0; j < glyph_count; ++j) {
          str_width += glyph_pos[j].x_advance / 64.0;
          if (focuspoint >= glyph_info[j].cluster) {
            cursorpoint = glyph_cnt;
          }
          ++glyph_cnt;
        }
        width_cnt += str_width;
        if (width_cnt > width) {
          if (str_width <= width) {
            ++line_cnt;
            linewidth.push_back(width_cnt - str_width);
            width_cnt = str_width;
          } else {
            UnitData unitdata = {.buf = buf,
                                 .line = line_cnt,
                                 .wcnt = width_cnt,
                                 .width = (double)str_width};
            width_cnt = 0;
            buffers.push(unitdata);
            linewidth.push_back(str_width);
            ++line_cnt;
            continue;
          }
        }
        if (i == possiblewrap.size() - 2) {
          linewidth.push_back(width_cnt);
        }
        UnitData unitdata = {.buf = buf,
                             .line = line_cnt,
                             .wcnt = width_cnt,
                             .width = (double)str_width};
        buffers.push(unitdata);
      }
      if (focuspoint == ustr_.length()) {
        cursorpoint = glyph_cnt + 1;
      }

      int total_glyph = glyph_cnt;
      if (buffers.empty()) {
        cursor1 = Vec2d(0, 0);
        cursor2 = Vec2d(0, FONT_SIZE);
      }
      double total_height = (line_cnt + 1) * FONT_SIZE * SPACING_RATIO;

      double total_width = 0;
      if (!linewidth.empty()) {
        total_width =
            max(0.0, *max_element(linewidth.begin(), linewidth.end()));
      }
      bitmap.setInfo(
          SkImageInfo::MakeN32(total_width, total_height, kOpaque_SkAlphaType));
      textw = total_width;
      texth = total_height;
      bitmap.allocPixels();
      bitmap.eraseColor(SK_ColorWHITE);
      SkCanvas offscr(bitmap);
      glyph_cnt = 0;
      while (!buffers.empty()) {
        hb_buffer_t* buf = buffers.front().buf;
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buf, NULL);
        hb_glyph_position_t* glyph_pos =
            hb_buffer_get_glyph_positions(buf, NULL);

        double current_y = SPACING_RATIO * FONT_SIZE * buffers.front().line;
        double str_width = buffers.front().width;
        double wcnt = buffers.front().wcnt;
        unsigned len = hb_buffer_get_length(buf);
        double x = 0.0, y = extents.ascender / 64.0;
        SkTextBlobBuilder builder;
        auto runBuffer = builder.allocRunPos(*font, len);
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        for (int i = 0; i < len; ++i) {
          runBuffer.glyphs[i] = glyph_info[i].codepoint;
          reinterpret_cast<SkPoint*>(runBuffer.pos)[i] =
              SkPoint::Make(x + glyph_pos[i].x_offset / 64.0,
                            y - glyph_pos[i].y_offset / 64.0);
          if (cursorpoint == glyph_cnt) {
            cursor1 = Vec2d(x + wcnt - str_width, current_y);
            cursor2 = Vec2d(x + wcnt - str_width, current_y + FONT_SIZE);
          }
          ++glyph_cnt;
          x += glyph_pos[i].x_advance / 64.0;
          y += glyph_pos[i].y_advance / 64.0;
        }

        if (buffers.size() == 1 && cursorpoint == total_glyph + 1) {
          cursor1 = Vec2d(x + wcnt - str_width, current_y);
          cursor2 = Vec2d(x + wcnt - str_width, current_y + FONT_SIZE);
        }

        offscr.drawTextBlob(builder.make(), wcnt - str_width, current_y, paint);
        hb_buffer_destroy(buf);
        buffers.pop();
      }
      should_rerender = false;
    }
    (*canvas)->writePixels(bitmap, center.x - textw / 2.0,
                           center.y - texth / 2.0);
    if (fc.frame % 60 < 30 && status_ == EDIT && cursor1.x >= 0 &&
        cursor2.x <= width && cursor1.y >= 0 && cursor2.y <= height) {
      SkPaint paint;
      paint.setAntiAlias(true);
      paint.setStrokeWidth(1.6);
      paint.setColor(SK_ColorBLACK);
      (*canvas)->drawLine(cursor1.x + center.x - textw / 2.0,
                          cursor1.y + center.y - texth / 2.0,
                          cursor2.x + center.x - textw / 2.0,
                          cursor2.y + center.y - texth / 2.0, paint);
    }
    fc.RenderFrame();
  }

  enum TextStatus { SHOW, EDIT };
  TextStatus status_ = SHOW;

  void OnDoubleClick() { status_ = EDIT; }

  void UnSelect() { status_ = SHOW; }

  void OnChar(unsigned codepoint) {
    if (!allow_focus) {
      return;
    }
    if (status_ == EDIT) {
      ustr_.insert(focuspoint, (UChar32)codepoint);
      ++focuspoint;
      should_rerender = true;
    }
  }

  void OnKeyboard(GLFWwindow* window, int key, int action, int modifier) {
    if (!allow_focus) {
      return;
    }
    if (key == GLFW_KEY_BACKSPACE &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
      if (focuspoint >= 1) {
        ustr_.remove(focuspoint - 1, 1);
        --focuspoint;
        should_rerender = true;
      }
    }
    if (key == GLFW_KEY_V && modifier == GLFW_MOD_CONTROL &&
        action == GLFW_PRESS) {
      const char* clip = glfwGetClipboardString(window);
      UnicodeString clip_u = UnicodeString::fromUTF8(clip);
      ustr_.insert(focuspoint, clip_u);
      focuspoint += clip_u.length();
      should_rerender = true;
    }
    if (key == GLFW_KEY_LEFT &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
      --focuspoint;
      if (focuspoint < 0) {
        focuspoint = 0;
      }
      fc.frame = 0;
      should_rerender = true;
    }
    if (key == GLFW_KEY_RIGHT &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
      ++focuspoint;
      if (focuspoint > ustr_.length()) {
        focuspoint = ustr_.length();
      }
      fc.frame = 0;
      should_rerender = true;
    }
  }
  // TODO: 鼠标拖动选择，鼠标双击选择，鼠标双击分词
};

}  // namespace mocoder
