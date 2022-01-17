#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include "png_chunk.h"
#include "debug.h"
#include "pixor.h"

PngChunk::PngChunk(ChunkType type, int length, std::shared_ptr<byte[]> &data) :
  type(type),
  length(length),
  data(data) {}


PngHeader::PngHeader(int length, std::shared_ptr<byte[]> &data) : PngChunk(IHDR, length, data)
{}

unsigned int PngHeader::get_width() const
{
  auto uints = (unsigned int *) data.get();
  return Pixor::byte_swap_32(uints[0]);
}

unsigned int PngHeader::get_height() const
{
  auto uints = (unsigned int *) data.get();
  return Pixor::byte_swap_32(uints[1]);
}

byte PngHeader::get_bit_depth() const {return data[8];}

byte PngHeader::get_colour_type() const {return data[9];}

byte PngHeader::get_compression_method() const {return data[10];}

byte PngHeader::get_filter_method() const {return data[11];}

byte PngHeader::get_interlace_method() const {return data[12];}


PngPalette::PngPalette(int length, std::shared_ptr<byte[]> &data) : PngChunk(PLTE, length, data) {}

RGBA PngPalette::get_pixel_value(int index)
{
  byte val[4];
  memcpy(val, data.get() + index * 3, 3);
  RGBA *res = (RGBA *) val;

  return *res;
}


PngData::PngData(int length, std::shared_ptr<byte[]> &data) : PngChunk(IDAT, length, data) {}

PngEnd::PngEnd(int length, std::shared_ptr<byte[]> &data) : PngChunk(IEND, length, data) {}


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
