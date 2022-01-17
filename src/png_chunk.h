#pragma once
#include <memory>
#include "pixor.h"

const unsigned int PNG_HEADER_CHUNK_TYPE = 0x52444849;
const unsigned int PNG_PALETTE_CHUNK_TYPE = 0x45544C50;
const unsigned int PNG_DATA_CHUNK_TYPE = 0x54414449;
const unsigned int PNG_END_CHUNK_TYPE = 0x444E4549;

enum ChunkType {
  IHDR,
  PLTE,
  IDAT,
  IEND
};

class PngChunk {
protected:
  ChunkType type;
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
  byte get_colour_type() const;
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
