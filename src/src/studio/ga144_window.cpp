#include <studio.hpp>
#include <connection.hpp>
#include <evb.hpp>
#include "boost/filesystem.hpp"

namespace studio {

Ga144Window::Ga144Window(EvbView& v, const std::string& title, ga144::Chip_ptr chip)
: owner_(v)
, view_(*this, chip)
//, viewport_()
{
  set_title(title);

  int w = Ga144View::width() + 10, h = Ga144View::height()+30;
  set_default_size(w, h); 

  view_event_.set_size_request(Ga144View::width(), Ga144View::height());

  view_event_.add(view_);
  view_event_.set_hexpand(true);
  view_event_.set_vexpand(true);
  view_event_.add_events(Gdk::BUTTON_MOTION_MASK);
  view_event_.add_events(Gdk::SCROLL_MASK);
  view_event_.signal_button_press_event().connect(sigc::mem_fun(view_, &Ga144View::on_button_press_event) );
  view_event_.signal_scroll_event().connect(sigc::mem_fun(view_, &Ga144View::on_scroll_event) );

  content_.pack_start(menu_, Gtk::PACK_SHRINK);
  content_.pack_start(scroll_, Gtk::PACK_EXPAND_WIDGET);
  menu_.append(chip_); chip_.set_label("Chip"); chip_.set_submenu(chip_menu_);
  menu_.append(action_); action_.set_label("Action"); action_.set_submenu(action_menu_);
  menu_.append(status_); status_.set_label("Status"); status_.set_submenu(status_menu_);

  chip_menu_.append(chip_sep1_);
  chip_menu_.append(close_); close_.set_label("Close");

  action_menu_.append(compile_rom_); compile_rom_.set_label("Compile ROM");

  //mode_menu_.append(ide_); ide_.set_label("IDE"); ide_.set_active(true);
  //mode_menu_.append(poly_); poly_.set_label("PolyForth"); ide_.set_active(false);

  //viewport_.add(scroll_);

  scroll_.add(view_event_);
  scroll_.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

  add(content_);
  show_all_children();

  close_.signal_activate().connect(sigc::mem_fun(*this, &Ga144Window::on_close));

}

Ga144Window::~Ga144Window()
{
}

void Ga144Window::modify()
{
  owner_.modify();
}

void Ga144Window::read(xml::Iterator& it)
{
  view_.read(it);
}

void Ga144Window::write(xml::Writer& w)
{
  view_.write(w);
  //if (connection_window_list_[0] != 0) {
  //  w.open("a", false, true);
  //  w.newline();
  //  w.indent(1);
  //  connection_window_list_[0]->write(w);
  //  w.indent(-1);
  //  w.open("a", true, true);
  //  w.newline();
  //}
}

//void Ga144Window::attach(evb::Board& b)
//{
//  view_.attach(b);
//  set_title(b.name());
//  queue_draw();
//}

//void Ga144Window::reset()
//{
//  view_.reset();
//}

//void Ga144Window::detach()
//{
//  view_.detach();
//  set_title("EVB");
//  queue_draw();
//}

//void Ga144Window::remove_image()
//{
//  if (image_) {
//    content_.remove(*image_);
//    image_->unreference();
//    image_ = 0;
//  }
//}
//
//void Ga144Window::set_image(Gtk::Image* img)
//{
//  if (img) {img->reference();}
//  remove_image();
//  image_ = img;
//  if (image_) {
//    int pw = image_->get_pixbuf()->get_width();
//    int ph = image_->get_pixbuf()->get_height();
//  }
//  content_.pack_start(*image_, Gtk::PACK_SHRINK);
//}

//void Ga144Window::on_close()
//{
//  hide();
//}
//
//void Ga144Window::on_host_chip()
//{
//}
//
//void Ga144Window::on_target_chip()
//{
//}

void Ga144Window::name(const std::string& val)
{
  view_.name(val);
}

void Ga144Window::on_close()
{
  hide();
}

}
