#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include "png_chunk.h"
#include "debug.h"
#include "pixor.h"

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
  return Pixor::byte_swap_32(uints[0]);
}

unsigned int PngHeader::get_height() const
{
  auto uints = (unsigned int *) data;
  return Pixor::byte_swap_32(uints[1]);
}

byte PngHeader::get_bit_depth() const {return ((byte *) data)[8];}

byte PngHeader::get_colour_type() const {return ((byte *) data)[9];}

byte PngHeader::get_compression_method() const {return ((byte *) data)[10];}

byte PngHeader::get_filter_method() const {return ((byte *) data)[11];}

byte PngHeader::get_interlace_method() const {return ((byte *) data)[12];}


PngPalette::PngPalette(int length, const char *data) : PngChunk(PLTE, length, data) {}

RGBA PngPalette::get_pixel_value(int index)
{
  byte val[4];
  memcpy(val, data + index * 3, 3);
  RGBA *res = (RGBA *) val;

  return *res;
}


PngData::PngData(int length, const char *data) : PngChunk(IDAT, length, data) {}

PngEnd::PngEnd(int length, const char *data) : PngChunk(IEND, length, data) {}


bool equal_signatures(const byte sig1[4], const byte sig2[4])
{
  for (int i = 0; i < 4; i++) {
    if (sig1[i] != sig2[i]) {
      return false;
    }
  }

  // TODO: Make signature checking an atomic operation by comparing two 4-byte integers
  return true;
}


PngChunk *create_png_chunk(const byte signature[4], int length, const char *data)
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
    dbgln("Found unknown chunk type: %s", std::string((char *) signature, 4).c_str());
    return nullptr;
  }
}
