#include "context.h"
#include "debug.h"
#include "pixor.h"
#include <memory>
#include <vector>
#include <cmath>

using namespace Pixor;

point Context::clamp_coord(point coord) const
{
  return {clamp(0, width - 1, coord.x), clamp(0, height - 1, coord.y)};
}

bool Context::coord_in_bounds(point p)
{
  return (p.x >= 0 && p.x < width && p.y >= 0 && p.y < height);
}

void Context::set_source_pattern(std::shared_ptr<Pattern> pattern)
{
  source_pattern = pattern;
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

void Context::set_pixel_clamped(point coord, RGBA value)
{
  set_pixel(clamp_coord(coord), value);
}

RGBA Context::get_pixel(point coord)
{
  return pixel_data[coord.y * width + coord.x];
}

RGBA *Context::get_pixel_ptr(point coord) const
{
  return pixel_data + coord.y * width + coord.x;
}

RGBA *Context::get_pixel_ptr_clamped(point coord) const
{
  point clamped = clamp_coord(coord);
  return pixel_data + clamped.y * width + clamped.x;
}

RGBA Context::get_pixel_safe(point coord, RGBA default_value)
{
  if (!coord_in_bounds(coord)) return default_value;
  return get_pixel(coord);
}

void Context::draw_pattern(std::vector<point> &points, Pattern &p)
{
  for (const auto &point : points) {
    p.draw_onto(*this, point);
  }
}

void Context::draw_line(point p1, point p2, int line_width)
{
  auto points = approx_line(p1, p2);
  auto drawing_pattern = Pattern::make_square(line_width, &source_color);
  draw_pattern(points, *drawing_pattern);
}

void Context::draw_line_with_pattern(point p1, point p2)
{
  if (!source_pattern) return;

  auto points = approx_line(p1, p2);
  draw_pattern(points, *source_pattern);
}

std::shared_ptr<Pattern> Context::scale(int new_width, int new_height) const
{
  auto bitmap = std::shared_ptr<RGBA *[]>(new RGBA *[new_width * new_height]);
  auto res = std::make_shared<Pattern>(bitmap, new_width, new_height);
  float scale_x = new_width / (float) width;
  float scale_y = new_height / (float) height;

  for (int x = 0; x < new_width; x++) {
    for (int y = 0; y < new_height; y++) {
      int src_x = std::round(x / scale_x);
      int src_y = std::round(y / scale_y);

      res->set_pixel({x, y}, get_pixel_ptr_clamped({src_x, src_y}));
    }
  }

  return res;
}

std::shared_ptr<Context> Context::convolve(Matrix<float> kernel)
{
  auto res = std::shared_ptr<Context>(new Context(*this));
  int kernel_width = kernel.get_width();
  int kernel_height = kernel.get_height();
  int offset = (kernel_width - 1) / 2;
  float weight_sum = kernel.sum();
  assert(kernel_width == kernel_height);
  assert(kernel_width % 2 == 1);

  for (int image_row = 0; image_row < height; image_row++) {
    for (int image_col = 0; image_col < width; image_col++) {
      RGBA pixel = get_pixel({image_col, image_row});
      RGBA r_val = 0;
      RGBA g_val = 0;
      RGBA b_val = 0;

      for (int kernel_row = 0; kernel_row < kernel_height; kernel_row++) {
        for (int kernel_col = 0; kernel_col < kernel_width; kernel_col++) {
          int src_row = image_row + kernel_row - offset;
          int src_col = image_col + kernel_col - offset;
          if (src_row < 0 || src_row > height - 1) {
            src_row = image_row + (kernel_height - kernel_row) - offset;
          }
          if (src_col < 0 || src_col > width - 1) {
            src_col = image_row + (kernel_width - kernel_col) - offset;
          }
          RGBA src_pixel = get_pixel({src_col, src_row});
          float k_val = kernel[kernel_height - 1 - kernel_row][kernel_width - 1 - kernel_col];

          r_val += k_val * red(src_pixel);
          g_val += k_val * green(src_pixel);
          b_val += k_val * blue(src_pixel);
        }
      }

      res->set_pixel({image_col, image_row}, rgba(
        r_val / weight_sum,
        g_val / weight_sum,
        b_val / weight_sum,
        alpha(pixel)));
    }
  }

  return res;
}
