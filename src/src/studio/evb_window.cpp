#include <studio.hpp>
#include <connection.hpp>
#include <evb.hpp>
#include "boost/filesystem.hpp"

namespace studio {


EvbWindow::EvbWindow(MainWindow& mw, evb::Board_ptr ptr)
: owner_(mw)
, view_(*this, ptr)
, modules_menu_(0)
, new_module_window_(0)
, packages_menu_(0)
, new_package_window_(0)
, manager_(0)
, test_echo_window_(0)
, host_io_(0)
, target_io_(0)
{
  set_title("EVB");

  int w = 760, h = 474;
  set_default_size(w, h); 

  view_event_.add(view_);
  view_event_.set_hexpand(true);
  view_event_.set_vexpand(true);
  view_event_.add_events(Gdk::BUTTON_MOTION_MASK);
  view_event_.add_events(Gdk::SCROLL_MASK);
  view_event_.signal_button_press_event().connect(sigc::mem_fun(view_, &EvbView::on_button_press_event) );
  view_event_.signal_scroll_event().connect(sigc::mem_fun(view_, &EvbView::on_scroll_event) );

  content_.pack_start(menu_, Gtk::PACK_SHRINK);
  content_.pack_start(view_event_, Gtk::PACK_EXPAND_WIDGET);
  menu_.append(board_); board_.set_label("Board"); board_.set_submenu(board_menu_);
  menu_.append(mview_); mview_.set_label("View"); mview_.set_submenu(view_menu_);
  menu_.append(action_); action_.set_label("Action"); action_.set_submenu(action_menu_);
  menu_.append(highlight_); highlight_.set_label("Highlight"); highlight_.set_submenu(highlight_menu_);
  menu_.append(modules_); modules_.set_label("Module");
  menu_.append(packages_); packages_.set_label("Package");

  board_menu_.append(boot_async_708_host_); boot_async_708_host_.set_label("Boot 708 host");
  board_menu_.append(boot_async_708_host_target_); boot_async_708_host_target_.set_label("Boot 708 host+target");
  board_menu_.append(boot_fast_708_host_); boot_fast_708_host_.set_label("Boot 708 host (fast)");
  board_menu_.append(boot_fast_708_host_target_); boot_fast_708_host_target_.set_label("Boot 708 host+target (fast)");
  board_menu_.append(file_sep1_);
  board_menu_.append(reset_); reset_.set_label("Reset");
  board_menu_.append(connect_); connect_.set_label("Connect");
  board_menu_.append(boot_async_708_); boot_async_708_.set_label("Boot 708");
  board_menu_.append(boot_async_708_kraken_707_); boot_async_708_kraken_707_.set_label("Boot 707+708");
  board_menu_.append(boot_async_708_kraken_709_); boot_async_708_kraken_709_.set_label("Boot 708+709");
  board_menu_.append(boot_async_708_kraken_707_709_); boot_async_708_kraken_707_709_.set_label("Boot 707+708+709");
  board_menu_.append(file_sep2_);
  board_menu_.append(remove_all_tentacles_); remove_all_tentacles_.set_label("Remove all tentacles");
  board_menu_.append(span_host_); span_host_.set_label("Span Host Chip");
  board_menu_.append(span_target_); span_target_.set_label("Span Target Chip");
  board_menu_.append(span_host_target_); span_host_target_.set_label("Span Host+Target Chip");
  board_menu_.append(file_sep3_);
  board_menu_.append(close_); close_.set_label("Close");

  view_menu_.append(view_connector_); view_connector_.set_label("Connector");
  view_menu_.append(view_jumper_); view_jumper_.set_label("Jumper");
  view_menu_.append(view_host_pin_); view_host_pin_.set_label("Host Pin");
  view_menu_.append(view_target_pin_); view_target_pin_.set_label("Target Pin");

  modules_menu_ = new Gtk::Menu();
  modules_.set_submenu(*modules_menu_);
  modules_menu_->append(modules_new_); modules_new_.set_label("New...");

  packages_menu_ = new Gtk::Menu();
  packages_.set_submenu(*packages_menu_);
  packages_menu_->append(packages_new_); packages_new_.set_label("New...");

  if (view_.show_connector()) {view_connector_.set_active(true);}
  if (view_.show_jumper()) {view_jumper_.set_active(true);}
  if (view_.show_host_pin()) {view_host_pin_.set_active(true);}
  if (view_.show_target_pin()) {view_target_pin_.set_active(true);}

  //mode_menu_.append(studio_); studio_.set_label("Studio"); studio_.set_active(true);
  //mode_menu_.append(poly_); poly_.set_label("PolyForth"); poly_.set_active(false);
  action_menu_.append(test_echo_); test_echo_.set_label("Test Echo");

  view_connector_.signal_toggled().connect(sigc::mem_fun(*this, &EvbWindow::on_view_connector));
  view_jumper_.signal_toggled().connect(sigc::mem_fun(*this, &EvbWindow::on_view_jumper));
  view_host_pin_.signal_toggled().connect(sigc::mem_fun(*this, &EvbWindow::on_view_host_pin));
  view_target_pin_.signal_toggled().connect(sigc::mem_fun(*this, &EvbWindow::on_view_target_pin));

  highlight_menu_.append(highlight_connector_); highlight_connector_.set_label("Connector"); highlight_connector_.set_submenu(highlight_connector_menu_);
  highlight_menu_.append(highlight_jumper_); highlight_jumper_.set_label("Jumper"); highlight_jumper_.set_submenu(highlight_jumper_menu_);
  highlight_menu_.append(highlight_host_pin_); highlight_host_pin_.set_label("Host Pin"); highlight_host_pin_.set_submenu(highlight_host_pin_menu_);
  highlight_menu_.append(highlight_target_pin_); highlight_target_pin_.set_label("Target Pin"); highlight_target_pin_.set_submenu(highlight_target_pin_menu_);

  action_menu_.append(action_host_io_); action_host_io_.set_label("Host I/O...");
  action_menu_.append(action_target_io_); action_target_io_.set_label("Target I/O...");
  action_host_io_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_host_io));
  action_target_io_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_target_io));

  remove_all_tentacles_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_remove_all_tentacles));
  span_host_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_span_host));
  span_target_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_span_target));
  span_host_target_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_span_host_target));

  close_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_close));
  test_echo_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_test_echo));
  reset_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_reset));

  modules_new_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_new_module));
  packages_new_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_new_package));

  boot_async_708_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_boot_async_708));
  boot_async_708_kraken_707_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_boot_async_708_kraken_707));
  boot_async_708_kraken_709_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_boot_async_708_kraken_709));
  boot_async_708_kraken_707_709_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_boot_async_708_kraken_707_709));
  boot_async_708_host_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_boot_async_708_host));
  boot_async_708_host_target_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_boot_async_708_host_target));
  boot_fast_708_host_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_boot_fast_708_host));
  boot_fast_708_host_target_.signal_activate().connect(sigc::mem_fun(*this, &EvbWindow::on_boot_fast_708_host_target));

  error_dispatcher_.connect(sigc::mem_fun(*this, &EvbWindow::on_error));
  success_dispatcher_.connect(sigc::mem_fun(*this, &EvbWindow::on_success));
  view_.model()->register_error_dispatcher(error_dispatcher_);
  view_.model()->register_success_dispatcher(success_dispatcher_);

  on_update_modules();
  on_update_packages();

  size_t i;
  std::string txt;
  std::string name;
  std::string descr;
  for (i=0; i<evb::Connector::limit; ++i) {
    evb::Connector::Enum curr = static_cast<evb::Connector::Enum>(i);
    txt.clear();
    if (evb::Connector::to_string(curr, name)) {
      txt = name;
    }
    if (!txt.empty()) {
      if (evb::Connector::to_description(curr, descr)) {
        txt += " (";
        txt += descr;
        txt.push_back(')');
      }
      connector_highlight_[i].set_label(txt);
      highlight_connector_menu_.append(connector_highlight_[i]);
      connector_highlight_[i].signal_toggled().connect(sigc::bind<size_t>( sigc::mem_fun(*this, &EvbWindow::on_highlight_connector), i));
    }
  }
  for (i=0; i<evb::Jumper::limit; ++i) {
    evb::Jumper::Enum curr = static_cast<evb::Jumper::Enum>(i);
    txt.clear();
    if (evb::Jumper::to_string(curr, name)) {
      txt = name;
    }
    if (!txt.empty()) {
      if (evb::Jumper::to_description(curr, descr)) {
        txt += " (";
        txt += descr;
        txt.push_back(')');
      }
      jumper_highlight_[i].set_label(txt);
      highlight_jumper_menu_.append(jumper_highlight_[i]);
      jumper_highlight_[i].signal_toggled().connect(sigc::bind<size_t>( sigc::mem_fun(*this, &EvbWindow::on_highlight_jumper), i));
    }
  }
  for (i=0; i<evb::Pin::t_bus; ++i) {
    evb::Pin::Enum curr = static_cast<evb::Pin::Enum>(i);
    txt.clear();
    if (evb::Pin::to_string(curr, name)) {
      txt = name;
    }
    if (!txt.empty()) {
      if (evb::Pin::to_description(curr, descr)) {
        txt += " (";
        txt += descr;
        txt.push_back(')');
      }
      pin_highlight_[i].set_label(txt);
      if (evb::Pin::host(curr)) {highlight_host_pin_menu_.append(pin_highlight_[i]);}
      else if (evb::Pin::target(curr)) {highlight_target_pin_menu_.append(pin_highlight_[i]);}
      pin_highlight_[i].signal_toggled().connect(sigc::bind<size_t>( sigc::mem_fun(*this, &EvbWindow::on_highlight_pin), i));
    }
  }

  add(content_);
  show_all_children();

}

EvbWindow::~EvbWindow()
{
  delete host_io_;
  delete target_io_;
  for (std::map<std::string, Gtk::MenuItem*>::iterator it = module_map_.begin(); it != module_map_.end(); ++it) {
    delete it->second;
  }
  for (std::map<std::string, Gtk::MenuItem*>::iterator it = package_map_.begin(); it != package_map_.end(); ++it) {
    delete it->second;
  }
  delete_all_module_windows();
  delete_all_package_windows();
  if (new_module_window_) {
    delete new_module_window_;
  }
  if (new_package_window_) {
    delete new_package_window_;
  }
}

void EvbWindow::delete_all_module_windows()
{
  for (std::vector<ModuleWindow*>::iterator it = module_window_list_.begin(); it != module_window_list_.end(); ++it) {
    delete *it;
  }
  module_window_list_.clear();
  module_window_map_.clear();
}

void EvbWindow::delete_all_package_windows()
{
  for (std::vector<PackageWindow*>::iterator it = package_window_list_.begin(); it != package_window_list_.end(); ++it) {
    delete *it;
  }
  package_window_list_.clear();
  package_window_map_.clear();
}


void EvbWindow::modify()
{
  owner_.modify();
}

void EvbWindow::on_view_connector()
{
  view_.show_connector(view_connector_.get_active());
}

void EvbWindow::on_view_jumper()
{
  view_.show_jumper(view_jumper_.get_active());
}

void EvbWindow::on_view_host_pin()
{
  view_.show_host_pin(view_host_pin_.get_active());
}

void EvbWindow::on_view_target_pin()
{
  view_.show_target_pin(view_target_pin_.get_active());
}

void EvbWindow::on_highlight_connector(size_t pos)
{
  view_.highlight_connector(pos, connector_highlight_[pos].get_active());
}

void EvbWindow::on_highlight_jumper(size_t pos)
{
  view_.highlight_jumper(pos, jumper_highlight_[pos].get_active());
}

void EvbWindow::on_highlight_pin(size_t pos)
{
  view_.highlight_pin(pos, pin_highlight_[pos].get_active());
}

void EvbWindow::on_error()
{
  if (manager_->has_error()) {
    Gtk::MessageDialog dialog(*this, "EVB Error");
    std::string err;
    const std::vector<std::string>& list = manager_->event_list();
    for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it) {
      err += *it;
      err += "\n";
    }
    manager_->reset_error_list();
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
}

void EvbWindow::on_success()
{
  if (manager_->has_error()) {
    Gtk::MessageDialog dialog(*this, "EVB Success");
    std::string err;
    const std::vector<std::string>& list = manager_->event_list();
    for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it) {
      err += *it;
      err += "\n";
    }
    manager_->reset_error_list();
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
}

void EvbWindow::on_reset()
{
  //if (manager_) {
  //  delete manager_;
  //  manager_ = 0;
  //}
  if (!view_.model()->has_connection(evb::Port::A)) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  //manager_ = new evb::Manager(*view_.model(), *view_.model()->connection(evb::Port::A), boot_method_);
  if (!manager()->reset()) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err;
    const std::vector<std::string>& list = manager_->event_list();
    for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it) {
      err += *it;
      err += "\n";
    }
    manager_->reset_error_list();
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
}

void EvbWindow::on_boot_async_708()
{
  if (!view_.model()->has_connection(evb::Port::A)) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  manager()->do_boot(evb::BootMethod::async_708);
}

void EvbWindow::on_boot_async_708_kraken_707()
{
  if (!view_.model()->has_connection(evb::Port::A)) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  manager()->do_boot(evb::BootMethod::async_708_kraken_707);
}

void EvbWindow::on_boot_async_708_kraken_709()
{
  if (!view_.model()->has_connection(evb::Port::A)) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  manager()->do_boot(evb::BootMethod::async_708_kraken_709);
}

void EvbWindow::on_boot_async_708_kraken_707_709()
{
  if (!view_.model()->has_connection(evb::Port::A)) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  manager()->do_boot(evb::BootMethod::async_708_kraken_707_709);
}

void EvbWindow::on_boot_async_708_host()
{
  if (!view_.model()->has_connection(evb::Port::A)) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  manager()->do_boot(evb::BootMethod::async_708_host);
}

void EvbWindow::on_boot_async_708_host_target()
{
  if (!view_.model()->has_connection(evb::Port::A)) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  manager()->do_boot(evb::BootMethod::async_708_host_target);
}

void EvbWindow::on_boot_fast_708_host()
{
  if (!view_.model()->has_connection(evb::Port::A)) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  manager()->do_boot(evb::BootMethod::fast_708_host);
}

void EvbWindow::on_boot_fast_708_host_target()
{
  if (!view_.model()->has_connection(evb::Port::A)) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  manager()->do_boot(evb::BootMethod::fast_708_host_target);
}

void EvbWindow::read(xml::Iterator& it)
{
  view_.read(it);
  std::string title = view_.model()->name();
  name(title);
}

void EvbWindow::write(xml::Writer& w)
{
  view_.write(w);
}

void EvbWindow::reset()
{
  //if (manager_) {
  //  delete manager_;
  //  manager_ = 0;
  //}
  if (!view_.model()->has_connection(evb::Port::A)) {
    Gtk::MessageDialog dialog(*this, "EVB Boot Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  //manager_ = new evb::Manager(*view_.model(), *view_.model()->connection(evb::Port::A), boot_method_);
  manager()->do_reset();
}

void EvbWindow::on_close()
{
  hide();
}

evb::Manager_ptr EvbWindow::manager()
{
  if (manager_.is_null()) {
    if (view_.model()->connection(evb::Port::A) == 0) {
      Gtk::MessageDialog dialog(*this, "EVB Error");
      dialog.set_secondary_text("no connection to board via USB port A");
      dialog.show();
      dialog.run();
      return evb::Manager_ptr();
    }
    //manager_ = new evb::Manager(*view_.model(), *view_.model()->connection(evb::Port::A));
    manager_ = new evb::Manager(view_.model());
  }
  return manager_;
}

void EvbWindow::on_remove_all_tentacles()
{
  if (manager_.is_null()) {
    Gtk::MessageDialog dialog(*this, "EVB Remove all Tentacles Error");
    dialog.set_secondary_text("board not ready");
    dialog.show();
    dialog.run();
    return;
  }
  //if (!manager_->connected()) {
  //  Gtk::MessageDialog dialog(*this, "EVB Remove all Tentacles Error");
  //  dialog.set_secondary_text("board not connected");
  //  dialog.show();
  //  dialog.run();
  //  return;
  //}
  //if (!manager_->kraken_ready()) {
  //  Gtk::MessageDialog dialog(*this, "EVB Remove all Tentacles Error");
  //  std::string err;
  //  const std::vector<std::string>& list = manager_->event_list();
  //  for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it) {
  //    err += *it;
  //    err += "\n";
  //  }
  //  manager_->reset_event_list();
  //  dialog.set_secondary_text(err);
  //  dialog.show();
  //  dialog.run();
  //  return;
  //}
  manager_->do_remove_all_tentacles();
}

void EvbWindow::on_span_host()
{
  if (manager_.is_null()) {
    Gtk::MessageDialog dialog(*this, "EVB Span Host Error");
    dialog.set_secondary_text("board not ready");
    dialog.show();
    dialog.run();
    return;
  }
  //if (!manager_->connected()) {
  //  Gtk::MessageDialog dialog(*this, "EVB Span Host Error");
  //  dialog.set_secondary_text("board not connected");
  //  dialog.show();
  //  dialog.run();
  //  return;
  //}
  //if (!manager_->kraken_ready()) {
  //  Gtk::MessageDialog dialog(*this, "EVB Span Host Error");
  //  std::string err;
  //  const std::vector<std::string>& list = manager_->event_list();
  //  for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it) {
  //    err += *it;
  //    err += "\n";
  //  }
  //  manager_->reset_error_list();
  //  dialog.set_secondary_text(err);
  //  dialog.show();
  //  dialog.run();
  //  return;
  //}
  manager_->do_span_host();
}

void EvbWindow::on_span_target()
{
  if (manager_.is_null()) {
    Gtk::MessageDialog dialog(*this, "EVB Span Target Error");
    dialog.set_secondary_text("board not ready");
    dialog.show();
    dialog.run();
    return;
  }
  //if (!manager_->connected()) {
  //  Gtk::MessageDialog dialog(*this, "EVB Span Target Error");
  //  dialog.set_secondary_text("board not connected");
  //  dialog.show();
  //  dialog.run();
  //  return;
  //}
  //if (!manager_->kraken_ready()) {
  //  Gtk::MessageDialog dialog(*this, "EVB Span Target Error");
  //  std::string err;
  //  const std::vector<std::string>& list = manager_->event_list();
  //  for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it) {
  //    err += *it;
  //    err += "\n";
  //  }
  //  manager_->reset_error_list();
  //  dialog.set_secondary_text(err);
  //  dialog.show();
  //  dialog.run();
  //  return;
  //}
  manager_->do_span_target();
}

void EvbWindow::on_span_host_target()
{
  if (manager_.is_null()) {
    Gtk::MessageDialog dialog(*this, "EVB Span Host+Target Error");
    dialog.set_secondary_text("board not ready");
    dialog.show();
    dialog.run();
    return;
  }
  //if (!manager_->connected()) {
  //  Gtk::MessageDialog dialog(*this, "EVB Span Host+Target Error");
  //  dialog.set_secondary_text("board not connected");
  //  dialog.show();
  //  dialog.run();
  //  return;
  //}
  //if (!manager_->kraken_ready()) {
  //  Gtk::MessageDialog dialog(*this, "EVB Span Host+Target Error");
  //  std::string err;
  //  const std::vector<std::string>& list = manager_->event_list();
  //  for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it) {
  //    err += *it;
  //    err += "\n";
  //  }
  //  manager_->reset_event_list();
  //  dialog.set_secondary_text(err);
  //  dialog.show();
  //  dialog.run();
  //  return;
  //}
  manager_->do_span_host_target();
}

void EvbWindow::on_host_chip()
{
}

void EvbWindow::on_target_chip()
{
}

void EvbWindow::name(const std::string& n)
{
  set_title(n);
  view_.name(n);
}

void EvbWindow::on_host_io()
{
  if (host_io_ == 0) {
    host_io_ = new IOWindow(manager(), evb::Pin::h_001_1, evb::Pin::h_717_ao);
    host_io_->set_title("Host I/O control");
  }
  host_io_->show();
  host_io_->present();
}

void EvbWindow::on_target_io()
{
  if (target_io_ == 0) {
    target_io_ = new IOWindow(manager(), evb::Pin::t_001_1, evb::Pin::t_008_17);
    target_io_->set_title("Target I/O control");
  }
  target_io_->show();
  target_io_->present();
}


void EvbWindow::on_test_echo()
{
  if (test_echo_window_ == 0) {
    test_echo_window_ = new EchoTestWindow();
    test_echo_window_->setup(view_);
  }
  test_echo_window_->show();
  test_echo_window_->present();
}
//
//void EvbWindow::collect_modules()
//{
//}

void EvbWindow::on_new_module()
{
  if (new_module_window_ == 0) {
    new_module_window_ = new ModuleWindow(*this, "");
  }
  new_module_window_->show();
}

void EvbWindow::accept_new_module(ModuleWindow& w)
{
  if (&w == new_module_window_) {
    new_module_window_->hide();
    delete new_module_window_;
    new_module_window_ = 0;

    on_update_modules();
  }
}

void EvbWindow::on_update_modules()
{
  Gtk::Menu* new_menu = new Gtk::Menu();
  modules_.set_submenu(*new_menu);
  delete modules_menu_;
  modules_menu_ = new_menu;
  modules_menu_->append(modules_new_); modules_new_.set_label("New...");

  for (std::map<std::string, Gtk::MenuItem*>::iterator it = module_map_.begin(); it != module_map_.end(); ++it) {
    delete it->second;
  }
  module_map_.clear();
  delete_all_module_windows();

  evb::Board_ptr board = view_.model();
  if (board.is_not_null()) {
    //board->update_modules();
    evb::Board::module_map_type& map = board->modules();
    if (!map.empty()) {
      modules_menu_->append(modules_sep1_);
      for (evb::Board::module_map_type::iterator it = map.begin(); it != map.end(); ++it) {
        Gtk::MenuItem* item = new Gtk::MenuItem();
        item->set_label(it->first);
        modules_menu_->append(*item);
        item->signal_activate().connect(sigc::bind<const std::string&>( sigc::mem_fun(*this, &EvbWindow::on_module_select), it->second->name()));
      }
    }
  }
  modules_menu_->show_all();
}

void EvbWindow::on_module_select(const std::string& name)
{
  evb::Board_ptr board = view_.model();
  if (board.is_not_null()) {
    std::map<std::string, ModuleWindow*>::iterator it = module_window_map_.find(name);
    if (it == module_window_map_.end()) {
      ModuleWindow* mod_win = new ModuleWindow(*this, name);
      module_window_map_[name] = mod_win;
      it = module_window_map_.find(name);
    }
    it->second->show();
    it->second->present();
  }
}

void EvbWindow::on_update_packages()
{
  Gtk::Menu* new_menu = new Gtk::Menu();
  packages_.set_submenu(*new_menu);
  delete packages_menu_;
  packages_menu_ = new_menu;
  packages_menu_->append(packages_new_); packages_new_.set_label("New...");

  for (std::map<std::string, Gtk::MenuItem*>::iterator it = package_map_.begin(); it != package_map_.end(); ++it) {
    delete it->second;
  }
  package_map_.clear();
  delete_all_package_windows();

  evb::Board_ptr board = view_.model();
  if (board.is_not_null()) {
    evb::Board::package_map_type& map = board->packages();
    if (!map.empty()) {
      packages_menu_->append(packages_sep1_);
      for (evb::Board::package_map_type::iterator it = map.begin(); it != map.end(); ++it) {
        Gtk::MenuItem* item = new Gtk::MenuItem();
        item->set_label(it->first);
        packages_menu_->append(*item);
        item->signal_activate().connect(sigc::bind<const std::string&>( sigc::mem_fun(*this, &EvbWindow::on_package_select), it->second->name()));
      }
    }
  }
  packages_menu_->show_all();
}

void EvbWindow::on_package_select(const std::string& name)
{
  evb::Board_ptr board = view_.model();
  if (board.is_not_null()) {
    std::map<std::string, PackageWindow*>::iterator it = package_window_map_.find(name);
    if (it == package_window_map_.end()) {
      PackageWindow* mod_win = new PackageWindow(*this, name);
      package_window_map_[name] = mod_win;
      it = package_window_map_.find(name);
    }
    evb::Package_ptr ptr;
    evb::Board_ptr board = view_.model();
    if (board.is_not_null()) {
      ptr = board->find_package(name);
    }
    if (ptr.is_null()) {
      ptr = new evb::Package(**board, name);
    }
    else {
      ptr = ptr->clone();
    }
    it->second->setup(ptr);
    it->second->show();
    it->second->present();
  }
}

void EvbWindow::on_new_package()
{
  if (new_package_window_ == 0) {
    new_package_window_ = new PackageWindow(*this, "");
    new_package_window_->setup(0);
  }
  new_package_window_->show();

}

void EvbWindow::accept_new_package(PackageWindow& w)
{
  if (&w == new_package_window_) {
    new_package_window_->hide();
    delete new_package_window_;
    new_package_window_ = 0;

    on_update_packages();
  }
}


}
