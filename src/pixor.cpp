#include "pixor.h"
#include <cmath>

using namespace Pixor;

unsigned int Pixor::red(RGBA c)
{
  return (0x000000ff & c) >> 0;
}

unsigned int Pixor::green(RGBA c)
{
  return (0x0000ff00 & c) >> 8;
}

unsigned int Pixor::blue(RGBA c)
{
  return (0x00ff0000 & c) >> 16;
}

unsigned int Pixor::alpha(RGBA c)
{
  return (0xff000000 & c) >> 24;
}

RGBA Pixor::rgba(unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{
  return (r << 0) | (g << 8) | (b << 16) | (a << 24);
}

std::vector<point> Pixor::approx_line(point p1, point p2)
{
  std::vector<point> res;
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

  for (int i = 0; i <= dx; i++) {
    float y_val = i * k + from.y;
    float rounded = std::round(y_val);

    if (y_fun) {
      res.push_back({(int) rounded, i + from.x});
    } else {
      res.push_back({i + from.x, (int) rounded});
    }
  }

  return res;
}

std::vector<point> Pixor::approx_circle(int radius)
{
  std::vector<point> res;

  for (int x = -radius; x <= radius; x++) {
    int y = (int) std::round(std::sqrt(std::pow(radius, 2) - std::pow(x, 2)));

    res.push_back({x + radius, y + radius});
    res.push_back({x + radius, -y + radius});
  }

  return res;
}
