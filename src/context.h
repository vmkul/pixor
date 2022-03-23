#pragma once
#include <memory>
#include <vector>
#include <cstring>
#include "pixor.h"
#include "debug.h"
#include "pattern.h"
#include "matrix.h"

namespace Pixor {

class Pattern;

class Context {
  std::shared_ptr<byte[]> bitmap;
  std::shared_ptr<Pattern> source_pattern;
  RGBA *pixel_data;
  int width;
  int height;
  RGBA source_color = 0;

  point clamp_coord(point coord) const;

public:
  Context(std::shared_ptr<byte[]> bitmap, int width, int height) :
    bitmap(bitmap),
    width(width),
    height(height)
  {
    pixel_data = (RGBA *) bitmap.get();
  }

  Context(int width, int height) :
    bitmap(new byte[width * height * 4]),
    width(width),
    height(height)
  {
    pixel_data = (RGBA *) bitmap.get();

    for (int i = 0; i < width * height * 4; i++) {
      bitmap[i] = 0;
    }
  }

  Context(const Context &context) :
    width(context.get_width()),
    height(context.get_height())
  {
    int bitmap_size = width * height * 4;
    byte *bitmap_data = new byte[bitmap_size];

    memcpy(bitmap_data, context.get_target_bitmap().get(), bitmap_size);

    pixel_data = (RGBA *) bitmap_data;
    bitmap.reset(bitmap_data);
  }

  int get_width() const {return width;}
  int get_height() const {return height;}
  bool coord_in_bounds(point p);
  void set_source_pattern(std::shared_ptr<Pattern> pattern);
  void set_source_rgba(RGBA color);
  void set_pixel(point coord, RGBA value);
  void set_pixel_safe(point coord, RGBA value);
  void set_pixel_clamped(point coord, RGBA value);
  RGBA get_pixel(point coord) const;
  RGBA *get_pixel_ptr(point coord) const;
  RGBA *get_pixel_ptr_clamped(point coord) const;
  RGBA get_pixel_safe(point coord, RGBA default_value = 0);
  void draw_pattern(std::vector<point> &points, Pattern &p);
  void draw_line(point p1, point p2, int line_width);
  void draw_line_with_pattern(point p1, point p2);
  const std::shared_ptr<byte[]> get_target_bitmap() const {return bitmap;}
  std::shared_ptr<Pattern> scale(int new_width, int new_height) const;
  std::shared_ptr<Context> convolve(Matrix<float> m);
  std::shared_ptr<Matrix<double>> get_matrix() const;
  void set_matrix(Matrix<double> &m);
};

}
