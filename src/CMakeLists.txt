cmake_minimum_required(VERSION 3.10)

project(PIXOR)
find_package(PkgConfig REQUIRED)
pkg_check_modules(gtkmm-3.0 REQUIRED gtkmm-3.0)
pkg_check_modules(zlib REQUIRED zlib)

add_executable(PIXOR
  main.cpp 
  main_window.cpp
  image_area.cpp
  png_chunk.cpp
  png.cpp
  crc.cpp
  application.cpp
  pixor.cpp
  pattern.cpp
  context.cpp
  canny.cpp)

target_include_directories(PIXOR PUBLIC
  ${gtkmm-3.0_INCLUDE_DIRS}
  ${zlib_INCLUDE_DIRS})
target_compile_options(PIXOR PUBLIC
  ${gtkmm-3.0_CFLAGS_OTHER}
  ${zlib_CFLAGS_OTHER})
target_link_libraries(PIXOR
  ${gtkmm-3.0_LIBRARIES}
  ${zlib_LIBRARIES})
