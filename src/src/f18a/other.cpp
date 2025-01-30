#include <f18a.hpp>
#include <evb.hpp>
#include <boost/lexical_cast.hpp>

namespace f18a {


cell Port::to_addr(Enum e)
{
  switch(e) {
  case up:    return 0x145;
  case left:  return 0x175;
  case down:  return 0x115;
  case right: return 0x1d5;
  default:;
  }
  return 0;
}

Port::Enum Port::opposite(Enum e)
{
  switch(e) {
  case up:    return down;
  case left:  return right;
  case down:  return up;
  case right: return left;
  default:;
  }
  return limit;
}

Port::Enum Port::from_addr(cell val)
{
  switch(val) {
  case 0x145:   return up;
  case 0x175:   return left;
  case 0x115:   return down;
  case 0x1d5:   return right;
  default:;
  }
  return limit;
}

const std::string& Port::to_call(Enum e)
{
  static std::map<Enum, std::string> map_;
  std::map<Enum, std::string>::const_iterator it = map_.find(e);
  if (it != map_.end()) {
    return it->second;
  }
  if (map_.empty()) {
    map_[up] = "---u";
    map_[left] = "--l-";
    map_[down] = "-d--";
    map_[right] = "r---";
    it = map_.find(e);
    if (it != map_.end()) {
      return it->second;
    }
  }
  static std::string not_found;
  return not_found;
}

// ======================================================================================

void RegisterSet::read(xml::Iterator& it)
{
  reset();
  while ((it.current() == xml::Element::tag) && (it.tag() == "reg")) {
    std::string name, val;
    if ((it.next() == xml::Element::attribute) && (it.key() == "name")) {
      name = it.value();
    }
    if ((it.next() == xml::Element::attribute) && (it.key() == "value")) {
      val = it.value();
    }
    try {
      cell value = boost::lexical_cast<cell>(val);
      if (name == "A") {A_ = value;}
      else if (name == "B") {B_ = value;}
      else if (name == "P") {P_ = value;}
      else if (name == "T") {T_ = value;}
      else if (name == "S") {S_ = value;}
      else if (name == "R") {R_ = value;}
      else if (name == "IO") {IO_ = value;}
    }
    catch (...) {}
    if (it.next() == xml::Element::empty) {
  	  it.next_tag();
    }
  };
}

void RegisterSet::write(xml::Writer& w, const char* name, cell val)
{
  w.newline();
  w.open("reg", false);
  w.attribute("name", name);
  w.attribute("value", boost::lexical_cast<std::string>(val));
  w.close(true);
}

void RegisterSet::write(xml::Writer& w)
{
  if (A_ != F18_INVALID_VALUE) {write(w, "A", A_);}
  if (B_ != F18_INVALID_VALUE) {write(w, "B", B_);}
  if (P_ != F18_INVALID_VALUE) {write(w, "P", P_);}
  if (T_ != F18_INVALID_VALUE) {write(w, "T", T_);}
  if (S_ != F18_INVALID_VALUE) {write(w, "S", S_);}
  if (R_ != F18_INVALID_VALUE) {write(w, "R", R_);}
  if (IO_ != F18_INVALID_VALUE) {write(w, "IO", IO_);}
}


bool RegisterSet::writeable() const
{
  if (A_ != F18_INVALID_VALUE) {return true;}
  if (B_ != F18_INVALID_VALUE) {return true;}
  if (P_ != F18_INVALID_VALUE) {return true;}
  if (T_ != F18_INVALID_VALUE) {return true;}
  if (S_ != F18_INVALID_VALUE) {return true;}
  if (R_ != F18_INVALID_VALUE) {return true;}
  if (IO_ != F18_INVALID_VALUE) {return true;}
  return false;
}

void RegisterSet::reset()
{
	T_ = F18_INVALID_VALUE;   // top of parameter stack
	S_ = F18_INVALID_VALUE;   // second of parameter stack
	R_ = F18_INVALID_VALUE;   // top of return stack
	P_ = F18_INVALID_VALUE;   // program counter
	I_ = F18_INVALID_VALUE;   // instruction register
	A_ = F18_INVALID_VALUE;   // register A
	B_ = F18_INVALID_VALUE;   // register B
	IO_ = F18_INVALID_VALUE;  // I/O register
	UP_ = F18_INVALID_VALUE;  // up port
	LT_ = F18_INVALID_VALUE;  // left port
	DN_ = F18_INVALID_VALUE;  // down port
	RT_ = F18_INVALID_VALUE;  // right port
}


// ======================================================================================


Opcode::Enum Opcode::from_string(const std::string& val)
{
  static std::map<std::string, Enum> map;
  std::map<std::string, Enum>::iterator it = map.find(val);
  if (it != map.end()) {
    return it->second;
  }
  if (map.empty()) {
    std::string tmp;
    for (size_t i=0; i<limit; ++i) {
      to_string(static_cast<Enum>(i), tmp);
      map[tmp] = static_cast<Enum>(i);
    }
    it = map.find(val);
    if (it != map.end()) {
      return it->second;
    }
  }
  return limit;
}

void Opcode::to_string(Enum op, std::string& value, size_t size)
{
  switch (op) {
  case ret:         value = "ret"; break;
  case ex:          value = "ex"; break;
  case jump:        value = "jump"; break;
  case call:        value = "call"; break;
  case unext:       value = "unext"; break;
  case next:        value = "next"; break;
  case _if:         value = "if"; break;
  case mif:         value = "-if"; break;
  case fetch_p:     value = "@p"; break;
  case fetch_plus:  value = "@+"; break;
  case fetch_b:     value = "@b"; break;
  case fetch:       value = "@"; break;
  case store_p:     value = "!p"; break;
  case store_plus:  value = "!+"; break;
  case store_b:     value = "!b"; break;
  case store:       value = "!"; break;
  case plus_mul:    value = "+*"; break;
  case mul2:        value = "2*"; break;
  case div2:        value = "2/"; break;
  case inv:         value = "inv"; break;
  case add:         value = "+"; break;
  case _and:        value = "and"; break;
  case _xor:        value = "xor"; break;
  case drop:        value = "drop"; break;
  case dup:         value = "dup"; break;
  case pop:         value = "r>"; break;
  case over:        value = "over"; break;
  case a:           value = "a"; break;
  case nop:         value = "."; break;
  case push:        value = ">r"; break;
  case b_store:     value = "b!"; break;
  case a_store:     value = "a!"; break;
  default: value.clear();
  }
  if (size > 0) {
    if (value.size() > size) {
      value.resize(size);
    }
    else {
      while (value.size() < size) {
        value.insert(value.begin(), ' ');
      }
    }
  }
}

size_t Opcode::max_size()
{
  static size_t res = 0;
  if (res == 0) {
    std::string instr;
    for (size_t i=0; i<limit; ++i) {
      to_string(static_cast<Enum>(i), instr);
      if (instr.size() > res) {
        res = instr.size();
      }
    }
  }
  return res;
}


bool Opcode::fit_slot3(Enum op)
{
  if ((op & F18A_SKIP_SLOT_MASK) == 0) {
    return true;
  }
  return false;
}

bool Opcode::has_addr_field(Enum op)
{
  switch (op) {
  case jump:
  case call:
  case next:
  case _if:
  case mif: return true;
  default:;
  }
  return false;
}

// ======================================================================================

void Register::to_string(Enum op, std::string& value, size_t size)
{
  switch (op) {
  case T:         value = "T"; break;
  case S:         value = "S"; break;
  case R:         value = "R"; break;
  case P:         value = "P"; break;
  case I:         value = "I"; break;
  case A:         value = "A"; break;
  case B:         value = "B"; break;
  case IO:        value = "IO"; break;
  default: value.clear();
  }
  if (size > 0) {
    if (value.size() > size) {
      value.resize(size);
    }
    else {
      while (value.size() < size) {
        value.insert(value.begin(), ' ');
      }
    }
  }
}

size_t Register::max_size()
{
  static size_t res = 0;
  if (res == 0) {
    std::string instr;
    for (size_t i=0; i<limit; ++i) {
      to_string(static_cast<Enum>(i), instr);
      if (instr.size() > res) {
        res = instr.size();
      }
    }
  }
  return res;
}

// ======================================================================================

Stack::Stack()
: limit_(0)
, pos_(0)
{
  reset();
}

Stack::Stack(size_t limit)
: limit_(limit)
, pos_(0)
{
  reset();
}

Stack::Stack(const Stack& src)
: limit_(src.limit_)
, pos_(0)
{
  for (size_t i=0; i<F18A_STACK_SIZE; ++i) {
    data_[i] = src.at(i);
  }
}

Stack& Stack::operator=(const Stack& src)
{
  limit_ = src.limit_;
  pos_ = 0;
  for (size_t i=0; i<F18A_STACK_SIZE; ++i) {
    data_[i] = src.at(i);
  }
  return *this;
}

Stack::~Stack()
{
}

void Stack::reset()
{
  pos_ = 0;
  for (size_t i=0; i<F18A_STACK_SIZE; ++i) {
    data_[i] = F18_INVALID_VALUE;
  }
}
 
void Stack::push(cell val)
{
  data_[pos_++] = val;
  if (pos_ >= limit_) {pos_ = 0;}
}

cell Stack::pop()
{
  if (pos_ == 0) {pos_ = limit_;}
  return data_[--pos_];
}


cell Stack::top() const
{
  if (pos_ == 0) {return data_[limit_-1];}
  return data_[pos_-1];
}

void Stack::data(size_t pos, cell val)
{
  at(pos) = val;
}

cell& Stack::at(size_t pos)
{
  size_t ind = pos_ - pos - 1;
  if (ind >= limit_) {ind += limit_;}
  return data_[ind];
}

cell Stack::at(size_t pos) const
{
  size_t ind = pos_ - pos - 1;
  if (ind >= limit_) {ind += limit_;}
  return data_[ind];
}


bool Stack::writeable() const
{
  size_t ind = limit_-1;
  while (at(ind) == F18_INVALID_VALUE) {
    if (ind == 0) {return false;}
    --ind;
  }
  return true;
}

void Stack::read(xml::Iterator& it)
{
  reset();
  if (it.current() == xml::Element::content) {
    std::vector<long> data;
    it.read(data);
    size_t limit = data.size();
    for (size_t i=0; i<limit; ++i) {
      push(data[i]);
    }
  	it.next_tag();
  }
}

void Stack::write(xml::Writer& w)
{
  cell val;
  std::vector<long> data;
  data.reserve(limit_);

  size_t ind = limit_-1;
  while (at(ind) == F18_INVALID_VALUE) {
    if (ind == 0) {
      w.close(false);
      return;
    }
    --ind;
    data.push_back(val);
  }
  while (ind > 0) {
    data.push_back(at(ind--));
  }
  data.push_back(at(0));
  w.close(false);
  w.write(data);
}

// ======================================================================================

Memory::Memory()
: limit_(0)
{
  reset();
}

Memory::Memory(size_t limit)
: limit_(limit)
{
  reset();
}

Memory::Memory(const Memory& src)
: limit_(src.limit_)
, source_(src.source_)
, comment_(src.comment_)
{
}

Memory& Memory::operator=(const Memory& src)
{
  limit_ = src.limit_;
  source_ = src.source_;
  comment_ = src.comment_;
  for (size_t i=0; i<limit_; ++i) {
    data_[i] = src.data_[i];
  }
  return *this;
}

Memory::~Memory()
{
}

void Memory::reset()
{
  label_map_.clear();
  for (size_t i=0; i<limit_; ++i) {
    data_[i] = F18_INVALID_VALUE;
  }
  source_.clear();
  comment_.clear();
}

void Memory::source(const std::string& val)
{
  source_ = val;
}

void Memory::comment(const std::string& val)
{
  comment_ = val;
}

void Memory::data(size_t begin, const cell* addr, size_t size)
{
  for (size_t i=0; i<size; ++i) {
    data_[begin+i] = addr[i];
  }
}

void Memory::insert_ram(size_t pos, cell val)
{
}

void Memory::remove_ram(size_t pos)
{
}

bool Memory::writeable() const
{
  if (!label_map_.empty()) {return true;}
  if (!source_.empty()) {return true;}
  if (!comment_.empty()) {return true;}
  for (size_t i=0; i<limit_; ++i) {
    if (data_[i] != F18_INVALID_VALUE) {return true;}
  }
  return false;
}

void Memory::read(xml::Iterator& it)
{
  reset();
	if ((it.current() == xml::Element::tag) && (it.tag() == "mem")) {
    it.next();
    if (it.current() == xml::Element::content) {
      std::vector<long> data;
      it.read(data);
      size_t limit = data.size();
      for (size_t i=0; i<limit; ++i) {
        data_[i] = data[i];
      }
  	  it.next_tag();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "mem")) {
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
}

void Memory::write(xml::Writer& w)
{
  std::vector<long> data;
  data.reserve(limit_);

  size_t ind = 0;
  for (size_t i=0; i<limit_; ++i) {
    data.push_back(data_[i]);
  }
  while (!data.empty() && (data.back() == F18_INVALID_VALUE)) {
    data.pop_back();
  }
  if (!data.empty()) {
    w.newline();
    w.open("mem", false, true);
    w.write(data);
    w.open("mem", true, true);
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
  if (!source_.empty()) {
    w.newline();
    w.open("source", false, true);
    w.write(source_);
    w.open("source", true, true);
  }
  if (!comment_.empty()) {
    w.newline();
    w.open("comment", false, true);
    w.write(comment_);
    w.open("comment", true, true);
  }
}

// ======================================================================================




// ======================================================================================


Module::Module()
: owner_(0)
, ps_(F18A_PARAMETER_STACK_SIZE)
, rs_(F18A_RETURN_STACK_SIZE)
, ram_(F18A_RAM_SIZE)
, rom_(F18A_ROM_SIZE)
{
  reset();
}

void Module::reset()
{
  reg_.named_.reset();
  ps_.reset();
  rs_.reset();
  ram_.reset();
  rom_.reset();
}

const std::string& Module::name() const
{
  if (owner_) {
    return owner_->name();
  }
  return name_;
}

void Module::owner(evb::Module& m)
{
  owner_ = &m;
}

bool Module::update(evb::Board& board)
{
  evb::Module_ptr mod = board.find_module(name_);
  if (mod.is_not_null()) {
    owner_ = mod.get();
    return true;
  }
  return false;
}

bool Module::writeable() const
{
  return
    !name().empty()
    || reg_.named_.writeable()
    || ps_.writeable()
    || rs_.writeable()
    || ram_.writeable()
    || rom_.writeable();
}

bool Module::find_label(const std::string& name, cell& addr)
{
  label_map_type::const_iterator it = ram().label_map().find(name);
  if (it != ram().label_map().end()) {
    addr = it->second;
    return true;
  }
  it = rom().label_map().find(name);
  if (it != rom().label_map().end()) {
    addr = it->second;
    return true;
  }
  return false;
}

void Module::read(xml::Iterator& it)
{
	if ((it.current() == xml::Element::tag) && (it.tag() == "name")) {
    it.next();
    if (it.current() == xml::Element::content) {
      name_ = it.value();
  	  it.next_tag();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "name")) {
      it.next_tag();
    }
  }
	if ((it.current() == xml::Element::tag) && (it.tag() == "regs")) {
	  it.next_tag();
    reg_.named_.read(it);
  	if ((it.current() == xml::Element::end) && (it.tag() == "regs")) {
      it.next_tag();
    }
  };
	if ((it.current() == xml::Element::tag) && (it.tag() == "ps")) {
	  it.next_tag();
    ps_.read(it);
  	if ((it.current() == xml::Element::end) && (it.tag() == "ps")) {
      it.next_tag();
    }
  };
	if ((it.current() == xml::Element::tag) && (it.tag() == "rs")) {
	  it.next_tag();
    rs_.read(it);
  	if ((it.current() == xml::Element::end) && (it.tag() == "rs")) {
      it.next_tag();
    }
  };
	if ((it.current() == xml::Element::tag) && (it.tag() == "ram")) {
	  it.next_tag();
    ram_.read(it);
  	if ((it.current() == xml::Element::end) && (it.tag() == "ram")) {
      it.next_tag();
    }
  };
	if ((it.current() == xml::Element::tag) && (it.tag() == "rom")) {
	  it.next_tag();
    rom_.read(it);
  	if ((it.current() == xml::Element::end) && (it.tag() == "rom")) {
      it.next_tag();
    }
  };
}

void Module::write(xml::Writer& w)
{
  if (!name().empty()) {
    w.newline();
    w.open("name", false, true);
    w.write(name());
    w.open("name", true, true);
  }
  if (reg_.named_.writeable()) {
    w.newline();
    w.open("regs", false, true);
    w.indent(1);
    reg_.named_.write(w);
    w.newline(-1);
    w.open("regs", true, true);
  }
  if (ps_.writeable()) {
    w.newline();
    w.open("ps", false, true);
    w.indent(1);
    ps_.write(w);
    w.newline(-1);
    w.open("ps", true, true);
  }
  if (rs_.writeable()) {
    w.newline();
    w.open("rs", false, true);
    w.indent(1);
    rs_.write(w);
    w.newline(-1);
    w.open("rs", true, true);
  }
  if (ram_.writeable()) {
    w.newline();
    w.open("ram", false, true);
    w.indent(1);
    ram_.write(w);
    w.newline(-1);
    w.open("ram", true, true);
  }
  if (rom_.writeable()) {
    w.newline();
    w.open("rom", false, true);
    w.indent(1);
    rom_.write(w);
    w.newline(-1);
    w.open("rom", true, true);
  }
}

size_t Module::append_init_node(cell_list_type& list, bool is_boot_frame, cell destination_adr)
{
  size_t len = 0;
  // write RAM
  size_t lower = 0;
  size_t upper = F18A_RAM_SIZE - 1;
  if (destination_adr == F18_INVALID_VALUE) {
    if (reg_.named_.P_ == F18_INVALID_VALUE) {
      destination_adr = F18A_COLD;
    }
    else {
      destination_adr = reg_.named_.P_;
    }
  }

  while ((upper > 0) && (ram_.data(upper) == F18_INVALID_VALUE)) {--upper;}
  while ((lower > F18A_RAM_SIZE - 1) && (ram_.data(lower) == F18_INVALID_VALUE)) {++lower;}
  if (is_boot_frame) {
    list.push_back(destination_adr);
    ++len;
    if (lower <= upper) {
      list.push_back(lower);
      ++len;
      list.push_back(upper - lower + 1);
      ++len;
      while (lower <= upper) {
        list.push_back(ram_.data(lower));
        ++lower;
        ++len;
      }
    }
    else {
      list.push_back(0);
      ++len;
      list.push_back(0);
      ++len;
    }
    return len;
  }
  if (lower <= upper) {
    // ram not empty
    list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_a));
    ++len;
    list.push_back(lower);
    ++len;
    list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
    ++len;
    list.push_back(upper - lower);
    ++len;
    list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop));
    ++len;
    while (lower <= upper) {list.push_back(ram_.data(lower)); ++lower; ++len;}
  }
  // write io
  if (reg_.named_.IO_ != F18_INVALID_VALUE) {
    list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_a));
    ++len;
    list.push_back(F18A_IO);
    ++len;
    list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::store_a));
    ++len;
    list.push_back(reg_.named_.IO_);
    ++len;
  }
  // write a
  if (reg_.named_.A_ != F18_INVALID_VALUE) {
    list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_a));
    ++len;
    list.push_back(reg_.named_.A_);
    ++len;
  }
  // write b
  if (reg_.named_.B_ == F18_INVALID_VALUE) {
     // default value of b is io
    list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_b));
    ++len;
    list.push_back(F18A_IO);
    ++len;
  }
  else {
    list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_b));
    ++len;
    list.push_back(reg_.named_.B_);
    ++len;
  }
  // write return stack
  if (reg_.named_.R_ != F18_INVALID_VALUE) {
    upper = 0;
    while ((upper < F18A_RETURN_STACK_SIZE) && (rs_.at(upper) != F18_INVALID_VALUE)) {++upper;}
    while (upper > 0) {
      --upper;
      list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
      ++len;
      list.push_back(rs_.at(upper));
      ++len;
    }
    list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
    ++len;
    list.push_back(reg_.named_.R_);
    ++len;
  }
  // write parameter stack
  if (reg_.named_.T_ != F18_INVALID_VALUE) {
    cell_list_type pstack;
    pstack.push_back(reg_.named_.T_);
    if (reg_.named_.S_ != F18_INVALID_VALUE) {
      pstack.push_back(reg_.named_.S_);
      upper = 0;
      while ((upper < F18A_RETURN_STACK_SIZE) && (ps_.at(upper) != F18_INVALID_VALUE)) {
        pstack.push_back(ps_.at(upper));
        ++upper;
      }
    }
    while (pstack.size() >= 4) {
      list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_stack4));
      ++len;
      list.push_back(pstack.back());
      pstack.pop_back();
      ++len;
      list.push_back(pstack.back());
      pstack.pop_back();
      ++len;
      list.push_back(pstack.back());
      pstack.pop_back();
      ++len;
      list.push_back(pstack.back());
      pstack.pop_back();
      ++len;
    }
    if (pstack.size() >= 3) {
      list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_stack3));
      ++len;
      list.push_back(pstack.back());
      pstack.pop_back();
      ++len;
      list.push_back(pstack.back());
      pstack.pop_back();
      ++len;
      list.push_back(pstack.back());
      pstack.pop_back();
      ++len;
    }
    if (pstack.size() >= 2) {
      list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_stack2));
      ++len;
      list.push_back(pstack.back());
      pstack.pop_back();
      ++len;
      list.push_back(pstack.back());
      pstack.pop_back();
      ++len;
    }
    if (pstack.size() >= 1) {
      list.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_stack1));
      ++len;
      list.push_back(pstack.back());
      pstack.pop_back();
      ++len;
    }
  }
  // jump to start address
  list.push_back(f18a::State::addr_to_jump(destination_adr));
  ++len;
  return len;
}

size_t Module::append_boot_stream(cell_list_type& list)
{
  size_t len = 0;
  list.push_back((reg_.named_.P_ == F18_INVALID_VALUE) ? 0xa9 : reg_.named_.P_);
  ++len;
  size_t lower = 0;
  size_t upper = F18A_RAM_SIZE - 1;
  while ((upper > 0) && (ram_.data(upper) == F18_INVALID_VALUE)) {--upper;}
  while ((lower > F18A_RAM_SIZE - 1) && (ram_.data(lower) == F18_INVALID_VALUE)) {++lower;}
  list.push_back(lower);
  ++len;
  if (lower <= upper) {
    // ram not empty
    list.push_back(upper - lower + 1);
    ++len;
    while (lower <= upper) {
      list.push_back(ram_.data(lower));
      ++lower;
      ++len;
    }
  }
  else {
    list.push_back(0);
    ++len;
  }
  return len;
}

void Module::tic()
{
}

void Module::toc()
{
}



}
