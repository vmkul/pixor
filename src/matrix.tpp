#include <iostream>
#include <cassert>
#include <cmath>
#include <thread>

using namespace Pixor;

template <class T>
Row<T>::Row(int length, T *r) :
  r(r),
  length(length)
  {}

template <class T>
T &Row<T>::operator[](int index)
{
  assert(index >= 0 && index < length);
  return r[index];
}

template <class T>
Matrix<T>::Matrix(int width, int height) :
  width(width),
  height(height)
{
  m = std::shared_ptr<T>(new T[width * height]);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      (*this)[i][j] = 0;
    }
  }
}

template <class T>
Matrix<T>::Matrix(std::vector<std::vector<T>> matrix) :
  width(matrix[0].size()),
  height(matrix.size())
{
  m = std::shared_ptr<T>(new T[width * height]);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      (*this)[i][j] = matrix[i][j];
    }
  }
}

template <class T>
Row<T> Matrix<T>::operator[](int index)
{
  assert(index >= 0 && index < height);
  return Row<T>(width, m.get() + index * width);
}

template <class T>
void Matrix<T>::print()
{
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      std::cout << (*this)[i][j] << " ";
    }
    std::cout << '\n';
  }
}

template <class T>
Matrix<T> Matrix<T>::power(int exponent)
{
  Matrix<T> res(width, height);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      res[i][j] = pow((*this)[i][j], exponent);
    }
  }
  
  return res;
}

template <class T>
Matrix<T> Matrix<T>::add(Matrix<T> other)
{
  Matrix<T> res(width, height);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      res[i][j] = (*this)[i][j] + other[i][j];
    }
  }
  
  return res;
}

template <class T>
Matrix<T> Matrix<T>::mult(float k)
{
  Matrix<T> res(width, height);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      res[i][j] = (*this)[i][j] * k;
    }
  }
  
  return res;
}

template <class T>
Matrix<T> Matrix<T>::div(float k)
{
  Matrix<T> res(width, height);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      res[i][j] = (*this)[i][j] / k;
    }
  }
  
  return res;
}

template <class T>
Matrix<T> Matrix<T>::exp()
{
  Matrix<T> res(width, height);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      res[i][j] = std::exp((*this)[i][j]);
    }
  }
  
  return res;
}

template <class T>
Matrix<T> Matrix<T>::neg() {
  Matrix<T> res(width, height);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      res[i][j] = -(*this)[i][j];
    }
  }
  
  return res;
}

template <class T>
T Matrix<T>::sum() {
  T res = 0;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      res += (*this)[i][j];
    }
  }

  return res;
}

template <class T>
T Matrix<T>::max() {
  T res = (*this)[0][0];

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      T val = (*this)[i][j];
      if (val > res) res = val;
    }
  }

  return res;
}

template <class T>
void convolve_stripe(Matrix<T> kernel, Matrix<T> src, Matrix<T> res, int from_row, int to_row)
{
  int width = src.get_width();
  int height = src.get_height();
  int kernel_width = kernel.get_width();
  int kernel_height = kernel.get_height();
  int offset = (kernel_width - 1) / 2;
  assert(kernel_width == kernel_height);
  assert(kernel_width % 2 == 1);

  for (int row = from_row; row < to_row; row++) {
    for (int col = 0; col < width; col++) {
      T val = 0;

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
          T src_val = src[src_row][src_col];
          T k_val = kernel[kernel_height - 1 - kernel_row][kernel_width - 1 - kernel_col];

          val += k_val * src_val;
        }
      }
      
      res[row][col] = val;
    }
  }
}

template <class T>
Matrix<T> Matrix<T>::convolve(Matrix<T> kernel, int num_threads) {
  Matrix<T> res(width, height);
  int rows_per_thread = height / num_threads;
  int start_row = 0;
  std::thread threads[num_threads];

  for (int i = 0; i < num_threads; i++) {
    if (i == num_threads - 1) {
      threads[i] = std::thread(convolve_stripe<T>, kernel, *this, res, start_row, height);
    } else {
      threads[i] = std::thread(convolve_stripe<T>, kernel, *this, res, start_row, start_row + rows_per_thread);
    }

    start_row += rows_per_thread;
  }

  for (int i = 0; i < num_threads; i++) {
    threads[i].join();
  }

  return res;
}

template <class T>
Matrix<T> Matrix<T>::hypot(Matrix<T> other)
{
  auto res = Matrix<T>(width, height);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      T val1 = (*this)[i][j];
      T val2 = other[i][j];
      T r = sqrt(val1 * val1 + val2 * val2);

      res[i][j] = r;
    }
  }

  return res;
}

template <class T>
Matrix<T> Matrix<T>::arctan2(Matrix<T> other)
{
  auto res = Matrix<T>(width, height);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      T val1 = (*this)[i][j];
      T val2 = other[i][j];
      T r = atan(val1 / val2);

      res[i][j] = r;
    }
  }

  return res;
}
