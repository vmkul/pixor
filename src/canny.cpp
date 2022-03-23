#include "canny.h"
#include <cassert>
#include <math.h>

Pixor::Matrix<double> x_mgrid(int val)
{
  int size = val * 2 + 1;
  Pixor::Matrix<double> res(size, size);

  for (int row = 0; row < size; row++) {
    for (int i = 0; i < size; i++) {
      res[row][i] = -val + i;
    }
  }

  return res;
}

Pixor::Matrix<double> y_mgrid(int val)
{
  int size = val * 2 + 1;
  Pixor::Matrix<double> res(size, size);

  for (int col = 0; col < size; col++) {
    for (int i = 0; i < size; i++) {
      res[i][col] = -val + i;
    }
  }

  return res;
}

Pixor::Matrix<double> gaussian_kernel(int size, double sigma)
{
  assert(size % 2 == 1);
  size /= 2;
  auto x = x_mgrid(size);
  auto y = y_mgrid(size);
  double normal = 1 / (2.0 * M_PI * pow(sigma, 2));

  auto kernel = x.power(2).add(y.power(2)).div(2.0 * pow(sigma, 2)).neg().exp().mult(normal);
  return kernel.div(kernel.sum());
}

Pixor::Matrix<double> sobel_filter(Pixor::Matrix<double> &m)
{
  std::vector<std::vector<double>> kx_v = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
  std::vector<std::vector<double>> ky_v = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
  Matrix<double> kx(kx_v);
  Matrix<double> ky(ky_v);

  auto ix = m.convolve(kx);
  auto iy = m.convolve(ky);
  auto hypot = ix.hypot(iy);
  hypot = hypot.div(hypot.max()).mult(255);

  return hypot;
}

Pixor::Matrix<double> canny_edge_detector(Pixor::Matrix<double> &m)
{
  auto res = m.convolve(gaussian_kernel(5));
  res = sobel_filter(res);

  return res;
}
