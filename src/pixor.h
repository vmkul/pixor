#pragma once

typedef unsigned char byte;
typedef int RGBA;
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
}
