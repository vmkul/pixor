#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include "png_chunk.h"
#include "debug.h"
#include "pixor.h"
#include "crc.h"

using namespace Pixor;

PngChunk::PngChunk(PngChunkType type, int length, std::shared_ptr<byte[]> data) :
  type(type),
  length(length),
  data(data) {}


PngHeader::PngHeader(int length, std::shared_ptr<byte[]> data) : PngChunk(IHDR, length, data)
{}

PngHeader::PngHeader(
  int width,
  int height,
  PngImageType colour_type,
  byte bit_depth,
  byte compression_method,
  byte filter_method,
  byte interlace_method
) : PngChunk(IHDR, 13, std::shared_ptr<byte[]>(new byte[13]))
{
  ((unsigned int *) data.get())[0] = Pixor::byte_swap_32(width);
  ((unsigned int *) data.get())[1] = Pixor::byte_swap_32(height);
  data[8] = bit_depth;
  data[9] = colour_type;
  data[10] = compression_method;
  data[11] = filter_method;
  data[12] = interlace_method;
}

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

PngImageType PngHeader::get_colour_type() const {return (PngImageType) data[9];}

byte PngHeader::get_compression_method() const {return data[10];}

byte PngHeader::get_filter_method() const {return data[11];}

byte PngHeader::get_interlace_method() const {return data[12];}


PngPalette::PngPalette(int length, std::shared_ptr<byte[]> data) : PngChunk(PLTE, length, data) {}

RGBA PngPalette::get_pixel_value(int index)
{
  byte val[4];
  memcpy(val, data.get() + index * 3, 3);
  RGBA *res = (RGBA *) val;

  return *res;
}


PngData::PngData(int length, std::shared_ptr<byte[]> data) : PngChunk(IDAT, length, data) {}

PngEnd::PngEnd(int length, std::shared_ptr<byte[]> data) : PngChunk(IEND, length, data) {}


std::ostream &Pixor::operator<<(std::ostream &os, PngChunk &chunk)
{
  unsigned int swapped_len = Pixor::byte_swap_32((unsigned int) chunk.length);
  unsigned int type = (unsigned int) chunk.type;

  os.write((const char *) &swapped_len, 4);
  os.write((const char *) &type, 4);
  os.write((const char *) chunk.data.get(), chunk.length);

  auto crc_data = std::unique_ptr<byte[]>(new byte[chunk.length + 4]);
  memcpy(crc_data.get(), &chunk.type, 4);
  memcpy(crc_data.get() + 4, chunk.data.get(), chunk.length);
  unsigned int calculated_crc = Pixor::byte_swap_32(crc(crc_data.get(), chunk.length + 4));
  os.write((const char *) &calculated_crc, 4);

  return os;
}
