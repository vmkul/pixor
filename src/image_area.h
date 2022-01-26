#pragma once
#include <deque>
#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/dragcontext.h>
#include <memory>
#include "image.h"
#include "context.h"

class ImageArea : public Gtk::DrawingArea
{
  bool on_mouse_motion(GdkEventMotion *motion_event);
  bool on_button_press_event(GdkEventButton *button_event) override;
  bool on_button_release_event(GdkEventButton *button_event) override;

  std::shared_ptr<byte[]> image_bitmap;
  Pixor::Context drawing_context;
  byte *saved_bitmap;
  std::deque<Pixor::point> mouse_pointer_trace; // deque has clear method vs. stack
  bool button1_pressed = false;

public:
  ImageArea(std::shared_ptr<Pixor::Image> &image);
  virtual ~ImageArea();

protected:
  bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

  Glib::RefPtr<Gdk::Pixbuf> m_image;
  std::shared_ptr<Pixor::Image> image;
};
