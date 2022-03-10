#pragma once
#include <vector>

typedef unsigned char byte;
typedef unsigned int RGBA;
#define UNUSED(expr) do { (void)(expr); } while (0)

namespace Pixor {

  template <class T>
  T byte_swap_32(T x)
  {
    byte res[4];
    byte *src = (byte *) &x;

    for (int i = 0; i < 4; i++) {
      res[i] = src[3 - i];
    }

    return *((T *) res);
  }

  template <class T>
  T clamp(T lower_bound, T upper_bound, T arg)
  {
    if (arg < lower_bound) return lower_bound;
    if (arg > upper_bound) return upper_bound;
    return arg;
  }

  unsigned int red(RGBA c);
  unsigned int green(RGBA c);
  unsigned int blue(RGBA c);
  unsigned int alpha(RGBA c);
  RGBA rgba(unsigned int r, unsigned int g, unsigned int b, unsigned int a);

  struct point {
    int x;
    int y;
  };

  std::vector<point> approx_circle(int radius);
  std::vector<point> approx_line(point p1, point p2);

}
