#include "canny.h"
#include "debug.h"
#include <cassert>
#include <math.h>

const int NUM_BLOCKS = 256;
const int THREADS_PER_BLOCK = 750;

Pixor::Matrix x_mgrid(int val)
{
  int size = val * 2 + 1;
  Pixor::Matrix res(size, size);

  for (int row = 0; row < size; row++) {
    for (int i = 0; i < size; i++) {
      res[row][i] = -val + i;
    }
  }

  return res;
}

Pixor::Matrix y_mgrid(int val)
{
  int size = val * 2 + 1;
  Pixor::Matrix res(size, size);

  for (int col = 0; col < size; col++) {
    for (int i = 0; i < size; i++) {
      res[i][col] = -val + i;
    }
  }

  return res;
}

Pixor::Matrix gaussian_kernel(int size, double sigma = 1)
{
  assert(size % 2 == 1);
  size /= 2;
  auto x = x_mgrid(size);
  auto y = y_mgrid(size);
  double normal = 1 / (2.0 * M_PI * pow(sigma, 2));

  auto kernel = x.power(2).add(y.power(2)).div(2.0 * pow(sigma, 2)).neg().exp().mult(normal);
  return kernel.div(kernel.sum());
}

Pixor::Matrix sobel_filter(Pixor::Matrix &m, Pixor::Matrix &theta)
{
  std::vector<std::vector<double>> kx_v = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
  std::vector<std::vector<double>> ky_v = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
  Pixor::Matrix kx(kx_v);
  Pixor::Matrix ky(ky_v);

  auto ix = m.convolve(kx);
  auto iy = m.convolve(ky);
  auto hypot = ix.hypot(iy);
  hypot = hypot.div(hypot.max()).mult(255);
  theta = iy.arctan2(ix);

  return hypot;
}

__global__
void normalize_angle(Pixor::Matrix angle)
{
  int width = angle.width;
  int height = angle.height;
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    auto val = angle.m[index];
    if (val < 0) {
      angle.m[index] += 180;
    }
  }

}

__device__
int get_index(int i, int j, int width) {
  return i * width + j;
}

__global__
void cuda_non_max_suppression(Pixor::Matrix angle, Pixor::Matrix m, Pixor::Matrix res)
{
  int width = angle.width;
  int height = angle.height;
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
      double q = 255;
      double r = 255;
      auto angle_val = angle.m[index];
      int i = index / width;
      int j = index - i * width;

      if ((angle_val >= 0 && angle_val < 22.5) || (angle_val >= 157.5 && angle_val <= 180)) {
        if (j + 1 > width - 1 || j - 1 < 0) continue;
        q = m.m[get_index(i, j + 1, width)];
        r = m.m[get_index(i, j - 1, width)];
      } else if (angle_val >= 22.5 && angle_val < 67.5) {
        if (j + 1 > width - 1 || j - 1 < 0) continue;
        if (i + 1 > height - 1 || i - 1 < 0) continue;
        q = m.m[get_index(i + 1, j - 1, width)];
        r = m.m[get_index(i - 1, j + 1, width)];
      } else if (angle_val >= 67.5 && angle_val < 112.5) {
        if (i + 1 > height - 1 || i - 1 < 0) continue;
        q = m.m[get_index(i + 1, j, width)];
        r = m.m[get_index(i - 1, j, width)];
      } else if (angle_val >= 112.5 && angle_val < 157.5) {
        if (j + 1 > width - 1 || j - 1 < 0) continue;
        if (i + 1 > height - 1 || i - 1 < 0) continue;
        q = m.m[get_index(i - 1, j - 1, width)];
        r = m.m[get_index(i + 1, j + 1, width)];
      }

      if (m.m[index] >= q && m.m[index] >= r) {
        res.m[index] = m.m[index];
      } else {
        res.m[index] = 0;
      }
  }
}

Pixor::Matrix non_max_suppression(Pixor::Matrix &m, Pixor::Matrix &theta)
{
  int width = m.get_width();
  int height = m.get_height();
  Pixor::Matrix res(width, height);
  auto angle = theta.mult(180).div(M_PI);
  normalize_angle<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(angle);
  cuda_non_max_suppression<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(angle, m, res);
  cudaDeviceSynchronize();

  return res;
}

__global__
void cuda_threshold(Pixor::Matrix src, Pixor::Matrix res, int weak, int strong, double low_threshold, double high_threshold)
{
  int width = src.width;
  int height = src.height;
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    double m_val = src.m[index];

    if (m_val >= high_threshold) {
      res.m[index] = strong;
    } else if (m_val >= low_threshold) {
      res.m[index] = weak;
    }
  }
}

Pixor::Matrix threshold(Pixor::Matrix &m, double low_threshold_ratio = 0.05, double high_threshold_ratio = 0.09, int weak_pixel = 25, int strong_pixel = 255)
{
  auto high_threshold = m.max() * high_threshold_ratio;
  auto low_threshold = high_threshold * low_threshold_ratio;
  int width = m.get_width();
  int height = m.get_height();
  Pixor::Matrix res(width, height);

  cuda_threshold<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(m, res, weak_pixel, strong_pixel, low_threshold, high_threshold);
  cudaDeviceSynchronize();

  return res;
}

__global__
void cuda_hysteresis(Pixor::Matrix m, int weak, int strong)
{
  int width = m.width;
  int height = m.height;
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    auto m_val = m.m[index];
    int i = index / width;
    int j = index - i * width;
    if (m_val != weak) continue;

    for (int row_offset = 0; row_offset < 3; row_offset++) {
      for (int col_offset = 0; col_offset < 3; col_offset++) {
        int src_row = i + row_offset - 1;
        int src_col = j + col_offset - 1;
        if (src_row == i && src_col == j) continue;
        if (src_row < 0 || src_row > height - 1 || src_col < 0 || src_col > width - 1) {
          continue;
        }
        if (m.m[get_index(src_row, src_col, width)] == strong) {
          m.m[index] = strong;
          goto endpixel;
        }
      }
    }
    m.m[index] = 0;
    endpixel:;
  }
}

Pixor::Matrix hysteresis(Pixor::Matrix &m, int weak = 25, int strong = 255)
{
  int width = m.get_width();
  int height = m.get_height();

  cuda_hysteresis<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(m, weak, strong);
  cudaDeviceSynchronize();

  return m;
}

Pixor::Matrix canny_edge_detector(Pixor::Matrix &m, double sigma, int kernel_size, double low_threshold, double high_threshold, int weak_pixel)
{
  auto res = m.convolve(gaussian_kernel(kernel_size, sigma));
  Pixor::Matrix theta(1, 1);
  res = sobel_filter(res, theta);
  res = non_max_suppression(res, theta);
  res = threshold(res, low_threshold, high_threshold, weak_pixel);
  res = hysteresis(res, weak_pixel);

  int err = cudaGetLastError();
  if (err != 0) {
    dbgln("CUDA ERROR!: %d", err);
    return Pixor::Matrix(0, 0);
  }

  return res;
}
