

#include <f18a.hpp>
#include <ga144.hpp>
#include <boost/lexical_cast.hpp>

#include <gtkmm.h>

namespace f18a {

size_t Processor::inc_addr(size_t addr)
{
  return (addr & ~F18A_INC_MASK) | ((addr+1) & F18A_INC_MASK);
}

int Processor::instruction_slots(cell cell)
{
  if (Opcode::has_addr_field(get_instruction(cell, 0))) {return 1;}
  if (Opcode::has_addr_field(get_instruction(cell, 1))) {return 2;}
  if (Opcode::has_addr_field(get_instruction(cell, 2))) {return 3;}
  return 4;
}

Opcode::Enum Processor::get_instruction(cell cell, int slot)
{
  cell ^= 0x15555;
  switch (slot) {
  case 0: return static_cast<Opcode::Enum>((cell >> F18A_SLOT123_BITS) & F18A_SLOT_MASK);
  case 1: return static_cast<Opcode::Enum>((cell >> F18A_SLOT23_BITS) & F18A_SLOT_MASK);
  case 2: return static_cast<Opcode::Enum>((cell >> F18A_FINAL_SLOT_BITS) & F18A_SLOT_MASK);
  case 3: return static_cast<Opcode::Enum>((cell & F18A_FINAL_SLOT_MASK) << F18A_SKIP_SLOT_BITS);
  }
  return Opcode::nop;
}

cell Processor::get_data_mask(int slot)
{
  switch (slot) {
  case 0: return F18A_ADDR_MASK;
  case 1: return F18A_ADDR_MASK;
  case 2: return F18A_SLOT23_MASK;
  case 3: return F18A_FINAL_SLOT_MASK;
  }
  return 0;
}

size_t Processor::get_data(cell cell, int slot)
{
  return static_cast<int>(cell & get_data_mask(slot));
}

// ======================================================================================

Processor::Processor()
: io_written_(0)
, row_(0)
, column_(0)
, current_(0)
, tentacle_index_(-1)
, io_is_written_(false)
{
  module_ = new f18a::Module();
  module_->reset();
  module_->ram().reset();
  module_->rom().reset();
  //for (size_t i=0; i<F18A_RAM_SIZE; ++i) {
  //  ram_[i] = F18A_INITIAL_CELL;
  //}
  //for (size_t i=0; i<F18A_ROM_SIZE; ++i) {
  //  rom_[i] = F18A_INITIAL_CELL;
  //}
}

Processor::~Processor()
{
}

void Processor::write_io(cell val)
{
  io_written_ = val;
  io_is_written_ = true;
}

void Processor::register_modify_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(modify_mutex_);
  modify_dispatcher_list_.push_back(&d);
}

void Processor::deregister_modify_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(modify_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = modify_dispatcher_list_.begin(); it != modify_dispatcher_list_.end(); ++it) {
    if (*it == &d) {
      modify_dispatcher_list_.erase(it);
      return;
    }
  }
}

void Processor::on_modify()
{
  modify();
  changed();
}

void Processor::modify()
{
  boost::mutex::scoped_lock lock(modify_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = modify_dispatcher_list_.begin(); it != modify_dispatcher_list_.end(); ++it) {
    (*it)->emit();
  }
}


void Processor::register_change_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(change_mutex_);
  change_dispatcher_list_.push_back(&d);
}

void Processor::deregister_change_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(change_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = change_dispatcher_list_.begin(); it != change_dispatcher_list_.end(); ++it) {
    if (*it == &d) {
      change_dispatcher_list_.erase(it);
      return;
    }
  }
}

void Processor::on_changed()
{
  changed();
}

void Processor::changed()
{
  boost::mutex::scoped_lock lock(change_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = change_dispatcher_list_.begin(); it != change_dispatcher_list_.end(); ++it) {
    (*it)->emit();
  }
}

//void Processor::source(const std::string& val)
//{
//  if (val != source_) {
//    source_ = val;
//    modify();
//  }
//}
//
//void Processor::rom_source(const std::string& val)
//{
//  if (val != rom_source_) {
//    rom_source_ = val;
//    modify();
//  }
//}
//
//void Processor::comment(const std::string& val)
//{
//  if (val != comment_) {
//    comment_ = val;
//    modify();
//  }
//}
//
//void Processor::rom_comment(const std::string& val)
//{
//  if (val != rom_comment_) {
//    rom_comment_ = val;
//    modify();
//  }
//}
//
//void Processor::rom_locked(bool val)
//{
//  if (val != rom_locked_) {
//    rom_locked_ = val;
//    modify();
//  }
//}

//Module_ptr Processor::assert_config_by_name(const std::string& name)
//{
//  Module_ptr res = config_by_name(name);
//  if (res.is_null()) {
//    append_config();
//    res = config_.back();
//    res->name(name);
//  }
//  return res;
//}

Module_ptr Processor::config_by_name(const std::string& name)
{
  std::vector<Module_ptr>::iterator it;
  for (it = config_.begin(); it != config_.end(); ++it) {
    if ((*it)->name() == name) {
      return *it;
    }
  }
  return Module_ptr();
}

Module_ptr Processor::assert_config(size_t pos)
{
  while (config_.size() <= pos) {
    append_config();
  }
  return config(pos);
}

Module_ptr Processor::rom_module()
{
  if (rom_module_.is_null()) {
    rom_module_ = new Module();
  }
  return rom_module_;
}

void Processor::append_config()
{
  Module_ptr res = new Module();
  config_.push_back(res);
  config_.back()->reset();
}

bool Processor::find_label(const std::string& name, size_t config_pos, cell& addr)
{
  if (config_size() > config_pos) {
    Module_ptr module = config(config_pos);
    return module->find_label(name, addr);
    //label_map_type::const_iterator it = module.ram().label_map().find(name);
    //if (it != module.ram().label_map().end()) {
    //  addr = it->second;
    //  return true;
    //}
    //it = module.rom().label_map().find(name);
    //if (it != module.rom().label_map().end()) {
    //  addr = it->second;
    //  return true;
    //}
  }
  return false;
}

bool Processor::startup_p(size_t& val) const
{
  if (current_ < config_.size()) {
    if (config_[current_]->reg().named_.P_ != F18_INVALID_VALUE) {
      val = config_[current_]->reg().named_.P_;
      return true; 
    }
  }
  if (module_->reg().named_.P_ != F18_INVALID_VALUE) {
    val = module_->reg().named_.P_;
    return true; 
  }
  return false;
}


void Processor::set_b(cell val)
{
  module_->reg().named_.B_ = val;
  modify();
}

size_t Processor::node() const
{
  return (row_ * 100) + column_;
}

bool Processor::is_h_border() const
{
  return (column_ == 0) || (column_ == (GA144_PROCESSOR_COLS-1));
}

bool Processor::is_v_border() const
{
  return (row_ == 0) || (row_ == (GA144_PROCESSOR_ROWS-1));
}

bool Processor::is_corner() const
{
  return is_h_border() && is_v_border();
}

bool Processor::includes(size_t node) const
{
  include_map_type::const_iterator it = include_map_.find(node);
  if (it == include_map_.end()) {return false;}
  return it->second;
}

void Processor::reset()
{
  tentacle_index_ = -1;
  module_->reset();
}

void Processor::execute()
{
}

void Processor::addr_map(addr_map_type& map)
{
  label_map_type& ram_labels = module_->ram().label_map();
  for (label_map_type::iterator it=ram_labels.begin(); it != ram_labels.end(); ++it) {
    map[it->second] = it->first;
  }
  label_map_type& rom_labels = module_->rom().label_map();
  for (label_map_type::iterator it=rom_labels.begin(); it != rom_labels.end(); ++it) {
    map[it->second] = it->first;
  }
}

//void Processor::insert_ram(size_t pos, cell val)
//{
//  size_t i = F18A_RAM_MASK;
//  while (i > pos) {
//    ram_[i] = ram_[i-1];
//    --i;
//  }
//  ram_[pos] = val;
//  std::string rem_lbl_name;
//  for (label_map_type::iterator it=label_map_.begin(); it != label_map_.end(); ++it) {
//    if (it->second >= static_cast<long>(pos)) {
//      if (++it->second >= F18A_RAM_SIZE) {rem_lbl_name = it->first;}
//    }
//  }
//  if (!rem_lbl_name.empty()) {
//    label_map_.erase(label_map_.find(rem_lbl_name));
//  }
//  modify();
//}
//
//void Processor::remove_ram(size_t pos)
//{
//  size_t i = pos;
//  while (i < F18A_RAM_MASK) {
//    ram_[i] = ram_[i+1];
//    ++i;
//  }
//  bool conflict = false;
//  std::string rem_lbl_name;
//  ram_[F18A_RAM_MASK] = 0;
//  for (label_map_type::iterator it=label_map_.begin(); it != label_map_.end(); ++it) {
//    if (it->second > static_cast<long>(pos)) {
//      if (--it->second == pos) {conflict = true;}
//    }
//    else if (it->second == pos) {rem_lbl_name = it->first;}
//  }
//  if (!rem_lbl_name.empty() && conflict) {
//    label_map_.erase(label_map_.find(rem_lbl_name));
//  }
//  modify();
//}

//void Processor::ram(size_t pos, cell val)
//{
//  val &= F18A_CELL_MASK;
//  if (val != ram_[pos]) {
//    ram_[pos] = val;
//    modify();
//  }
//}
//
//void Processor::rom(size_t pos, cell val, bool force)
//{
//  if (!rom_locked_ || force) {
//    val &= F18A_CELL_MASK;
//    if (val != rom_[pos]) {
//      rom_[pos] = val;
//      modify();
//    }
//  }
//}

//void Processor::ram(size_t begin, const cell* addr, size_t size) // set ram
//{
//  bool modified = false;
//  if (begin < F18A_RAM_SIZE) {
//    size_t end = begin + size;
//    if (end > F18A_RAM_SIZE) {end = F18A_RAM_SIZE;}
//    for (size_t i=begin; i<end; ++i) {
//      if (ram_[i] != *addr) {
//        ram_[i] = *addr;
//        modified = true;
//      }
//      ++addr;
//    }
//  }
//  if (modified) {
//    modify();
//  }
//}
//
//void Processor::rom(size_t begin, const cell* addr, size_t size) // set rom
//{
//  if (!rom_locked_) {
//    bool modified = false;
//    if (begin < F18A_ROM_SIZE) {
//      size_t end = begin + size;
//      if (end > F18A_ROM_SIZE) {end = F18A_ROM_SIZE;}
//      for (size_t i=begin; i<end; ++i) {
//        if (rom_[i] != *addr) {
//          rom_[i] = *addr;
//          modified = true;
//        }
//        ++addr;
//      }
//    }
//    if (modified) {
//      modify();
//    }
//  }
//}

long Processor::instruction4(Opcode::Enum slot0, Opcode::Enum slot1, Opcode::Enum slot2, Opcode::Enum slot3)
{
  long res = slot0;
  res <<= F18A_SLOT_BITS;
  res |= slot1;
  res <<= F18A_SLOT_BITS;
  res |= slot2;
  res <<= F18A_SLOT_BITS;
  res |= slot3;
  if ((res & F18A_SKIP_SLOT_MASK) != 0) {
    throw fit_exception("f18a::instruction4 final slot does not fit");
  }
  res >>= F18A_SKIP_SLOT_BITS;
  return res;
}

long Processor::instruction3(Opcode::Enum slot0, Opcode::Enum slot1, Opcode::Enum slot2, int value)
{
  if ((value & ~F18A_FINAL_SLOT_MASK) != 0) {
    throw fit_exception("f18a::instruction3 value does not fit into slot 3");
  }
  long res = slot0;
  res <<= F18A_SLOT_BITS;
  res |= slot1;
  res <<= F18A_SLOT_BITS;
  res |= slot2;
  res <<= F18A_FINAL_SLOT_BITS;
  res |= value;
  return res;
}

long Processor::instruction2(Opcode::Enum slot0, Opcode::Enum slot1, int value)
{
  if ((value & ~F18A_SLOT23_MASK) != 0) {
    throw fit_exception("f18a::instruction2 value does not fit into slot 2+3");
  }
  long res = slot0;
  res <<= F18A_SLOT_BITS;
  res |= slot1;
  res <<= F18A_SLOT23_BITS;
  res |= value;
  return res;
}

long Processor::instruction1(Opcode::Enum slot0, int value)
{
  if ((value & ~F18A_SLOT123_MASK) != 0) {
    throw fit_exception("f18a::instruction1 value does not fit into slot 1+2+3");
  }
  long res = slot0;
  res <<= F18A_SLOT123_BITS;
  res |= value;
  return res;
}

long Processor::instruction0(int value)
{
  return value;
}

//bool Processor::has_data(Opcode::Enum op)
//{
//  if (op == Opcode::jump) {return true;}
//  if (op == Opcode::call) {return true;}
//  if (op == Opcode::_if) {return true;}
//  if (op == Opcode::mif) {return true;}
//  if (op == Opcode::next) {return true;}
//  return false;
//}

size_t Processor::warm() const
{
  if (is_corner()) {
    return 0x195;
  }
  else if (is_h_border()) {
    return 0x185;
  }
  else if (is_v_border()) {
    return 0x1b5;
  }
  return 0x1a5;
}


cell Processor::memory(size_t addr) const
{
  addr &= F18A_EXT_ARITH_MASK;
  if (addr < F18A_ROM_START) {
    return module_->ram().data(addr & F18A_RAM_MASK);
  }
  if (addr < F18A_IO_START) {
    return module_->rom().data(addr & F18A_ROM_MASK);
  }
  else { // maybe io register
    switch (addr ^ 0x155) {
    case IOPortAddr::io:
    case IOPortAddr::data:
    case IOPortAddr::___u:
    case IOPortAddr::__l_:
    case IOPortAddr::__lu:
    case IOPortAddr::_d__:
    case IOPortAddr::_d_u:
    case IOPortAddr::_dl_:
    case IOPortAddr::_dlu:
    case IOPortAddr::r___:
    case IOPortAddr::r__u:
    case IOPortAddr::r_l_:
    case IOPortAddr::r_lu:
    case IOPortAddr::rd__:
    case IOPortAddr::rd_u:
    case IOPortAddr::rdl_:
    case IOPortAddr::rdlu:
    default:;
    }
  }
  return F18_INVALID_VALUE;
}

void Processor::memory(size_t addr, cell val)
{
  bool modified = false;
  if (addr < F18A_ROM_START) {
    module_->ram().data(addr & F18A_RAM_MASK, val);
    modified = true;
  }
  else if (addr < F18A_IO_START) {
    module_->rom().data(addr & F18A_ROM_MASK, val);
    modified = true;
  }
  else { // maybe io register
    switch (addr ^ 0x155) {
    case IOPortAddr::io:
    case IOPortAddr::data:
    case IOPortAddr::___u:
    case IOPortAddr::__l_:
    case IOPortAddr::__lu:
    case IOPortAddr::_d__:
    case IOPortAddr::_d_u:
    case IOPortAddr::_dl_:
    case IOPortAddr::_dlu:
    case IOPortAddr::r___:
    case IOPortAddr::r__u:
    case IOPortAddr::r_l_:
    case IOPortAddr::r_lu:
    case IOPortAddr::rd__:
    case IOPortAddr::rd_u:
    case IOPortAddr::rdl_:
    case IOPortAddr::rdlu:
    default: return;
    }
    modified = true;
  }
  if (modified) {
    modify();
  }
}

void Processor::read(xml::Iterator& it)
{
	while ((it.current() == xml::Element::tag) && (it.tag() == "config")) {
	  it.next_tag();
    append_config();
    config_.back()->read(it);
  	if ((it.current() == xml::Element::end) && (it.tag() == "config")) {
      it.next_tag();
    }
  }

	if ((it.current() == xml::Element::tag) && (it.tag() == "rom")) {
	  it.next_tag();
    append_config();
    rom_module_ = new Module();
    rom_module_->reset();
    rom_module_->read(it);
  	if ((it.current() == xml::Element::end) && (it.tag() == "config")) {
      it.next_tag();
    }
  }

/*
	if ((it.current() == xml::Element::tag) && (it.tag() == "source")) {
	  it.next();
  	if (it.current() == xml::Element::content) {
      source_ = it.value();
  	  it.next_tag();
    }
    else {
      source_.clear();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "source")) {
      it.next_tag();
    }
  }
	if ((it.current() == xml::Element::tag) && (it.tag() == "ram")) {
	  it.next();
  	if (it.current() == xml::Element::content) {
      std::vector<long> ram_data;
      it.read(ram_data);
      size_t limit = ram_data.size();
      for (size_t i=0; i<limit; ++i) {
        ram_[i] = ram_data[i];
      }
      //std::string ram_data = it.value();
  	  it.next_tag();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "ram")) {
      it.next_tag();
    }
  }
	if ((it.current() == xml::Element::tag) && (it.tag() == "comment")) {
	  it.next();
  	if (it.current() == xml::Element::content) {
      comment_ = it.value();
  	  it.next_tag();
    }
    else {
      comment_.clear();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "comment")) {
      it.next_tag();
    }
  }
	if ((it.current() == xml::Element::tag) && (it.tag() == "labels")) {
	  it.next_tag();
	  while ((it.current() == xml::Element::tag) && (it.tag() == "label")) {
      std::string name, pos;
      if ((it.next() == xml::Element::attribute) && (it.key() == "name")) {
        name = it.value();
      }
      if ((it.next() == xml::Element::attribute) && (it.key() == "pos")) {
        pos = it.value();
      }
      try {
        size_t val = boost::lexical_cast<size_t>(pos);
        label_map_[name] = val;
      }
      catch (...) {}
      if (it.next() == xml::Element::empty) {
  	    it.next_tag();
      }
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "labels")) {
      it.next_tag();
    }
  };
	if ((it.current() == xml::Element::tag) && (it.tag() == "rom_source")) {
	  it.next();
  	if (it.current() == xml::Element::content) {
      rom_source_ = it.value();
  	  it.next_tag();
    }
    else {
      rom_source_.clear();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "rom_source")) {
      it.next_tag();
    }
  }
	if ((it.current() == xml::Element::tag) && (it.tag() == "rom")) {
	  it.next();
  	if (it.current() == xml::Element::content) {
      std::vector<long> rom_data;
      it.read(rom_data);
      size_t limit = rom_data.size();
      for (size_t i=0; i<limit; ++i) {
        rom_[i] = rom_data[i];
      }
      //std::string ram_data = it.value();
  	  it.next_tag();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "rom")) {
      it.next_tag();
    }
  }
	if ((it.current() == xml::Element::tag) && (it.tag() == "rom_comment")) {
	  it.next();
  	if (it.current() == xml::Element::content) {
      rom_comment_ = it.value();
  	  it.next_tag();
    }
    else {
      rom_comment_.clear();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "rom_comment")) {
      it.next_tag();
    }
  }
	if ((it.current() == xml::Element::tag) && (it.tag() == "rom_labels")) {
	  it.next_tag();
	  while ((it.current() == xml::Element::tag) && (it.tag() == "label")) {
      std::string name, pos;
      if ((it.next() == xml::Element::attribute) && (it.key() == "name")) {
        name = it.value();
      }
      if ((it.next() == xml::Element::attribute) && (it.key() == "pos")) {
        pos = it.value();
      }
      try {
        size_t val = boost::lexical_cast<size_t>(pos);
        rom_label_map_[name] = val;
      }
      catch (...) {}
      if (it.next() == xml::Element::empty) {
  	    it.next_tag();
      }
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "rom_labels")) {
      it.next_tag();
    }
  };
	if ((it.current() == xml::Element::tag) && (it.tag() == "task")) {
	  it.next_tag();
    task_.read(it);
  	if ((it.current() == xml::Element::end) && (it.tag() == "task")) {
      it.next_tag();
    }
  };
*/
  modify();
}

void Processor::write(xml::Writer& w)
{

  size_t limit = config_.size();
  for (size_t i=0; i<limit; ++i) {
    if (config_[i]->writeable()) {
      w.newline();
      w.open("config", false, true);
      w.indent(1);
      config_[i]->write(w);
      w.newline(-1);
      w.open("config", true, true);
    }
  }
  if (rom_module_.is_not_null()) {
    if (rom_module_->writeable()) {
      w.newline();
      w.open("rom", false, true);
      w.indent(1);
      rom_module_->write(w);
      w.newline(-1);
      w.open("rom", true, true);
    }
  }

/*
  size_t limit;
  if (!source_.empty()) {
    w.newline();
    w.open("source", false, true);
    w.write(source_);
    w.open("source", true, true);
    //
  }
  {
    limit = F18A_RAM_SIZE;
    while ((limit > 0) && (ram_[limit-1] == F18A_INITIAL_CELL)) {--limit;}
    if (limit > 0) {
      std::vector<long> ram_data;
      ram_data.reserve(limit);
      for (size_t i=0; i<limit; ++i) {
        ram_data.push_back(ram_[i]);
      }
      w.newline();
      w.open("ram", false, true);
      w.write(ram_data);
      w.open("ram", true, true);
    }
  }
  if (!comment_.empty()) {
    w.newline();
    w.open("comment", false, true);
    w.write(comment_);
    w.open("comment", true, true);
  }
  if (!label_map_.empty()) {
    w.newline();
    w.open("labels", false, true);
    w.indent(1);
    for (label_map_type::iterator it = label_map_.begin(); it != label_map_.end(); ++it) {
      w.newline();
      w.open("label", false);
      w.attribute("name", it->first);
      w.attribute("pos", boost::lexical_cast<std::string>(it->second));
      w.close(true);
    }
    w.newline(-1);
    w.open("labels", true, true);
  }
  if (!rom_source_.empty()) {
    w.newline();
    w.open("rom_source", false, true);
    w.write(rom_source_);
    w.open("rom_source", true, true);
  }
  {
    limit = F18A_ROM_SIZE;
    while ((limit > 0) && (rom_[limit-1] == F18A_INITIAL_CELL)) {--limit;}
    if (limit > 0) {
      std::vector<long> rom_data;
      rom_data.reserve(F18A_ROM_SIZE);
      for (size_t i=0; i<F18A_ROM_SIZE; ++i) {
        rom_data.push_back(rom_[i]);
      }
      w.newline();
      w.open("rom", false, true);
      w.write(rom_data);
      w.open("rom", true, true);
    }
  }
  if (!rom_comment_.empty()) {
    w.newline();
    w.open("rom_comment", false, true);
    w.write(rom_comment_);
    w.open("rom_comment", true, true);
  }
  if (!rom_label_map_.empty()) {
    w.newline();
    w.open("rom_labels", false, true);
    w.indent(1);
    for (label_map_type::iterator it = rom_label_map_.begin(); it != rom_label_map_.end(); ++it) {
      w.newline();
      w.open("label", false);
      w.attribute("name", it->first);
      w.attribute("pos", boost::lexical_cast<std::string>(it->second));
      w.close(true);
    }
    w.newline(-1);
    w.open("rom_labels", true, true);
  }
  if (task_.writeable()) {
    w.newline();
    w.open("task", false, true);
    w.indent(1);
    task_.write(w);
    w.newline(-1);
    w.open("task", true, true);
  }
*/
}

void Processor::update(evb::Board& board)
{
  std::vector<Module_ptr>::iterator it;
  for (it = config_.begin(); it != config_.end(); ++it) {
    (*it)->update(board);
  }
}

void Processor::tic()
{
  module_->tic();
}

void Processor::toc()
{
  module_->toc();
}

RomType::Enum Processor::rom_type() const
{
  switch (node()) {
  case 001:
  case 701: return RomType::serdes_boot;
  case 705: return RomType::spi_boot;
  case 300: return RomType::sync_boot;
  case 708: return RomType::async_boot;
  case 117:
  case 617:
  case 717:
  case 713:
  case 709: return RomType::analog;
  case 100:
  case 500:
  case 600:
  case 417:
  case 317: return RomType::digital;
  case 200: return RomType::wire1;
  default:return RomType::basic;
  }
  return RomType::undefined;
}

std::string Processor::rom_source() const
{
  static std::string relay = 
  " # xA1 org"
  " : relay ( a) r> a! @+ >r @+ zif drop ahead SWAP then r> over >r @p a relay !b !b !b begin @+ !b unext"
  " : done then a >r a! ;";
  static std::string warm = 
  " # xA9 org"
  " : warm await ;";
  static std::string poly = 
  " # xAA org"
  " : poly ( xn-xy) r> a! >r @+ a begin >r *. r> a! @+ + a next >r ;";
  static std::string interp = 
  " : interp ( ims-v) dup >r >r over begin 2/ unext a! and >r @+ dup @+ inv . + inv r> a! dup dup xor begin +* unext >r drop r> . + ;";
  static std::string filter = 
  " : taps ( y x c - y' x') r> a! >r begin @+ @ >r a >r *.17 r> a! >r !+ r> . + r> next @ a! ;";
  static std::string mul17 = 
  " : *.17 ( a b - a a*b) a! 16. >r dup dup xor begin +* unext inv +* a -if drop inv 2* ; then drop 2* inv ;";
  static std::string shift = 
  " : lsh ( w n-1 - w') for 2* unext ;"
  " : rsh ( w n-1 - w') for 2/ unext ;";
  static std::string triangle = 
  " : triangle ( ang*4) x1.0000 over -if drop . + ; then drop inv . + inv ;";
  static std::string muldot = 
  " : *. ( f1 f2 - f1 f1*f2) *.17 a 2* -if drop inv 2* inv ; then drop 2* ;";
  static std::string divide = 
  " +cy"
  " : clc ( -2-) dup dup xor dup . + drop ;"
  " : --u/mod ( hld-rq) clc"
  " : -u/mod ( hld-ra) a! 17. >r begin begin dup . + . >r dup . + dup a . + -if drop r> *next dup . + ; then over xor xor . r> next dup . + ;"
  " -cy";
  static std::string wire1 = 
  " # x9E org"
  " : rcv ( s-sn) a >r dup dup a! 17. for begin"
  " : bit @ drop @b -if drop inv 2* inv *next r> a! ; then drop 2* next r> a! ;"
  " # xAA org"
  " : cold left center a! . io b! dup dup xB7. dup >r >r 16. >r @ drop @b # await -until drop a! . bit ;"
  " : fwd-b7 >r rcv a! rcv >r zif ; thn begin rcv !+ next ;";
  static std::string dac = 
  " : -dac ( legacy entry name ...)"
  " : dac27 ( m c p a w - m c p) dup >r >r over r> inv . +  >r >r  x155. r> over xor a begin unext !b . begin unext !b !b ;";
  static std::string spi = 
  " # x2A NL ---  # x2B NL --+  # x3A NL +--  # x3B NL +-+ # x2F NL -++"
  " # xC2 org"
  " : 8obits ( dw-dw') 7. for  ( obit) leap 2* *next ;"
  " : ibit ( dw-dw') @b . -if  drop inv 2* ;  then drop 2* inv ;"
  " : half ( dwc-dw) !b over for . . unext ;"
  " : select ( dw-dw) -++ half --+ half ;"
  " : obit ( dw-dw) then  -if +-- half +-+ half ; then"
  " : rbit ( dw-dw) --- half --+ half ;"
  " : 18ibits ( d-dw) dup 17. >r"
  " : ibits   begin rbit ibit inv next ;"
  " : u2/ ( n - n) 2/ x1FFFF. and ;"
  " # 497 NL spispeed"
  " # xC00 NL spicmd"
  " # 0 NL spiadr"
  " : cold @b inv ..  avail -until  spispeed spiadr >r spicmd "
  " : spi-boot ( d ah - d x) select  8obits 8obits drop r> . 8obits 8obits"
  " : spi-exec ( dx-dx) drop 18ibits x1E000. . +  # rdl- -until >r 18ibits a! 18ibits"
  " : spi-copy ( dn-dx) >r zif ; then begin 18ibits !+ next  dup ;";
  static std::string async = 
  " # xCB equ 18ibits"
  " : cold   x31A5. a! @  @b .. -if"
  " : ser-exec ( x - d)   18ibits drop >r 18ibits drop a! 18ibits"
  " : ser-copy ( xnx-d)   drop >r zif ;  then begin 18ibits drop !+ next ;  then drop avail alit >r >r ;"
  " : wait ( x-1/1)   begin . drop @b -until  . drop ;"
  " : sync ( x-3/2-d)   dup dup wait  xor inv >r begin @b . -if . drop *next await ;  then . drop r> inv 2/ ;"
  " : start ( dw-4/2-dw,io) dup wait over dup 2/ . + >r"
  " : delay ( -1/1-io) begin @b . -if then . drop next @b ;"
  " ( 18ibits ( x-4/6-dwx) sync sync dup start ( 2bits) leap leap"
  " : byte   then drop start leap"
  " : 4bits   then leap"
  " : 2bits   then then leap"
  " : 1bit ( nw,io - nw,io) then >r 2/ r> over xor x20000. and xor over >r delay ;";
  static std::string sync = 
  " # xBE equ sget"
  " : cold   x31A5. a! @ @b . . -if avail # x3.FC00 [+] lit  dup >r dup begin drop @b . -if ( rising) *next  SWAP then avail alit >r  drop >r ; then"
  " : ser-exec ( x - x)   sget >r  sget a!  sget"
  " : ser-copy ( n)   >r zif ;  then begin sget !+ next ;"
  " ( sget) ( -4/3-w)   dup leap leap"
  " : 6in   then then leap leap"
  " : 2in   then then  2* 2*  dup begin . drop @b . inv -until  inv 2. and dup begin . drop @b . . -until  2. and 2/ xor xor ;";
  std::string res;
  switch (rom_type()) {

  case RomType::basic:
    res += relay;
    res += warm;
    res += " # xB0 org";
    res += mul17;
    res += muldot;
    res += filter;
    res += interp;
    res += triangle;
    res += divide;
    res += poly;
    break;

  case RomType::analog:
    res += relay;
    res += warm;
    res += " # xB0 org";
    res += mul17;
    res += muldot;
    res += dac;
    res += interp;
    res += triangle;
    res += divide;
    res += poly;
    break;

  case RomType::spi_boot:
    res += relay;
    res += warm;
    res += spi;
    break;

  case RomType::wire1:
    res += warm;
    res += wire1;
    res += triangle;
    res += mul17;
    res += muldot;
    res += divide;
    break;

  case RomType::serdes_boot:
    res += relay;
    res += warm;
    res += " : cold x3141. a! x3FFFE. dup !  rdlu cold ;";
    res += " # xB0 org";
    res += mul17;
    res += muldot;
    res += filter;
    res += interp;
    res += triangle;
    res += divide;
    break;

  case RomType::async_boot:
    res += relay;
    res += warm;
    res += async;
    res += shift;
    break;

  case RomType::sync_boot:
    res += relay;
    res += warm;
    res += sync;
    res += mul17;
    res += filter;
    res += triangle;
    break;

  default:;
  }
  return res;
}


}
