#include <studio.hpp>
#include <connection.hpp>
#include <evb.hpp>
#include "boost/filesystem.hpp"

namespace studio {

ModuleWindow::ModuleWindow(EvbWindow& w, const std::string& name)
: owner_(w)
, name_(name)
, lines_(0)
{
  set_default_size(300, 200); 

  table_.set_column_homogeneous (true);
  //table_.set_homo

  std::string title;
  title = "Module '";
  title += name_;
  title += "'";
  set_title(title);

  name_entry_.set_editable(false);
  name_entry_.set_sensitive(false);
  name_entry_.set_text(name_);

  name_label_.set_text("Name");
  name_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  rename_button_.set_label(name_.empty() ? "Create" : "Rename");
  compile_.set_label("Compile");
  write_.set_label("Write");
  start_.set_label("Start");
  run_.set_label("Run");

  button_box_.pack_start(compile_, Gtk::PACK_SHRINK);
  button_box_.set_border_width(5);
  button_box_.set_layout(Gtk::BUTTONBOX_END);
  compile_.signal_clicked().connect(sigc::mem_fun(*this, &ModuleWindow::on_compile) );
    
  button_box_.pack_start(write_, Gtk::PACK_SHRINK);
  write_.signal_clicked().connect(sigc::mem_fun(*this, &ModuleWindow::on_write) );
    
  button_box_.pack_start(start_, Gtk::PACK_SHRINK);
  start_.signal_clicked().connect(sigc::mem_fun(*this, &ModuleWindow::on_start) );
    
  button_box_.pack_start(run_, Gtk::PACK_SHRINK);
  run_.signal_clicked().connect(sigc::mem_fun(*this, &ModuleWindow::on_run) );

  rename_button_.signal_clicked().connect(sigc::mem_fun(*this, &ModuleWindow::on_rename));

  name_box_.pack_start(name_label_, Gtk::PACK_SHRINK);
  name_box_.pack_start(name_entry_, Gtk::PACK_EXPAND_WIDGET);
  name_box_.pack_start(rename_button_, Gtk::PACK_SHRINK);
  name_box_.pack_start(new_name_entry_, Gtk::PACK_EXPAND_WIDGET);

  size_t pos = 0;
  table_.attach(name_box_,              0, pos, 7, 1);
  //table_.attach(name_label_,            0, pos, 1, 1);
  //table_.attach(name_entry_,            1, pos, 4, 1);
  //table_.attach(rename_button_,         5, pos, 1, 1);
  //table_.attach(new_name_entry_,        6, pos, 1, 1);
  ++pos;

  table_.attach(button_box_,            0, pos, 7, 1);
  ++pos;

  setup();
  add(table_);
  show_all_children();
}

ModuleWindow::~ModuleWindow()
{
  clear();
}


void ModuleWindow::rename(const std::string& new_name)
{
  evb::Module_ptr mod;
  if (name_.empty()) { // create
    mod = owner_.view().model()->find_module(name_);
    if (mod.is_not_null()) {
      // already exist
      Gtk::MessageDialog dialog(*this, "Rename module '"+name_+"' failed");
      dialog.set_secondary_text("New name '"+new_name+"' already exist");
      dialog.run();
      return;
    }
    mod = owner_.view().model()->add_module(new_name);
    owner_.accept_new_module(*this);
    return;
  }
  // rename
  mod = owner_.view().model()->find_module(name_);
  if (mod.is_null()) {
    Gtk::MessageDialog dialog(*this, "Rename module '"+name_+"' failed");
    dialog.set_secondary_text("Module not found");
    dialog.run();
    return;
  }
  owner_.view().model()->rename_module(name_, new_name);
  owner_.on_update_modules();
  name_ = new_name;
  std::string title;
  title = "Module '";
  title += name_;
  title += "'";
  set_title(title);
  name_entry_.set_text(name_);
}

void ModuleWindow::clear()
{
  while(lines_ > 0) {
    table_.remove_row(lines_);
    --lines_;
  }
  for (std::map<size_t, Gtk::Button*>::iterator it = node_map_.begin(); it != node_map_.end(); ++it) {
    delete it->second;
  }
  node_map_.clear();
}

void ModuleWindow::setup()
{
  clear();
  //name_entry_.set_text(name_);

  evb::Module_ptr mod = owner_.view().model()->find_module(name_);
  if (mod.is_not_null()) {
    size_t limit = mod->size();
    if (limit > 0) {
      for (size_t i=0; i<limit; ++i) {
        size_t node = mod->node(i);
        Gtk::Button* button = new Gtk::Button();
        button->signal_clicked().connect(sigc::bind<size_t>( sigc::mem_fun(*this, &ModuleWindow::on_node), node));
        button->set_label(boost::lexical_cast<std::string>(node));
        node_map_[node] = button;
      }
      size_t col = 0;
      size_t row = lines_+2;
      for (std::map<size_t, Gtk::Button*>::iterator it = node_map_.begin(); it != node_map_.end(); ++it) {
        table_.attach(*it->second, col++, row, 1, 1);
        if (col >= 6) {
          col = 0;
          ++row;
          ++lines_;
        }
      }
      ++lines_;
      table_.show_all_children();
    }
  }

}

void ModuleWindow::on_node(size_t node)
{
  F18aWindow* win = owner_.node_window(node, true);
  SourceWindow* src = win->source(name_);
  src->show();
  src->present();
}

void ModuleWindow::on_rename()
{
  std::string new_name = new_name_entry_.get_text();
  if (new_name.empty()) {
    Gtk::MessageDialog dialog(*this, "Rename module '"+name_+"' failed");
    dialog.set_secondary_text("No new name");
    dialog.run();
    return;
  }
  //evb::Module_ptr mod = owner_.view().model()->find_module(new_name);
  //if (mod.is_not_null()) {
  //  Gtk::MessageDialog dialog(*this, "Rename module '"+name_+"' failed");
  //  dialog.set_secondary_text("New name '"+new_name+"' already exist");
  //  dialog.run();
  //  return;
  //}
  //mod = owner_.view().model()->find_module(name_);
  //if (mod.is_null()) {
  //  Gtk::MessageDialog dialog(*this, "Rename module '"+name_+"' failed");
  //  dialog.set_secondary_text("Module not found");
  //  dialog.run();
  //  return;
  //}
  rename(new_name);
}

void ModuleWindow::on_compile()
{
  compile();
}

void ModuleWindow::on_write()
{
  write();
}

void ModuleWindow::on_start()
{
  start();
}

void ModuleWindow::on_run()
{
  if (!compile()) {return;}
  if (!write()) {return;}
  start();
}


bool ModuleWindow::compile()
{
  evb::Module_ptr mod = owner_.view().model()->find_module(name_);
  if (mod.is_not_null()) {
    if (!owner_.manager()->compile_module(*mod)) {
      Gtk::MessageDialog dialog(*this, "Compile module '"+name_+"' failed");
      dialog.set_secondary_text("Compile error");
      dialog.run();
    }
    Gtk::MessageDialog dialog(*this, "Compile module '"+name_+"' success");
    dialog.set_secondary_text("Compilation ok");
    dialog.run();
    return true;
  }
  Gtk::MessageDialog dialog(*this, "Compile module '"+name_+"' failed");
  dialog.set_secondary_text("There is no module "+name_);
  dialog.run();
  return false;
}

bool ModuleWindow::write()
{
  evb::Module_ptr mod = owner_.view().model()->find_module(name_);
  if (mod.is_not_null()) {
    if (!owner_.manager()->write_module(*mod)) {
      Gtk::MessageDialog dialog(*this, "Write module '"+name_+"' failed");
      dialog.set_secondary_text("Write error");
      dialog.run();
    }
    return true;
  }
  Gtk::MessageDialog dialog(*this, "Writing module '"+name_+"' failed");
  dialog.set_secondary_text("There is no module "+name_);
  dialog.run();
  return false;
}

bool ModuleWindow::start()
{
  evb::Module_ptr mod = owner_.view().model()->find_module(name_);
  if (mod.is_not_null()) {
    if (!owner_.manager()->start_module(*mod)) {
      Gtk::MessageDialog dialog(*this, "Running module '"+name_+"' failed");
      dialog.set_secondary_text("Run error");
      dialog.run();
    }
    return true;
  }
  Gtk::MessageDialog dialog(*this, "Starting module '"+name_+"' failed");
  dialog.set_secondary_text("There is no module "+name_);
  dialog.run();
  return false;
}




}
