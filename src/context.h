#pragma once
#include <memory>
#include "pixor.h"

namespace Pixor {

struct point {
  int x;
  int y;
};

class Context {
  std::shared_ptr<byte[]> bitmap;
  RGBA *pixel_data;
  int width;
  int height;
  RGBA source_color = 0;

public:
  Context(std::shared_ptr<byte[]> bitmap, int width, int height) :
    bitmap(bitmap),
    width(width),
    height(height)
  {
    pixel_data = (RGBA *) bitmap.get();
  }
  bool coord_in_bounds(point p);
  void set_source_rgba(RGBA color);
  void set_pixel(point coord, RGBA value);
  void set_pixel_safe(point coord, RGBA value);
  RGBA get_pixel(point coord);
  RGBA get_pixel_safe(point coord, RGBA default_value = 0);
  void draw_line(point p1, point p2, int line_width);
  std::shared_ptr<byte[]> get_target_bitmap() {return bitmap;}
};

}
