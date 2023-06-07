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
#include "component/arrow.h"
#include "component/manager.h"
#include "component/process.h"
// #include "component/textinput.h"
#include "utils/quadtree.h"
#include "utils/vec2d.h"

#define SK_GANESH
#define SK_GL
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"

using namespace mocoder;

UIManager mng(1200, 800);

int width, height;

void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
  // glViewport(0, 0, w, h);
  width = w;
  height = h;
  mng.ProcessFrame(w, h);
  glfwSwapBuffers(window);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  mng.OnCursorEvent(xpos, ypos);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mode) {
  mng.OnKeyboardEvent(window, key, action, mode);
}

void mousebutton_callback(GLFWwindow* window, int button, int action,
                          int other) {
  mng.OnButtonEvent(button, action);
}

void char_callback(GLFWwindow* window, unsigned ch) { mng.OnCharEvent(ch); }

int main(int argc, char** argv) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  GLFWwindow* window = glfwCreateWindow(1200, 800, "GraphDraw", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwGetFramebufferSize(window, &width, &height);

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetMouseButtonCallback(window, mousebutton_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCharCallback(window, char_callback);

  mng.InitSkia(1200, 800);

  while (!glfwWindowShouldClose(window)) {
    _sleep(10);

    glfwPollEvents();

    mng.ProcessFrame(width, height);

    glfwSwapBuffers(window);
  }
  mng.Close();
  glfwTerminate();
  return 0;
}
