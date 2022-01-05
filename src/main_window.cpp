#include "main_window.h"
#include "image_area.h"

MainWindow::MainWindow(PngImage *image) : image_area(image)
{
  set_title("My application!");
  set_default_size(1000, 1000);
  set_child(image_area);
}
