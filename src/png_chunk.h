#pragma once
#include <memory>
#include "pixor.h"

enum PngChunkType {
  IHDR = 0x52444849,
  PLTE = 0x45544C50,
  IDAT = 0x54414449,
  IEND = 0x444E4549,
};

enum PngImageType {
  PNG_TYPE_GREYSCALE = 0,
  PNG_TYPE_TRUECOLOUR = 2,
  PNG_TYPE_INDEXED_COLOUR =	3,
  PNG_TYPE_GREYSCALE_ALPHA = 4,
  PNG_TYPE_TRUECOLOUR_ALPHA = 6,
};

class PngChunk {
protected:
  PngChunkType type;
  int length;
  std::shared_ptr<byte[]> data;

public:
  PngChunk(ChunkType type, int length, std::shared_ptr<byte[]> &data);
  int get_length() const {return length;};
  ChunkType get_type() const {return type;}
};

class PngHeader : public PngChunk {
public:
  PngHeader(int length, std::shared_ptr<byte[]> &data);

  unsigned int get_width() const;
  unsigned int get_height() const;
  byte get_bit_depth() const;
  PngImageType get_colour_type() const;
  byte get_compression_method() const;
  byte get_filter_method() const;
  byte get_interlace_method() const;
};

class PngPalette : public PngChunk {
public:
  PngPalette(int length, std::shared_ptr<byte[]> &data);
  RGBA get_pixel_value(int index);
};

class PngData : public PngChunk {
public:
  PngData(int length, std::shared_ptr<byte[]> &data);
  byte *get_data() const {return data.get();}
};

class PngEnd : public PngChunk {
public:
  PngEnd(int length, std::shared_ptr<byte[]> &data);
};
