#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "pixor.h"
#include "png_chunk.h"
#include "image.h"

namespace Pixor {

enum FilterType {
  FILTER_TYPE_NONE = 0,
  FILTER_TYPE_SUB = 1,
  FILTER_TYPE_UP = 2,
  FILTER_TYPE_AVERAGE = 3,
  FILTER_TYPE_PAETH = 4,
};

const byte PNG_SIGNATURE[] = {137, 80, 78, 71, 13, 10, 26, 10};

class PngImage : public Pixor::Image {
  std::shared_ptr<PngHeader> header;
  std::vector<std::shared_ptr<PngData>> data_chunks;
  std::shared_ptr<PngPalette> palette;

  int get_compressed_size() const;
  int get_pixel_width() const;
  byte *get_joined_chunks() const;

public:
  void set_header(PngHeader *header) {this->header = std::shared_ptr<PngHeader>(header);}
  void set_palette(PngPalette *palette) {this->palette = std::shared_ptr<PngPalette>(palette);}
  void add_data_chunk(PngData *chunk) {data_chunks.push_back(std::shared_ptr<PngData>(chunk));}
  void set_bitmap(byte *bitmap);
  std::shared_ptr<byte[]> get_image_bitmap() const;
  std::shared_ptr<byte[]> get_image_bitmap_with_alpha() const;
  int get_width() const {return header->get_width();};
  int get_height() const {return header->get_height();};
  bool has_alpha() const;
  PngImageType get_image_type() const {return header->get_colour_type();}
  void print_image_info() const;
  friend std::ostream &operator<<(std::ostream &os, PngImage &image);
};

PngImage *decode_png(std::istream& data_stream);

std::ostream &operator<<(std::ostream &os, PngImage &image);

}
