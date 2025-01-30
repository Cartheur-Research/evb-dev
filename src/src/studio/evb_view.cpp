#include <studio.hpp>
#include <connection.hpp>
#include <evb001.hpp>
#include <evb002.hpp>
#include "boost/filesystem.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

namespace studio {


EvbView::EvbView(EvbWindow& w, evb::Board_ptr ptr)
: window_(w)
, model_(ptr)
, image_(0)
, host_window_(0)
, target_window_(0)
, connection_(0)
, package_(0)
//, tooltip_(new Gtk::Tooltip())
, zoom_(1)
, trans_x_(0)
, trans_y_(0)
, image_w_(0)
, image_h_(0)
, show_connector_(false)
, show_jumper_(false)
, show_host_pin_(true)
, show_target_pin_(true)
{
  set_has_tooltip(true);

  signal_size_allocate().connect(sigc::mem_fun(*this, &EvbView::on_area_size_changed));

  signal_query_tooltip().connect(sigc::mem_fun(*this, &EvbView::on_tooltip));

  //connection_window_list_.resize(evb::Port::limit);

  connection_status_change_dispatcher_.connect(sigc::mem_fun(*this, &EvbView::on_connection_change));

  size_t i;
  for (i=0; i<evb::Connector::limit; ++i) {
    connector_highlight_[i] = false;
  }
  for (i=0; i<evb::Jumper::limit; ++i) {
    jumper_highlight_[i] = false;
  }
  for (i=0; i<evb::Pin::limit; ++i) {
    pin_highlight_[i] = false;
  }

  //model_ = new evb::Board();
  std::string file = model_->picture_file();
  if (boost::filesystem::exists(file)) {
    Gtk::Image* image = new Gtk::Image(file);
    set_image(image);
  }
}

EvbView::~EvbView()
{
  //for (size_t i=0; i<evb::Port::limit; ++i) {
  //  if (connection_window_list_[i] != 0) {
  //    connection_window_list_[i]->hide();
  //    delete connection_window_list_[i];
  //    connection_window_list_[i] = 0;
  //  }
  //}
  remove_image();
  if (host_window_) {
    host_window_->hide();
    delete host_window_;
    host_window_ = 0;
  }
  if (target_window_) {
    target_window_->hide();
    delete target_window_;
    target_window_ = 0;
  }
  if (connection_) {
    connection_->hide();
    delete connection_;
    connection_ = 0;
  }
  if (package_) {
    package_->hide();
    delete package_;
    package_ = 0;
  }
}

void EvbView::modify()
{
  window_.modify();
}

void EvbView::on_connection_change()
{
  this->queue_draw();
}

double EvbView::width() const
{
  return image_w_;
}

double EvbView::height() const
{
  return image_h_;
}

void EvbView::reset()
{
}

void EvbView::show_connector(bool val)
{
  if (val != show_connector_) {
    show_connector_ = val;
    queue_draw();
  }
}

void EvbView::show_jumper(bool val)
{
  if (val != show_jumper_) {
    show_jumper_ = val;
    queue_draw();
  }
}

void EvbView::show_host_pin(bool val)
{
  if (val != show_host_pin_) {
    show_host_pin_ = val;
    queue_draw();
  }
}

void EvbView::show_target_pin(bool val)
{
  if (val != show_target_pin_) {
    show_target_pin_ = val;
    queue_draw();
  }
}

void EvbView::highlight_connector(size_t pos, bool val)
{
  if (connector_highlight_[pos] != val) {
    connector_highlight_[pos] = val;
    queue_draw();
  }
}

void EvbView::highlight_jumper(size_t pos, bool val)
{
  if (jumper_highlight_[pos] != val) {
    jumper_highlight_[pos] = val;
    queue_draw();
  }
}

void EvbView::highlight_pin(size_t pos, bool val)
{
  if (pin_highlight_[pos] != val) {
    pin_highlight_[pos] = val;
    queue_draw();
  }
}

void EvbView::remove_image()
{
  if (image_) {
    //content_.remove(*image_);
    image_->unreference();
    image_ = 0;
    image_w_ = image_h_ = 0;
  }
}

void EvbView::set_image(Gtk::Image* img)
{
  if (img) {img->reference();}
  remove_image();
  image_ = img;
  if (image_) {
    image_w_ = image_->get_pixbuf()->get_width();
    image_h_ = image_->get_pixbuf()->get_height();
  }
  else {
    image_w_ = image_h_ = 0;
  }
  //content_.pack_start(*image_, Gtk::PACK_SHRINK);
}

void EvbView::read(xml::Iterator& it)
{
	if ((it.current() == xml::Element::tag) && (it.tag() == "model")) {
  	std::string tag = it.tag();
	  it.next_tag();

    std::string version;
	  if (it.tag() == "version") {
  	  if (it.next() == xml::Element::content) {
        version = it.value();
        if ((it.next_tag() == xml::Element::end) && (it.tag() == "version")) {
          it.next_tag();
        }
      }
    }

    if (!version.empty()) {
      if (version == "001") {
        model_ = new evb::Board001();
      }
      else if (version == "002") {
        model_ = new evb::Board002();
      }
      else {
        throw xml_exception("EvbView::read evalboard version not supported");
      }
    }
    else if (model_.is_null()) {
      if ((version == "001") || version.empty()) {
        model_ = new evb::Board001();
      }
      else {
        throw xml_exception("EvbView::read evalboard version not supported");
      }
    }
    model_->read(it);
  	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw xml_exception("EvbView::read invalid file");}
    it.next_tag();
    // install dispatcher for port failure notification
    size_t i;
    for (i=0; i<evb::Port::limit; ++i) {
      if (model_->has_connection(static_cast<evb::Port::Enum>(i))) {
        // we must create a connection and set the dispatcher
        model_->assert_connection(static_cast<evb::Port::Enum>(i))->register_dispatcher(connection_status_change_dispatcher_);
      }
    }
  }
}

void EvbView::write(xml::Writer& w)
{
  if (model_.is_not_null()) {
  	w.newline();
    w.open("model", false, true);
    w.indent(1);
    model_->write(w);
    w.newline(-1);
    w.open("model", true, true);
  }
}

bool EvbView::on_draw (const ::Cairo::RefPtr<::Cairo::Context>& cairo)
{
  Glib::RefPtr<Pango::Layout> pl = Pango::Layout::create(cairo);
  draw(cairo, pl, 0, 0);
  return true;
}

void EvbView::draw(const Cairo::RefPtr<Cairo::Context>& cairo, const Glib::RefPtr<Pango::Layout>& pango, double x, double y)
{
  double w = width();
  double h = height();

  cairo->save();
  Gtk::Allocation allocation = get_allocation();
  double width = allocation.get_width();
  double height = allocation.get_height();

  cairo->set_source_rgb(1, 1, 1);
  cairo->rectangle(0, 0, width, height);
  cairo->fill();
  
  Cairo::Matrix m;
  cairo->get_matrix(m);
  m.translate(trans_x_, trans_y_);
  m.scale(zoom_, zoom_);
  cairo->set_matrix(m);

  cairo->set_source_rgb(0.9, 0.9, 0.9);
  cairo->rectangle(x, y, w, h);
  cairo->fill();

  cairo->set_source_rgb(0,0,0);
  cairo->select_font_face("Consolas,monospace", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
  cairo->set_font_size(6);

  cairo->set_line_width(1);


  Gdk::Rectangle r;
  Gdk::Point p;
  size_t i;
  std::string txt;
  Pango::FontDescription fdesc;
  fdesc.set_family("Consolas,monospace");
  fdesc.set_size(3 * PANGO_SCALE);

  if (image_ != 0) {
    Gdk::Cairo::set_source_pixbuf(cairo, image_->get_pixbuf(), x, y);
    cairo->rectangle(x, y, w, h);
    cairo->fill();
  }
  if (model_.is_not_null()) {
    if (show_connector_) {
      double y1;
      Pango::FontDescription fdesc1;
      fdesc1.set_family("Consolas,monospace");
      fdesc1.set_size(static_cast<int>(1.5 * PANGO_SCALE));

      std::string txt1 = "1";

      for (i=0; i<evb::Connector::limit; ++i) {
        if (connector_rect(i, r, p, y1)) {
          if (connector_highlight_[i]) {
            cairo->set_source_rgba(1, 0.5, 0.5, 1);
          }
          else {
            cairo->set_source_rgba(1, 1, 1, 1);
          }
          cairo->rectangle(r.get_x(), r.get_y(), r.get_width(), r.get_height());
          cairo->stroke();
          if (evb::Connector::to_string(static_cast<evb::Connector::Enum>(i), txt)) {
            Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
            layout->set_font_description(fdesc);
            layout->set_text(txt);
            //cairo->move_to(r.get_x()+r.get_width()/2, r.get_y()+r.get_height());
            cairo->move_to(r.get_x()+p.get_x(), r.get_y()+r.get_height()+p.get_y());
            layout->show_in_cairo_context(cairo);
          }
          if (evb::Connector::has_pin1(static_cast<evb::Connector::Enum>(i))) {
            Glib::RefPtr<Pango::Layout> layout1 = Pango::Layout::create(cairo);
            layout1->set_font_description(fdesc1);
            layout1->set_text(txt1);
            cairo->move_to(r.get_x()+1, r.get_y()+y1);
            layout1->show_in_cairo_context(cairo);
          }
        }
      }
    }

    if (show_jumper_) {
      for (i=0; i<evb::Jumper::limit; ++i) {
        if (model_->jumper(static_cast<evb::Jumper::Enum>(i)) && jumper_rect(i, r)) {
          if (jumper_highlight_[i]) {
            cairo->set_source_rgba(0.6, 0.1, 0.1, 1);
          }
          else {
            cairo->set_source_rgba(0.1, 0.1, 0.1, 1);
          }
          cairo->rectangle(r.get_x(), r.get_y(), r.get_width(), r.get_height());
          cairo->fill();
        }
      }
    }

    if (show_host_pin_ || show_target_pin_) {
      Pango::FontDescription fdesc_pin;
      fdesc_pin.set_family("Consolas,monospace");
      fdesc_pin.set_size(static_cast<int>(1.2 * PANGO_SCALE));
      size_t j, size;
      bool has_text;

      for (i=0; i<evb::Pin::t_bus; ++i) {
        evb::Pin::Enum pin = static_cast<evb::Pin::Enum>(i);
        if ((show_host_pin_  && evb::Pin::host(pin)) || (show_target_pin_  && evb::Pin::target(pin))) {
          if (pin_rect(i, 0, r, p, size)) {
            if (pin_highlight_[i]) {
              cairo->set_source_rgba(1, 1, 0.1, 1);
            }
            else {
              cairo->set_source_rgba(0.8, 0.8, 0.8, 1);
            }
            cairo->rectangle(r.get_x(), r.get_y(), r.get_width(), r.get_height());
            cairo->stroke();
            has_text = evb::Pin::to_string(pin, txt);
            if (has_text) {
              Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
              layout->set_font_description(fdesc_pin);
              layout->set_text(txt);
              //cairo->move_to(r.get_x()+r.get_width()/2, r.get_y()+r.get_height());
              cairo->move_to(r.get_x()+p.get_x(), r.get_y()+p.get_y()-0.5);
              layout->show_in_cairo_context(cairo);
            }
            if (size > 1) {
              for (j=1; j<size; ++j) {
                if (pin_rect(i, j, r, p, size)) {
                  //if (pin_highlight_[i]) {
                  //  cairo->set_source_rgba(0.5, 0.5, 0.9, 1);
                  //}
                  //else {
                  //  cairo->set_source_rgba(0.5, 0.5, 0.5, 1);
                  //}
                  cairo->rectangle(r.get_x(), r.get_y(), r.get_width(), r.get_height());
                  cairo->stroke();
                  if (has_text) {
                    Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
                    layout->set_font_description(fdesc_pin);
                    layout->set_text(txt);
                    //cairo->move_to(r.get_x()+r.get_width()/2, r.get_y()+r.get_height());
                    cairo->move_to(r.get_x()+p.get_x(), r.get_y()+p.get_y()-0.5);
                    layout->show_in_cairo_context(cairo);
                  }
                }
              }
            }
          }
        }
      }
    }

    for (i=0; i<evb::Port::limit; ++i) {
      if (model_->has_connection(static_cast<evb::Port::Enum>(i)) && usb_rect(i, r)) {
        if (model_->connection(static_cast<evb::Port::Enum>(i))->connected()) {
          cairo->set_source_rgba(0, 1, 0, 0.9);
        }
        else {
          cairo->set_source_rgba(1, 0, 0, 0.9);
        }
        double w2 = r.get_width();
        w2 *= 0.5;
        double rad = w2 * 0.5;
        cairo->arc(
          r.get_x() + w2,
          r.get_y() + w2,
          rad,
          0,
          2*M_PI
        );
        cairo->fill();
      }
    }
  }
  cairo->restore();
}

void EvbView::on_area_size_changed(const Gdk::Rectangle& r)
{
  double zx = 1;
  double zy = 1;
  if (image_w_ > 0) {zx = r.get_width() / image_w_;}
  if (image_h_ > 0) {zy = r.get_height() / image_h_;}
  zoom_ = MIN(zx, zy);
}

bool EvbView::pin_rect(size_t pin, size_t pos, Gdk::Rectangle& rect, Gdk::Point& move_tag, size_t& size)
{
  size = 1;
  move_tag.set_x(-1);
  move_tag.set_y(-2);
  switch (pin) {
  // J23
  case evb::Pin::h_708_17:   rect = Gdk::Rectangle(112, 57,  2,  2); return true;
  case evb::Pin::h_708_1:    rect = Gdk::Rectangle(112, 62,  2,  2); return true;
  case evb::Pin::h_200_17:   rect = Gdk::Rectangle(112, 67,  2,  2); return true;
  case evb::Pin::h_100_17:   rect = Gdk::Rectangle(112, 72,  2,  2); return true;
  case evb::Pin::t_708_17:   rect = Gdk::Rectangle(112, 77,  2,  2); return true;
  case evb::Pin::t_708_1:    rect = Gdk::Rectangle(112, 82,  2,  2); return true;
  //J21
  case evb::Pin::h_617_ao:   rect = Gdk::Rectangle(164,174,  2,  2); return true;
  case evb::Pin::h_617_ai:   rect = Gdk::Rectangle(164,179,  2,  2); return true;
  case evb::Pin::h_517_17:   rect = Gdk::Rectangle(164,184,  2,  2); return true;
  case evb::Pin::h_417_17:   rect = Gdk::Rectangle(164,189,  2,  2); return true;
  case evb::Pin::h_317_17:   rect = Gdk::Rectangle(164,194,  2,  2); return true;
  case evb::Pin::h_217_17:   rect = Gdk::Rectangle(164,199,  2,  2); return true;
  case evb::Pin::h_117_ao:   rect = Gdk::Rectangle(164,204,  2,  2); return true;
  case evb::Pin::h_117_ai:   rect = Gdk::Rectangle(164,209,  2,  2); return true;
  // J27
  case evb::Pin::h_709_ao:   rect = Gdk::Rectangle(134,128,  2,  2); return true;
  case evb::Pin::h_709_ai:   rect = Gdk::Rectangle(139,128,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::h_713_ai:   rect = Gdk::Rectangle(144,128,  2,  2); return true;
  case evb::Pin::h_713_ao:   rect = Gdk::Rectangle(149,128,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::h_715_17:   rect = Gdk::Rectangle(154,128,  2,  2); return true;
  case evb::Pin::h_717_ao:   rect = Gdk::Rectangle(159,128,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::h_717_ai:   rect = Gdk::Rectangle(164,128,  2,  2); return true;
  // J30
  case evb::Pin::t_600_17:   rect = Gdk::Rectangle(123, 67,  2,  2); return true;
  case evb::Pin::t_500_17:   rect = Gdk::Rectangle(118, 72,  2,  2); return true;
  case evb::Pin::t_200_17:   rect = Gdk::Rectangle(118, 92,  2,  2); return true;
  case evb::Pin::t_100_17:   rect = Gdk::Rectangle(123, 92,  2,  2); move_tag.set_x(3); move_tag.set_y(0); return true;
  // J31
  case evb::Pin::t_008_5:    rect = Gdk::Rectangle(153,102,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::t_008_17:   rect = Gdk::Rectangle(153,107,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::t_008_1:    rect = Gdk::Rectangle(158,102,  2,  2); return true;
  case evb::Pin::t_008_3:    rect = Gdk::Rectangle(158,107,  2,  2); return true;
  // J32
  case evb::Pin::t_705_5:    rect = Gdk::Rectangle(128, 52,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::t_705_17:   rect = Gdk::Rectangle(128, 57,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::t_705_1:    rect = Gdk::Rectangle(133, 52,  2,  2); return true;
  case evb::Pin::t_705_3:    rect = Gdk::Rectangle(133, 57,  2,  2); return true;
  case evb::Pin::t_709_ai:   rect = Gdk::Rectangle(138, 52,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::t_709_ao:   rect = Gdk::Rectangle(138, 57,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::t_713_ao:   rect = Gdk::Rectangle(143, 52,  2,  2); return true;
  case evb::Pin::t_713_ai:   rect = Gdk::Rectangle(143, 57,  2,  2); return true;
  case evb::Pin::t_717_ao:   rect = Gdk::Rectangle(148, 52,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::t_715_17:   rect = Gdk::Rectangle(148, 57,  2,  2); move_tag.set_y(3); return true;
  case evb::Pin::t_617_ao:   rect = Gdk::Rectangle(163, 52,  2,  2); return true;
  case evb::Pin::t_717_ai:   rect = Gdk::Rectangle(153, 57,  2,  2); return true;
  // J36
  case evb::Pin::t_617_ai:   rect = Gdk::Rectangle(168, 62,  2,  2); move_tag.set_x(-4); return true;
  case evb::Pin::t_517_17:   rect = Gdk::Rectangle(173, 62,  2,  2); move_tag.set_x(-2); return true;
  case evb::Pin::t_417_17:   rect = Gdk::Rectangle(168, 67,  2,  2); move_tag.set_x(-4); return true;
  case evb::Pin::t_317_17:   rect = Gdk::Rectangle(173, 82,  2,  2); move_tag.set_x(-2); return true;
  case evb::Pin::t_217_17:   rect = Gdk::Rectangle(168, 87,  2,  2); move_tag.set_x(-4); return true;
  case evb::Pin::t_117_ao:   rect = Gdk::Rectangle(173, 87,  2,  2); move_tag.set_x(-2); return true;
  case evb::Pin::t_117_ai:   rect = Gdk::Rectangle(168, 92,  2,  2); move_tag.set_x(-4); return true;
  // J28
  case evb::Pin::t_701_17:   rect = Gdk::Rectangle(123, 52,  2,  2); move_tag.set_x(-2); return true;
  case evb::Pin::t_701_1:    rect = Gdk::Rectangle(123, 57,  2,  2); move_tag.set_x(-2); return true;

  default:;
  }
  size = 0;
  rect = Gdk::Rectangle(0,0,0,0);
  return false;
}

bool EvbView::connector_rect(size_t connector, Gdk::Rectangle& rect, Gdk::Point& move_tag, double& y1)
{
  y1 = 1;
  move_tag.set_x(0);
  move_tag.set_y(0);
  switch (connector) {
  case evb::Connector::J1:   rect = Gdk::Rectangle(  2, 20,  14, 103); return true;
  case evb::Connector::J2:   rect = Gdk::Rectangle(  0,130,  28,  18); move_tag.set_x(1); return true;
  case evb::Connector::J3:   rect = Gdk::Rectangle( 20,  0,  24,  32); move_tag.set_x(8); return true;
  case evb::Connector::J4:   rect = Gdk::Rectangle( 17, 34,   4,   4); return true;
  case evb::Connector::J5:   rect = Gdk::Rectangle( 17, 51,   4,   4); return true;
  case evb::Connector::J6:   rect = Gdk::Rectangle( 17, 68,   4,   4); return true;
  case evb::Connector::J7:   rect = Gdk::Rectangle( 23, 34,   4,   4); return true;
  case evb::Connector::J8:   rect = Gdk::Rectangle( 25, 60,  19,   4); return true;
  case evb::Connector::J9:   rect = Gdk::Rectangle( 54,  0,  24,  32); move_tag.set_x(8); return true;
  case evb::Connector::J10:  rect = Gdk::Rectangle( 55, 98,  14,   4); move_tag.set_y(-9); return true;
  case evb::Connector::J11:  rect = Gdk::Rectangle( 55,105,  14,   4); return true;
  case evb::Connector::J12:  rect = Gdk::Rectangle( 54, 34,   4,   4); return true;
  case evb::Connector::J13:  rect = Gdk::Rectangle( 51, 60,  19,   4); return true;
  case evb::Connector::J14:  rect = Gdk::Rectangle( 75, 98,  14,   4); move_tag.set_y(-9); return true;
  case evb::Connector::J15:  rect = Gdk::Rectangle( 75,103,  14,   4); move_tag.set_x(15); move_tag.set_y(-4); return true;
  case evb::Connector::J16:  rect = Gdk::Rectangle( 75,108,  14,   4); return true;
  case evb::Connector::J17:  rect = Gdk::Rectangle( 77, 60,  19,   4); return true;
  case evb::Connector::J18:  rect = Gdk::Rectangle( 88,  0,  24,  32); move_tag.set_x(8); return true;
  case evb::Connector::J19:  rect = Gdk::Rectangle( 83, 34,   4,   4); return true;
  case evb::Connector::J20:  rect = Gdk::Rectangle( 93,128,   9,   9); return true;
  case evb::Connector::J21:  rect = Gdk::Rectangle(163,173,   4,  39); return true;
  case evb::Connector::J22:  rect = Gdk::Rectangle(101, 86,  14,   9); y1 = 5; return true;
  case evb::Connector::J23:  rect = Gdk::Rectangle(106, 56,   9,  29); move_tag.set_x(-7); move_tag.set_y(-14); return true;
  case evb::Connector::J24:  rect = Gdk::Rectangle( 78, 51,   4,   4); return true;
  case evb::Connector::J25:  rect = Gdk::Rectangle(105,128,   4,  14); move_tag.set_x(-3); return true;
  case evb::Connector::J26:  rect = Gdk::Rectangle(110,133,   4,   9); return true;
  case evb::Connector::J27:  rect = Gdk::Rectangle(128,127,  39,   4); return true;
  case evb::Connector::J28:  rect = Gdk::Rectangle(117, 51,   9,   9); move_tag.set_y(-14); return true;
  case evb::Connector::J29:  rect = Gdk::Rectangle(117,101,   9,   9); return true;
  case evb::Connector::J30:  rect = Gdk::Rectangle(117, 61,   9,  39); move_tag.set_x(10); move_tag.set_y(-14); return true;
  case evb::Connector::J31:  rect = Gdk::Rectangle(137,101,  39,   9); y1 = 5; return true;
  case evb::Connector::J32:  rect = Gdk::Rectangle(127, 51,  39,   9); move_tag.set_x(10); y1 = 5; return true;
  case evb::Connector::J33:  rect = Gdk::Rectangle(154, 40,  19,   4); move_tag.set_y(-9); return true;
  case evb::Connector::J34:  rect = Gdk::Rectangle(148,121,   9,   4); move_tag.set_y(-9); return true;
  case evb::Connector::J35:  rect = Gdk::Rectangle(158,121,   9,   4); move_tag.set_y(-9); return true;
  case evb::Connector::J36:  rect = Gdk::Rectangle(167, 61,   9,  39); move_tag.set_x(-7); move_tag.set_y(-14); return true;
  case evb::Connector::J37:  rect = Gdk::Rectangle(175,144,   9,   9); return true;
  case evb::Connector::J38:  rect = Gdk::Rectangle(177, 35,   4,  24); move_tag.set_x(-7); move_tag.set_y(-9); return true;
  case evb::Connector::J39:  rect = Gdk::Rectangle(177,121,   4,  14); return true;
  case evb::Connector::J40:  rect = Gdk::Rectangle(182, 35,   4,  24); return true;
  case evb::Connector::J41:  rect = Gdk::Rectangle(185,210,  11,  11); return true;
  case evb::Connector::J48:  rect = Gdk::Rectangle(188,200,   9,   4); return true;
  case evb::Connector::J49:  rect = Gdk::Rectangle(205,210,  11,  11); return true;
  case evb::Connector::J50:  rect = Gdk::Rectangle(208,200,   9,   4); return true;
  case evb::Connector::J51:  rect = Gdk::Rectangle(225,210,  11,  11); return true;
  case evb::Connector::J52:  rect = Gdk::Rectangle(224,  0,  62,  16); move_tag.set_x(2); move_tag.set_y(-5); return true;
  case evb::Connector::J53:  rect = Gdk::Rectangle(228,200,   9,   4); return true;
  case evb::Connector::J54:  rect = Gdk::Rectangle(228, 20,   4,  14); move_tag.set_x(-7); move_tag.set_y(-14); return true;
  case evb::Connector::J55:  rect = Gdk::Rectangle(245,210,  11,  11); return true;
  case evb::Connector::J56:  rect = Gdk::Rectangle(248,200,   9,   4); return true;
  case evb::Connector::J57:  rect = Gdk::Rectangle(281, 35,  19,   4); move_tag.set_x(-7); move_tag.set_y(-4); return true;
  case evb::Connector::J58:  rect = Gdk::Rectangle(287,  0,  62,  16); move_tag.set_x(2); move_tag.set_y(-5); return true;
  case evb::Connector::J59:  rect = Gdk::Rectangle(306, 20,   4,  14); move_tag.set_x(-7); move_tag.set_y(-14); return true;
  case evb::Connector::J60:  rect = Gdk::Rectangle(339,184,  19,   4); return true;
  case evb::Connector::J61:  rect = Gdk::Rectangle(339,207,  19,   4); return true;
  case evb::Connector::J62:  rect = Gdk::Rectangle(346,151,  34,   9); move_tag.set_x(2); move_tag.set_y(-5); return true;
  case evb::Connector::J63:  rect = Gdk::Rectangle(346,173,  34,   9); move_tag.set_x(2); move_tag.set_y(-5); return true;
  case evb::Connector::J64:  rect = Gdk::Rectangle(346,196,  34,   9); move_tag.set_x(2); move_tag.set_y(-5); return true;
  case evb::Connector::J65:  rect = Gdk::Rectangle(339,162,  19,   4); return true;
  case evb::Connector::J66:  rect = Gdk::Rectangle(342,114,   4,  24); move_tag.set_x(-2); return true;
  case evb::Connector::J67:  rect = Gdk::Rectangle(342, 38,   4,  29); return true;
  case evb::Connector::J68:  rect = Gdk::Rectangle(342, 89,   4,  14); return true;
  case evb::Connector::J69:  rect = Gdk::Rectangle(347,112,  33,  31); move_tag.set_x(2); move_tag.set_y(-5); return true;
  case evb::Connector::J70:  rect = Gdk::Rectangle(352, 17,  28,  63); move_tag.set_x(2); move_tag.set_y(-5); return true;
  case evb::Connector::J71:  rect = Gdk::Rectangle(353, 83,  27,  25); move_tag.set_x(2); move_tag.set_y(-5); return true;
  case evb::Connector::J74:  rect = Gdk::Rectangle( 64,210,  11,  11); return true;
  case evb::Connector::J75:  rect = Gdk::Rectangle( 84,210,  11,  11); return true;

  case evb::Connector::JP1:  rect = Gdk::Rectangle(122,  2,  54,  32); return true;



  default:;
  }
  rect = Gdk::Rectangle(0,0,0,0);
  return false;
}


bool EvbView::jumper_rect(size_t jumper, Gdk::Rectangle& rect)
{
  switch (jumper) {
  case evb::Jumper::J10_1_2:   rect = Gdk::Rectangle( 55, 98,  9,  4); return true;
  case evb::Jumper::J10_2_3:   rect = Gdk::Rectangle( 60, 98,  9,  4); return true;
  case evb::Jumper::J11_1_2:   rect = Gdk::Rectangle( 55,105,  9,  4); return true;
  case evb::Jumper::J11_2_3:   rect = Gdk::Rectangle( 60,105,  9,  4); return true;
  case evb::Jumper::J14_1_2:   rect = Gdk::Rectangle( 75, 98,  9,  4); return true;
  case evb::Jumper::J14_2_3:   rect = Gdk::Rectangle( 80, 98,  9,  4); return true;
  case evb::Jumper::J15_1_2:   rect = Gdk::Rectangle( 75,103,  9,  4); return true;
  case evb::Jumper::J15_2_3:   rect = Gdk::Rectangle( 80,103,  9,  4); return true;
  case evb::Jumper::J16_1_2:   rect = Gdk::Rectangle( 75,108,  9,  4); return true;
  case evb::Jumper::J16_2_3:   rect = Gdk::Rectangle( 80,108,  9,  4); return true;
  case evb::Jumper::J20_1_3:   rect = Gdk::Rectangle( 93,128,  4,  9); return true;
  case evb::Jumper::J20_2_4:   rect = Gdk::Rectangle( 98,128,  4,  9); return true;
  case evb::Jumper::J22_1_2:   rect = Gdk::Rectangle(101, 86,  4,  9); return true;
  case evb::Jumper::J22_3_4:   rect = Gdk::Rectangle(106, 86,  4,  9); return true;
  case evb::Jumper::J22_5_6:   rect = Gdk::Rectangle(111, 86,  4,  9); return true;
  case evb::Jumper::J23_1_2:   rect = Gdk::Rectangle(106, 56,  9,  4); return true;
  case evb::Jumper::J23_3_4:   rect = Gdk::Rectangle(106, 61,  9,  4); return true;
  case evb::Jumper::J23_5_6:   rect = Gdk::Rectangle(106, 66,  9,  4); return true;
  case evb::Jumper::J23_7_8:   rect = Gdk::Rectangle(106, 71,  9,  4); return true;
  case evb::Jumper::J23_9_10:  rect = Gdk::Rectangle(106, 76,  9,  4); return true;
  case evb::Jumper::J23_11_12: rect = Gdk::Rectangle(106, 81,  9,  4); return true;
  case evb::Jumper::J25_1_2:   rect = Gdk::Rectangle(105,128,  4,  9); return true;
  case evb::Jumper::J25_2_3:   rect = Gdk::Rectangle(105,133,  4,  9); return true;
  case evb::Jumper::J26_1_2:   rect = Gdk::Rectangle(110,133,  4,  9); return true;
  case evb::Jumper::J34_1_2:   rect = Gdk::Rectangle(148,117,  9,  4); return true;
  case evb::Jumper::J35_1_2:   rect = Gdk::Rectangle(158,117,  9,  4); return true;
  case evb::Jumper::J37_1_2:   rect = Gdk::Rectangle(175,144,  9,  4); return true;
  case evb::Jumper::J37_3_4:   rect = Gdk::Rectangle(175,149,  9,  4); return true;
  case evb::Jumper::J38_1_J40_1:   rect = Gdk::Rectangle(177, 35,  9,  4); return true;
  case evb::Jumper::J38_2_J40_2:   rect = Gdk::Rectangle(177, 40,  9,  4); return true;
  case evb::Jumper::J38_3_J40_3:   rect = Gdk::Rectangle(177, 45,  9,  4); return true;
  case evb::Jumper::J38_4_J40_4:   rect = Gdk::Rectangle(177, 50,  9,  4); return true;
  case evb::Jumper::J38_5_J40_5:   rect = Gdk::Rectangle(177, 55,  9,  4); return true;
  case evb::Jumper::J39_1_2:   rect = Gdk::Rectangle(177,121,  4,  9); return true;
  case evb::Jumper::J39_2_3:   rect = Gdk::Rectangle(177,126,  4,  9); return true;
  default:;
  }
  rect = Gdk::Rectangle(0,0,0,0);
  return false;
}

bool EvbView::usb_rect(size_t usb, Gdk::Rectangle& rect)
{
  switch (usb) {
  case evb::Port::A: rect = Gdk::Rectangle( 20,  0, 24, 44); return true;
  case evb::Port::B: rect = Gdk::Rectangle( 54,  0, 24, 44); return true;
  case evb::Port::C: rect = Gdk::Rectangle( 88,  0, 24, 44); return true;
  default:;
  }
  rect = Gdk::Rectangle(0,0,0,0);
  return false;
}

bool EvbView::chip_rect(size_t chip, Gdk::Rectangle& rect)
{
  switch (chip) {
  case evb::Chip::host:   rect = Gdk::Rectangle(110,164, 20, 20); return true;
  case evb::Chip::target: rect = Gdk::Rectangle(138, 74, 20, 20); return true;
  default:;
  }
  rect = Gdk::Rectangle(0,0,0,0);
  return false;
}

bool EvbView::inside_rect(int x, int y, const Gdk::Rectangle& rect)
{
  if (rect.has_zero_area()) {return false;}
  if (x < rect.get_x()) {return false;}
  if (y < rect.get_y()) {return false;}
  if (x >= (rect.get_x() + rect.get_width())) {return false;}
  if (y >= (rect.get_y() + rect.get_height())) {return false;}
  return true;
}



bool EvbView::on_button_press_event(GdkEventButton* evnt)
{
  if (evnt->type == GDK_BUTTON_PRESS && (model_ != 0)) {
    size_t i;
    Gdk::Rectangle rect;
    double mx = evnt->x;
    double my = evnt->y;
    mx /= zoom_;
    my /= zoom_;
    int ix = static_cast<int>(floor(mx));
    int iy = static_cast<int>(floor(my));
    if (evnt->button == 1) { // left button
      if (show_jumper_) {
        size_t j1 = evb::Jumper::limit;
        size_t j2 = evb::Jumper::limit;
        for (i=0; i<evb::Jumper::limit; ++i) {
          if (jumper_rect(i, rect) && inside_rect(ix, iy, rect)) {
            if (j1 == evb::Jumper::limit) {j1 = i;}
            else if (j2 == evb::Jumper::limit) {j2 = i;}
          }
        }
        if (j1 != evb::Jumper::limit) {
          if (j2 != evb::Jumper::limit) {
            if (model_->jumper(static_cast<evb::Jumper::Enum>(j1))) {
              model_->jumper(static_cast<evb::Jumper::Enum>(j1), false);
              model_->jumper(static_cast<evb::Jumper::Enum>(j2), true);
            }
            else {
              model_->jumper(static_cast<evb::Jumper::Enum>(j1), true);
              model_->jumper(static_cast<evb::Jumper::Enum>(j2), false);
            }
          }
          else {
            model_->jumper(static_cast<evb::Jumper::Enum>(j1), !model_->jumper(static_cast<evb::Jumper::Enum>(j1)));
          }
          queue_draw(); // refresh
          return true;
        }
      }
      for (i=0; i<evb::Port::limit; ++i) {
        if (usb_rect(i, rect) && inside_rect(ix, iy, rect) && model_->has_connection(static_cast<evb::Port::Enum>(i))) {
          connection::Serial_ptr s = model_->connection(static_cast<evb::Port::Enum>(i));
          if (s->connected()) {
            get_window()->set_cursor(Gdk::Cursor::create(Gdk::WATCH));
            s->close();
            get_window()->set_cursor(Gdk::Cursor::create(Gdk::ARROW));
            queue_draw(); // refresh
          }
          else {
            get_window()->set_cursor(Gdk::Cursor::create(Gdk::WATCH));
            if (s->connect()) {
              get_window()->set_cursor(Gdk::Cursor::create(Gdk::ARROW));
              queue_draw(); // refresh
            }
            else {
              get_window()->set_cursor(Gdk::Cursor::create(Gdk::ARROW));
              Gtk::MessageDialog dialog(window_, "Port open failed");
              dialog.set_secondary_text("The port could not be connected. Please check the cable and communication parameter.");
              dialog.run();
            }
          }
        }
      }
      for (i=0; i<evb::Chip::limit; ++i) {
        if (chip_rect(i, rect) && inside_rect(ix, iy, rect)) {
          if (i == evb::Chip::host) {
            //if (host_window_ == 0) {
            //  std::string name;
            //  name = model_->name();
            //  name += " - host";
            //  host_window_ = new Ga144Window(*this, name, model_->chip(i));
            //  host_window_->name(name);
            //}
            assert_host_window().show();
            host_window_->present();
          }
          else if (i == evb::Chip::target) {
            //if (target_window_ == 0) {
            //  std::string name;
            //  name = model_->name();
            //  name += " - target";
            //  target_window_ = new Ga144Window(*this, name, model_->chip(i));
            //  target_window_->name(name);
            //}
            assert_target_window().show();
            target_window_->present();
          }
          return true;
        }
      }
    }
    else if (evnt->button == 2) { // middle button
    }
    else if (evnt->button == 3) { // right button
      for (i=0; i<evb::Port::limit; ++i) {
        if (usb_rect(i, rect) && inside_rect(ix, iy, rect)) {
          if (connection_ == 0) {
            connection_ = new ConnectionWindow();
          }
          if (!model_->has_connection(static_cast<evb::Port::Enum>(i))) {
            // we must create a connection and set the dispatcher
            model_->assert_connection(static_cast<evb::Port::Enum>(i))->register_dispatcher(connection_status_change_dispatcher_);
          }
          connection_->write(model_->assert_connection(static_cast<evb::Port::Enum>(i)));
          connection_->setup(*this, i);
          connection_->show();
          return true;
        }
      }
    }
  }
  return false; // event not processed
}

void EvbView::update(ConnectionWindow& conn, size_t port)
{
  evb::Port::Enum p = static_cast<evb::Port::Enum>(port);
  if (model_->has_connection(p)) {
    conn.read(model_->connection(p).get());
  }
  queue_draw();
}

bool EvbView::on_scroll_event(GdkEventScroll *ev)
{
  return false; // event not processed
}

void EvbView::name(const std::string& n)
{
  model_->name(n);
  if (host_window_) {
    std::string val = n;
    val += " - host";
    host_window_->name(val);
  }
  if (target_window_) {
    std::string val = n;
    val += " - target";
    target_window_->name(val);
  }
}

const std::string& EvbView::name() const
{
  return model_->name();
}

bool EvbView::on_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip)
{
  double mx = x;
  double my = y;
  mx /= zoom_;
  my /= zoom_;
  Gdk::Rectangle r;
  Gdk::Point p;
  size_t i, size;
  std::string txt, ptxt, descr;
  for (i=0; i<evb::Pin::t_bus; ++i) {
    evb::Pin::Enum pin = static_cast<evb::Pin::Enum>(i);
    if (pin_rect(i, 0, r, p, size)) {
      if (inside_rect(static_cast<int>(mx), static_cast<int>(my), r) && evb::Pin::to_string(pin, ptxt)) {
        txt = "<span font_family=\"Consolas,monospace\" foreground=\"blue\" size=\"140\"><b>";
        if (evb::Pin::host(pin)) {
          txt += "host: ";
        }
        else {
          txt += "target: ";
        }
        txt += ptxt;
        if (evb::Pin::to_description(pin, descr)) {
          txt += " (";
          txt += descr;
          txt += ")";
        }
        txt += "</b></span>";
        tooltip->set_markup(txt);
        return true;
      }
    }
  }
  return false;
}

F18aWindow* EvbView::node_window(size_t node, bool assert)
{
  if (node >= GA144_CHIP_OFFSET) {
    return assert_target_window().node_window(node - GA144_CHIP_OFFSET, assert);
  }
  return assert_host_window().node_window(node, assert);
}

Ga144Window& EvbView::assert_host_window()
{
  if (host_window_ == 0) {
    std::string name;
    name = model_->name();
    name += " - host";
    host_window_ = new Ga144Window(*this, name, model_->chip(evb::Chip::host));
    host_window_->name(name);
  }
  return *host_window_;
}

Ga144Window& EvbView::assert_target_window()
{
  if (target_window_ == 0) {
    std::string name;
    name = model_->name();
    name += " - target";
    target_window_ = new Ga144Window(*this, name, model_->chip(evb::Chip::target));
    target_window_->name(name);
  }
  return *target_window_;
}


}
