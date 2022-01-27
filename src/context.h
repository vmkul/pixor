#pragma once
#include <memory>
#include <vector>
#include <cmath>
#include <cstring>
#include "pixor.h"
#include "debug.h"

namespace Pixor {

struct point {
  int x;
  int y;
};

std::vector<point> approx_circle(int radius);
std::vector<point> approx_line(point p1, point p2);

class Pattern;

class Context {
  std::shared_ptr<byte[]> bitmap;
  std::shared_ptr<Pattern> source_pattern;
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
  RGBA get_pixel(point coord);
  RGBA *get_pixel_ptr(point coord);
  RGBA get_pixel_safe(point coord, RGBA default_value = 0);
  void draw_pattern(std::vector<point> &points, Pattern &p);
  void draw_line(point p1, point p2, int line_width);
  void draw_line_with_pattern(point p1, point p2);
  const std::shared_ptr<byte[]> get_target_bitmap() const {return bitmap;}
};


class Pattern {
  Context source_context;
  std::vector<std::vector<RGBA *>> bit_pattern;
  int width;
  int height;

public:
  Pattern(std::vector<std::vector<RGBA *>> bit_pattern) :
    source_context(0, 0),
    bit_pattern(bit_pattern)
  {
    if (bit_pattern.size() == 0) return;
    width = bit_pattern[0].size();
    height = bit_pattern.size();
  }

  Pattern(Context &source_context, RGBA *mask_color = nullptr) :
    source_context(source_context),
    width(source_context.get_width()),
    height(source_context.get_height())
  {
    std::vector<RGBA *> row(width, nullptr);
    std::vector<std::vector<RGBA *>> pattern(height, row);

    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
	RGBA *pixel = this->source_context.get_pixel_ptr({x, y});

	if (mask_color && *pixel == *mask_color) continue;
	pattern[y][x] = pixel;
      }
    }

    bit_pattern = pattern;
  }

  void draw_onto(Context &context, point center);

  static std::shared_ptr<Pattern> make_square(int side, RGBA *color)
  {
    std::vector<std::vector<RGBA *>> pattern;

    for (int i = 0; i < side; i++) {
      std::vector<RGBA *> row;

      for (int j = 0; j < side; j++) {
	row.push_back(color);
      }

      pattern.push_back(row);
    }

    return std::make_shared<Pattern>(pattern);
  }

  static std::shared_ptr<Pattern> make_circle(int radius, RGBA *color)
  {
    int side_len = radius * 2 + 1;
    Context source_context(side_len, side_len);
    auto circle_points = approx_circle(radius);
    const point *prev_point = nullptr;

    source_context.set_source_rgba(*color);

    // TODO: The way circle points are currently connected is not entirely correct
    for (const auto &point : circle_points) {
      if (!prev_point) {
	prev_point = &point;
	continue;
      }

      source_context.draw_line(*prev_point, point, 1);
      prev_point = &point;
    }

    RGBA mask = 0;
    return std::make_shared<Pattern>(source_context, &mask);
  }

};

}
