#include "canny.h"
#include <cassert>
#include <math.h>

Pixor::Matrix<float> x_mgrid(int val)
{
  int size = val * 2 + 1;
  Pixor::Matrix<float> res(size, size);

  for (int row = 0; row < size; row++) {
    for (int i = 0; i < size; i++) {
      res[row][i] = -val + i;
    }
  }

  return res;
}

Pixor::Matrix<float> y_mgrid(int val)
{
  int size = val * 2 + 1;
  Pixor::Matrix<float> res(size, size);

  for (int col = 0; col < size; col++) {
    for (int i = 0; i < size; i++) {
      res[i][col] = -val + i;
    }
  }

  return res;
}

Pixor::Matrix<float> gaussian_kernel(int size, float sigma)
{
  assert(size % 2 == 1);
  size /= 2;
  auto x = x_mgrid(size);
  auto y = y_mgrid(size);
  float normal = 1 / (2.0 * M_PI * pow(sigma, 2));

  return x.power(2).add(y.power(2)).div(2.0 * pow(sigma, 2)).neg().exp().mult(normal);
}
