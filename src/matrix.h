#pragma once
#include <memory>
#include <vector>

namespace Pixor {

template<class T>
class Row {
  T *r;
  int length;

public:
  Row(int length, T *r);
  T &operator[](int index);
};

template <class T>
class Matrix {
  std::shared_ptr<T> m;
  int width;
  int height;

public:
  Matrix(int width, int height);
  Matrix(std::vector<std::vector<T>> matrix);
  int get_width() {return width;};
  int get_height() {return height;};
  Row<T> operator[](int index);
  Matrix<T> power(int exponent);
  Matrix<T> add(Matrix<T> other);
  Matrix<T> mult(float k);
  Matrix<T> div(float k);
  Matrix<T> exp();
  Matrix<T> neg();
  Matrix<T> convolve(Matrix<T> kernel);
  Matrix<T> hypot(Matrix<T> other);
  Matrix<T> arctan2(Matrix<T> other);
  T sum();
  T max();
  void print();
};

}

#include "matrix.tpp"
