#pragma once
#include <memory>

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
  int get_width() {return width;};
  int get_height() {return height;};
  Row<T> operator[](int index);
  Matrix<T> power(int exponent);
  Matrix<T> add(Matrix<T> other);
  Matrix<T> mult(float k);
  Matrix<T> div(float k);
  Matrix<T> exp();
  Matrix<T> neg();
  T sum();
  void print();
};

}

#include "matrix.tpp"
