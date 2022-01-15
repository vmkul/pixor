#pragma once
#include <gtkmm.h>
#include <memory>
#include "image_area.h"

class MainWindow : public Gtk::Window
{
public:
  MainWindow(std::shared_ptr<Pixor::Image> &image);

protected:
  ImageArea image_area;
};
