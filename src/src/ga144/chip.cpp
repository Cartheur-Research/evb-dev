#include <ga144.hpp>

#include <boost/lexical_cast.hpp>


#include <gtkmm.h>

namespace ga144 {

Location::Location()
: x_(0)
, y_(0)
, valid_(false)
{
}

Location::Location(size_t x, size_t y)
: x_(x)
, y_(y)
, valid_(true)
{
}

Location::~Location()
{
}

Location Location::north() const
{
  if (y_ < (GA144_PROCESSOR_ROWS-1)) {
    return Location(x_, y_+1);
  }
  return Location();
}

Location Location::south() const
{
  if (y_ > 0) {
    return Location(x_, y_-1);
  }
  return Location();
}

Location Location::east() const
{
  if (x_ < (GA144_PROCESSOR_COLS-1)) {
    return Location(x_+1, y_);
  }
  return Location();
}

Location Location::west() const
{
  if (x_ > 0) {
    return Location(x_-1, y_);
  }
  return Location();
}

f18a::Port::Enum Location::north_port() const
{
  if (valid_) {
    if ((y_ & 1) != 0) { // odd
      return f18a::Port::up;
    }
    return f18a::Port::down;
  }
  return f18a::Port::limit;
}

f18a::Port::Enum Location::south_port() const
{
  if (valid_) {
    if ((y_ & 1) != 0) { // odd
      return f18a::Port::down;
    }
    return f18a::Port::up;
  }
  return f18a::Port::limit;
}

f18a::Port::Enum Location::east_port() const
{
  if (valid_) {
    if ((x_ & 1) != 0) { // odd
      return f18a::Port::left;
    }
    return f18a::Port::right;
  }
  return f18a::Port::limit;
}

f18a::Port::Enum Location::west_port() const
{
  if (valid_) {
    if ((x_ & 1) != 0) { // odd
      return f18a::Port::right;
    }
    return f18a::Port::left;
  }
  return f18a::Port::limit;
}

//=========================================================================

//size_t Direction::f18a_value(Enum e)
//{
//  switch (e) {
//  case up:    return 0x145;
//  case left:  return 0x175;
//  case down:  return 0x115;
//  case right: return 0x1d5;
//  default:;
//  }
//  return 0;
//}

//=========================================================================

Chip::Chip(size_t offset)
: offset_(offset)
, modify_dispatcher_(0)
, change_dispatcher_(0)
{
  modify_dispatcher_ = new Glib::Dispatcher();
  change_dispatcher_ = new Glib::Dispatcher();
  //f18a::Processor* p1 = proc_list();
  //f18a::Processor* p2 = &processor(0, 0);
  f18a::Processor_ptr proc;
  for (size_t row=0; row<GA144_PROCESSOR_ROWS; ++row) {
    for (size_t col=0; col<GA144_PROCESSOR_COLS; ++col) {
      size_t ind = index(col, row);
      proc = proc_[ind] = new f18a::Processor();
      proc->column(col);
      proc->row(row);
    }
  }
  size_t i;
  for (i=0; i<GA144_PROCESSOR_SIZE; ++i) {
    proc = processor(i);
    proc->register_modify_dispatcher(*modify_dispatcher_);
    proc->register_change_dispatcher(*change_dispatcher_);
    types_[i] = 0;
    info_[i] = 1 << InfoBit::reset;
  }
  modify_dispatcher_->connect(sigc::mem_fun(*this, &Chip::on_modify));
  change_dispatcher_->connect(sigc::mem_fun(*this, &Chip::on_changed));
}

Chip::~Chip()
{
  f18a::Processor_ptr proc;
  size_t i;
  for (i=0; i<GA144_PROCESSOR_SIZE; ++i) {
    proc = processor(i);
    proc->deregister_modify_dispatcher(*modify_dispatcher_);
    proc->deregister_change_dispatcher(*change_dispatcher_);
  }
  delete modify_dispatcher_;
  delete change_dispatcher_;
}

void Chip::register_modify_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(modify_mutex_);
  modify_dispatcher_list_.push_back(&d);
}

void Chip::deregister_modify_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(modify_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = modify_dispatcher_list_.begin(); it != modify_dispatcher_list_.end(); ++it) {
    if (*it == &d) {
      modify_dispatcher_list_.erase(it);
      return;
    }
  }
}

void Chip::on_modify()
{
  modify();
}

void Chip::modify()
{
  boost::mutex::scoped_lock lock(modify_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = modify_dispatcher_list_.begin(); it != modify_dispatcher_list_.end(); ++it) {
    (*it)->emit();
  }
}

void Chip::register_change_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(change_mutex_);
  change_dispatcher_list_.push_back(&d);
}

void Chip::deregister_change_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(change_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = change_dispatcher_list_.begin(); it != change_dispatcher_list_.end(); ++it) {
    if (*it == &d) {
      change_dispatcher_list_.erase(it);
      return;
    }
  }
}

void Chip::on_changed()
{
  changed();
}

void Chip::changed()
{
  boost::mutex::scoped_lock lock(change_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = change_dispatcher_list_.begin(); it != change_dispatcher_list_.end(); ++it) {
    (*it)->emit();
  }
}

size_t Chip::index(size_t col, size_t row) const
{
  if ((col >= GA144_PROCESSOR_COLS) || (row >= GA144_PROCESSOR_ROWS)) {
    return static_cast<size_t>(-1);
  }
  return (col * GA144_PROCESSOR_ROWS) + row;
}

size_t Chip::index(size_t node) const
{
  return index(node % 100, node / 100);
}

size_t Chip::node(size_t ind) const
{
  const f18a::Processor_ptr proc = processor(ind);
  if (proc.is_not_null()) {
    return proc->row() * 100 + proc->column();
  }
  return invalid_index;
}

void Chip::node_type(size_t ind, NodeType_ptr val)
{
  bool changed = false;
  if (ind < GA144_PROCESSOR_SIZE) {
    boost::mutex::scoped_lock lock(mutex_);
    types_[ind] = val;
    changed = true;
  }
  if (changed) {
    proc_[ind]->changed();
  }
}

void Chip::reset_node_type(size_t ind)
{
  bool changed = false;
  if (ind < GA144_PROCESSOR_SIZE) {
    boost::mutex::scoped_lock lock(mutex_);
    if (types_[ind].is_not_null()) {
      types_[ind].reset();
      changed = true;
    }
  }
  if (changed) {
    proc_[ind]->changed();
  }
}

void Chip::info(size_t ind, long val)
{
  bool changed = false;
  if (ind < GA144_PROCESSOR_SIZE) {
    boost::mutex::scoped_lock lock(mutex_);
    if (info_[ind] != val) {
      info_[ind] = val;
      changed = true;
    }
  }
  if (changed) {
    proc_[ind]->changed();
  }
}

bool Chip::can_be_tentacle_segment(size_t ind) const
{
  long info = info_[ind];
  if (info == (1 << InfoBit::reset)) {
    return true;
  }
  if (info == (1 << InfoBit::warm)) {
    return true;
  }
  return false;
}

bool Chip::neighbour_enum(size_t index, size_t neighbour_ind, Neighbour::Enum& val) const
{
  size_t test_ind;
  const f18a::Processor_ptr p1 = processor(index);
  const f18a::Processor_ptr p2 = processor(neighbour_ind);
  if ((p1 == 0) || (p2 == 0)) {return false;}
  if (p1->row() == p2->row()) {
    if (neighbour(index, Neighbour::Enum::east, test_ind) && (test_ind == neighbour_ind)) {
      val = Neighbour::Enum::east;
      return true;
    }
    if (neighbour(index, Neighbour::Enum::west, test_ind) && (test_ind == neighbour_ind)) {
      val = Neighbour::Enum::west;
      return true;
    }
  }
  else if (p1->column() == p2->column()) {
    if (neighbour(index, Neighbour::Enum::north, test_ind) && (test_ind == neighbour_ind)) {
      val = Neighbour::Enum::north;
      return true;
    }
    if (neighbour(index, Neighbour::Enum::south, test_ind) && (test_ind == neighbour_ind)) {
      val = Neighbour::Enum::south;
      return true;
    }
  }
  return false;
}

bool Chip::neighbour(size_t ind, Neighbour::Enum n, size_t& neighbour_ind) const
{
  const f18a::Processor_ptr proc = processor(ind);
  if (proc != 0) {
    size_t row = proc->row();
    size_t col = proc->column();
    switch (n) {
    case Neighbour::south:
      if (row == 0) {
        return false;
      }
      --row;
      break;

    case Neighbour::west:
      if (col == 0) {
        return false;
      }
      --col;
      break;

    case Neighbour::north:
      if (row >= (GA144_PROCESSOR_ROWS-1)) {
        return false;
      }
      ++row;
      break;

    case Neighbour::east:
      if (col >= (GA144_PROCESSOR_COLS-1)) {
        return false;
      }
      ++col;
      break;

    default: return false;
    }
    neighbour_ind = index(col, row);
    return true;
  }
  return false;
}

bool Chip::find_port(size_t from_node, size_t to_node, f18a::Port::Enum& port) const
{
  const f18a::Processor_ptr from = processor(index(from_node));
  const f18a::Processor_ptr to = processor(index(to_node));
  if ((from != 0) && (to != 0)) {
    int from_x = from->column();
    int from_y = from->row();
    int to_x = to->column();
    int to_y = to->row();
    int dx = to_x - from_x;
    int dy = to_y - from_y;
    if (dy == 0) {
      if (dx == 1) {
        return neighbour_to_port(from_node, Neighbour::east, port);
      }
      if (dx == -1) {
        return neighbour_to_port(from_node, Neighbour::west, port);
      }
    }
    else if (dx == 0) {
      if (dy == 1) {
        return neighbour_to_port(from_node, Neighbour::north, port);
      }
      else if (dy == -1) {
        return neighbour_to_port(from_node, Neighbour::south, port);
      }
    }
  }
  return false;
}

bool Chip::neighbour_to_port(size_t ind, Neighbour::Enum n, f18a::Port::Enum& port_to_neighbour) const
{
  const f18a::Processor_ptr proc = processor(ind);
  if (proc != 0) {
    switch (n) {
    case Neighbour::north:
      if ((proc->row() & 1) == 0) {port_to_neighbour = f18a::Port::down; return true;}
      port_to_neighbour = f18a::Port::up;
      return true;

    case Neighbour::west:
      if ((proc->column() & 1) == 0) {port_to_neighbour = f18a::Port::left; return true;}
      port_to_neighbour = f18a::Port::right;
      return true;

    case Neighbour::south:
      if ((proc->row() & 1) == 0) {port_to_neighbour = f18a::Port::up; return true;}
      port_to_neighbour = f18a::Port::down;
      return true;

    case Neighbour::east:
      if ((proc->column() & 1) == 0) {port_to_neighbour = f18a::Port::right; return true;}
      port_to_neighbour = f18a::Port::left;
      return true;

    default:;
    }
  }
  return false;
}

f18a::Port::Enum Chip::calc_neighbour(size_t& node, Neighbour::Enum dir)
{
  size_t row = node / 100;
  size_t col = node % 100;
  switch (dir) {
  case Neighbour::north:
    if (row < (GA144_PROCESSOR_ROWS-1)) {
      if ((row & 1) == 0) {
        node = (row+1) * 100 + col;
        return f18a::Port::down;
      }
      node = (row+1) * 100 + col;
      return f18a::Port::up;
    }
    break;

  case Neighbour::west:
    if (col > 0) {
      if ((col & 1) == 0) {
        --node;
        return f18a::Port::left;
      }
      --node;
      return f18a::Port::right;
    }
    break;

  case Neighbour::south:
    if (row > 0) {
      if ((row & 1) == 0) {
        node = (row-1) * 100 + col;
        return f18a::Port::up;
      }
      node = (row-1) * 100 + col;
      return f18a::Port::down;
    }
    break;

  case Neighbour::east:
    if (col < (GA144_PROCESSOR_COLS-1)) {
      if ((col & 1) == 0) {
        ++node;
        return f18a::Port::right;
      }
      ++node;
      return f18a::Port::left;
    }
    break;

  default:;
  }
  return f18a::Port::limit;
}

f18a::Port::Enum Chip::port_between(size_t from_node, size_t to_node)
{
  size_t tmp;
  size_t from_row = from_node / 100;
  size_t from_col = from_node % 100;
  size_t to_row = to_node / 100;
  size_t to_col = to_node % 100;
  if (to_row < from_row) {tmp = from_row; from_row = to_row; to_row = tmp;}
  if (to_col < from_col) {tmp = from_col; from_col = to_col; to_col = tmp;}
  if (from_row == to_row) {
    if (from_col+1 == to_col) {
      if ((from_col & 1) == 0) {
        return f18a::Port::right;
      }
      return f18a::Port::left;
    }
  }
  else if (from_col == to_col) {
    if (from_row+1 == to_row) {
      if ((from_row & 1) == 0) {
        return f18a::Port::down;
      }
      return f18a::Port::up;
    }
  }
  return f18a::Port::limit;
}


f18a::Processor_ptr Chip::processor_by_node(size_t node)
{
  return processor(index(node));
}

const f18a::Processor_ptr Chip::processor_by_node(size_t node) const
{
  return processor(index(node));
}

f18a::Processor_ptr Chip::processor(size_t ind)
{
  f18a::Processor_ptr res;
  if (ind < GA144_PROCESSOR_SIZE) {
    return proc_[ind];
  }
  return res;
}

const f18a::Processor_ptr Chip::processor(size_t ind) const
{
  if (ind < GA144_PROCESSOR_SIZE) {
    return proc_[ind];
  }
  return 0;
}

void Chip::reset()
{
  {
    boost::mutex::scoped_lock lock(mutex_);
    size_t i;
    for (i=0; i<GA144_PROCESSOR_SIZE; ++i) {
      proc_[i]->reset();
      types_[i].reset();
      info_[i] = 1 << InfoBit::reset;
    }
  }
  changed();
}

void Chip::update(evb::Board& board)
{
  size_t i;
  for (i=0; i<GA144_PROCESSOR_SIZE; ++i) {
    processor(i)->update(board);
  }
}

void Chip::compile_rom()
{
}

void Chip::execute()
{
  size_t i;
  for (i=0; i<GA144_PROCESSOR_SIZE; ++i) {
    processor(i)->execute();
  }
}

void Chip::read(xml::Iterator& it)
{
  size_t ind, i=0;
	while ((it.current() == xml::Element::tag) && (it.tag() == "node")) {
    ind = i;
    if ((it.next() == xml::Element::attribute) && (it.key() == "n")) {
      try {
        ind = index(boost::lexical_cast<size_t>(it.value()));
      }
      catch (...) {}
    }
	  it.next_tag();
    while ((it.current() != xml::Element::tag) && (it.current() != xml::Element::end)) {
  	  it.next_tag();
    }
    processor(ind)->read(it);
    ++i;
  	if ((it.current() == xml::Element::end) && (it.tag() == "node")) {
      it.next_tag();
    }
  }
}

void Chip::write(xml::Writer& w)
{
  size_t i;
  for (i=0; i<GA144_PROCESSOR_SIZE; ++i) {
    f18a::Processor_ptr proc = processor(i);
    w.newline();
    w.open("node", false, false);
    w.attribute("n", boost::lexical_cast<std::string>(proc->node()));
    w.close(false);
    w.indent(1);
    proc->write(w);
    w.newline(-1);
    w.open("node", true, true);
  }
}

void Chip::tic()
{
  size_t i;
  for (i=0; i<GA144_PROCESSOR_SIZE; ++i) {
    processor(i)->tic();
  }
}

void Chip::toc()
{
  size_t i;
  for (i=0; i<GA144_PROCESSOR_SIZE; ++i) {
    processor(i)->toc();
  }
}

size_t Chip::port(size_t index, f18a::Port::Enum port) const
{
  return (node(index) * f18a::Port::limit) + port;
}



}
