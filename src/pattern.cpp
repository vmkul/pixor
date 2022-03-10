#include "pattern.h"
#include "debug.h"
#include "pixor.h"
#include <cmath>
#include <memory>

using namespace Pixor;

void Pattern::draw_onto(Context &context, point center) {
  if (bit_pattern.size() == 0)
    return;

  point start{center.x - (int)std::floor(width / 2.0),
              center.y - (int)std::floor(height / 2.0)};

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      RGBA *val = bit_pattern[y][x];
      if (!val)
        continue;

      context.set_pixel_safe({x + start.x, y + start.y}, *val);
    }
  }
}

Pattern::Pattern(std::vector<std::vector<RGBA *>> bit_pattern)
    : bit_pattern(bit_pattern) {
  if (bit_pattern.size() == 0)
    return;
  width = bit_pattern[0].size();
  height = bit_pattern.size();
}

Pattern::Pattern(std::shared_ptr<Context> source_context, RGBA *mask_color)
    : source_context(source_context), width(source_context->get_width()),
      height(source_context->get_height()) {
  std::vector<RGBA *> row(width, nullptr);
  std::vector<std::vector<RGBA *>> pattern(height, row);

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      RGBA *pixel = this->source_context->get_pixel_ptr({x, y});

      if (mask_color && *pixel == *mask_color)
        continue;
      pattern[y][x] = pixel;
    }
  }

  bit_pattern = pattern;
}

std::shared_ptr<Pattern> Pattern::make_square(int side, RGBA *color) {
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

std::shared_ptr<Pattern> Pattern::make_circle(int radius, RGBA *color) {
  int side_len = radius * 2 + 1;
  std::shared_ptr<Context> source_context = std::make_shared<Context>(side_len, side_len);
  auto circle_points = approx_circle(radius);
  const point *prev_point = nullptr;

  source_context->set_source_rgba(*color);

  // TODO: The way circle points are currently connected is not entirely
  // correct
  for (const auto &point : circle_points) {
    if (!prev_point) {
      prev_point = &point;
      continue;
    }

    source_context->draw_line(*prev_point, point, 1);
    prev_point = &point;
  }

  RGBA mask = 0;
  return std::make_shared<Pattern>(source_context, &mask);
}

std::shared_ptr<byte[]> Pattern::hydrate() const
{
  auto bitmap = std::shared_ptr<byte[]>(new byte[width * height * 4]);
  auto context = Context(bitmap, width, height);

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      RGBA *pixel_ptr = bit_pattern[y][x];
      if (!pixel_ptr) {
        dbgln("Can't hydrate pattern that contains null pointers!");
        return nullptr;
      }
      
      context.set_pixel_safe({x, y}, *pixel_ptr);
    }
  }

  return bitmap;
}
