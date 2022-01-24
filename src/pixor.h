#pragma once

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

  unsigned int red(RGBA c);
  unsigned int green(RGBA c);
  unsigned int blue(RGBA c);
  unsigned int alpha(RGBA c);
  RGBA rgba(unsigned int r, unsigned int g, unsigned int b, unsigned int a);
}
