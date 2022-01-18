#pragma once
#include <memory>
#include "pixor.h"

namespace Pixor {

class Image {
public:
  virtual std::shared_ptr<byte[]> get_image_bitmap() const = 0;
  virtual int get_width() const = 0;
  virtual int get_height() const = 0;
  virtual bool has_alpha() const = 0;
  virtual void print_image_info() const = 0;
  virtual void set_bitmap(byte *bitmap) = 0;
};

}
