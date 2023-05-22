# GraphDraw构建指南

## 1.依赖库

GraphDraw依赖以下库:

- Libicu
- Freetype2
- Gtk4

在构建与执行前，需要将显卡驱动更新至最新，并确保显卡驱动支持的OpenGL版本不低于4.2

构建额外依赖以下软件包:

- clang(>14)
- cmake
- ninja

## 2.依赖库的安装
GraphDraw使用MSYS2环境进行构建，在构建之前需要确保计算机内有MSYS2环境，并装有以下软件包：