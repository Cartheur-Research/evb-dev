#include <studio.hpp>
#include <connection.hpp>
#include <f18a.hpp>

namespace studio {


IOWindow::IOWindow(evb::Manager_ptr man, size_t from, size_t to)
: manager_(man)
{
  std::string name, descr;

  while (from <= to) {
    evb::Pin::Enum pin = static_cast<evb::Pin::Enum>(from++);
    size_t ind;
    Gtk::Label* name_lbl = 0;
    Gtk::Label* descr_lbl = 0;
    Gtk::Label* read = 0;
    Gtk::Entry* write = 0;
    Gtk::RadioButton* r0;
    Gtk::RadioButton* r1;
    Gtk::RadioButton* r2;
    Gtk::RadioButton* r3;
    Gtk::RadioButton* r4;
    Gtk::Button* action;

    evb::PinType::Enum type = evb::Pin::type(pin);
    switch (type) {
    case evb::PinType::io_1:
    case evb::PinType::io_3:
    case evb::PinType::io_5:
    case evb::PinType::io_17:
    case evb::PinType::analog_in:
    case evb::PinType::analog_out:
      break;
    default: continue;
    }
    if (!evb::Pin::to_string(pin, name)) {name.clear();}
    if (!evb::Pin::to_description(pin, descr)) {descr.clear();}
    name_lbl = new Gtk::Label(name);
    descr_lbl = new Gtk::Label(descr);
    //
    name_lbl->set_alignment(Gtk::JUSTIFY_LEFT);
    descr_lbl->set_alignment(Gtk::JUSTIFY_LEFT);
    //

    switch (type) {
    case evb::PinType::io_1:
    case evb::PinType::io_3:
    case evb::PinType::io_5:
    case evb::PinType::io_17:
      ind = pin_list_.size();
      //
      {
        Gtk::RadioButton::Group group;
        r0 = new Gtk::RadioButton(group, "hi");
        r1 = new Gtk::RadioButton(group, "lo");
        r2 = new Gtk::RadioButton(group, "wk-lo");
        r3 = new Gtk::RadioButton(group, "input");
        r4 = new Gtk::RadioButton(group);
        r4->set_active();
      }
      read = new Gtk::Label("-");
      action = new Gtk::Button("Write");
      //
      r0->signal_toggled().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_high), ind));
      r1->signal_toggled().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_low), ind));
      r2->signal_toggled().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_weak_low), ind));
      r3->signal_toggled().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_input), ind));
      action->signal_clicked().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_write), ind));
      //
      pin_list_.push_back(pin);
      name_list_.push_back(name_lbl);
      descr_list_.push_back(descr_lbl);
      value_list_.push_back(read);
      //
      high_list_.push_back(r0);
      low_list_.push_back(r1);
      weak_low_list_.push_back(r2);
      input_list_.push_back(r3);
      hidden_list_.push_back(r4);
      action_list_.push_back(action);
      break;

    case evb::PinType::analog_in:
      ind = ain_pin_list_.size();
      //
      {
        Gtk::RadioButton::Group group;
        r0 = new Gtk::RadioButton(group, "input");
        r1 = new Gtk::RadioButton(group, "Vdd calib");
        r2 = new Gtk::RadioButton(group, "Vss calib");
        r3 = new Gtk::RadioButton(group, "disable");
        r4 = new Gtk::RadioButton(group);
        r4->set_active();
      }
      read = new Gtk::Label("-");
      action = new Gtk::Button("Analog read");
      //
      r0->signal_toggled().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_high), ind));
      r1->signal_toggled().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_low), ind));
      r2->signal_toggled().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_weak_low), ind));
      r3->signal_toggled().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_input), ind));
      action->signal_clicked().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_aread), ind));
      //
      ain_pin_list_.push_back(pin);
      ain_name_list_.push_back(name_lbl);
      ain_descr_list_.push_back(descr_lbl);
      ain_value_list_.push_back(read);
      //
      ain_input_list_.push_back(r0);
      ain_vdd_calib_list_.push_back(r1);
      ain_vss_calib_list_.push_back(r2);
      ain_disable_list_.push_back(r3);
      ain_hidden_list_.push_back(r4);
      ain_action_list_.push_back(action);
      break;

    case evb::PinType::analog_out:
      ind = aout_pin_list_.size();
      //
      read = new Gtk::Label("-");
      write = new Gtk::Entry();
      action = new Gtk::Button("Analog write");
      //
      action->signal_clicked().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &IOWindow::on_awrite), ind));
      //
      aout_pin_list_.push_back(pin);
      aout_name_list_.push_back(name_lbl);
      aout_descr_list_.push_back(descr_lbl);
      aout_value_list_.push_back(read);
      aout_write_list_.push_back(write);
      //
      aout_action_list_.push_back(action);
      break;
    }
  }

  size_t limit = pin_list_.size();
  size_t line = 0;

  for (size_t i=0; i<limit; ++i) {
    table_.attach(*name_list_[i],           1, line, 1, 1);
    table_.attach(*high_list_[i],           2, line, 1, 1);
    table_.attach(*low_list_[i],            3, line, 1, 1);
    table_.attach(*weak_low_list_[i],       4, line, 1, 1);
    table_.attach(*input_list_[i],          5, line, 1, 1);
    table_.attach(*value_list_[i],          6, line, 1, 1);
    table_.attach(*action_list_[i],         7, line, 1, 1);
    table_.attach(*descr_list_[i],          8, line, 1, 1);
    ++line;
  }

  limit = ain_pin_list_.size();
  for (size_t i=0; i<limit; ++i) {
    table_.attach(*ain_name_list_[i],           1, line, 1, 1);
    table_.attach(*ain_input_list_[i],          2, line, 1, 1);
    table_.attach(*ain_vdd_calib_list_[i],      3, line, 1, 1);
    table_.attach(*ain_vss_calib_list_[i],      4, line, 1, 1);
    table_.attach(*ain_disable_list_[i],        5, line, 1, 1);
    table_.attach(*ain_value_list_[i],          6, line, 1, 1);
    table_.attach(*ain_action_list_[i],         7, line, 1, 1);
    table_.attach(*ain_descr_list_[i],          8, line, 1, 1);
    ++line;
  }

  limit = aout_pin_list_.size();
  for (size_t i=0; i<limit; ++i) {
    table_.attach(*aout_name_list_[i],          1, line, 1, 1);
    table_.attach(*aout_write_list_[i],         2, line, 3, 1);
    table_.attach(*aout_value_list_[i],         6, line, 1, 1);
    table_.attach(*aout_action_list_[i],        7, line, 1, 1);
    table_.attach(*aout_descr_list_[i],         8, line, 1, 1);
    ++line;
  }

  table_.set_column_homogeneous (false);
  table_.set_column_spacing(4);

  scroll_.add(table_);
  scroll_.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  add(scroll_);
  show_all_children();

  int w = 750, h = 500;
  set_default_size(w, h); 

}

IOWindow::~IOWindow()
{
}

void IOWindow::on_high(size_t index)
{
}

void IOWindow::on_low(size_t index)
{
}

void IOWindow::on_weak_low(size_t index)
{
}

void IOWindow::on_input(size_t index)
{
}

void IOWindow::on_write(size_t index)
{
  long val = -1;
  //long mask = 3;
  if (high_list_[index]->get_active()) {val = 3;}
  else if (low_list_[index]->get_active()) {val = 2;}
  else if (weak_low_list_[index]->get_active()) {val = 1;}
  else if (input_list_[index]->get_active()) {val = 0;}
  if (val >= 0) {
    evb::Pin::Enum pin = static_cast<evb::Pin::Enum>(pin_list_[index]);
    //evb::PinType::Enum type = evb::Pin::type(pin);
    //switch (type) {
    //case evb::PinType::io_1: break;
    //case evb::PinType::io_3: val <<= 2; mask <<= 2; break;
    //case evb::PinType::io_5: val <<= 4; mask <<= 4; break;
    //case evb::PinType::io_17: val <<= 16; mask <<= 16; break;
    //default: return;
    //}
    //size_t node = evb::Pin::node(pin);
    manager_->do_set_pin(pin, val);
  }
}

void IOWindow::on_vdd_calib(size_t index)
{
}

void IOWindow::on_vss_calib(size_t index)
{
}

void IOWindow::on_disable(size_t index)
{
}

void IOWindow::on_ainput(size_t index)
{
}

void IOWindow::on_aread(size_t index)
{
}

void IOWindow::on_awrite(size_t index)
{
  std::string value_txt = aout_write_list_[index]->get_text();
  long value;
  try {
    value = boost::lexical_cast<long>(value_txt);
  }
  catch (...) {
    Gtk::MessageDialog dialog(*this, "Writing ananlog value failed");
    dialog.set_secondary_text("The value '"+value_txt+"' is not a number");
    dialog.run();
    return;
  }
  if ((value < 0) || (value >= F18A_AOUT_SIZE)) {
    Gtk::MessageDialog dialog(*this, "Writing ananlog value failed");
    dialog.set_secondary_text("The value '"+value_txt+"' is not in range [0, 511]");
    dialog.run();
    return;
  }
  aout_value_list_[index]->set_text(boost::lexical_cast<std::string>(value));
  evb::Pin::Enum pin = static_cast<evb::Pin::Enum>(aout_pin_list_[index]);
  manager_->do_set_pin(pin, value);
}




}
