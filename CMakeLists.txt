cmake_minimum_required(VERSION 3.0.0)
project(graphdraw VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

find_package(ICU COMPONENTS data uc i18n io REQUIRED)

find_package(Freetype REQUIRED)

include_directories(${ICU_INCLUDE_DIRS})
link_directories(${ICU_LIBRARY_DIRS})
include_directories(${FREETYPE_INCLUDE_DIRS})
link_directories(${FREETYPE_LIBRARY_DIRS})

include_directories(src)
include_directories(third_party/glfw/include)
include_directories(third_party/glad/include)
link_directories(third_party/glfw/lib-mingw-w64)

#find_package(PkgConfig REQUIRED)
#pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
#include_directories(${GTKMM_INCLUDE_DIRS}) 
#link_directories(${GTKMM_LIBRARY_DIRS})  

file(GLOB_RECURSE srcs src/*.cc)

add_executable(graphdraw ${srcs} third_party/glad/glad.cc)

target_link_libraries(graphdraw ${ICU_LIBRARIES} glfw3 ${FREETYPE_LIBRARIES})
