#pragma once
#include "matrix.h"

Pixor::Matrix<float> x_mgrid(int val);

Pixor::Matrix<float> y_mgrid(int val);

Pixor::Matrix<float> gaussian_kernel(int size, float sigma = 1);
