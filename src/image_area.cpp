#include <cairomm/context.h>
#include <giomm/resource.h>
#include <gdkmm/general.h> // set_source_pixbuf()
#include <glibmm/fileutils.h>
#include <iostream>
#include <stdio.h>
#include "image_area.h"
#include "debug.h"

ImageArea::ImageArea(Pixor::Image *image)
{
  if (!image) {
    return;
  }

  bool has_alpha = image->has_alpha();
  int pixel_width = has_alpha ? 4 : 3;

  try
  {
    m_image = Gdk::Pixbuf::create_from_data(image->get_image_bitmap(), Gdk::Colorspace::RGB, has_alpha, 8, image->get_width(), image->get_height(), (image->get_width()) * pixel_width);
  }
  catch(const Gio::ResourceError& ex)
  {
    dbgln("ResourceError: %s", ex.what());
  }
  catch(const Gdk::PixbufError& ex)
  {
    dbgln("PixbufError: %s", ex.what());
  }

  if (m_image)
  {
    set_content_width(m_image->get_width());
    set_content_height(m_image->get_height());
  }

  set_draw_func(sigc::mem_fun(*this, &ImageArea::on_draw));
}

ImageArea::~ImageArea()
{
}

void ImageArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height)
{
  if (!m_image)
    return;

  Gdk::Cairo::set_source_pixbuf(cr, m_image,
    (width - m_image->get_width())/2, (height - m_image->get_height())/2);
  cr->paint();
}
