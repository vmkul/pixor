#include <iostream>
#include <stdio.h>
#include "png.h"
#include "png_chunk.h"

PngImage *decode_png(std::istream& data_stream)
{
  char signature[8];
  unsigned int chunk_len[1];
  char chunk_type[4];
  char *chunk_data;
  char crc[4];

  printf("Decoding PNG...\n");

  data_stream.read(signature, 8);

  for (int i = 0; i < 8; i++) {
    if ((unsigned char) signature[i] != PNG_SIGNATURE[i]) {
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

    auto chunk = create_png_chunk((unsigned char *) chunk_type, *chunk_len, chunk_data);

    if (chunk && chunk->get_type() == IHDR) {
      auto header = (PngHeader *) chunk;

      printf("Image info:\n");
      printf("  Image width: %d\n", header->get_width());
      printf("  Image height: %d\n", header->get_height());
      printf("  Bit depth: %d\n", header->get_bit_depth());
      printf("  Colour type: %d\n", header->get_colour_type());
      printf("  Compression method: %d\n", header->get_compression_method());
      printf("  Filter method: %d\n", header->get_filter_method());
      printf("  Interlace method: %d\n", header->get_interlace_method());
    }
  }

  return new PngImage();
}
