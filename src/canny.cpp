#include "canny.h"
#include "debug.h"
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

Pixor::Matrix<double> gaussian_kernel(int size, double sigma = 1)
{
  assert(size % 2 == 1);
  size /= 2;
  auto x = x_mgrid(size);
  auto y = y_mgrid(size);
  double normal = 1 / (2.0 * M_PI * pow(sigma, 2));

  auto kernel = x.power(2).add(y.power(2)).div(2.0 * pow(sigma, 2)).neg().exp().mult(normal);
  return kernel.div(kernel.sum());
}

Pixor::Matrix<double> sobel_filter(Pixor::Matrix<double> &m, Pixor::Matrix<double> &theta)
{
  std::vector<std::vector<double>> kx_v = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
  std::vector<std::vector<double>> ky_v = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
  Matrix<double> kx(kx_v);
  Matrix<double> ky(ky_v);

  auto ix = m.convolve(kx);
  auto iy = m.convolve(ky);
  auto hypot = ix.hypot(iy);
  hypot = hypot.div(hypot.max()).mult(255);
  theta = iy.arctan2(ix);

  return hypot;
}

Pixor::Matrix<double> non_max_suppression(Pixor::Matrix<double> &m, Pixor::Matrix<double> &theta)
{
  int width = m.get_width();
  int height = m.get_height();
  Pixor::Matrix<double> res(width, height);
  auto angle = theta.mult(180).div(M_PI);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      auto val = angle[i][j];
      if (val < 0) {
        angle[i][j] += 180;
      }
    }
  }

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      double q = 255;
      double r = 255;
      auto angle_val = angle[i][j];

      if ((angle_val >= 0 && angle_val < 22.5) || (angle_val >= 157.5 && angle_val <= 180)) {
        if (j + 1 > width - 1 || j - 1 < 0) continue;
        q = m[i][j + 1];
        r = m[i][j - 1];
      } else if (angle_val >= 22.5 && angle_val < 67.5) {
        if (j + 1 > width - 1 || j - 1 < 0) continue;
        if (i + 1 > height - 1 || i - 1 < 0) continue;
        q = m[i + 1][j - 1];
        r = m[i - 1][j + 1];
      } else if (angle_val >= 67.5 && angle_val < 112.5) {
        if (i + 1 > height - 1 || i - 1 < 0) continue;
        q = m[i + 1][j];
        r = m[i - 1][j];
      } else if (angle_val >= 112.5 && angle_val < 157.5) {
        if (j + 1 > width - 1 || j - 1 < 0) continue;
        if (i + 1 > height - 1 || i - 1 < 0) continue;
          q = m[i - 1][j - 1];
        r = m[i + 1][j + 1];
      }

      if (m[i][j] >= q && m[i][j] >= r) {
        res[i][j] = m[i][j];
      } else {
        res[i][j] = 0;
      }

    }
  }

  return res;
}

Pixor::Matrix<double> threshold(Pixor::Matrix<double> &m, double low_threshold_ratio = 0.03, double high_threshold_ratio = 0.12)
{
  auto high_threshold = m.max() * high_threshold_ratio;
  auto low_threshold = high_threshold * low_threshold_ratio;
  int width = m.get_width();
  int height = m.get_height();
  Pixor::Matrix<double> res(width, height);
  int weak = 25;
  int strong = 255;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      auto m_val = m[i][j];

      if (m_val >= high_threshold) {
        res[i][j] = strong;
      } else if (m_val >= low_threshold) {
        res[i][j] = weak;
      }
    }
  }

  return res;
}

Pixor::Matrix<double> hysteresis(Pixor::Matrix<double> &m, int weak = 25, int strong = 255)
{
  int width = m.get_width();
  int height = m.get_height();
  Pixor::Matrix<double> res(m);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      res[i][j] = m[i][j];
    }
  }

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      auto m_val = m[i][j];
      if (m_val != weak) continue;

      for (int row_offset = 0; row_offset < 3; row_offset++) {
        for (int col_offset = 0; col_offset < 3; col_offset++) {
          int src_row = i + row_offset - 1;
          int src_col = j + col_offset - 1;
          if (src_row == i && src_col == j) continue;
          if (src_row < 0 || src_row > height - 1 || src_col < 0 || src_col > width - 1) {
            continue;
          }
          if (m[src_row][src_col] == strong) {
            res[i][j] = strong;
            goto endpixel;
          }
        }
      }
      m[i][j] = 0;
      endpixel:;
    }
  }

  return res;
}

Pixor::Matrix<double> canny_edge_detector(Pixor::Matrix<double> &m)
{
  auto res = m.convolve(gaussian_kernel(5));
  Pixor::Matrix<double> theta(100, 100);
  res = sobel_filter(res, theta);
  res = non_max_suppression(res, theta);
  res = threshold(res);
  res = hysteresis(res);
  
  return res;
}
