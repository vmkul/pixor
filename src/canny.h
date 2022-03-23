#pragma once
#include "matrix.h"
#include "context.h"

Pixor::Matrix<double> x_mgrid(int val);

Pixor::Matrix<double> y_mgrid(int val);

Pixor::Matrix<double> gaussian_kernel(int size, double sigma = 1);

Pixor::Matrix<double> sobel_filter(Pixor::Matrix<double> &m);

Pixor::Matrix<double> canny_edge_detector(Pixor::Matrix<double> &m);
