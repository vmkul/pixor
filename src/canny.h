#pragma once
#include "matrix.h"

Pixor::Matrix canny_edge_detector(Pixor::Matrix &m, double sigma=1.4, int kernel_size=5, double low_threshold=0.09, double high_threshold=0.17, int weak_pixel=100);
