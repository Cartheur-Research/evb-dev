#include <studio.hpp>
#include <connection.hpp>
#include <evb001.hpp>
#include "boost/filesystem.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

#define XOFF  4
#define YOFF  4

#define Y_SPACER  5

//#define TOP_MARGIN  10
//#define BOTTOM_MARGIN  10
//#define LEFT_MARGIN  10
//#define RIGHT_MARGIN  10

namespace studio {

size_t F18aView::width()
{
  return 500;
}

size_t F18aView::height()
{
  return 1700;
}


F18aView::F18aView(F18aWindow& w, f18a::Processor_ptr m)
: window_(w)
, model_(m)
, change_dispatcher_(0)
, font_width_(0)
, font_height_(0)
, instr_window_(0)
{
  change_dispatcher_ = new Glib::Dispatcher();
  m->register_change_dispatcher(*change_dispatcher_);
  change_dispatcher_->connect(sigc::mem_fun(*this, &F18aView::on_changed));

}

F18aView::~F18aView()
{
  if (instr_window_) {
    instr_window_->hide();
    delete instr_window_;
    instr_window_ = 0;
  }
}

void F18aView::modify()
{
  window_.modify();
}

void F18aView::on_changed()
{
  queue_draw();
}

void F18aView::read(xml::Iterator& it)
{
}

void F18aView::write(xml::Writer& w)
{

}

bool F18aView::on_draw (const ::Cairo::RefPtr<::Cairo::Context>& cairo)
{
  Glib::RefPtr<Pango::Layout> pl = Pango::Layout::create(cairo);
  draw(cairo, pl, XOFF, YOFF);
  return true;
}

//void F18aView::draw_data_stack(
//  f18a::Processor& proc,
//  const Cairo::RefPtr<Cairo::Context>& cairo,
//  const Glib::RefPtr<Pango::Layout>& pango,
//  Pango::FontDescription& fdesc,
//  int font_width,
//  int font_height,
//  double x,
//  double y)
//{
//}

void F18aView::draw(const Cairo::RefPtr<Cairo::Context>& cairo, const Glib::RefPtr<Pango::Layout>& pango, double x, double y)
{
  cairo->save();

  Pango::FontDescription fdesc;
  fdesc.set_family("Consolas,monospace");
  fdesc.set_size(8 * PANGO_SCALE);

  if (font_width_ == 0) {
    Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
    layout->set_font_description(fdesc);
    layout->set_text(" ");
    //get the text dimensions (it updates the variables -- by reference)
    layout->get_pixel_size(font_width_, font_height_);
  }
  draw_node(*model_, cairo, pango, fdesc, x, y);

  cairo->restore();
}


double F18aView::draw_memory(
  const long* data,
  size_t size,
  size_t base_addr,
  std::map<long, std::string>& addr_map,
  const Cairo::RefPtr<Cairo::Context>& cairo,
  const Glib::RefPtr<Pango::Layout>& pango,
  Pango::FontDescription& fdesc,
  double x,
  double y
)
{
  size_t s, i, shadow_addr, curr_addr = base_addr & ~F18A_EXT_ARITH_SIZE;
  double cx, cy = y;
  f18a::cell cell, mask;
  std::string txt;
  for (i=0; i<size; ++i) {
    cell = data[i];
    cx = x;
    //if (curr_addr == 0xa5) {
    //  curr_addr = 0xa5;
    //}

    txt.clear();
    s = 0;
    // address column
    utils::append_hex(curr_addr, 3, txt);
    s += 4;
    txt.resize(s, ' ');
    // label column
    shadow_addr = curr_addr;
    if ((curr_addr >= F18A_RAM_START) && (curr_addr < F18A_ROM_START)) {shadow_addr ^= F18A_RAM_SHADOW_OFFSET;}
    else if ((curr_addr >= F18A_ROM_START) && (curr_addr < F18A_IO_START)) {shadow_addr ^= F18A_ROM_SHADOW_OFFSET;}
    f18a::addr_map_type::iterator it = addr_map.find(curr_addr);
    if (it == addr_map.end()) {
      it = addr_map.find(shadow_addr);
    }
    if (it == addr_map.end()) {
      it = addr_map.find(curr_addr | F18A_EXT_ARITH_SIZE);
      if (it != addr_map.end()) {
        txt[3] = '+';
      }
    }
    if (it == addr_map.end()) {
      it = addr_map.find(shadow_addr | F18A_EXT_ARITH_SIZE);
      if (it != addr_map.end()) {
        txt[3] = '+';
      }
    }
    if (it != addr_map.end()) {
      txt += it->second;
    }
    s += 11;
    txt.resize(s, ' ');

    if (cell == F18_INVALID_VALUE) {
      txt.push_back('-');
    }
    else {

      // value column
      utils::append_hex(cell, 5, txt);
      s += 6;
      txt.resize(s, ' ');
      //// value column
      //append_hex(cell ^ 0x15555, 5, txt);
      //s += 6;
      //txt.resize(s, ' ');
      // slot columns
      size_t dest_addr;
      bool dest_valid = false;
      size_t slot = 0;
      size_t ms = f18a::Opcode::max_size();
      std::string instr, addr;
      f18a::Opcode::Enum op;
    
      op = f18a::Processor::get_instruction(cell, 0);
      f18a::Opcode::to_string(op, instr);
      instr.resize(ms, ' ');
      txt += instr;
      s += ms+1;
      txt.resize(s, ' ');

      if (f18a::Opcode::has_addr_field(op)) {
        mask = f18a::Processor::get_data_mask(1);
        dest_addr = (cell & mask) | (curr_addr & ~mask);
        dest_valid = true;
        s += (ms+1)*3;
        txt.resize(s, ' ');
      }
      else {
        op = f18a::Processor::get_instruction(cell, 1);
        f18a::Opcode::to_string(op, instr);
        instr.resize(ms, ' ');
        txt += instr;
        s += ms+1;
        txt.resize(s, ' ');
        if (f18a::Opcode::has_addr_field(op)) {
          mask = f18a::Processor::get_data_mask(2);
          dest_addr = (cell & mask) | (curr_addr & ~mask);
          dest_valid = true;
          s += (ms+1)*2;
          txt.resize(s, ' ');
        }
        else {
          op = f18a::Processor::get_instruction(cell, 2);
          f18a::Opcode::to_string(op, instr);
          instr.resize(ms, ' ');
          txt += instr;
          s += ms+1;
          txt.resize(s, ' ');
          if (f18a::Opcode::has_addr_field(op)) {
            mask = f18a::Processor::get_data_mask(3);
            dest_addr = (cell & mask) | (curr_addr & ~mask);
            dest_valid = true;
            s += ms+1;
            txt.resize(s, ' ');
          }
          else {
            op = f18a::Processor::get_instruction(cell, 3);
            f18a::Opcode::to_string(op, instr);
            instr.resize(ms, ' ');
            txt += instr;
            s += ms+1;
            txt.resize(s, ' ');
          }
        }
      }
      if (dest_valid) {
        utils::append_hex(dest_addr, 3,  addr);
        txt += addr;
        s += 5;
        txt.resize(s, ' ');
        it = addr_map.find(dest_addr);
        if (it != addr_map.end()) {
          txt += it->second;
        }
        else if (dest_addr >= F18A_IO_START) {
          // address might be an I/O address
          if (f18a::convert_io_to_name(dest_addr, addr)) {
            txt += addr;
          }
        }
      }
    }
    Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
    layout->set_font_description(fdesc);
    layout->set_text(txt);
    cairo->move_to(cx, cy);
    layout->show_in_cairo_context(cairo);
    //cx += font_width_*txt.size();

    //cx += font_width;
    cy += font_height_;
    ++curr_addr;
  }
  return cy;
}

double F18aView::draw_register(
  size_t reg,
  long value,
  const Cairo::RefPtr<Cairo::Context>& cairo,
  const Glib::RefPtr<Pango::Layout>& pango,
  Pango::FontDescription& fdesc,
  double x,
  double y
)
{
  std::string txt;
  f18a::Register::to_string(static_cast<f18a::Register::Enum>(reg), txt, 3);
  return draw_register(txt, value, cairo, pango, fdesc, x, y);
}

double F18aView::draw_register(
  const std::string& reg,
  long value,
  const Cairo::RefPtr<Cairo::Context>& cairo,
  const Glib::RefPtr<Pango::Layout>& pango,
  Pango::FontDescription& fdesc,
  double x,
  double y
)
{
  std::string txt = reg;
  //f18a::Register::to_string(static_cast<f18a::Register::Enum>(reg), txt, 3);
  size_t s = 4;
  txt.resize(s, ' ');
  if (value == F18_INVALID_VALUE) {
    txt.push_back('-');
  }
  else {
    // hex value
    utils::append_hex(value, 5, txt);
    s += 6;
    txt.resize(s, ' ');
    // decimal value
    std::string tmp = boost::lexical_cast<std::string>(value);
    s += 8-tmp.size();
    txt.resize(s, ' ');
    txt += tmp;
  }

  Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
  layout->set_font_description(fdesc);
  layout->set_text(txt);
  cairo->move_to(x, y);
  layout->show_in_cairo_context(cairo);
  return y + font_height_;
}

void F18aView::draw_hex(
  long val,
  size_t digits,
  const Cairo::RefPtr<Cairo::Context>& cairo,
  Pango::FontDescription& fdesc,
  double x,
  double y
)
{
  std::stringstream ss;
  ss << std::setfill ('0') << std::setw(digits)  << std::hex << val;
  Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
  layout->set_font_description(fdesc);
  layout->set_text(ss.str());
  cairo->move_to(x, y);
  layout->show_in_cairo_context(cairo);
}

void F18aView::draw_node(
  f18a::Processor_ptr proc,
  const Cairo::RefPtr<Cairo::Context>& cairo,
  const Glib::RefPtr<Pango::Layout>& pango,
  Pango::FontDescription& fdesc,
  double x,
  double y
)
{
  //double cell_width = font_width * 6;
  //double cell_indent = font_width / 2;
  size_t i;
  double cx, cy = y;

  std::string memory_title("adr label      cell  s0    s1    s2    s3    addr destination");

  cx = x;
  {
    Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
    layout->set_font_description(fdesc);
    layout->set_text(memory_title);
    cairo->move_to(cx, cy);
    layout->show_in_cairo_context(cairo);
    cy += font_height_+Y_SPACER;
  }

  f18a::addr_map_type addr_map;
  proc->addr_map(addr_map);

  cy = draw_memory(proc->module()->ram().data(), F18A_RAM_SIZE, F18A_RAM_START, addr_map, cairo, pango, fdesc, cx, cy);
  cy += Y_SPACER;
  cy = draw_memory(proc->module()->rom().data(), F18A_ROM_SIZE, F18A_ROM_START, addr_map, cairo, pango, fdesc, cx, cy);

  std::string register_title("reg   hex  decimal addr");


  std::string txt;
  // set register position
  cx += font_width_*(memory_title.size()+10);
  cy = y;
  // register title
  {
    Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
    layout->set_font_description(fdesc);
    layout->set_text(register_title);
    cairo->move_to(cx, cy);
    layout->show_in_cairo_context(cairo);
    cy += font_height_+Y_SPACER;
  }
  // draw register I
  cy = draw_register(f18a::Register::I, proc->module()->reg().named_.I_, cairo, pango, fdesc, cx, cy);
  cy += Y_SPACER;
  // draw register A
  cy = draw_register(f18a::Register::A, proc->module()->reg().named_.A_, cairo, pango, fdesc, cx, cy);
  // draw register B
  cy = draw_register(f18a::Register::B, proc->module()->reg().named_.B_, cairo, pango, fdesc, cx, cy);
  // draw register P
  cy = draw_register(f18a::Register::P, proc->module()->reg().named_.P_, cairo, pango, fdesc, cx, cy);
  // draw register I/O
  cy = draw_register(f18a::Register::IO, proc->module()->reg().named_.IO_, cairo, pango, fdesc, cx, cy);
  // draw interesting flags
  long reg = proc->module()->reg().named_.IO_;
  if (reg != F18_INVALID_VALUE) {
    txt = "   ";
    if (reg & F18A_READ17) {txt += " 17";}
    else {txt += "   ";}
    if (reg & F18A_READ5) {txt += " 5";}
    else {txt += "  ";}
    if (reg & F18A_READ3) {txt += " 3";}
    else {txt += "  ";}
    if (reg & F18A_READ1) {txt += " 1";}
    else {txt += "  ";}
    if ((reg & F18A_RIGHT_MASK) == F18A_RIGHT_READ) {txt += " R@";}
    else if ((reg & F18A_RIGHT_MASK) == F18A_RIGHT_WRITE) {txt += " R!";}
    else {txt += "   ";}
    if ((reg & F18A_DOWN_MASK) == F18A_DOWN_READ) {txt += " D@";}
    else if ((reg & F18A_DOWN_MASK) == F18A_DOWN_WRITE) {txt += " D!";}
    else {txt += "   ";}
    if ((reg & F18A_LEFT_MASK) == F18A_LEFT_READ) {txt += " L@";}
    else if ((reg & F18A_LEFT_MASK) == F18A_LEFT_WRITE) {txt += " L!";}
    else {txt += "   ";}
    if ((reg & F18A_UP_MASK) == F18A_UP_READ) {txt += " U@";}
    else if ((reg & F18A_UP_MASK) == F18A_UP_WRITE) {txt += " U!";}
    else {txt += "   ";}
    Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
    layout->set_font_description(fdesc);
    layout->set_text(txt);
    cairo->move_to(cx, cy);
    layout->show_in_cairo_context(cairo);
    //cy += font_height_;
  }

  cy += font_height_+Y_SPACER;
  // draw register T
  cy = draw_register(f18a::Register::T, proc->module()->reg().named_.T_, cairo, pango, fdesc, cx, cy);
  // draw register S
  cy = draw_register(f18a::Register::S, proc->module()->reg().named_.S_, cairo, pango, fdesc, cx, cy);
  for (i=0; i<F18A_PARAMETER_STACK_SIZE; ++i) {
    txt = "  ";
    txt += boost::lexical_cast<std::string>(i);
    cy = draw_register(txt, proc->module()->ps().at(i), cairo, pango, fdesc, cx, cy);
  }
  cy += Y_SPACER;
  // draw register R
  cy = draw_register(f18a::Register::R, proc->module()->reg().named_.R_, cairo, pango, fdesc, cx, cy);
  for (i=0; i<F18A_RETURN_STACK_SIZE; ++i) {
    txt = "  ";
    txt += boost::lexical_cast<std::string>(i);
    cy = draw_register(txt, proc->module()->rs().at(i), cairo, pango, fdesc, cx, cy);
  }
  if (proc->has_config()) {
    cy += Y_SPACER + Y_SPACER+ Y_SPACER;
    f18a::Module_ptr config = proc->current_config();
    // startup title
    {
      Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
      layout->set_font_description(fdesc);
      layout->set_text("startup register");
      cairo->move_to(cx, cy);
      layout->show_in_cairo_context(cairo);
      cy += font_height_+Y_SPACER;
    }
    cy += Y_SPACER;
    // draw register A
    cy = draw_register(f18a::Register::A, config->reg().named_.A_, cairo, pango, fdesc, cx, cy);
    // draw register B
    cy = draw_register(f18a::Register::B, config->reg().named_.B_, cairo, pango, fdesc, cx, cy);
    // draw register P
    cy = draw_register(f18a::Register::P, config->reg().named_.P_, cairo, pango, fdesc, cx, cy);
    // draw register I/O
    cy = draw_register(f18a::Register::IO, config->reg().named_.IO_, cairo, pango, fdesc, cx, cy);
    cy += Y_SPACER;
    // draw register T
    cy = draw_register(f18a::Register::T, config->reg().named_.T_, cairo, pango, fdesc, cx, cy);
    // draw register S
    cy = draw_register(f18a::Register::S, config->reg().named_.S_, cairo, pango, fdesc, cx, cy);
    for (i=0; i<F18A_PARAMETER_STACK_SIZE; ++i) {
      txt = "  ";
      txt += boost::lexical_cast<std::string>(i);
      cy = draw_register(txt, config->ps().at(i), cairo, pango, fdesc, cx, cy);
    }
    cy += Y_SPACER;
    // draw register R
    cy = draw_register(f18a::Register::R, config->reg().named_.R_, cairo, pango, fdesc, cx, cy);
    for (i=0; i<F18A_RETURN_STACK_SIZE; ++i) {
      txt = "  ";
      txt += boost::lexical_cast<std::string>(i);
      cy = draw_register(txt, config->rs().at(i), cairo, pango, fdesc, cx, cy);
    }
  }
}

//void F18aView::draw_cell(
//  size_t addr,
//  long cell,
//  f18a::Processor::AddrMapType& lbl_map,
//  const Cairo::RefPtr<Cairo::Context>& cairo,
//  const Glib::RefPtr<Pango::Layout>& pango,
//  Pango::FontDescription& fdesc,
//  int font_width,
//  int font_height,
//  double& x,
//  double y
//)
//{
//  size_t s = 0;
//  std::string txt;
//  // address column
//  append_hex(addr, 3, txt);
//  s += 4;
//  txt.resize(s, ' ');
//  // label column
//  f18a::Processor::AddrMapType::iterator it = lbl_map.find(i);
//  if (it != lbl_map.end()) {
//    txt += it->second;
//  }
//  s += 11;
//  txt.resize(s, ' ');
//  // value column
//  append_hex(cell, 5, txt);
//  draw_hex(cell, 5, cairo, fdesc, x, y);
//  s += 6;
//  txt.resize(s, ' ');
//  // slot columns
//  size_t dest_addr;
//  bool dest_valid = false;
//  size_t slot = 0;
//  size_t ms = f18a::Opcode::max_instr_size();
//  std::string instr;
//  f18a::Opcode::Enum op;
//
//  //op = f18a::Processor::get_instruction(cell, 0);
//
//  //while (slot < F18A_NO_OF_SLOTS) {
//  //  op = f18a::Processor::get_instruction(cell, slot);
//  //  f18a::Opcode::to_string(op, instr);
//  //  { // write instruction
//  //    Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
//  //    layout->set_font_description(fdesc);
//  //    layout->set_text(it->second);
//  //    cairo->move_to(x, y);
//  //    layout->show_in_cairo_context(cairo);
//  //  }
//  //  x += f18a::Opcode::max_instr_size() * font_width + font_width;
//  //  if (f18a::Opcode::has_addr_field(op)) {
//  //    size_t data = f18a::Processor::get_data(cell, slot+1);
//  //    switch (slot) {
//  //    case 0: dest_addr = (cell & F18A_SLOT123_MASK) | (addr & ~F18A_SLOT123_MASK); break;
//  //    case 1: dest_addr = (cell & F18A_SLOT23_MASK) | (addr & ~F18A_SLOT23_MASK); break;
//  //    case 2: dest_addr = (cell & F18A_FINAL_SLOT_MASK) | (addr & ~F18A_FINAL_SLOT_MASK); break;
//  //    }
//  //    dest_valid = true;
//  //  }
//  //}
//  //// target address
//  //if (dest_valid) {
//  //  draw_hex(dest_addr, 3, cairo, fdesc, x+font_width, y);
//  //}
//  //x += font_width*5;
//  //// target columns
//  //if (dest_valid) {
//  //}
//
//  Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(cairo);
//  layout->set_font_description(fdesc);
//  layout->set_text(txt);
//  cairo->move_to(x, y);
//  layout->show_in_cairo_context(cairo);
//  x += font_width*txt.size();
//  
//}

bool F18aView::on_button_press_event(GdkEventButton* evnt)
{
  if (evnt->type == GDK_BUTTON_PRESS) {
    if (evnt->button == 1) { // left button
    }
    else if (evnt->button == 2) { // middle button
    }
    else if (evnt->button == 3) { // right button
      Gdk::Rectangle rect;
      size_t addr;
      if (find_ram_rect(evnt->x, evnt->y, rect, XOFF, YOFF, addr)) {
        if (instr_window_ == 0) {
          instr_window_ = new InstructionWindow();
        }
        instr_window_->setup(*this, addr);
        instr_window_->show();
      }
    }
  }
  return false; // event not processed
}

bool F18aView::on_scroll_event(GdkEventScroll *ev)
{
  return false; // event not processed
}

bool F18aView::find_ram_rect(double mx, double my, Gdk::Rectangle& rect, double x, double y, size_t& ram_offset)
{
  if (
    (mx >= x)
    && (my > y)
    && (mx < (x + 300))
  ) {
    ram_offset = static_cast<size_t>(floor((my - y - font_height_ - Y_SPACER) / font_height_));
    if (ram_offset < F18A_RAM_SIZE) {
      return true;
    }
  }
  ram_offset = 0;
  return false;
}
//
//void F18aView::source(const std::string& src)
//{
//  model_->assert_config().ram().source(src);
//}
//
//void F18aView::rom_source(const std::string& src)
//{
//  model_->assert_config().rom().source(src);
//}
//
//void F18aView::comment(const std::string& src)
//{
//  model_->assert_config().ram().comment(src);
//}
//
//void F18aView::rom_comment(const std::string& src)
//{
//  model_->assert_config().rom().comment(src);
//}

//F18aWindow* F18aView::node_window(size_t node, bool assert)
//{
//}


}
