#pragma once
#include "context.h"
#include <memory>

namespace Pixor {

class Context;

class Pattern {
  std::shared_ptr<Context> source_context;
  std::shared_ptr<RGBA *[]> bitmap;
  int width;
  int height;

public:
  Pattern(std::shared_ptr<RGBA *[]> bit_pattern, int width, int height);
  Pattern(std::shared_ptr<Context> source_context, RGBA mask_color);
  Pattern(std::shared_ptr<Context> source_context);
  int get_width() const {return width;}
  int get_height() const {return height;}
  static std::shared_ptr<Pattern> make_square(int side, RGBA *color);
  static std::shared_ptr<Pattern> make_circle(int radius, RGBA *color);
  void draw_onto(Context &context, point center);
  std::shared_ptr<byte[]> hydrate() const;
  RGBA *get_pixel(point coord) const;
  void set_pixel(point coord, RGBA *value);
};

} // namespace Pixor
