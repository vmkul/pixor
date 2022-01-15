#include "main_window.h"
#include "image_area.h"

MainWindow::MainWindow(std::shared_ptr<Pixor::Image> &image) : image_area(image)
{
  set_title("Pixor");
  set_default_size(1000, 1000);
  add(image_area);
  image_area.show();
}
