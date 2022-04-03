#include <iostream>
#include <cassert>
#include <cmath>
#include "debug.h"
#include "matrix.h"

using namespace Pixor;

const int NUM_BLOCKS = 100;
const int THREADS_PER_BLOCK = 512;

// Try to mutate current matrix instead of returning new ones
// and calling cudaMallocManaged too many times.

__host__
__device__
Row::Row(int length, double *r) :
  r(r),
  length(length)
  {}

__host__
__device__
double &Row::operator[](int index)
{
  assert(index >= 0 && index < length);
  return r[index];
}


Matrix::Matrix(int width, int height) :
  width(width),
  height(height)
{
  cudaMallocManaged(&m, width * height * sizeof(double));

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      (*this)[i][j] = 0;
    }
  }
}


Matrix::Matrix(std::vector<std::vector<double>> matrix) :
  width(matrix[0].size()),
  height(matrix.size())
{
  cudaMallocManaged(&m, width * height * sizeof(double));

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      (*this)[i][j] = matrix[i][j];
    }
  }
}

__host__
__device__
int Matrix::get_width()
{
  return width;
}

__host__
__device__
int Matrix::get_height()
{
  return height;
}

__host__
__device__
Row Matrix::operator[](int index)
{
  assert(index >= 0 && index < height);
  return Row(width, m + index * width);
}


void Matrix::print()
{
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      std::cout << (*this)[i][j] << " ";
    }
    std::cout << '\n';
  }
}

__global__
void matrix_power(Matrix src, Matrix dest, int exponent)
{
  int width = src.get_width();
  int height = src.get_height();
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    dest.m[index] = pow(src.m[index], exponent);
  }
}

Matrix Matrix::power(int exponent)
{
  Matrix res(width, height);

  matrix_power<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(*this, res, exponent);
  cudaDeviceSynchronize();
  
  return res;
}

__global__
void matrix_add(Matrix src, Matrix dest, Matrix other)
{
  int width = src.get_width();
  int height = src.get_height();
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    dest.m[index] = src.m[index] + other.m[index];
  }
}

Matrix Matrix::add(Matrix other)
{
  Matrix res(width, height);

  matrix_add<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(*this, res, other);
  cudaDeviceSynchronize();
  
  return res;
}

__global__
void matrix_mult(Matrix src, Matrix dest, double k)
{
  int width = src.get_width();
  int height = src.get_height();
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    dest.m[index] = src.m[index] * k;
  }
}

Matrix Matrix::mult(double k)
{
  Matrix res(width, height);

  matrix_mult<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(*this, res, k);
  cudaDeviceSynchronize();
  
  return res;
}

__global__
void matrix_div(Matrix src, Matrix dest, double k)
{
  int width = src.get_width();
  int height = src.get_height();
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    dest.m[index] = src.m[index] / k;
  }
}

Matrix Matrix::div(double k)
{
  Matrix res(width, height);

  matrix_div<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(*this, res, k);
  cudaDeviceSynchronize();
  
  return res;
}

__global__
void matrix_exp(Matrix src, Matrix dest)
{
  int width = src.get_width();
  int height = src.get_height();
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    dest.m[index] = std::exp(src.m[index]);
  }
}

Matrix Matrix::exp()
{
  Matrix res(width, height);

  matrix_exp<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(*this, res);
  cudaDeviceSynchronize();
  
  return res;
}

__global__
void matrix_neg(Matrix src, Matrix dest)
{
  int width = src.get_width();
  int height = src.get_height();
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    dest.m[index] = -src.m[index];
  }
}

Matrix Matrix::neg() {
  Matrix res(width, height);

  matrix_neg<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(*this, res);
  cudaDeviceSynchronize();
  
  return res;
}

double Matrix::sum() {
  double res = 0;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      res += (*this)[i][j];
    }
  }

  return res;
}


double Matrix::max() {
  double res = (*this)[0][0];

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      double val = (*this)[i][j];
      if (val > res) res = val;
    }
  }

  return res;
}

__global__
void matrix_convolve(Matrix src, Matrix dest, Matrix kernel, int width, int height, int kernel_width, int kernel_height)
{
  int offset = (kernel_width - 1) / 2;
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    int row = index / width;
    int col = index - row * width;
    double val = 0;

    for (int kernel_row = 0; kernel_row < kernel_height; kernel_row++) {
      for (int kernel_col = 0; kernel_col < kernel_width; kernel_col++) {
        int src_row = row + kernel_row - offset;
        int src_col = col + kernel_col - offset;
        if (src_row < 0 || src_row > height - 1) {
          src_row = row + (kernel_height - kernel_row) - offset;
        }
        if (src_col < 0 || src_col > width - 1) {
          src_col = col + (kernel_width - kernel_col) - offset;
        }
        double src_val = src[src_row][src_col];
        double k_val = kernel[kernel_height - 1 - kernel_row][kernel_width - 1 - kernel_col];

        val += k_val * src_val;
      }
    }

    dest[row][col] = val;
  }
}


Matrix Matrix::convolve(Matrix kernel) {
  Matrix res(width, height);
  int kernel_width = kernel.get_width();
  int kernel_height = kernel.get_height();
  assert(kernel_width == kernel_height);
  assert(kernel_width % 2 == 1);

  matrix_convolve<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(*this, res, kernel, width, height, kernel_width, kernel_height);
  cudaDeviceSynchronize();

  return res;
}

__global__
void matrix_hypot(Matrix src, Matrix dest, Matrix other)
{
  int width = src.get_width();
  int height = src.get_height();
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    double val1 = src.m[index];
    double val2 = other.m[index];
    double r = sqrt(val1 * val1 + val2 * val2);

    dest.m[index] = r;
  }
}

Matrix Matrix::hypot(Matrix other)
{
  Matrix res(width, height);

  matrix_hypot<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(*this, res, other);
  cudaDeviceSynchronize();
  
  return res;
}

__global__
void matrix_arctan2(Matrix src, Matrix dest, Matrix other)
{
  int width = src.get_width();
  int height = src.get_height();
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  
  for (; index < width * height; index += stride) {
    double val1 = src.m[index];
    double val2 = other.m[index];
    double r = atan(val1 / val2);

    dest.m[index] = r;
  }
}

Matrix Matrix::arctan2(Matrix other)
{
  Matrix res(width, height);

  matrix_arctan2<<<NUM_BLOCKS, THREADS_PER_BLOCK>>>(*this, res, other);
  cudaDeviceSynchronize();
  
  return res;
}
