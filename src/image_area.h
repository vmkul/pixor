#pragma once
#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include "image.h"

class ImageArea : public Gtk::DrawingArea
{
public:
  ImageArea(Pixor::Image *image);
  virtual ~ImageArea();

protected:
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

  Glib::RefPtr<Gdk::Pixbuf> m_image;
};
