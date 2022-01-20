#pragma once
#include <gtkmm.h>
#include "image.h"

class Application : public Gtk::Application
{
public:
  static Glib::RefPtr<Application> create();

protected:
  //Overrides of default signal handlers:
  Application();
  void on_startup() override;
  void on_activate() override;

private:
  void create_window();

  void on_window_hide(Gtk::Window* window);
  void on_menu_file_new_generic();
  void on_menu_file_quit();
  void on_menu_help_about();
  void on_file_open(const type_vec_files &files, const Glib::ustring &name);

  Glib::RefPtr<Gtk::Builder> m_refBuilder;
  std::shared_ptr<Pixor::Image> current_image;
};
