#include <gtkmm.h>
#include <stdio.h>
#include <fstream>
#include "main_window.h"
#include "png.h"

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

  decode_png(file);
  file.close();

  return 0;

  auto app = Gtk::Application::create("org.gtkmm.examples.base");

  return app->make_window_and_run<MyWindow>(argc, argv);
}
