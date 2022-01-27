#include <cairomm/context.h>
#include <giomm/resource.h>
#include <gdkmm/general.h> // set_source_pixbuf()
#include <glibmm/fileutils.h>
#include <iostream>
#include <stdio.h>
#include "image_area.h"
#include "debug.h"

ImageArea::ImageArea(std::shared_ptr<Pixor::Image> &image) :
  drawing_context(image->get_image_bitmap(), image->get_width(), image->get_height())
{
  set_events(Gdk::BUTTON_MOTION_MASK|Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK);
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &ImageArea::on_mouse_motion));
  if (!image) return;

  this->image = image;
  bool has_alpha = image->has_alpha();
  int pixel_width = has_alpha ? 4 : 3;
  image_bitmap = drawing_context.get_target_bitmap();
  saved_bitmap = image_bitmap.get();

  try
  {
    m_image = Gdk::Pixbuf::create_from_data(image_bitmap.get(),
					    Gdk::Colorspace::COLORSPACE_RGB,
					    has_alpha,
					    8,
					    image->get_width(),
					    image->get_height(),
					    image->get_width() * pixel_width);
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
  auto color = Pixor::rgba(0, 255, 0, 255);
  auto pattern = Pixor::Pattern::make_circle(5, &color);

  drawing_context.set_source_pattern(pattern);
  drawing_context.set_source_rgba(color);

  if (button1_pressed) {
    if (mouse_pointer_trace.size() >= 2) {
      auto last_point = mouse_pointer_trace.back();
      auto second_to_last_point = *(mouse_pointer_trace.rbegin() + 1);

      drawing_context.draw_line_with_pattern(second_to_last_point, last_point);
    } else if (mouse_pointer_trace.size() == 1) {
      auto cur_point = mouse_pointer_trace[0];

      pattern->draw_onto(drawing_context, {cur_point.x, cur_point.y});
    }
  }

  auto pixbuf = Gdk::Pixbuf::create_from_data(saved_bitmap,
					      Gdk::Colorspace::COLORSPACE_RGB,
					      true,
					      8,
					      image->get_width(),
					      image->get_height(),
					      image->get_width() * 4);
  Gdk::Cairo::set_source_pixbuf(cr, pixbuf, 0, 0);
  cr->paint();

  return true;
}

bool ImageArea::on_mouse_motion(GdkEventMotion *motion_event)
{
  if (!button1_pressed) return true;

  mouse_pointer_trace.push_back({(int) motion_event->x, (int) motion_event->y});
  queue_draw();

  return true;
}

bool ImageArea::on_button_press_event(GdkEventButton *button_event)
{
  if (button_event->button != 1) return true;

  int pointer_x;
  int pointer_y;
  get_pointer(pointer_x, pointer_y);
  button1_pressed = true;
  mouse_pointer_trace.push_back({pointer_x, pointer_y});
  queue_draw();

  return true;
}

bool ImageArea::on_button_release_event(GdkEventButton *button_event)
{
  if (button_event->button != 1) return true;

  button1_pressed = false;
  mouse_pointer_trace.clear();

  return true;
}
