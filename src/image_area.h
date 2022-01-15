#pragma once
#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include <memory>
#include "image.h"

class ImageArea : public Gtk::DrawingArea
{
  std::shared_ptr<byte[]> image_bitmap;

public:
  ImageArea(std::shared_ptr<Pixor::Image> &image);
  virtual ~ImageArea();

protected:
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

  Glib::RefPtr<Gdk::Pixbuf> m_image;
};
