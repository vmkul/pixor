#include <cairomm/context.h>
#include <giomm/resource.h>
#include <gdkmm/general.h> // set_source_pixbuf()
#include <glibmm/fileutils.h>
#include <iostream>
#include <stdio.h>
#include "image_area.h"
#include "debug.h"

ImageArea::ImageArea(std::shared_ptr<Pixor::Image> &image)
{
  if (!image) {
    return;
  }

  bool has_alpha = image->has_alpha();
  int pixel_width = has_alpha ? 4 : 3;
  image_bitmap = image->get_image_bitmap();

  try
  {
    m_image = Gdk::Pixbuf::create_from_data(image_bitmap.get(), Gdk::Colorspace::COLORSPACE_RGB,
      has_alpha, 8, image->get_width(), image->get_height(), (image->get_width()) * pixel_width);
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
    set_size_request(m_image->get_width(), m_image->get_height());
  }
}

ImageArea::~ImageArea()
{
}

bool ImageArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  if (!m_image)
    return false;

  Gdk::Cairo::set_source_pixbuf(cr, m_image,
    (get_width() - m_image->get_width()) / 2, (get_height() - m_image->get_height()) / 2);
  cr->paint();

  return true;
}
