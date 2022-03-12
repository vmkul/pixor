#include "pattern.h"
#include "debug.h"
#include "pixor.h"
#include <cmath>
#include <memory>

using namespace Pixor;

RGBA *Pattern::get_pixel(point coord) const
{
  return bitmap[coord.y * width + coord.x];
}

void Pattern::set_pixel(point coord, RGBA *value)
{
  bitmap[coord.y * width + coord.x] = value;
}

void Pattern::draw_onto(Context &context, point center) {
  if (!bitmap)
    return;

  point start{center.x - (int)std::floor(width / 2.0),
              center.y - (int)std::floor(height / 2.0)};

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      RGBA *val = get_pixel({x, y});
      if (!val)
        continue;

      context.set_pixel_safe({x + start.x, y + start.y}, *val);
    }
  }
}

Pattern::Pattern(std::shared_ptr<RGBA *[]> bitmap, int width, int height)
    : bitmap(bitmap) {
  this->width = width;
  this->height = height;
}

Pattern::Pattern(std::shared_ptr<Context> source_context, RGBA mask_color)
    : source_context(source_context), width(source_context->get_width()),
      height(source_context->get_height()) {
  bitmap = std::shared_ptr<RGBA *[]>(new RGBA *[width * height]);

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      RGBA *pixel = this->source_context->get_pixel_ptr({x, y});
      if (*pixel == mask_color) {
        set_pixel({x, y}, nullptr);
      } else {
        set_pixel({x, y}, pixel);
      }
    }
  }
}

Pattern::Pattern(std::shared_ptr<Context> source_context)
    : source_context(source_context), width(source_context->get_width()),
      height(source_context->get_height()) {
  bitmap = std::shared_ptr<RGBA *[]>(new RGBA *[width * height]);

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      RGBA *pixel = this->source_context->get_pixel_ptr({x, y});
      set_pixel({x, y}, pixel);
    }
  }
}

std::shared_ptr<Pattern> Pattern::make_square(int side, RGBA *color) {
  auto bitmap = std::shared_ptr<RGBA *[]>(new RGBA *[side * side]);
  auto res = std::make_shared<Pattern>(bitmap, side, side);

  for (int i = 0; i < side; i++) {
    for (int j = 0; j < side; j++) {
      res->set_pixel({i, j}, color);
    }
  }

  return res;
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

  return std::make_shared<Pattern>(source_context, 0);
}

std::shared_ptr<byte[]> Pattern::hydrate() const
{
  auto dest = new RGBA[width * height];
  auto src = (RGBA **) bitmap.get();

  for (int i = 0; i < width * height; i++) {
    RGBA *pixel_ptr = src[i];
    if (!pixel_ptr) {
      dbgln("Can't hydrate pattern that contains null pointers!");
      return nullptr;
    }
    
    dest[i] = *pixel_ptr;
  }

  return std::shared_ptr<byte[]>((byte *) dest);
}
