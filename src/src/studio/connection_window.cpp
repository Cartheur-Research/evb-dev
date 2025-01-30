#include <studio.hpp>
#include <connection.hpp>

namespace studio {


ConnectionWindow::ConnectionWindow()
: view_(0)
, port_(0)
{
  this->set_modal(true);
  this->set_deletable(false);

#ifdef _WIN32
    device_.set_text("\\\\.\\COM1");
#else
    device_.set_text(name = "/dev/ttyS1");
#endif
  baud_.append("0", "-");
  baud_.append("9600", "9'600");
  baud_.append("14400", "14'400");
  baud_.append("19200", "19'200");
  baud_.append("38400", "38'400");
  baud_.append("57600", "57'600");
  baud_.append("115200", "115'200");
  baud_.append("230400", "230'400");
  baud_.append("460800", "460'800");
  baud_.append("921600", "921'600");

  bits_.append("0", "-");
  bits_.append("7", "7");
  bits_.append("8", "8");

  flow_.append("", "-");
  flow_.append("none", "none");
  flow_.append("software", "software");
  flow_.append("hardware", "hardware");

  parity_.append("", "-");
  parity_.append("none", "none");
  parity_.append("odd", "odd");
  parity_.append("even", "even");

  stop_.append("", "-");
  stop_.append("1", "1");
  stop_.append("1.5", "1.5");
  stop_.append("2", "2");

  set_default();

  device_label_.set_text("Serial device");
  device_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  delay_label_.set_text("Delay (ms)");
  delay_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  baud_label_.set_text("Baudrate");
  baud_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  bits_label_.set_text("Bits per byte");
  bits_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  flow_label_.set_text("Flow control");
  flow_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  parity_label_.set_text("Parity");
  parity_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  stop_label_.set_text("Stop bits");
  stop_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  // buttons
  ok_button_.set_label("OK");
  button_box_.pack_start(ok_button_, Gtk::PACK_SHRINK);
  button_box_.set_border_width(5);
  button_box_.set_layout(Gtk::BUTTONBOX_END);
  ok_button_.signal_clicked().connect(sigc::mem_fun(*this, &ConnectionWindow::on_button_ok) );
    
  cancel_button_.set_label("Cancel");
  button_box_.pack_start(cancel_button_, Gtk::PACK_SHRINK);
  //button_box_.set_border_width(5);
  //button_box_.set_layout(Gtk::BUTTONBOX_END);
  cancel_button_.signal_clicked().connect(sigc::mem_fun(*this, &ConnectionWindow::on_button_cancel) );

  table_.set_column_homogeneous(false);
  table_.set_border_width(10);
  table_.set_row_spacing(5);
  table_.set_column_spacing(5);

  size_t pos = 0;
  table_.attach(device_label_,        0, pos, 1, 1);
  table_.attach(device_,              1, pos, 1, 1);
  ++pos;
  table_.attach(delay_label_,         0, pos, 1, 1);
  table_.attach(delay_,               1, pos, 1, 1);
  ++pos;
  table_.attach(baud_label_,          0, pos, 1, 1);
  table_.attach(baud_,                1, pos, 1, 1);
  ++pos;
  table_.attach(bits_label_,          0, pos, 1, 1);
  table_.attach(bits_,                1, pos, 1, 1);
  ++pos;
  table_.attach(flow_label_,          0, pos, 1, 1);
  table_.attach(flow_,                1, pos, 1, 1);
  ++pos;
  table_.attach(parity_label_,        0, pos, 1, 1);
  table_.attach(parity_,              1, pos, 1, 1);
  ++pos;
  table_.attach(stop_label_,          0, pos, 1, 1);
  table_.attach(stop_,                1, pos, 1, 1);
  ++pos;
  table_.attach(button_box_,          0, pos, 2, 1);

  add(table_);
  show_all_children();

}

ConnectionWindow::~ConnectionWindow()
{
}

void ConnectionWindow::setup(EvbView& view, size_t port)
{
  view_ = &view;
  port_ = port;
}

void ConnectionWindow::set_default()
{
  delay_.set_text("1");
  baud_.set_active_id("921600");
  bits_.set_active_id("8");
  flow_.set_active_id("none");
  parity_.set_active_id("none");
  stop_.set_active_id("1");
}

void ConnectionWindow::set_undefined()
{
  delay_.set_text("");
  baud_.set_active_id("0");
  bits_.set_active_id("0");
  flow_.set_active_id("");
  parity_.set_active_id("");
  stop_.set_active_id("");
}

void ConnectionWindow::open()
{
  show();
}

void ConnectionWindow::on_button_ok()
{
  hide();
  if (view_) {
    view_->update(*this, port_);
  }
}

void ConnectionWindow::on_button_cancel()
{
  hide();
}

void ConnectionWindow::read(connection::Serial_ptr conn)
{
  long br = 0;
  int b = 0;
  size_t d = 0;
  try {
    br = boost::lexical_cast<long>(baud_.get_active_id());
  }
  catch(...) {
  }
  try {
    b = boost::lexical_cast<long>(bits_.get_active_id());
  }
  catch(...) {
  }
  try {
    d = boost::lexical_cast<size_t>(delay_.get_text());
  }
  catch(...) {
  }
  conn->close();
  conn->name(device_.get_text());
  if (br > 0) {conn->baud_rate(br);}
  if (b > 0) {conn->bits(b);}
  if (d > 0) {conn->delay(d);}
  else {conn->reset_delay();}
  std::string tmp = flow_.get_active_id();
  if (!tmp.empty()) {conn->flow(tmp);}
  tmp = parity_.get_active_id();
  if (!tmp.empty()) {conn->parity(tmp);}
  tmp = stop_.get_active_id();
  if (!tmp.empty()) {conn->stop(tmp);}
}

void ConnectionWindow::write(const connection::Serial_ptr serial)
{
  if (!serial->name().empty()) {
    device_.set_text(serial->name());
  }
  delay_.set_text(boost::lexical_cast<std::string>(serial->delay_ms()));
  baud_.set_active_id(boost::lexical_cast<std::string>(serial->baud_rate()));
  bits_.set_active_id(boost::lexical_cast<std::string>(serial->bits()));
  flow_.set_active_id(serial->flow());
  parity_.set_active_id(serial->parity());
  stop_.set_active_id(serial->stop());
}

//void ConnectionWindow::read(xml::Iterator& it)
//{
//	while (it.current() == xml::Element::tag) {
//		if (it.tag() == "name") {
//  		if (it.next() == xml::Element::content) {
//        device_.set_text(it.value());
//      }
//		}
//		else if (it.tag() == "baud") {
//  		if (it.next() == xml::Element::content) {
//        baud_.set_active_id(it.value());
//      }
//    }
//		else if (it.tag() == "bits") {
//  		if (it.next() == xml::Element::content) {
//        bits_.set_active_id(it.value());
//      }
//    }
//		else if (it.tag() == "flow") {
//  		if (it.next() == xml::Element::content) {
//        flow_.set_active_id(it.value());
//      }
//    }
//		else if (it.tag() == "parity") {
//  		if (it.next() == xml::Element::content) {
//        parity_.set_active_id(it.value());
//      }
//    }
//		else if (it.tag() == "stop") {
//  		if (it.next() == xml::Element::content) {
//        stop_.set_active_id(it.value());
//      }
//    }
//    it.next_tag();
//	}
//}
//
//void ConnectionWindow::write(xml::Writer& w)
//{
//  std::string value;
//  value = device_.get_text();
//  w.tag("name", value);
//  w.newline();
//  value = baud_.get_active_id();
//  w.tag("baud", value);
//  w.newline();
//  value = bits_.get_active_id();
//  w.tag("bits", value);
//  w.newline();
//  value = flow_.get_active_id();
//  w.tag("flow", value);
//  w.newline();
//  value = parity_.get_active_id();
//  w.tag("parity", value);
//  w.newline();
//  value = stop_.get_active_id();
//  w.tag("stop", value);
//  w.newline();
//}

}
