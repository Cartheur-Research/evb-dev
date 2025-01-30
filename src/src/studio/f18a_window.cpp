#include <studio.hpp>
#include <connection.hpp>
#include <evb001.hpp>
#include "boost/filesystem.hpp"

namespace studio {

F18aWindow::F18aWindow(Ga144View& v, const std::string& title, f18a::Processor_ptr chip)
: owner_(v)
, rom_source_window_(0)
, view_(*this, chip)
//, source_window_(0)
//, rom_source_window_(0)
{
  set_title(title);

  int w = 800, h = 600;
  set_default_size(w, h); 

  view_event_.set_size_request(F18aView::width(), F18aView::height());

  view_event_.add(view_);
  view_event_.set_hexpand(true);
  view_event_.set_vexpand(true);
  view_event_.add_events(Gdk::BUTTON_MOTION_MASK);
  view_event_.add_events(Gdk::SCROLL_MASK);
  view_event_.signal_button_press_event().connect(sigc::mem_fun(view_, &F18aWindow::on_button_press_event) );
  view_event_.signal_scroll_event().connect(sigc::mem_fun(view_, &F18aWindow::on_scroll_event) );

  content_.pack_start(menu_, Gtk::PACK_SHRINK);
  content_.pack_start(scroll_, Gtk::PACK_EXPAND_WIDGET);
  menu_.append(proc_); proc_.set_label("Processor"); proc_.set_submenu(proc_menu_);
  menu_.append(action_); action_.set_label("Action"); action_.set_submenu(action_menu_);
  menu_.append(simulation_); simulation_.set_label("Simulation"); simulation_.set_submenu(simulation_menu_);
  menu_.append(status_); status_.set_label("Status"); status_.set_submenu(status_menu_);

  proc_menu_.append(source_); source_.set_label("Source");
  proc_menu_.append(add_source_); add_source_.set_label("New Source...");
  proc_menu_.append(rom_source_); rom_source_.set_label("ROM Source...");
  //proc_menu_.append(add_rom_source_); add_rom_source_.set_label("+ ROM Source");
  proc_menu_.append(proc_sep1_);
  proc_menu_.append(close_); close_.set_label("Close");

  action_menu_.append(action_compile_rom_); action_compile_rom_.set_label("Compile ROM");
  action_menu_.append(action_sep1_);
  action_menu_.append(action_read_all_); action_read_all_.set_label("Read all");
  action_menu_.append(action_read_ram_); action_read_ram_.set_label("Read RAM");
  action_menu_.append(action_read_rom_); action_read_rom_.set_label("Read ROM");
  action_menu_.append(action_read_and_compare_rom_); action_read_and_compare_rom_.set_label("Read & compare ROM");
  action_menu_.append(action_read_ram_and_rom_); action_read_ram_and_rom_.set_label("Read RAM & ROM");
  action_menu_.append(action_read_stack_); action_read_stack_.set_label("Read par stack");
  action_menu_.append(action_read_return_stack_); action_read_return_stack_.set_label("Read ret stack");
  action_menu_.append(action_read_a_); action_read_a_.set_label("Read A");
  action_menu_.append(action_read_io_); action_read_io_.set_label("Read I/O");
  action_menu_.append(action_write_all_); action_write_all_.set_label("Write all");
  action_menu_.append(action_write_ram_); action_write_ram_.set_label("Write RAM");
  action_menu_.append(action_write_stack_); action_write_stack_.set_label("Write par stack");
  action_menu_.append(action_write_return_stack_); action_write_return_stack_.set_label("Write ret stack");
  action_menu_.append(action_write_a_); action_write_a_.set_label("Write A");
  action_menu_.append(action_write_b_); action_write_b_.set_label("Write B");
  action_menu_.append(action_write_io_); action_write_io_.set_label("Write I/O");
  action_menu_.append(action_start_); action_start_.set_label("Start");
  action_menu_.append(action_run_); action_run_.set_label("Run");
  action_menu_.append(action_test_); action_test_.set_label("Test");

  simulation_menu_.append(sim_step_); sim_step_.set_label("Step");

  action_compile_rom_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_compiler_rom));
  action_read_all_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_read_all));
  action_read_ram_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_read_ram));
  action_read_rom_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_read_rom));
  action_read_and_compare_rom_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_read_and_compare_rom));
  action_read_ram_and_rom_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_read_ram_and_rom));
  action_read_stack_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_read_stack));
  action_read_return_stack_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_read_return_stack));
  action_read_a_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_read_a));
  action_read_io_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_read_io));
  action_write_all_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_write_all));
  action_write_ram_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_write_ram));
  action_write_stack_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_write_stack));
  action_write_return_stack_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_write_return_stack));
  action_write_a_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_write_a));
  action_write_b_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_write_b));
  action_write_io_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_write_io));
  action_start_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_start));
  action_run_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_run));
  action_test_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_test));

  sim_step_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_sim_step));

  scroll_.add(view_event_);
  scroll_.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

  source_.set_submenu(source_menu_);
  //rom_source_.set_submenu(rom_source_menu_);

  add(content_);
  show_all_children();

  close_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_close));
  add_source_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_add_source));
  //add_rom_source_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_add_rom_source));
  rom_source_.signal_activate().connect(sigc::mem_fun(*this, &F18aWindow::on_rom_source));

  update_source_menu();

}

F18aWindow::~F18aWindow()
{
  {
    std::vector<SourceWindow*>::iterator it;
    for (it = source_window_list_.begin(); it != source_window_list_.end(); ++it) {
      delete *it;
    }
    //for (it = rom_source_window_list_.begin(); it != rom_source_window_list_.end(); ++it) {
    //  delete *it;
    //}
  }
  {
    std::vector<Gtk::MenuItem*>::iterator it;
    for (it = source_list_.begin(); it != source_list_.end(); ++it) {
      delete *it;
    }
    //for (it = rom_source_list_.begin(); it != rom_source_list_.end(); ++it) {
    //  delete *it;
    //}
  }
  if (rom_source_window_ != 0) {
    delete rom_source_window_;
  }
}

void F18aWindow::modify()
{
  owner_.modify();
}

SourceWindow* F18aWindow::source(const std::string& name)
{
  size_t limit = view_.model()->config_size();
  for (size_t i=0; i<limit; ++i) {
    f18a::Module_ptr mod = view_.model()->config(i);
    if (mod->name() == name) {
      while (i >= source_window_list_.size()) {
        SourceWindow* win = new SourceWindow(**owner_.window().owner().model(), false);
        win->setup(view_, source_window_list_.size());
        source_window_list_.push_back(win);
      }
      return source_window_list_[i];
    }
  }
  return 0;
}

void F18aWindow::update_source_menu()
{
  size_t limit = view_.model()->config_size();
  size_t pos = source_list_.size();
  while ((pos < limit) || (pos == 0)) {
    Gtk::MenuItem* item = new Gtk::MenuItem();
    std::string name = "Config ";
    if ((pos < limit) && !view_.model()->config(pos)->name().empty()) {
      name.push_back('"');
      name += view_.model()->config(pos)->name();
      name.push_back('"');
    }
    else {
      name += boost::lexical_cast<std::string>(pos);
    }
    item->set_label(name);
    source_list_.push_back(item);
    source_list_.back()->signal_activate().connect(sigc::bind<size_t>( sigc::mem_fun(*this, &F18aWindow::on_source), pos));
    source_menu_.append(*item);
    item->show();
    ++pos;
  }
  //pos = rom_source_list_.size();
  //while ((pos < limit) || (pos == 0)) {
  //  Gtk::MenuItem* item = new Gtk::MenuItem();
  //  std::string name = "Config ";
  //  if ((pos < limit) && !view_.model()->config(pos).name().empty()) {
  //    name.push_back('"');
  //    name += view_.model()->config(pos).name();
  //    name.push_back('"');
  //  }
  //  else {
  //    name += boost::lexical_cast<std::string>(pos);
  //  }
  //  item->set_label(name);
  //  rom_source_list_.push_back(item);
  //  rom_source_list_.back()->signal_activate().connect(sigc::bind<size_t>( sigc::mem_fun(*this, &F18aWindow::on_rom_source), pos));
  //  rom_source_menu_.append(*item);
  //  item->show();
  //  ++pos;
  //}
}


void F18aWindow::read(xml::Iterator& it)
{
  view_.read(it);
}

void F18aWindow::write(xml::Writer& w)
{
  view_.write(w);
}

void F18aWindow::name(size_t pos, const std::string& val)
{
  std::string name = "Config '";
  name += val;
  name += "'";
  source_list_[pos]->set_label(name);
}

void F18aWindow::on_close()
{
  hide();
}

void F18aWindow::on_source(size_t pos)
{
  while (source_window_list_.size() <= pos) {
    SourceWindow* win = new SourceWindow(**owner_.window().owner().model(), false);
    win->setup(view_, source_window_list_.size());
    source_window_list_.push_back(win);
  }
  //if (source_window_ == 0) {
  //  source_window_ = new SourceWindow();
  //  source_window_->setup(view_, pos, false);
  //}
  source_window_list_[pos]->show();
  source_window_list_[pos]->present();
}

//void F18aWindow::on_rom_source(size_t pos)
//{
//  while (rom_source_window_list_.size() <= pos) {
//    SourceWindow* win = new SourceWindow();
//    win->setup(view_, rom_source_window_list_.size(), true);
//    rom_source_window_list_.push_back(win);
//  }
//  rom_source_window_list_[pos]->show();
//  rom_source_window_list_[pos]->present();
//}

void F18aWindow::on_rom_source()
{
  if (rom_source_window_ == 0) {
    rom_source_window_ = new SourceWindow(**owner_.window().owner().model(), true);
    rom_source_window_->setup(view_, 0);
  }
  rom_source_window_->show();
  rom_source_window_->present();
}

void F18aWindow::on_add_source()
{
  size_t pos = source_list_.size();
  Gtk::MenuItem* item = new Gtk::MenuItem();
  std::string name = "Config ";
  name += boost::lexical_cast<std::string>(pos);
  item->set_label(name);
  source_list_.push_back(item);
  source_list_.back()->signal_activate().connect(sigc::bind<size_t>( sigc::mem_fun(*this, &F18aWindow::on_source), pos));
  source_menu_.append(*item);
  item->show();
}

//void F18aWindow::on_add_rom_source()
//{
//  size_t pos = rom_source_list_.size();
//  Gtk::MenuItem* item = new Gtk::MenuItem();
//  std::string name = "Config ";
//  name += boost::lexical_cast<std::string>(pos);
//  item->set_label(name);
//  rom_source_list_.push_back(item);
//  rom_source_list_.back()->signal_activate().connect(sigc::bind<size_t>( sigc::mem_fun(*this, &F18aWindow::on_rom_source), pos));
//  rom_source_menu_.append(*item);
//  item->show();
//}

size_t F18aWindow::full_node() const
{
  size_t node = view_.model()->node();
  return owner().window().owner().model()->join_node(owner().model(), node);
}

void F18aWindow::on_compiler_rom()
{
  if (rom_source_window_ == 0) {
    rom_source_window_ = new SourceWindow(**owner_.window().owner().model(), true);
    rom_source_window_->setup(view_, 0);
  }
  size_t node = view_.model()->node();
  f18a::Processor_ptr proc = owner_.model()->processor_by_node(node);
  std::string rom_src = proc->rom_source();
  rom_source_window_->text(rom_src);
  rom_source_window_->on_button_assemble();
}

void F18aWindow::on_read_all()
{
  size_t node = full_node();
  if (owner().window().owner().window().manager().is_not_null()) {
    owner().window().owner().window().manager()->do_read(node);
  }
}

void F18aWindow::on_write_all()
{
  size_t node = full_node();
  if (owner().window().owner().window().manager().is_not_null()) {
    owner().window().owner().window().manager()->do_write(node);
  }
}

void F18aWindow::on_read_ram()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_read_ram(node);
}

void F18aWindow::on_read_rom()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_read_rom(node);
}

void F18aWindow::on_read_and_compare_rom()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_read_and_compare_rom(node);
}

void F18aWindow::on_read_ram_and_rom()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_read_ram_and_rom(node);
}

void F18aWindow::on_write_ram()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_write_ram(node);
}

void F18aWindow::on_read_stack()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_read_stack(node);
}

void F18aWindow::on_write_stack()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_write_stack(node);
}

void F18aWindow::on_read_return_stack()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_read_return_stack(node);
}

void F18aWindow::on_write_return_stack()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_write_return_stack(node);
}

void F18aWindow::on_read_a()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_read_a(node);
}

void F18aWindow::on_write_a()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_write_a(node);
}

void F18aWindow::on_write_b()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_write_b(node);
}

void F18aWindow::on_read_io()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_read_io(node);
}

void F18aWindow::on_write_io()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_write_io(node);
}

void F18aWindow::on_start()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_start(node);
}

void F18aWindow::on_run()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_run(node);
}

void F18aWindow::on_test()
{
  size_t node = full_node();
  owner().window().owner().window().manager()->do_test(node);
}

void F18aWindow::on_sim_step()
{
}



}
