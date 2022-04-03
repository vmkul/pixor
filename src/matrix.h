#pragma once
#include <memory>
#include <vector>

namespace Pixor {

class Row {
  double *r;
  int length;

public:
  Row(int length, double *r);
  double &operator[](int index);
};

class Matrix {
public:
  double *m;
  int width;
  int height;

  Matrix(int width, int height);
  Matrix(std::vector<std::vector<double>> matrix);
  int get_width();
  int get_height();
  Row operator[](int index);
  Matrix power(int exponent);
  Matrix add(Matrix other);
  Matrix mult(double k);
  Matrix div(double k);
  Matrix exp();
  Matrix neg();
  Matrix convolve(Matrix kernel);
  Matrix hypot(Matrix other);
  Matrix arctan2(Matrix other);
  double sum();
  double max();
  void print();
};

}
