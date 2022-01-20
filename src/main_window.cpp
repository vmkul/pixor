#include "debug.h"
#include "main_window.h"
#include "image_area.h"

MainWindow::MainWindow(std::shared_ptr<Pixor::Image> &image) :
  image_area(image),
  m_Box(Gtk::ORIENTATION_VERTICAL)
{
  set_title("Pixor");
  set_default_size(1000, 1000);
  add_action("new", sigc::mem_fun(*this, &MainWindow::on_action_file_new));
  add_action("open", sigc::mem_fun(*this, &MainWindow::on_action_file_open));
  add_action("quit", sigc::mem_fun(*this, &MainWindow::on_action_file_quit));

    // ExampleApplication displays the menubar. Other stuff, such as a toolbar,
  // is put into the box.
  add(m_Box);

  // Create actions for menus and toolbars.
  // We can use add_action() because Gtk::ApplicationWindow derives from Gio::ActionMap.
  // This Action Map uses a "win." prefix for the actions.
  // Therefore, for instance, "win.copy", is used in ExampleApplication::on_startup()
  // to layout the menu.

  //Edit menu:
  add_action("copy", sigc::mem_fun(*this, &MainWindow::on_menu_others));
  add_action("paste", sigc::mem_fun(*this, &MainWindow::on_menu_others));
  add_action("something", sigc::mem_fun(*this, &MainWindow::on_menu_others));

  //Choices menus, to demonstrate Radio items,
  //using our convenience methods for string and int radio values:
  m_refChoice = add_action_radio_string("choice",
    sigc::mem_fun(*this, &MainWindow::on_menu_choices), "a");

  m_refChoiceOther = add_action_radio_integer("choiceother",
    sigc::mem_fun(*this, &MainWindow::on_menu_choices_other), 1);

  m_refToggle = add_action_bool("sometoggle",
    sigc::mem_fun(*this, &MainWindow::on_menu_toggle), false);

  //Help menu:
  add_action("about", sigc::mem_fun(*this, &MainWindow::on_menu_others));

  //Create the toolbar and add it to a container widget:

  m_refBuilder = Gtk::Builder::create();

  std::string layout_file(LAYOUT_DIR);
  layout_file.append("/toolbar.xml");

  try
  {
    m_refBuilder->add_from_file(layout_file);
  }
  catch (const Glib::Error& ex)
  {
    dbgln("Building toolbar failed: " + ex.what());
  }

  Gtk::Toolbar* toolbar = nullptr;
  m_refBuilder->get_widget("toolbar", toolbar);
  if (!toolbar)
    g_warning("GtkToolbar not found");
  else {
    m_Box.pack_start(*toolbar, Gtk::PACK_SHRINK);
  }
  
  image_area.show();
  m_Box.pack_start(image_area);
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_menu_others()
{
  dbgln("A menu item was selected.");
}

void MainWindow::on_menu_choices(const Glib::ustring& parameter)
{
  //The radio action's state does not change automatically:
  m_refChoice->change_state(parameter);

  Glib::ustring message;
  if (parameter == "a")
    message = "Choice a was selected.";
  else
    message = "Choice b was selected.";

  dbgln(message);
}

void MainWindow::on_menu_choices_other(int parameter)
{
  //The radio action's state does not change automatically:
  m_refChoiceOther->change_state(parameter);

  Glib::ustring message;
  if (parameter == 1)
    message = "Choice 1 was selected.";
  else
    message = "Choice 2 was selected.";

  dbgln(message);
}

void MainWindow::on_menu_toggle()
{
  bool active = false;
  m_refToggle->get_state(active);

  //The toggle action's state does not change automatically:
  active = !active;
  m_refToggle->change_state(active);

  Glib::ustring message;
  if (active)
    message = "Toggle is active.";
  else
    message = "Toggle is not active.";

  dbgln(message);
}
