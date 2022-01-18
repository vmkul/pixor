#include <exception>
#include <iostream>
#include <cmath>
#include <stdio.h>
#include <cstring>
#include <zlib.h>
#include <cassert>
#include "png.h"
#include "png_chunk.h"
#include "pixor.h"
#include "debug.h"
#include "crc.h"

using namespace Pixor;

PngImage *Pixor::decode_png(std::istream &data_stream)
{
  char signature[8];
  unsigned int chunk_len;
  unsigned int chunk_type;
  std::unique_ptr<byte[]> chunk_type_with_data;
  std::shared_ptr<byte[]> chunk_data;
  unsigned int chunk_crc;
  unsigned int calculated_crc;
  auto image = new PngImage();

  dbgln("Decoding PNG...");

  data_stream.read(signature, 8);
  for (int i = 0; i < 8; i++) {
    if ((byte) signature[i] != PNG_SIGNATURE[i]) {
      throw std::invalid_argument("PNG signature check failed");
    }
  }

  while (data_stream.good()) {
    data_stream.read((char *) &chunk_len, 4);
    chunk_len = Pixor::byte_swap_32(chunk_len);
    chunk_data.reset(new byte[chunk_len]);

    data_stream.read((char *) &chunk_type, 4);
    data_stream.read((char *) chunk_data.get(), chunk_len);
    data_stream.read((char *) &chunk_crc, 4);

    chunk_type_with_data.reset(new byte[chunk_len + 4]);
    memcpy(chunk_type_with_data.get(), (char *) &chunk_type, 4);
    memcpy(chunk_type_with_data.get() + 4, (char *) chunk_data.get(), chunk_len);

    calculated_crc = crc((byte *) chunk_type_with_data.get(), chunk_len + 4);
    if (calculated_crc != Pixor::byte_swap_32(chunk_crc)) {
      dbgln("CRC check failed");
      return NULL;
    }

    if (chunk_type == IHDR) {
      dbgln("Header chunk found");
      image->set_header(new PngHeader(chunk_len, chunk_data));
    } else if (chunk_type == PLTE) {
      dbgln("Palette chunk found");
      image->set_palette(new PngPalette(chunk_len, chunk_data));
    } else if (chunk_type == IDAT) {
      dbgln("Data chunk found");
      image->add_data_chunk(new PngData(chunk_len, chunk_data));
    } else if (chunk_type == IEND) {
      dbgln("End chunk found");
      break;
    } else {
      dbgln("Found unknown chunk type: %s", std::string((char *) &chunk_type, 4).c_str());
    }
  }

  return image;
}

byte paeth_predictor(int a, int b, int c)
{
  int pr;
  int p = a + b - c;
  int pa = std::abs(p - a);
  int pb = std::abs(p - b);
  int pc = std::abs(p - c);

  if (pa <= pb && pa <= pc) {
    pr = a;
  } else if (pb <= pc) {
    pr = b;
  } else {
    pr = c;
  } 

  return pr;
}

byte none_recon_filter(byte x, byte a, byte b, byte c)
{
  UNUSED(a);
  UNUSED(b);
  UNUSED(c);
  return x;
}

byte sub_recon_filter(byte x, byte a, byte b, byte c)
{
  UNUSED(b);
  UNUSED(c);
  return x + a;
}

byte up_recon_filter(byte x, byte a, byte b, byte c)
{
  UNUSED(a);
  UNUSED(c);
  return x + b;
}

byte avg_recon_filter(byte x, byte a, byte b, byte c)
{
  UNUSED(c);
  return x + std::floor((a + b) / 2);
}

byte paeth_recon_filter(byte x, byte a, byte b, byte c)
{
  return x + paeth_predictor(a, b, c);
}

typedef byte (*png_filter_func)(byte, byte, byte, byte);

png_filter_func get_recon_filter(FilterType filter_type)
{
  png_filter_func res;

  switch(filter_type) {
    case FILTER_TYPE_NONE:
      res = &none_recon_filter;
      break;
    case FILTER_TYPE_SUB:
      res = &sub_recon_filter;
      break;
    case FILTER_TYPE_UP:
      res = &up_recon_filter;
      break;
    case FILTER_TYPE_AVERAGE:
      res = &avg_recon_filter;
      break;
    case FILTER_TYPE_PAETH:
      res = &paeth_recon_filter;
      break;
    default:
      dbgln("Unknown filter type %d", filter_type);
      res = &none_recon_filter;
  }

  return res;
}

class ByteMatrix {
  byte *data;
  int width;
  int height;
  int row_start_offset;

  bool coord_in_bounds(int i, int j) const
  {
    return !(i < 0 || j < 0 || i > height - 1 || j > width - 1 - row_start_offset);
  }

public:
  ByteMatrix(byte *data, int width, int height, int row_start_offset = 0) :
    data(data),
    width(width),
    height(height),
    row_start_offset(row_start_offset)
    {}

  byte try_get_byte(int i, int j) const
  {
    if (!coord_in_bounds(i, j)) {
      return 0;
    }

    return data[i * width + j + row_start_offset];
  }

  void try_set_byte(int i, int j, byte value)
  {
    if (!coord_in_bounds(i, j)) {
      return;
    }

    data[i * width + j + row_start_offset] = value;
  }
};

byte reconstruct_filtered_byte(ByteMatrix &image_matrix, png_filter_func filter_func, int pixel_width, int i, int j)
{
  byte x = image_matrix.try_get_byte(i, j);
  byte a = image_matrix.try_get_byte(i, j - pixel_width);
  byte b = image_matrix.try_get_byte(i - 1, j);
  byte c = image_matrix.try_get_byte(i - 1, j - pixel_width);

  return filter_func(x, a, b, c);
}

bool PngImage::has_alpha() const
{
  PngImageType type = get_image_type();
  return (type == PNG_TYPE_GREYSCALE_ALPHA || type == PNG_TYPE_TRUECOLOUR_ALPHA);
}

void PngImage::print_image_info() const
{
  if (!header) {
    return;
  }

  printf("Image info:\n");
  printf("  Image width: %d\n", header->get_width());
  printf("  Image height: %d\n", header->get_height());
  printf("  Bit depth: %d\n", header->get_bit_depth());
  printf("  Colour type: %d\n", header->get_colour_type());
  printf("  Compression method: %d\n", header->get_compression_method());
  printf("  Filter method: %d\n", header->get_filter_method());
  printf("  Interlace method: %d\n", header->get_interlace_method());
}

int PngImage::get_compressed_size() const
{
  int res = 0;

  for (auto chunk : data_chunks) {
    res += chunk->get_length();
  }

  return res;
}

int PngImage::get_pixel_width() const
{
  switch (get_image_type()) {
    case PNG_TYPE_GREYSCALE:
      return 1;
    case PNG_TYPE_TRUECOLOUR:
      return 3;
    case PNG_TYPE_INDEXED_COLOUR:
      return 1;
    case PNG_TYPE_GREYSCALE_ALPHA:
      return 2;
    case PNG_TYPE_TRUECOLOUR_ALPHA:
      return 4;
    default:
      return -1;
  }
}

byte *PngImage::get_joined_chunks() const
{
  int chunk_start = 0;
  byte *joined_chunks = new byte[get_compressed_size()];

  for (auto chunk : data_chunks) {
    int chunk_len = chunk->get_length();
    memcpy(joined_chunks + chunk_start, chunk->get_data(), chunk_len);
    chunk_start += chunk_len;
  }

  return joined_chunks;
}

void PngImage::set_bitmap(byte *bitmap)
{
  if (!header) return;
  data_chunks.clear();

  int pixel_width = get_pixel_width();
  int width = get_width() * pixel_width + 1;
  int height = get_height();
  unsigned long initial_size = width * height * pixel_width;
  unsigned long compressed_size = initial_size * 1.2 + 12;
  std::unique_ptr<byte[]> data_to_compress(new byte[initial_size]);
  std::shared_ptr<byte[]> compressed_data(new byte[compressed_size]);

  for (int i = 0; i < height; i++) {
    data_to_compress[i * width] = FILTER_TYPE_NONE;
    // TODO: Add filtering to improve compression

    for (int j = 1; j < width; j++) {
      data_to_compress[i * width + j] = bitmap[i * (width - 1) + j - 1];
    }
  }

  int result = compress(compressed_data.get(), &compressed_size, data_to_compress.get(), initial_size);
  if (result != 0) {
    dbgln("Compression error code: %d", result);
    return;
  }

  data_chunks.push_back(std::make_shared<PngData>(compressed_size, compressed_data));
}

std::shared_ptr<byte[]> PngImage::get_image_bitmap() const
{
  if (data_chunks.size() == 0) {
    return NULL;
  }

  PngImageType image_type = get_image_type();
  int compressed_size = get_compressed_size();
  int pixel_width = get_pixel_width();
  int width = get_width();
  int height = get_height();
  long unsigned dest_length = compressed_size * 1000;
  auto joined_chunks = std::unique_ptr<byte[]>(get_joined_chunks());
  auto uncompressed_data = std::unique_ptr<byte[]>(new byte[dest_length]);

  int res = uncompress(uncompressed_data.get(), &dest_length, joined_chunks.get(), compressed_size);
  dbgln("Decompressed size: %ld\n", dest_length);
  byte *decoded = new byte[width * height * (has_alpha() ? 4 : 3)];
  ByteMatrix image_matrix(uncompressed_data.get(), width * pixel_width + 1, height, 1);

  if (res != 0) {
    dbgln("Uncompress error! %d\n", res);
    return NULL;
  }

  for (int i = 0; i < height; i++) {
    FilterType filter_type = (FilterType) uncompressed_data[i * (width * pixel_width + 1)];
    auto filter_func = get_recon_filter(filter_type);

    for (int j = 0; j < width * pixel_width; j++) {
      byte value = reconstruct_filtered_byte(image_matrix, filter_func, pixel_width, i, j);
      image_matrix.try_set_byte(i, j, value);

      if (image_type == PNG_TYPE_GREYSCALE) {
        int dest_index = (i * width + j) * 3;
        decoded[dest_index] = value;
        decoded[dest_index + 1] = value;
        decoded[dest_index + 2] = value;
      } else if (image_type == PNG_TYPE_TRUECOLOUR) {
        int dest_index = i * width * 3 + j;
        decoded[dest_index] = value;
      } else if (image_type == PNG_TYPE_TRUECOLOUR_ALPHA) {
        int dest_index = i * width * 4 + j;
        decoded[dest_index] = value;
      } else if (image_type == PNG_TYPE_GREYSCALE_ALPHA) {
        if (j % 2 == 0) {
          int dest_index = (i * width + j / 2) * 4;
          decoded[dest_index] = value;
          decoded[dest_index + 1] = value;
          decoded[dest_index + 2] = value;
        } else {
          int dest_index = (i * width + (j - 1) / 2) * 4;
          decoded[dest_index + 3] = value;
        }
      } else if (image_type == PNG_TYPE_INDEXED_COLOUR) {
        RGBA pixel = palette->get_pixel_value(value);
        byte *channels = (byte *) &pixel;
        int dest_index = (i * width + j) * 3;

        decoded[dest_index] = channels[0];
        decoded[dest_index + 1] = channels[1];
        decoded[dest_index + 2] = channels[2];
      } else {
        return NULL;
      }
    }
  }

  return std::shared_ptr<byte[]>(decoded);
}

std::ostream &Pixor::operator<<(std::ostream &os, PngImage &image)
{
  for (int i = 0; i < 8; i++) {
    os << PNG_SIGNATURE[i];
  }

  os << *image.header.get();
  for (const auto &chunk : image.data_chunks) {
    os << *chunk.get();
  }

  PngEnd end_chunk(0, nullptr);
  os << end_chunk;

  return os;
}
