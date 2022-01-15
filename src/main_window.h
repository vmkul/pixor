#pragma once
#include <gtkmm.h>
#include "image_area.h"

class MainWindow : public Gtk::Window
{
public:
  MainWindow(Pixor::Image *image);

protected:
  ImageArea image_area;
};
