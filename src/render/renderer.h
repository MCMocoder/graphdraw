/**
 * @file renderer.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-17
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#pragma once

#include <freetype/freetype.h>
#include <glad/glad.h>

#include <algorithm>
#include <cstdio>
#include <vector>

#include "freetype2/ft2build.h"
#include "utils/vec2d.h"
#include "utils/vec3d.h"

namespace mocoder {

using namespace std;

class GeometryShader {
 public:
  const char* vert =
      "#version 420 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}\0";

  const char* frag =
      "#version 420 core\n"
      "out vec4 FragColor;\n"
      "uniform vec4 color;\n"
      "void main()\n"
      "{\n"
      "   FragColor = color;\n"
      "}\n\0";

  unsigned int shaderprog_;

  void LoadShader() {
    unsigned int vertshader;
    vertshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertshader, 1, &vert, NULL);
    glCompileShader(vertshader);
    unsigned int fragshader;
    fragshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragshader, 1, &frag, NULL);
    glCompileShader(fragshader);
    shaderprog_ = glCreateProgram();
    glAttachShader(shaderprog_, vertshader);
    glAttachShader(shaderprog_, fragshader);
    glLinkProgram(shaderprog_);
    glDeleteShader(vertshader);
    glDeleteShader(fragshader);
  }

  GeometryShader() { LoadShader(); }

  static int Get() {
    static GeometryShader shader;
    return shader.shaderprog_;
  }
};

class GeometryRenderer {
 public:
  GeometryRenderer(double _w, double _h) : w(_w), h(_h) {}
  double w, h;
  float* vertarr_ = nullptr;
  size_t vertnum_;

  ~GeometryRenderer() {
    if (vertarr_ != nullptr) {
      delete[] vertarr_;
      vertarr_ = nullptr;
    }
  }

  void SetVertnum(size_t num) {
    if (vertarr_ != nullptr) {
      delete[] vertarr_;
    }
    vertnum_ = num;
    vertarr_ = new float[num * 3];
    for (int i = 0; i < num * 3; ++i) {
      vertarr_[i] = 0.0f;
    }
  }

  void TransformCoord() {
    for (int i = 0; i < vertnum_ * 3; i += 3) {
      vertarr_[i] = vertarr_[i] / w * 2 - 1;
      vertarr_[i + 1] = 1 - vertarr_[i + 1] / h * 2;
      vertarr_[i + 2] = 0.0f;
    }
  }

  void AddVertex(const vector<Vec2d>& vertex) {
    SetVertnum(vertex.size());
    for (int i = 0; i < vertnum_; ++i) {
      vertarr_[i * 3] = vertex[i].x;
      vertarr_[i * 3 + 1] = vertex[i].y;
      vertarr_[i * 3 + 2] = 0.0f;
    }
    TransformCoord();
  }

  void Render(double r,double g,double b) {
    glLineWidth(2.0f);

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertnum_ * 3 * sizeof(float), vertarr_,
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(GeometryShader::Get());

    glUniform4f(glGetUniformLocation(GeometryShader::Get(), "color"),r,g,b,1.0f);
    glBindVertexArray(vao);

    glDrawArrays(GL_LINE_LOOP, 0, vertnum_);
    //glDrawArrays(GL_TRIANGLE_STRIP,0,vertnum_);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
  }
};

class TextShader {};

class TextRenderer {
 public:
  struct Charcter {
    unsigned int id;
    Vec2d size;
    Vec2d bearing;
    unsigned int advance;
  };
};

}  // namespace mocoder
