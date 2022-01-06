#include <exception>
#include <iostream>
#include <cmath>
#include <stdio.h>
#include <cstring>
#include <zlib.h>
#include "png.h"
#include "png_chunk.h"
#include "pixor.h"
#include "debug.h"

PngImage *decode_png(std::istream& data_stream)
{
  char signature[8];
  unsigned int chunk_len[1];
  char chunk_type[4];
  char *chunk_data;
  char crc[4];
  auto image = new PngImage();

  dbgln("Decoding PNG...");

  data_stream.read(signature, 8);

  for (int i = 0; i < 8; i++) {
    if ((byte) signature[i] != PNG_SIGNATURE[i]) {
      throw std::invalid_argument("PNG signature check failed");
    }
  }

  while (data_stream.good()) {
    data_stream.read((char *) chunk_len, 4);
    *chunk_len = __builtin_bswap32(*chunk_len);
    chunk_data = new char[*chunk_len];

    data_stream.read(chunk_type, 4);
    data_stream.read(chunk_data, *chunk_len);
    data_stream.read(crc, 4);

    auto chunk = create_png_chunk((byte *) chunk_type, *chunk_len, chunk_data);

    if (chunk && chunk->get_type() == IHDR) {
      auto header = (PngHeader *) chunk;
      image->set_header(header);
    }

    if (chunk && chunk->get_type() == IDAT) {
      image->add_data_chunk((PngData *) chunk);
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
      dbgln("Unknown filter type %d\n", filter_type);
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

byte *PngImage::get_image_data() const
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
  byte *joined_chunks = get_joined_chunks();
  byte *uncompressed_data = new byte[dest_length];

  int res = uncompress(uncompressed_data, &dest_length, joined_chunks, compressed_size);
  dbgln("Decompressed size: %ld\n", dest_length);
  byte *decoded = new byte[width * height * (has_alpha() ? 4 : 3)];
  ByteMatrix image_matrix(uncompressed_data, width * pixel_width + 1, height, 1);

  if (res != 0) {
    dbgln("Uncompress error! %d\n", res);
    // TODO: Free resources in case of fail
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
        // TODO: Implement greyscale alpha
        return NULL;
      } else if (image_type == PNG_TYPE_INDEXED_COLOUR) {
        // TODO: Implement indexed colour
        return NULL;
      } else {
        return NULL;
      }
    }
  }

  return decoded;
}
