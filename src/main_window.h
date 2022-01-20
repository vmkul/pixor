#pragma once
#include <gtkmm.h>
#include <memory>
#include "image_area.h"

class MainWindow : public Gtk::ApplicationWindow
{
  void on_action_file_new() {}
  void on_action_file_open() {}
  void on_action_file_quit() {}

public:
  MainWindow(std::shared_ptr<Pixor::Image> &image);
  virtual ~MainWindow();
  
protected:
  ImageArea image_area;

  void on_menu_others();

  void on_menu_choices(const Glib::ustring &parameter);
  void on_menu_choices_other(int parameter);
  void on_menu_toggle();

  //Child widgets:
  Gtk::Box m_Box;

  Glib::RefPtr<Gtk::Builder> m_refBuilder;

  //Two sets of choices:
  Glib::RefPtr<Gio::SimpleAction> m_refChoice;
  Glib::RefPtr<Gio::SimpleAction> m_refChoiceOther;

  Glib::RefPtr<Gio::SimpleAction> m_refToggle;
};
