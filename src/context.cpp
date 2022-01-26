#include "math.h"
#include "context.h"
#include "debug.h"

using namespace Pixor;

bool Context::coord_in_bounds(point p)
{
  return (p.x >= 0 && p.x < width && p.y >= 0 && p.y < height);
}

void Context::set_source_rgba(RGBA color)
{
  source_color = color;
}

void Context::set_pixel(point coord, RGBA value)
{
  pixel_data[coord.y * width + coord.x] = value;
}

void Context::set_pixel_safe(point coord, RGBA value)
{
  if (!coord_in_bounds(coord)) return;
  set_pixel(coord, value);
}

RGBA Context::get_pixel(point coord)
{
  return pixel_data[coord.y * width + coord.x];
}

RGBA Context::get_pixel_safe(point coord, RGBA default_value)
{
  if (!coord_in_bounds(coord)) return default_value;
  return get_pixel(coord);
}

void Context::draw_line(point p1, point p2, int line_width)
{
  point from;
  point to;
  int dx = std::abs(p1.x - p2.x);
  int dy = std::abs(p1.y - p2.y);
  bool y_fun = false;

  if (dx >= dy) {
    if (p1.x > p2.x) {
      from = p2;
      to = p1;
    } else {
      from = p1;
      to = p2;
    }
  } else {
    y_fun = true;
    if (p1.y > p2.y) {
      from = {p2.y, p2.x};
      to = {p1.y, p1.x};
    } else {
      from = {p1.y, p1.x};
      to = {p2.y, p2.x};
    }
  }

  dx = to.x - from.x;
  dy = to.y - from.y;
  float k = dy / (float) dx;
  Pattern drawing_pattern = Pattern::make_square(line_width, &source_color);

  for (int i = 0; i <= dx; i++) {
    float y_val = i * k + from.y;
    float rounded = std::round(y_val);

    if (y_fun) {
      drawing_pattern.draw_onto(*this, {(int) rounded, i + from.x});
    } else {
      drawing_pattern.draw_onto(*this, {i + from.x, (int) rounded});
    }
  }
}

void Pattern::draw_onto(Context &context, point center)
{
  if (bit_pattern.size() == 0) return;

  point start {
    center.x - (int) std::floor(width / 2.0),
    center.y - (int) std::floor(height / 2.0)
  };

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      RGBA *val = bit_pattern[y][x];
      if (!val) continue;

      context.set_pixel_safe({x + start.x, y + start.y}, *val);
    }
  }
}
