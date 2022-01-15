#include <gtkmm.h>
#include <stdio.h>
#include <fstream>
#include "main_window.h"
#include "png.h"
#include "image_area.h"

int main(int argc, char* argv[])
{
  if (argc == 1) {
    printf("Usage: pixor <file>\n");
    return 1;
  }

  std::ifstream file (argv[1], std::ios::in|std::ios::binary);

  if (!file.is_open()) {
    printf("cannot open %s (No such file or directory)\n", argv[1]);
    return 1;
  }

  auto image = Pixor::decode_png(file);
  image->print_image_info();

  file.close();

  auto app = Gtk::Application::create("org.gtkmm.examples.base");
  MainWindow window(image);

  return app->run(window);
}
