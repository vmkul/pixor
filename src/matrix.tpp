#include <iostream>
#include <cassert>
#include <cmath>

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
