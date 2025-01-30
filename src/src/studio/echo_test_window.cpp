#include <studio.hpp>
#include <connection.hpp>
#include <evb.hpp>
#include "boost/filesystem.hpp"

namespace studio {


EchoTestWindow::EchoTestWindow()
: board_(0)
, manager_(0)
, data_index_(0)
, cnt_(0)
, ok_(0)
, nok_(0)
, curr_(0)
, active_(false)
, running_(false)
, finished_(false)
{

  ok_count_label_.set_text("# ok");
  ok_count_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  nok_count_label_.set_text("# nok");
  nok_count_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  total_count_label_.set_text("# total");
  total_count_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  sample_label_.set_text("sample");
  sample_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  out_label_.set_text("out");
  out_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  in_label_.set_text("in");
  in_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  // buttons
  button_box_.set_border_width(5);
  button_box_.set_layout(Gtk::BUTTONBOX_END);

  start_button_.set_label("Start");
  button_box_.pack_start(start_button_, Gtk::PACK_SHRINK);
  start_button_.signal_clicked().connect(sigc::mem_fun(*this, &EchoTestWindow::start) );

  stop_button_.set_label("Stop");
  button_box_.pack_start(stop_button_, Gtk::PACK_SHRINK);
  stop_button_.signal_clicked().connect(sigc::mem_fun(*this, &EchoTestWindow::stop) );

  reset_button_.set_label("Reset");
  button_box_.pack_start(reset_button_, Gtk::PACK_SHRINK);
  reset_button_.signal_clicked().connect(sigc::mem_fun(*this, &EchoTestWindow::on_reset) );

  table_.set_column_homogeneous(false);
  table_.set_border_width(10);
  table_.set_row_spacing(5);
  table_.set_column_spacing(5);

  size_t line = 0;

  table_.attach(button_box_,                0, line, 3, 1);
  ++line;

  table_.attach(total_count_label_,         0, line, 1, 1);
  table_.attach(total_count_,               1, line, 1, 1);
  ++line;

  table_.attach(ok_count_label_,            0, line, 1, 1);
  table_.attach(ok_count_,                  1, line, 1, 1);
  ++line;

  table_.attach(nok_count_label_,           0, line, 1, 1);
  table_.attach(nok_count_,                 1, line, 1, 1);
  ++line;

  table_.attach(sample_label_,              0, line, 1, 1);
  table_.attach(out_label_,                 1, line, 1, 1);
  table_.attach(in_label_,                  2, line, 1, 1);
  ++line;

  for (size_t i=0; i<ECHO_TEST_SIZE; ++i) {
    table_.attach(sample_[i],               0, line, 1, 1);
    table_.attach(out_[i],                  1, line, 1, 1);
    table_.attach(in_[i],                   2, line, 1, 1);
    ++line;
  }

  //set_title("EchoTest on");

  //int w = 760, h = 474;
  //set_default_size(w, h); 

  //view_event_.add(view_);
  //view_event_.set_hexpand(true);
  //view_event_.set_vexpand(true);
  //view_event_.add_events(Gdk::BUTTON_MOTION_MASK);
  //view_event_.add_events(Gdk::SCROLL_MASK);
  //view_event_.signal_button_press_event().connect(sigc::mem_fun(view_, &EvbView::on_button_press_event) );
  //view_event_.signal_scroll_event().connect(sigc::mem_fun(view_, &EvbView::on_scroll_event) );

  //content_.pack_start(menu_, Gtk::PACK_SHRINK);
  //content_.pack_start(view_event_, Gtk::PACK_EXPAND_WIDGET);
  //menu_.append(board_); board_.set_label("Board"); board_.set_submenu(board_menu_);
  //menu_.append(mode_); mode_.set_label("Mode"); mode_.set_submenu(mode_menu_);

  //board_menu_.append(reset_); reset_.set_label("Reset");
  //board_menu_.append(connect_); connect_.set_label("Connect");
  //board_menu_.append(boot_); boot_.set_label("Boot");
  //board_menu_.append(file_sep1_);
  //board_menu_.append(close_); close_.set_label("Close");

  //mode_menu_.append(ide_); ide_.set_label("IDE"); ide_.set_active(true);
  //mode_menu_.append(poly_); poly_.set_label("PolyForth"); ide_.set_active(false);

  //add(content_);
  //show_all_children();

  //close_.signal_activate().connect(sigc::mem_fun(*this, &EchoTestWindow::on_close));

  dispatcher_.connect(sigc::mem_fun(*this, &EchoTestWindow::display_samples));

  add(table_);
  show_all_children();

}

EchoTestWindow::~EchoTestWindow()
{
}

void EchoTestWindow::copy_sample_data()
{
  boost::mutex::scoped_lock lock(mutex_);
  size_t pos = data_index_+1;
  if (pos >= ECHO_TEST_SIZE) {pos = 0;}
  for (size_t i=0; i<ECHO_TEST_SIZE; ++i) {
    display_sample_data_[i] = sample_data_[pos];
    display_out_data_[i] = out_data_[pos];
    display_in_data_[i] = in_data_[pos];
    if (++pos >= ECHO_TEST_SIZE) {pos = 0;}
  }
  display_cnt_ = cnt_;
  display_ok_ = ok_;
  display_nok_ = nok_;
}

void EchoTestWindow::notify()
{
  dispatcher_.emit();
}

void EchoTestWindow::add_sample(long out, long in)
{
  {
    boost::mutex::scoped_lock lock(mutex_);
    if (++data_index_ >= ECHO_TEST_SIZE) {data_index_ = 0;}
    sample_data_[data_index_] = cnt_;
    out_data_[data_index_] = out;
    in_data_[data_index_] = in;

    ++cnt_;
    if (in == out) {
      ++ok_;
    }
    else {
      ++nok_;
    }
  }
  this->notify();
}

void EchoTestWindow::display_samples()
{
  copy_sample_data();

  for (size_t i=0; i<ECHO_TEST_SIZE; ++i) {
    if (display_sample_data_[i] >= 0) {
      sample_[i].set_text(boost::lexical_cast<std::string>(display_sample_data_[i]));
      out_[i].set_text(boost::lexical_cast<std::string>(display_out_data_[i]));
      if (display_in_data_[i] >= 0) {
        in_[i].set_text(boost::lexical_cast<std::string>(display_in_data_[i]));
      }
      else {
        in_[i].set_text("timeout");
      }
    }
    else {
      sample_[i].set_text("");
      out_[i].set_text("");
      in_[i].set_text("");
    }
  }
  total_count_.set_text(boost::lexical_cast<std::string>(display_cnt_));
  ok_count_.set_text(boost::lexical_cast<std::string>(display_ok_));
  nok_count_.set_text(boost::lexical_cast<std::string>(display_nok_));
}

void EchoTestWindow::setup(EvbView& view)
{
  std::string title = "EchoTest on ";
  title += view.name();
  set_title(title);
  board_ = view.model();
  manager_ = view.window().manager();

  //active_ = true;
  //running_ = false;
  //finished_ = false;

  reset();
}

void EchoTestWindow::on_reset()
{
  reset();
}

bool EchoTestWindow::reset()
{
  if ((board_ == 0) || (manager_ == 0)) {return false;}

  if (!running_) {
    if (!manager_->boot_async(708, "Echo Test", true)) {
      Gtk::MessageDialog dialog(*this, "Reset Error");
      std::string err = "Cannot boot the board";
      dialog.set_secondary_text(err);
      dialog.show();
      dialog.run();
      return false;
    }

    if (active_) {
      active_ = false;
      cond_.notify_one();
      while (!finished_) {
        connection::sleep(10);
      }
    }
    finished_ = false;
    active_ = true;
    new boost::thread(sigc::mem_fun(*this, &EchoTestWindow::run));

    curr_ = cnt_ = ok_ = nok_ = 0;
    data_index_ = 0;
    total_count_.set_text("0");
    ok_count_.set_text("0");
    nok_count_.set_text("0");

    for (size_t i=0; i<ECHO_TEST_SIZE; ++i) {
      sample_[i].set_text("");
      out_[i].set_text("");
      in_[i].set_text("");
      sample_data_[i] = -1;
      out_data_[i] = 0;
      in_data_[i] = 0;
    }
  }
  //if (!board_->reset()) {
  //  Gtk::MessageDialog dialog(*this, "Reset Error");
  //  std::string err = "Cannot reset board";
  //  dialog.set_secondary_text(err);
  //  dialog.show();
  //  dialog.run();
  //  return false;
  //}
  //if (!board_->boot_async(evb::Port::A)) {
  //  Gtk::MessageDialog dialog(*this, "Reset Error");
  //  std::string err = "Cannot boot board";
  //  dialog.set_secondary_text(err);
  //  dialog.show();
  //  dialog.run();
  //  return false;
  //}
  return true;
}

void EchoTestWindow::start()
{
  if (running_) {return;}
  if (board_ == 0) {
    Gtk::MessageDialog dialog(*this, "Start Error");
    std::string err = "No board defined";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  if (!board_->has_connection(evb::Port::A) || !board_->connection(evb::Port::A)->connected()) {
    Gtk::MessageDialog dialog(*this, "Start Error");
    std::string err = "No connection through USB port A to board";
    dialog.set_secondary_text(err);
    dialog.show();
    dialog.run();
    return;
  }
  if (!active_) {
    if (!reset()) {
      return;
    }
  }
  cond_.notify_one();
  running_ = true;
}

void EchoTestWindow::stop()
{
  running_ = false;
}

void EchoTestWindow:: run()
{
  finished_ = false;
  while (active_) {
    if (running_) {
      long out = (curr_++) & F18A_CELL_MASK;
      long in;
      board_->connection(evb::Port::A)->write(out);

      size_t cnt = 0;
      while (!board_->connection(evb::Port::A)->read_wait(in, 0)) {
        if (++cnt < 100) {
          connection::sleep(1);
        }
        else {
          in = -1;
          break;
        }
      }
      add_sample(out, in);
    }
    else {
      boost::mutex::scoped_lock lock(mutex_);
      boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
      time += boost::posix_time::milliseconds(1000);
      cond_.timed_wait(lock, time);
    }
  }
  finished_ = true;
}



}
