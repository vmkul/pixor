#include <iostream>
#include <stdio.h>
#include "png_chunk.h"
#include "debug.h"

PngChunk::PngChunk(ChunkType type, int length, const char *data) :
  type(type),
  length(length),
  data(data) {}


PngHeader::PngHeader(int length, const char *data) : PngChunk(IHDR, length, data)
{
  if (length != 13) {
    throw std::invalid_argument("Length of header chunk is not 13");
  }
}

unsigned int PngHeader::get_width() const
{
  auto uints = (unsigned int *) data;
  return __builtin_bswap32(uints[0]);
}

unsigned int PngHeader::get_height() const
{
  auto uints = (unsigned int *) data;
  return __builtin_bswap32(uints[1]);
}

unsigned char PngHeader::get_bit_depth() const {return ((unsigned char *) data)[8];}

unsigned char PngHeader::get_colour_type() const {return ((unsigned char *) data)[9];}

unsigned char PngHeader::get_compression_method() const {return ((unsigned char *) data)[10];}

unsigned char PngHeader::get_filter_method() const {return ((unsigned char *) data)[11];}

unsigned char PngHeader::get_interlace_method() const {return ((unsigned char *) data)[12];}


PngPalette::PngPalette(int length, const char *data) : PngChunk(PLTE, length, data) {}

PngData::PngData(int length, const char *data) : PngChunk(IDAT, length, data) {}

PngEnd::PngEnd(int length, const char *data) : PngChunk(IEND, length, data) {}


bool equal_signatures(const unsigned char sig1[4], const unsigned char sig2[4])
{
  for (int i = 0; i < 4; i++) {
    if (sig1[i] != sig2[i]) {
      return false;
    }
  }

  // TODO: Make signature checking an atomic operation by comparing two 4-byte integers
  return true;
}


PngChunk *create_png_chunk(const unsigned char signature[4], int length, const char *data)
{
  if (equal_signatures(signature, PNG_HEADER_CHUNK_TYPE)) {
    dbgln("Header chunk found");
    return new PngHeader(length, data);
  } else if (equal_signatures(signature, PNG_PALETTE_CHUNK_TYPE)) {
    dbgln("Palette chunk found");
    return new PngPalette(length, data);
  } else if (equal_signatures(signature, PNG_DATA_CHUNK_TYPE)) {
    dbgln("Data chunk found");
    return new PngData(length, data);
  } else if (equal_signatures(signature, PNG_END_CHUNK_TYPE)) {
    dbgln("End chunk found");
    return new PngEnd(length, data);
  } else {
    dbgln("Found unknown chunk type");
    return nullptr;
  }
}
