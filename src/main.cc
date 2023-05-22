/**
 * @file main.cpp
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-05-17
 *
 * @copyright Copyright (c) 2023 Mocoder Studio
 *
 */

#include <glad/glad.h>

#include <iostream>

#include "GLFW/glfw3.h"
#include "component/manager.h"
#include "component/process.h"
#include "render/renderer.h"
#include "utils/quadtree.h"
#include "utils/vec2d.h"

using namespace mocoder;

// ProcessBlock block;

UIManager mng(800, 600);

QuadTreeNode root(Box(Vec2d(-1, -1), Vec2d(2, 2)));

bool leftdown = false;

int width,height;

void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
  glViewport(0, 0, w, h);
  width = w;
  height = h;
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  mng.ProcessFrame(w,h);
  glfwSwapBuffers(window);
}

Vec2d cursorpos;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  /*xpos = (xpos) / 800 * 2 - 1;
  ypos = -(ypos) / 600 * 2 + 1;*/
  // Vec2d(0.005, 0.005) +
  mng.OnCursorEvent(xpos, ypos);
  /*Vec2d velocity = (Vec2d(xpos, ypos) - cursorpos).Abs();
  cursorpos = Vec2d(xpos, ypos);
  // cout << xpos << " " << ypos << endl;
  auto t = mng.tree_.Retrieve(Box(cursorpos - velocity, velocity * 2));
  if (t.size() != 0) {
    for (int i = 0; i < t.size(); ++i) {
      Component* ti = (Component*)t[i];
      if (ti->box_.IsCollided(Box(cursorpos - velocity, velocity * 2))) {
        ti->CursorEvent(&mng.tree_, mng.leftdown, xpos, ypos, velocity * 2);
      }
    }
  }*/
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  mng.ProcessFrame(width, height);
  glfwSwapBuffers(window);
}

void mousebutton_callback(GLFWwindow* window, int button, int action,
                          int other) {
  mng.OnButtonEvent(button, action);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  mng.ProcessFrame(width, height);
  glfwSwapBuffers(window);
}

int main(int argc, char** argv) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  GLFWwindow* window = glfwCreateWindow(800, 600, "GraphDraw", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  glEnable(GL_MULTISAMPLE);
  glfwGetFramebufferSize(window, &width, &height);

  glViewport(0, 0, width, height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetMouseButtonCallback(window, mousebutton_callback);

  mng.components.push_back(make_shared<ProcessBlock>(
      ProcessBlock(800, 600, Box(Vec2d(400, 400), Vec2d(100, 150)))));
  mng.components.push_back(make_shared<ProcessBlock>(
      ProcessBlock(800, 600, Box(Vec2d(500, 20), Vec2d(100, 100)))));

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  mng.ProcessFrame(width, height);

  
  glfwSwapBuffers(window);

  while (!glfwWindowShouldClose(window)) {
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);

    //mng.ProcessFrame(width,height);

    //glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}