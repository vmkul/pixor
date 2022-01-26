#pragma once
#include <memory>
#include <vector>
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


class Pattern {
  std::vector<std::vector<RGBA *>> bit_pattern;
  int width;
  int height;

public:
  Pattern(std::vector<std::vector<RGBA *>> &bit_pattern) :
    bit_pattern(bit_pattern)
  {
    if (bit_pattern.size() == 0) return;
    width = bit_pattern[0].size();
    height = bit_pattern.size();
  }

  void draw_onto(Context &context, point center);

  static Pattern make_square(int side, RGBA *color)
  {
    std::vector<std::vector<RGBA *>> pattern;

    for (int i = 0; i < side; i++) {
      std::vector<RGBA *> row;

      for (int j = 0; j < side; j++) {
	row.push_back(color);
      }

      pattern.push_back(row);
    }

    return Pattern(pattern);
  }
};

}
