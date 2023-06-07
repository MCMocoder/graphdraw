# GraphDraw:一个实验性的流程图绘制工具

## 1 简述
这是一个实验性的流程图绘制工具。在这个里面实现了MVC模型。

## 2 构建指南
在Windows下构建时，该项目依赖以下外部库:

- Skia
- Harfbuzz
- GLFW
- ICU

并使用原CMakeLists.txt

同时，在构建时还额外依赖以下软件包:

- clang(>=14)
- cmake(>=3)
- libwebp
- libpng
- libwebpmux
- libwebpdemux

请按照以下步骤构建：

1. 安装MSYS2环境，并安装以下软件包: mingw-w64-clang-x86_64-clang-toolchain,mingw-w64-clang-x86_64-ninja,mingw-w64-clang-x86-64-cmake
2. 安装Skia:安装软件包mingw-w64-clang-x86_64-skia
3. 安装icu:安装软件包mingw-w64-clang-x86_64-icu
4. 安装Harfbuzz:安装软件包mingw-w64-clang-x86_64-harfbuzz
5. 将```include_directories(C:/msys64/clang64/include/skia)```一行改为实际的skia包含目录
6. 执行cmake构建
