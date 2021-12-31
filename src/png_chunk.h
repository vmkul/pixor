#pragma once

const unsigned char PNG_HEADER_CHUNK_TYPE[] = {73, 72, 68, 82};
const unsigned char PNG_PALETTE_CHUNK_TYPE[] = {80, 76, 84, 69};
const unsigned char PNG_DATA_CHUNK_TYPE[] = {73, 68, 65, 84};
const unsigned char PNG_END_CHUNK_TYPE[] = {73, 69, 78, 68};

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

  ChunkType get_type() {return type;}
};

class PngHeader : public PngChunk {
public:
  PngHeader(int length, const char *data);

  unsigned int get_width();
  unsigned int get_height();
  unsigned char get_bit_depth();
  unsigned char get_colour_type();
  unsigned char get_compression_method();
  unsigned char get_filter_method();
  unsigned char get_interlace_method();
};

class PngPalette : public PngChunk {
public:
  PngPalette(int length, const char *data);
};

class PngData : public PngChunk {
public:
  PngData(int length, const char *data);
};

class PngEnd : public PngChunk {
public:
  PngEnd(int length, const char *data);
};

PngChunk *create_png_chunk(const unsigned char signature[4], int length, const char *data);
