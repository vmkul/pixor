#include <fstream>
#include "png.h"
#include "debug.h"
#include "application.h"
#include "main_window.h"

Application::Application()
: Gtk::Application("org.gtkmm.example.main_menu")
{
  Glib::set_application_name("Main Menu Example");
  set_flags(Gio::APPLICATION_HANDLES_OPEN);
}

Glib::RefPtr<Application> Application::create()
{
  return Glib::RefPtr<Application>(new Application());
}

void Application::on_startup()
{
  //Call the base class's implementation:
  Gtk::Application::on_startup();

  //Create actions for menus and toolbars.
  //We can use add_action() because Gtk::Application derives from Gio::ActionMap.

  signal_open().connect(sigc::mem_fun(*this, &Application::on_file_open));

  //File|New sub menu:
  add_action("newstandard",
    sigc::mem_fun(*this, &Application::on_menu_file_new_generic));

  add_action("newfoo",
    sigc::mem_fun(*this, &Application::on_menu_file_new_generic));

  add_action("newgoo",
    sigc::mem_fun(*this, &Application::on_menu_file_new_generic));

  //File menu:
  add_action("quit", sigc::mem_fun(*this, &Application::on_menu_file_quit));

  //Help menu:
  add_action("about", sigc::mem_fun(*this, &Application::on_menu_help_about));

  m_refBuilder = Gtk::Builder::create();
  
  std::string layout_file(LAYOUT_DIR);
  layout_file.append("/menubar.xml");
  
  try
  {
    m_refBuilder->add_from_file(layout_file);
  }
  catch (const Glib::Error& ex)
  {
    dbgln("Building menus failed: " + ex.what());
  }

  //Get the menubar and the app menu, and add them to the application:
  auto object = m_refBuilder->get_object("menu-example");
  auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  object = m_refBuilder->get_object("appmenu");
  auto appMenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  if (!(gmenu && appMenu)) {
    g_warning("GMenu or AppMenu not found");
  }
  else
  {
    set_app_menu(appMenu);
    set_menubar(gmenu);
  }
}

void Application::on_activate()
{
  //std::cout << "debug1: " << G_STRFUNC << std::endl;
  // The application has been started, so let's show a window.
  // A real application might want to reuse this window in on_open(),
  // when asked to open a file, if no changes have been made yet.
  create_window();
}

void Application::create_window()
{
  if (!current_image) {
    dbgln("No current image available");
    return;
  }
  
  auto win = new MainWindow(current_image);
  dbgln("Created main window");
  //Make sure that the application runs for as long this window is still open:
  add_window(*win);

  //Delete the window when it is hidden.
  //That's enough for this simple example.
  win->signal_hide().connect(sigc::bind<Gtk::Window*>(
    sigc::mem_fun(*this, &Application::on_window_hide), win));

  win->show_all();
}

void Application::on_window_hide(Gtk::Window* window)
{
  delete window;
}

void Application::on_menu_file_new_generic()
{
  dbgln("A File|New menu item was selected.");
}

void Application::on_menu_file_quit()
{
  dbgln(G_STRFUNC);
  quit(); // Not really necessary, when Gtk::Widget::hide() is called.

  // Gio::Application::quit() will make Gio::Application::run() return,
  // but it's a crude way of ending the program. The window is not removed
  // from the application. Neither the window's nor the application's
  // destructors will be called, because there will be remaining reference
  // counts in both of them. If we want the destructors to be called, we
  // must remove the window from the application. One way of doing this
  // is to hide the window.
  std::vector<Gtk::Window*> windows = get_windows();
  if (windows.size() > 0)
    windows[0]->hide(); // In this simple case, we know there is only one window.
}

void Application::on_menu_help_about()
{
  dbgln("App|Help|About was selected.");
}

void Application::on_file_open(const type_vec_files &files, const Glib::ustring &name) {
  dbgln("Called on_file_open!");
  dbgln("Name: %s", name.c_str());
  for (const auto &file : files) {
    dbgln("File path: %s", file.get()->get_path().c_str());
  }

  std::ifstream file(files[0].get()->get_path(), std::ios::in|std::ios::binary);

  if (!file.is_open()) {
    printf("cannot open %s (No such file or directory)\n", "some file");
    return;
  }

  auto image_ptr = Pixor::decode_png(file);
  auto image = std::shared_ptr<Pixor::Image>((Pixor::Image *) image_ptr);
  image->print_image_info();
  current_image = image;
  on_activate();
}
