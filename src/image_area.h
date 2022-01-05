#pragma once
#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include "png.h"

class ImageArea : public Gtk::DrawingArea
{
public:
  ImageArea(PngImage *image);
  virtual ~ImageArea();

protected:
  void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);

  Glib::RefPtr<Gdk::Pixbuf> m_image;
};
