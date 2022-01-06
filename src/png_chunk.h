#pragma once
#include "pixor.h"

const byte PNG_HEADER_CHUNK_TYPE[] = {73, 72, 68, 82};
const byte PNG_PALETTE_CHUNK_TYPE[] = {80, 76, 84, 69};
const byte PNG_DATA_CHUNK_TYPE[] = {73, 68, 65, 84};
const byte PNG_END_CHUNK_TYPE[] = {73, 69, 78, 68};

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
  const char *data;

public:
  PngChunk(ChunkType type, int length, const char *data);
  int get_length() const {return length;};
  ChunkType get_type() const {return type;}
};

class PngHeader : public PngChunk {
public:
  PngHeader(int length, const char *data);

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
  PngPalette(int length, const char *data);
  RGBA get_pixel_value(int index);
};

class PngData : public PngChunk {
public:
  PngData(int length, const char *data);
  const char *get_data() const {return data;}
};

class PngEnd : public PngChunk {
public:
  PngEnd(int length, const char *data);
};

PngChunk *create_png_chunk(const byte signature[4], int length, const char *data);
