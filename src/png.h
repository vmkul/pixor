#pragma once
#include <iostream>

const unsigned char PNG_SIGNATURE[] = {137, 80, 78, 71, 13, 10, 26, 10};

class PngImage {

};

PngImage *decode_png(std::istream& data_stream);
