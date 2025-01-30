#include <studio.hpp>
#include <connection.hpp>
#include <evb.hpp>
#include "boost/filesystem.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

#define XOFF  4
#define YOFF  4

#define NODE_WIDTH  40
#define NODE_HEIGHT  60

#define PORT_LINE_SIZE  10

#define TOP_MARGIN  10
#define BOTTOM_MARGIN  10
#define LEFT_MARGIN  10
#define RIGHT_MARGIN  10

namespace studio {

bool Ga144View::node_rect(size_t col, size_t row, Gdk::Rectangle& rect)
{
  if ((col < GA144_PROCESSOR_COLS) && (row < GA144_PROCESSOR_ROWS)) {
    rect = Gdk::Rectangle(
      col * NODE_WIDTH + LEFT_MARGIN,
      (GA144_PROCESSOR_ROWS - 1 - row) * NODE_HEIGHT + TOP_MARGIN,
      NODE_WIDTH - LEFT_MARGIN - RIGHT_MARGIN,
      NODE_HEIGHT - TOP_MARGIN - BOTTOM_MARGIN);
    return true;
  }
  rect = Gdk::Rectangle(0,0,0,0);
  return false;
}

size_t Ga144View::node_to_pos(size_t col, size_t row)
{
  if ((col < GA144_PROCESSOR_COLS) && (row < GA144_PROCESSOR_ROWS)) {
    return model_->index(col, row);
  }
  return 0;
}

size_t Ga144View::node(size_t col, size_t row, bool with_chip)
{
  if ((col < GA144_PROCESSOR_COLS) && (row < GA144_PROCESSOR_ROWS)) {
    size_t index = model_->index(col, row);
    size_t node = model_->node(index);
    if (with_chip) {
      return window_.owner().model()->join_node(model_, node);
    }
    return node;
  }
  return 0;
}


void Ga144View::node_to_name(size_t col, size_t row, std::string& name)
{
  name.clear();
  if ((col < GA144_PROCESSOR_COLS) && (row < GA144_PROCESSOR_ROWS)) {
    name = boost::lexical_cast<std::string>(model_->processor(model_->index(col, row))->node());
    while (name.size() < 3) {name.insert(0, "0");}
    //name = boost::lexical_cast<std::string>(col);
    //if (name.size() < 2) {name.insert(0, "0");}
    //name.insert(0, boost::lexical_cast<std::string>(row));
  }
}


size_t Ga144View::width()
{
  return NODE_WIDTH * GA144_PROCESSOR_COLS;
}

size_t Ga144View::height()
{
  return NODE_HEIGHT * GA144_PROCESSOR_ROWS;
}

// ======================================================================================

Ga144View::Ga144View(Ga144Window& w, ga144::Chip_ptr m)
: window_(w)
, model_(m)
, node_menu_(0)
, node_index_(0)
, selected_node_(0)
{
  //signal_size_allocate().connect(sigc::mem_fun(*this, &Ga144View::on_area_size_changed));

  //for (size_t i=0; i<GA144_PROCESSOR_SIZE; ++i) {
  //  model_->processor(i)->register_modify_dispatcher(modify_dispatcher_);
  //  model_->processor(i)->register_change_dispatcher(change_dispatcher_);
  //}

  model_->register_modify_dispatcher(modify_dispatcher_);
  model_->register_change_dispatcher(change_dispatcher_);

  modify_dispatcher_.connect(sigc::mem_fun(*this, &Ga144View::on_modify));
  change_dispatcher_.connect(sigc::mem_fun(*this, &Ga144View::on_changed));

}

Ga144View::~Ga144View()
{
}

void Ga144View::modify()
{
  window_.modify();
}

void Ga144View::read(xml::Iterator& it)
{
	//while (it.current() == xml::Element::tag) {
	//	if (it.tag() == "a") {
 //     if (connection_window_list_[0] == 0) {connection_window_list_[0] = new ConnectionWindow();}
 //     connection_window_list_[0]->read(it);
 //   	if ((it.current() != xml::Element::end) || (it.tag() != "a")) {throw xml::exception("Ga144View::read missing </a>");}
	//	}
	//	else if (it.tag() == "b") {
 //     if (connection_window_list_[1] == 0) {connection_window_list_[1] = new ConnectionWindow();}
 //     connection_window_list_[1]->read(it);
 //   	if ((it.current() != xml::Element::end) || (it.tag() != "b")) {throw xml::exception("Ga144View::read missing </b>");}
	//	}
	//	else if (it.tag() == "c") {
 //     if (connection_window_list_[2] == 0) {connection_window_list_[2] = new ConnectionWindow();}
 //     connection_window_list_[2]->read(it);
 //   	if ((it.current() != xml::Element::end) || (it.tag() != "c")) {throw xml::exception("Ga144View::read missing </c>");}
	//	}
 // }
}

void Ga144View::write(xml::Writer& w)
{
  //if (connection_window_list_[0] != 0) {
  //  w.open("a", false, true);
  //  w.newline(1);
  //  connection_window_list_[0]->write(w);
  //  w.indent(-1);
  //  w.open("a", true, true);
  //  w.newline();
  //}
  //if (connection_window_list_[1] != 0) {
  //  w.open("b", false, true);
  //  w.newline(1);
  //  connection_window_list_[1]->write(w);
  //  w.indent(-1);
  //  w.open("b", true, true);
  //  w.newline();
  //}
  //if (connection_window_list_[2] != 0) {
  //  w.open("c", false, true);
  //  w.newline(1);
  //  connection_window_list_[2]->write(w);
  //  w.indent(-1);
  //  w.open("c", true, true);
  //  w.newline();
  //}
}

#define SPIKE_LEN 10

bool Ga144View::on_draw (const ::Cairo::RefPtr<::Cairo::Context>& cairo)
{
  Glib::RefPtr<Pango::Layout> pl = Pango::Layout::create(cairo);
  draw(cairo, pl, XOFF, YOFF);
  return true;
}

void Ga144View::draw_node(
  f18a::Processor_ptr proc,
  long info,
  NodeType_ptr type,
  const Cairo::RefPtr<Cairo::Context>& cairo,
  const Glib::RefPtr<Pango::Layout>& pango,
  Pango::FontDescription& fdesc,
  double x,
  double y,
  double w,
  double h)
{
  //std::string ident = boost::lexical_cast<std::string>(proc.column());
  //if (ident.size() < 2) {ident.insert(0, "0");}
  //ident.insert(0, boost::lexical_cast<std::string>(GA144_PROCESSOR_ROWS - proc.row() - 1));
  std::string ident;
  node_to_name(proc->column(), proc->row(), ident);

  x += LEFT_MARGIN;
  y += TOP_MARGIN;
  w -= LEFT_MARGIN + RIGHT_MARGIN;
  h -= TOP_MARGIN + BOTTOM_MARGIN;

  if ((info != (1 << ga144::InfoBit::warm)) && (info != (1 << ga144::InfoBit::reset))) {
    bool fill_rect = false;
    if (info == (1 << ga144::InfoBit::umbilical)) {
      // umbilical node
      cairo->set_source_rgba(1, 1, 0, 0.5);
      fill_rect = true;
    }
    else if (info == (1 << ga144::InfoBit::kraken)) {
      // Kraken node
      cairo->set_source_rgba(0, 0, 1, 0.35);
      fill_rect = true;
    }
    else if (info == (1 << ga144::InfoBit::active_kraken)) {
      // Kraken node
      cairo->set_source_rgba(0, 0, 1, 0.5);
      fill_rect = true;
    }
    else if (info == (1 << ga144::InfoBit::tentacle)) {
      // tentacle node
      cairo->set_source_rgba(0, 0, 1, 0.2);
      fill_rect = true;
    }
    else if (info == (1 << ga144::InfoBit::bridge)) {
      // tentacle node
      cairo->set_source_rgba(1, 0, 1, 0.5);
      fill_rect = true;
    }
    else if (info == (1 << ga144::InfoBit::running)) {
      // tentacle node
      cairo->set_source_rgba(1, 0, 0, 0.3);
      fill_rect = true;
    }
    else {
      // normal node
      cairo->set_source_rgba(0, 0, 0, 0.7);
      fill_rect = true;
    }
    if (fill_rect) {
      cairo->rectangle(x, y, w, h);
      cairo->fill();
    }
  }

  cairo->set_source_rgba(0, 0, 0, 1);
  cairo->rectangle(x, y, w, h);
  cairo->stroke();


  Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
  layout->set_font_description(fdesc);
  layout->set_text(ident);

  int text_width;
  int text_height;

  //get the text dimensions (it updates the variables -- by reference)
  layout->get_pixel_size(text_width, text_height);

  // Position the text in the middle
  //cairo->move_to(x+(NODE_WIDTH-text_width)/2, y+(NODE_HEIGHT-text_height)/2);

  // Position the text at top+center
  cairo->move_to(x+(w-text_width)/2, y+h-text_height);
  layout->show_in_cairo_context(cairo);

  int tentacle_index = proc->tentacle_index();
  if (tentacle_index >= 0) {
    layout = Pango::Layout::create(cairo);
    layout->set_font_description(fdesc);
    layout->set_text(boost::lexical_cast<std::string>(tentacle_index));
    cairo->move_to(x+2, y+2);
    layout->show_in_cairo_context(cairo);
  }


  double p1x, p1y, p2x, p2y, p3x, p3y;
  bool draw_triangle = false;

  size_t port = f18a::Port::from_addr(proc->module()->reg().named_.B_);
  if (port < f18a::Port::limit) {
    if ((port == f18a::Port::up) || (port == f18a::Port::down)) {
      if ((proc->row() & 1) == 0) {
        port ^= 2;
      }
    }
    else {
      if ((proc->column() & 1) != 0) {
        port ^= 2;
      }
    }
    switch (port) {
    case f18a::Port::up:
      p1x = x + w/2;
      p1y = y - 2 - SPIKE_LEN;
      p2x = p1x + 3;
      p2y = p1y + SPIKE_LEN;
      p3x = p1x - 3;
      p3y = p1y + SPIKE_LEN;
      draw_triangle = true;
      break;

    case f18a::Port::left:
      p1y = y + h/2;
      p1x = x - 2 - SPIKE_LEN;
      p2x = p1x + SPIKE_LEN;
      p2y = p1y + 3;
      p3x = p1x + SPIKE_LEN;
      p3y = p1y - 3;
      draw_triangle = true;
      break;

    case f18a::Port::right:
      p1y = y + h/2;
      p1x = x + w + 2 + SPIKE_LEN;
      p2x = p1x - SPIKE_LEN;
      p2y = p1y + 3;
      p3x = p1x - SPIKE_LEN;
      p3y = p1y - 3;
      draw_triangle = true;
      break;

    case f18a::Port::down:
      p1x = x + w/2;
      p1y = y + h + 2 + SPIKE_LEN;
      p2x = p1x + 3;
      p2y = p1y - SPIKE_LEN;
      p3x = p1x - 3;
      p3y = p1y - SPIKE_LEN;
      draw_triangle = true;
      break;


    default:;
    }
  }
  if (draw_triangle) {
    cairo->set_source_rgba(0, 0, 0, 0.85);
    cairo->move_to(p1x, p1y);
    cairo->line_to(p2x, p2y);
    cairo->line_to(p3x, p3y);
    cairo->close_path();
    cairo->fill();
  }



}

void Ga144View::draw(const Cairo::RefPtr<Cairo::Context>& cairo, const Glib::RefPtr<Pango::Layout>& pango, double x, double y)
{
  cairo->save();

  Pango::FontDescription fdesc;
  fdesc.set_family("Consolas,monospace");
  fdesc.set_size(8 * PANGO_SCALE);

  double nx, ny;
  ny = y;
  size_t ind;
  {
    boost::mutex::scoped_lock lock(model_->mutex());
    for (size_t row=0; row<GA144_PROCESSOR_ROWS; ++row) {
      nx = x;
      for (size_t col=0; col<GA144_PROCESSOR_COLS; ++col) {
        ind = model_->index(col, GA144_PROCESSOR_ROWS-1-row);
        f18a::Processor_ptr proc = model_->processor(ind);
        //cairo->save();
        draw_node(proc, model_->info(ind), model_->node_type(ind), cairo, pango, fdesc, nx, ny, NODE_WIDTH, NODE_HEIGHT);
        //cairo->restore();

        NodeType_ptr type = model_->node_type(ind);
        if (type != 0) {
          type->draw(cairo, pango, fdesc, nx, ny, NODE_WIDTH, NODE_HEIGHT);
        }

        nx += NODE_WIDTH;
      }
      ny += NODE_HEIGHT;
    }
  }

  double gw = width();
  double gh = height();
  // draw port lines
  //ny = y;
  //for (size_t row=0; row<GA144_PROCESSOR_ROWS; ++row) {
  nx = x;
  for (size_t col=0; col<GA144_PROCESSOR_COLS+1; ++col) {
    if (col & 1) {
      cairo->set_source_rgba(1.0, 0.5, 0.3, 0.25);
    }
    else {
      cairo->set_source_rgba(0.3, 1.0, 0.5, 0.25);
    }
    cairo->rectangle(nx-PORT_LINE_SIZE/2, y, PORT_LINE_SIZE, gh);
    cairo->fill();
    nx += NODE_WIDTH;
  }
  ny = y;
  for (size_t row=0; row<GA144_PROCESSOR_ROWS+1; ++row) {
    if (row & 1) {
      cairo->set_source_rgba(0.2, 1.0, 1.0, 0.25);
    }
    else {
      cairo->set_source_rgba(1.0, 1.0, 0.2, 0.25);
    }
    cairo->rectangle(x, ny-PORT_LINE_SIZE/2, gw, PORT_LINE_SIZE);
    cairo->fill();
    ny += NODE_HEIGHT;
  }
  //  ny += NODE_HEIGHT;
  //}


  cairo->restore();
}

void Ga144View::on_modify()
{
  queue_draw();
}

//void Ga144View::modify()
//{
//  queue_draw();
//}

void Ga144View::on_changed()
{
  changed();
}

void Ga144View::changed()
{
  queue_draw();
}

F18aWindow& Ga144View::assert_window(size_t ind)
{
  if (ind >= node_window_list_.size()) {
    node_window_list_.resize(ind+1);
  }
  //while (node_window_list_.size() <= ind) {
  //  node_window_list_.push_back(0);
  //}
  if (node_window_list_[ind] == 0) {
    std::string name, node;
    f18a::Processor_ptr proc = model_->processor(ind);
    size_t row = proc->row();
    size_t col = proc->column();
    node_to_name(col, row, node);
    name = name_;
    name += " - ";
    name += node;
    node_window_list_[ind] = new F18aWindow(*this, name, *model_->processor(model_->index(col, row)));
  }
  return *node_window_list_[ind];
}

bool Ga144View::on_button_press_event(GdkEventButton* evnt)
{
  Gdk::Rectangle rect;
  double mx = evnt->x;
  double my = evnt->y;
  int ix = static_cast<int>(floor(mx));
  int iy = static_cast<int>(floor(my));
  if (evnt->type == GDK_BUTTON_PRESS) {
    if (evnt->button == 1) { // left button
      for (size_t row=0; row<GA144_PROCESSOR_ROWS; ++row) {
        for (size_t col=0; col<GA144_PROCESSOR_COLS; ++col) {
          if (node_rect(col, row, rect) && EvbView::inside_rect(ix, iy, rect)) {
            F18aWindow& win = assert_window(node_to_pos(col, row));
            win.show();
            win.present();
            //size_t pos = node_to_pos(col, row);
            //if (pos >= node_window_list_.size()) {
            //  node_window_list_.resize(pos+1);
            //}
            //if (node_window_list_[pos] == 0) {
            //  std::string name, node;
            //  node_to_name(col, row, node);
            //  name = name_;
            //  name += " - ";
            //  name += node;
            //  node_window_list_[pos] = new F18aWindow(*this, name, *model_->processor(model_->index(col, row)));
            //}
            //node_window_list_[pos]->show();
            //node_window_list_[pos]->present();
            return true;
          }
        }
      }
    }
    else if (evnt->button == 2) { // middle button
      for (size_t row=0; row<GA144_PROCESSOR_ROWS; ++row) {
        for (size_t col=0; col<GA144_PROCESSOR_COLS; ++col) {
          if (node_rect(col, row, rect) && EvbView::inside_rect(ix, iy, rect)) {
            node_index_ = node_to_pos(col, row);
            selected_node_ = node(col, row, true);
            if (&window_.owner().window().manager() != 0) {
              long info = model_->info(node_index_);

              if ((info == (1 << ga144::InfoBit::warm)) || (info == (1 << ga144::InfoBit::reset))) {
                window_.owner().window().manager()->do_expand_tentacle_to(selected_node_);
              }
              else if (info == (1 << ga144::InfoBit::kraken)) {
                window_.owner().window().manager()->do_activate_kraken(selected_node_);
              }
              else if (info == (1 << ga144::InfoBit::active_kraken)) {
                window_.owner().window().manager()->do_destroy_tentacle();
              }
              else if (info == (1 << ga144::InfoBit::tentacle)) {
                window_.owner().window().manager()->do_reduce_tentacle_to(selected_node_);
              }
            }
            return true;
          }
        }
      }
    }
    else if (evnt->button == 3) { // right button
      for (size_t row=0; row<GA144_PROCESSOR_ROWS; ++row) {
        for (size_t col=0; col<GA144_PROCESSOR_COLS; ++col) {
          if (node_rect(col, row, rect) && EvbView::inside_rect(ix, iy, rect)) {
            node_index_ = node_to_pos(col, row);
            selected_node_ = node(col, row, true);
            if (node_menu_ == 0) {
              node_menu_ = new Gtk::Menu();
              Gtk::MenuItem* item = new Gtk::MenuItem("Read all");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_read_node));
              item = new Gtk::MenuItem("Read RAM");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_read_ram));
              item = new Gtk::MenuItem("Read ROM");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_read_rom));
              item = new Gtk::MenuItem("Read RAM & ROM");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_read_ram_and_rom));
              item = new Gtk::MenuItem("Read par stack");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_read_stack));
              item = new Gtk::MenuItem("Read ret stack");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_read_return_stack));
              item = new Gtk::MenuItem("Read A");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_read_a));
              item = new Gtk::MenuItem("Read I/O");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_read_io));
              item = new Gtk::MenuItem("Write all");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_write_node));
              item = new Gtk::MenuItem("Write RAM");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_write_ram));
              item = new Gtk::MenuItem("Write par stack");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_write_stack));
              item = new Gtk::MenuItem("Write ret stack");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_write_return_stack));
              item = new Gtk::MenuItem("Write A");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_write_a));
              item = new Gtk::MenuItem("Write B");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_write_b));
              item = new Gtk::MenuItem("Write I/O");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_write_io));
              item = new Gtk::MenuItem("Start");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_start));
              item = new Gtk::MenuItem("Run");
              node_menu_->add(*item);
              item->signal_activate().connect(sigc::mem_fun(*this, &Ga144View::on_run));
              node_menu_->show_all();
            }
            if (&window_.owner().window().manager() != 0) {
              node_menu_->popup(evnt->button, evnt->time);
            }
            return true;
          }
        }
      }
    }
  }
  return false; // event not processed
}

bool Ga144View::on_scroll_event(GdkEventScroll *ev)
{
  return false; // event not processed
}


void Ga144View::on_read_node()
{
  window_.owner().window().manager()->do_read(selected_node_);
}

void Ga144View::on_write_node()
{
  window_.owner().window().manager()->do_write(selected_node_);
}

void Ga144View::on_read_ram()
{
  window_.owner().window().manager()->do_read_ram(selected_node_);
}

void Ga144View::on_read_rom()
{
  window_.owner().window().manager()->do_read_rom(selected_node_);
}

void Ga144View::on_read_ram_and_rom()
{
  window_.owner().window().manager()->do_read_ram_and_rom(selected_node_);
}

void Ga144View::on_write_ram()
{
  window_.owner().window().manager()->do_write_ram(selected_node_);
}

void Ga144View::on_read_stack()
{
  window_.owner().window().manager()->do_read_stack(selected_node_);
}

void Ga144View::on_write_stack()
{
  window_.owner().window().manager()->do_write_stack(selected_node_);
}

void Ga144View::on_read_return_stack()
{
  window_.owner().window().manager()->do_read_return_stack(selected_node_);
}

void Ga144View::on_write_return_stack()
{
  window_.owner().window().manager()->do_write_return_stack(selected_node_);
}

void Ga144View::on_read_a()
{
  window_.owner().window().manager()->do_read_a(selected_node_);
}

void Ga144View::on_write_a()
{
  window_.owner().window().manager()->do_write_a(selected_node_);
}

void Ga144View::on_write_b()
{
  window_.owner().window().manager()->do_write_b(selected_node_);
}

void Ga144View::on_read_io()
{
  window_.owner().window().manager()->do_read_io(selected_node_);
}

void Ga144View::on_write_io()
{
  window_.owner().window().manager()->do_write_io(selected_node_);
}

void Ga144View::on_start()
{
  window_.owner().window().manager()->do_start(selected_node_);
}

void Ga144View::on_run()
{
  window_.owner().window().manager()->do_run(selected_node_);
}

size_t Ga144View::node_to_index(size_t node)
{
  return model_->index(node);
}

F18aWindow* Ga144View::node_window(size_t node, bool assert)
{
  size_t ind = node_to_index(node);
  if (assert && ((ind >= node_window_list_.size()) || (node_window_list_[ind] == 0))) {
    return &assert_window(ind);
  }
  if (ind < node_window_list_.size()) {
    return node_window_list_[ind];
  }
  return 0;
}



}
