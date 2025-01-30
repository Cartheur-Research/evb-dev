#include <f18a.hpp>
#include <evb.hpp>
#include <deque>

namespace f18a {

Command::Enum Command::from_string(const std::string& val)
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
  return undefined;
}

void Command::to_string(Enum op, std::string& value, size_t size)
{
  switch (op) {
  case number:      value = "#"; break;
  case colon:       value = ":"; break;
  case dot:         value = "."; break;
  case skip:        value = ".."; break;
  case lesssl:      value = "<sl"; break;
  case comma:       value = ","; break;
  case semicolon:   value = ";"; break;
  case comment:     value = "("; break;
  case pcy:         value = "+cy"; break;
  case mcy:         value = "-cy"; break;
  case abegin:      value = "A["; break;
  case setp:        value = "=P"; break;
  case enda:        value = "]]"; break;
  case lcomment:    value = "//"; break;
  case here:        value = "here"; break;
  case begin:       value = "begin"; break;
  case _else:       value = "else"; break;
  case then:        value = "then"; break;
  case _if:         value = "if"; break;
  case _while:      value = "while"; break;
  case until:       value = "until"; break;
  case mif:         value = "-if"; break;
  case mwhile:      value = "-while"; break;
  case muntil:      value = "-until"; break;
  case zif:         value = "zif"; break;
  case ahead:       value = "ahead"; break;
  case avail:       value = "avail"; break;
  case leap:        value = "leap"; break;
  case again:       value = "again"; break;
  case repeat:      value = "repeat"; break;
  case _for:        value = "for"; break;
  case next:        value = "next"; break;
  case snext:       value = "*next"; break;
  case org:         value = "org"; break;
  case qorg:        value = "?org"; break;
  case nl:          value = "NL"; break;
  case ndl:         value = "NDL"; break;
  case equ:         value = "equ"; break;
  case its:         value = "its"; break;
  case swap:        value = "SWAP"; break;
  case lit:         value = "lit"; break;
  case alit:        value = "alit"; break;
  case await:       value = "await"; break;
  case load:        value = "LOAD"; break;
  case assign_a:    value = "/A"; break;
  case assign_b:    value = "/B"; break;
  case assign_io:   value = "/IO"; break;
  case assign_p:    value = "/P"; break;
  case include:     value = "include"; break;
  case do_add:      value = "[+]"; break;
  case do_sub:      value = "[-]"; break;
  case do_mul:      value = "[*]"; break;
  case do_div:      value = "[/]"; break;
  case do_and:      value = "AND"; break;
  case do_or:       value = "OR"; break;
  case do_xor:      value = "XOR"; break;
  case do_inv:      value = "INV"; break;
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

bool scan_next(const std::string& src, size_t& pos, size_t& start_pos, std::string& value, char del)
{
  size_t limit = src.size();
  while ((pos < limit) && ((src[pos] < ' ') || (src[pos] == del))) {
    ++pos;
  }
  if (pos >= limit) {
    return false;
  }
  start_pos = pos;
  value.clear();
  while ((pos < limit) && (src[pos] > ' ') && (src[pos] != del)) {
    value.push_back(src[pos++]);
  }
  return true;
}

bool skip_next(const std::string& src, size_t& pos, char del)
{
  size_t limit = src.size();
  while ((pos < limit) && (src[pos] != del)) {
    ++pos;
  }
  if (++pos >= limit) {
    return false;
  }
  return true;
}

bool skip_line(const std::string& src, size_t& pos)
{
  unsigned char ch;
  size_t limit = src.size();
  while (pos < limit) {
    ch = src[pos];
    if ((ch == 10) || (ch == 13)) {
      break;
    }
    ++pos;
  }
  while (pos < limit) {
    ch = src[pos];
    if ((ch >= ' ')) {
      break;
    }
    ++pos;
  }
  return true;
}

int to_digit(unsigned char ch, size_t base)
{
  if (base > 10) {
    if ((ch >= 'a') && (ch < ('a' + base - 10))) {
      return ch - 'a' + 10;
    }
    if ((ch >= 'A') && (ch < ('A' + base - 10))) {
      return ch - 'A' + 10;
    }
    if ((ch >= '0') && (ch <= '9')) {
      return ch - '0';
    }
  }
  else if ((ch >= '0') && (ch < ('0'+base))) {
    return ch - '0';
  }
  return -1;
}

bool convert_io_to_name(size_t addr, std::string& name)
{
  switch (addr) {
  case 0x175: name = "--l-"; return true;
  case 0x115: name = "-d--"; return true;
  case 0x135: name = "-dl-"; return true;
  case 0x1d5: name = "r---"; return true;
  case 0x1f5: name = "r-l-"; return true;
  case 0x195: name = "rd--"; return true;
  case 0x1b5: name = "rdl-"; return true;
  case 0x145: name = "---u"; return true;
  case 0x165: name = "--lu"; return true;
  case 0x105: name = "-d-u"; return true;
  case 0x125: name = "-dlu"; return true;
  case 0x1c5: name = "r--u"; return true;
  case 0x1e5: name = "r-lu"; return true;
  case 0x185: name = "rd-u"; return true;
  case 0x1a5: name = "rdlu"; return true;
  default: name.clear();
  }
  return false;
}

bool convert_to_number(const std::string& txt, cell& val)
{
  bool is_neg = false;
  size_t limit = txt.size();
  size_t radix = 10;
  cell res = 0;
  if (limit > 0) {
    size_t i = 0;
    if (i < limit) {
      switch (txt[i]) {
      case '+': ++i; break;
      case '-': is_neg = true; ++i; break;
      default:;
      }
    }
    if (i < limit) {
      switch (txt[i]) {
      case '%':
        radix = 2;
        break;

      case '$':
      case 'x':
      case 'X':
        radix = 16;
        ++i;
        break;
      default:;
      }
    }
    if (i >= limit) {return false;}
    while (i < limit) {
      char ch = txt[i++];
      int dval = to_digit(ch, radix);
      if (dval >= 0) {
        res *= radix;
        res += dval;
      }
      else {
        switch (ch) {
        case '.':
          break;

        case 'R':
        case 'r':
          if ((res < 2) || (res > 36)) {
            return false;
          }
          radix = res;
          res = 0;
          break;

        default:
          return false;
        }
      }
    }
    val = res;
    if (is_neg) {val = -val;}
    val &= 0x3ffff;
    return true;
  }
  return false;
}

State::State()
{
  module_ = new Module();
}


bool State::failed(const char* txt)
{
  error_ = txt;
  error_ += " token(";
  error_ += token_;
  error_ += ")";
  return false;
}

bool State::failed(const std::string& txt)
{
  error_ = txt;
  error_ += " token(";
  error_ += token_;
  error_ += ")";
  return false;
}

bool State::memory(size_t addr, cell val)
{
  addr &= ~F18A_EXT_ARITH_SIZE;
  if ((addr >= F18A_RAM_START) && (addr < F18A_ROM_START)) {
    module_->ram().data(addr & F18A_RAM_MASK, val);
    return true;
  }
  if ((addr >= F18A_ROM_START) && (addr < F18A_IO_START)) {
    module_->rom().data(addr & F18A_ROM_MASK, val);
    return true;
  }
  return failed("invalid memory address");
}

cell State::memory(size_t addr)
{
  addr &= ~F18A_EXT_ARITH_SIZE;
  if ((addr >= F18A_RAM_START) && (addr < F18A_ROM_START)) {
    return module_->ram().data(addr & F18A_RAM_MASK);
  }
  if ((addr >= F18A_ROM_START) && (addr < F18A_IO_START)) {
    return module_->rom().data(addr & F18A_ROM_MASK);
  }
  return 0;
}

bool State::convert_to_number(const std::string& txt, cell& val)
{
  label_map_type::iterator it = literals_.find(txt);
  if (it != literals_.end()) {
    val = it->second;
    return true;
  }
  it = rom_literals_.find(txt);
  if (it != rom_literals_.end()) {
    val = it->second;
    return true;
  }
  if (f18a::convert_to_number(txt, val)) {
    return true;
  }
  return false;
}

bool State::label_to_number(const std::string& txt, cell& val)
{
  label_map_type::iterator it = labels_.find(txt);
  if (it != labels_.end()) {
    val = it->second;
    return true;
  }
  it = rom_labels_.find(txt);
  if (it != rom_labels_.end()) {
    val = it->second;
    return true;
  }
  it = ext_labels_.find(txt);
  if (it != ext_labels_.end()) {
    val = it->second;
    return true;
  }
  return false;
}

size_t read_slot(cell data, size_t slot)
{
  data ^= 0x15555;
  if (slot == 3) {
    return static_cast<size_t>(data & F18A_FINAL_SLOT_MASK);
  }
  data >>= F18A_FINAL_SLOT_BITS;
  if (slot == 2) {
    return static_cast<size_t>(data & F18A_SLOT_MASK);
  }
  data >>= F18A_SLOT_BITS;
  if (slot == 1) {
    return static_cast<size_t>(data & F18A_SLOT_MASK);
  }
  data >>= F18A_SLOT_BITS;
  return static_cast<size_t>(data & F18A_SLOT_MASK);
}

cell write_slot(cell data, size_t slot, size_t val)
{
  cell slot_mask, slot_data = val;
  switch (slot) {
  case 0: slot_data <<= F18A_SLOT123_BITS; slot_mask = F18A_SLOT_MASK << F18A_SLOT123_BITS; break;
  case 1: slot_data <<= F18A_SLOT23_BITS; slot_mask = F18A_SLOT_MASK << F18A_SLOT23_BITS; break;
  case 2: slot_data <<= F18A_FINAL_SLOT_BITS; slot_mask = F18A_SLOT_MASK << F18A_FINAL_SLOT_BITS; break;
  case 3: slot_data >>= F18A_SKIP_SLOT_BITS; slot_mask = F18A_FINAL_SLOT_MASK; break;
  }
  slot_data ^= 0x15555 & slot_mask;
  return (data & ~slot_mask) | slot_data;
}

bool State::finish()
{
  if (single_processing_) {
    single_ = curr_;
  }
  else {
    if (!memory(instr_pos_, curr_)) {return false;}
  }
  instr_pos_ = memory_pos_;
  memory_pos_ = Processor::inc_addr(memory_pos_);
  slot_ = 0;
  curr_ = F18A_INITIAL_CELL;
  return true;
}

bool State::flush(bool fill_nop)
{
  if (slot_ == 0) {
    return true;
  }
  if (fill_nop) {
    while (slot_ < F18A_NO_OF_SLOTS) {
      curr_ = write_slot(curr_, slot_++, Opcode::nop);
    }
  }
  return finish();
}

bool State::append_cell(cell val)
{
  if (!memory(memory_pos_, val)) {return false;}
  memory_pos_ = Processor::inc_addr(memory_pos_);
  return true;
}

bool State::append_slot(size_t val, bool do_finish)
{
  if (slot_ < F18A_LAST_SLOT) {
    if (val >= F18A_SLOT_SIZE) {
      return failed("invalid slot value");
    }
    curr_ = write_slot(curr_, slot_++, val);
    return true;
  }
  if ((val & F18A_SKIP_SLOT_MASK) == 0) {
    // instruction does fit in final slot
    curr_ = write_slot(curr_, slot_++, val);
    if (do_finish) {
      return finish();
    }
    return true;
  }
  if (!flush()) {return false;}
  curr_ = write_slot(curr_, slot_++, val);
  return true;
}

bool State::append_call(Opcode::Enum op, size_t destination)
{
  bool full_jump = true;
  size_t adr_change = memory_pos_ ^ destination;
  size_t no_of_bits_in_addr = 0;
  while (adr_change != 0) {adr_change >>= 1; ++no_of_bits_in_addr;}
  if (no_of_bits_in_addr <= F18A_FINAL_SLOT_BITS) {
    // addr will fit into slot 3
    if (slot_ < 3) {
      full_jump = false;
    }
  }
  else if (no_of_bits_in_addr <= (F18A_FINAL_SLOT_BITS + F18A_SLOT_BITS)) {
    // addr will fit into slot 2+3
    if (slot_ < 2) {
      full_jump = false;
    }
  }
  if (full_jump) {
    // addr occupies whole word
    if (!flush()) {return false;}
  }
  if (op == Opcode::call) {
    last_call_pos_ = instr_pos_;
    last_call_slot_ = slot_;
  }
  else {
    last_call_slot_ = F18A_NO_OF_SLOTS;
  }
  if (!append_slot(op)) {return false;}
  long mask = Processor::get_data_mask(slot_);
  destination &= mask;
  curr_ &= ~mask;
  curr_ |= destination;
  return finish();
}

long State::sa()
{
  long res = memory_pos_;
  res -= instr_pos_;
  res <<= 3;
  res += slot_;
  res <<= F18A_ADDR_BITS;
  res += instr_pos_;
  return res;
}

size_t State::sa_instr_addr(long sa)
{
  return static_cast<size_t>(sa & F18A_ADDR_MASK);
}

size_t State::sa_mem_addr(long sa)
{
  return sa_instr_addr(sa) + sa_offset(sa);
}

size_t State::sa_slot(long sa)
{
  return static_cast<size_t>((sa >> (F18A_ADDR_BITS)) & 7);
}

size_t State::sa_offset(long sa)
{
  return static_cast<size_t>(sa >> (F18A_ADDR_BITS+3));
}

bool State::stack_empty()
{
  if (stack_.empty()) {
    return !failed("stack empty for call");
  }
  return false;
}

bool State::here()
{
  if (!flush()) {return false;}
  stack_.push_back(instr_pos_);
  return true;
}

bool State::less_slot(size_t no)
{
  if (slot_ >= no) {
    return flush();
  }
  return true;
}

bool State::then()
{
  if (stack_empty()) {return false;}
  if (!flush()) {return false;}
  long sa = stack_.back();
  stack_.pop_back();
  size_t slot = sa_slot(sa);
  size_t instr_addr = sa_instr_addr(sa);
  size_t jmp_addr = sa_mem_addr(sa);
  size_t diff = jmp_addr ^ instr_pos_;
  long instr = memory(instr_addr);
  switch(slot) {
  case 0: // instruction is in slot 0
    instr &= ~F18A_ADDR_MASK;
    instr &= ~F18A_ADDR_UNDEF_MASK;
    instr |= F18A_ADDR_UNDEF_BITS;
    instr |= instr_pos_ & F18A_ADDR_MASK;
    break;

  case 1: // instruction is in slot 1
    if (diff >= F18A_SLOT23_SIZE) {
      return failed("fixup address does not fit into slot 2+3");
    }
    instr &= ~F18A_SLOT23_MASK;
    instr |= instr_pos_ & F18A_SLOT23_MASK;
    break;

  case 2: // instruction is in slot 1
    if (diff >= F18A_FINAL_SLOT_SIZE) {
      return failed("fixup address does not fit into slot 2+3");
    }
    instr &= ~F18A_FINAL_SLOT_MASK;
    instr |= instr_pos_ & F18A_FINAL_SLOT_MASK;
    break;

  default:
    return failed("no space for address");
  }
  memory(instr_addr, instr);
  return true;
}

bool State::fit(long sa)
{
  size_t diff = sa_instr_addr(sa) ^ memory_pos_;
  switch (slot_) {
  case 0: return true;
  case 1: return diff < F18A_SLOT23_SIZE;
  case 2: return diff < F18A_FINAL_SLOT_SIZE;
  }
  return false;
}

bool State::swap()
{
  if (stack_empty()) {return false;}
  cell val1 = stack_.back();
  stack_.pop_back();
  if (stack_empty()) {return false;}
  cell val2 = stack_.back();
  stack_.pop_back();
  stack_.push_back(val1);
  stack_.push_back(val2);
  return true;
}

bool State::forward(Opcode::Enum op)
{
  if (slot_ == F18A_LAST_SLOT) {
    if (!flush()) {return false;}
  }
  stack_.push_back(sa());
  if (!append_slot(op)) {return false;}
  return flush();
}

void State::preset()
{
  //module_->reset();
  error_.clear();
  include_map_.clear();
  if (rom_literals_.empty()) {
    rom_literals_["io"]     = 0x15d;
    rom_literals_["right"]  = 0x1d5;
    rom_literals_["down"]   = 0x115;
    rom_literals_["left"]   = 0x175;
    rom_literals_["up"]     = 0x145;
    rom_literals_["ldata"]  = 0x171;
    rom_literals_["data"]   = 0x141;
    rom_literals_["warp"]   = 0x157;
    rom_literals_["center"] = 0x1a5;
    rom_literals_["top"]    = 0x1b5;
    rom_literals_["side"]   = 0x185;
    rom_literals_["corner"] = 0x195;
    //rom_literals_[""] = 0x;
  }
  if (rom_labels_.empty()) {
    rom_labels_["--l-"] = 0x175;
    rom_labels_["-d--"] = 0x115;
    rom_labels_["-dl-"] = 0x135;
    rom_labels_["r---"] = 0x1d5;
    rom_labels_["r-l-"] = 0x1f5;
    rom_labels_["rd--"] = 0x195;
    rom_labels_["rdl-"] = 0x1b5;
    rom_labels_["---u"] = 0x145;
    rom_labels_["--lu"] = 0x165;
    rom_labels_["-d-u"] = 0x105;
    rom_labels_["-dlu"] = 0x125;
    rom_labels_["r--u"] = 0x1c5;
    rom_labels_["r-lu"] = 0x1e5;
    rom_labels_["rd-u"] = 0x185;
    rom_labels_["rdlu"] = 0x1a5;
  }
  curr_ = F18A_INITIAL_CELL;
  slot_ = 0;
  last_call_slot_ = F18A_NO_OF_SLOTS;
  last_token_pos_ = 0;
  board_ = 0;
  stack_.clear();
  single_processing_ = false;
  instr_pos_ = 0;
  memory_pos_ = Processor::inc_addr(instr_pos_);
}


bool State::assemble(const std::string& module, const std::string& src, bool is_rom, Processor_ptr proc, evb::Board_ptr board)
{
  preset();
  board_ = board;
  //size_t i;
  module_->name(module);
  if (proc.is_not_null()) {
    rom_labels_["await"] = proc->warm();
    //if (proc->is_corner()) {
    //  rom_literals_["avail"] = rom_labels_["rd--"];
    //}
    //else if (proc->is_h_border()) {
    //  rom_literals_["avail"] = rom_labels_["rd-u"];
    //}
    //else if (proc->is_v_border()) {
    //  rom_literals_["avail"] = rom_labels_["rdl-"];
    //}
    //else {
    //  rom_literals_["avail"] = rom_labels_["rdlu"];
    //}
  }
  module_->reg().named_.reset();
  if (is_rom) {
    module_->rom().reset();
    //for (i=0; i<F18A_ROM_SIZE; ++i) {
    //  module_->rom().data(i, F18A_INITIAL_CELL);
    //}
    instr_pos_ = F18A_ROM_START;
  }
  else {
    module_->ram().reset();
    module_->ps().reset();
    module_->rs().reset();
    //for (i=0; i<F18A_RAM_SIZE; ++i) {
    //  ram_[i] = F18A_INITIAL_CELL;
    //}
    instr_pos_ = F18A_RAM_START;
    // read rom labels
    if (proc.is_not_null() && (proc->config_size() > 0)) {
      const label_map_type& rom_labels = proc->config(0)->rom().label_map();
      for (label_map_type::const_iterator it=rom_labels.begin(); it != rom_labels.end(); ++it) {
        rom_labels_[it->first] = it->second;
      }
    }
  }
  memory_pos_ = Processor::inc_addr(instr_pos_);
  if (!load(src, is_rom, proc)) {return false;}
  if (!stack_.empty()) {
    return failed("stack is not empty after processing");
  }
  return true;
}

bool State::include_labels(Processor_ptr proc)
{
  if (proc->config_size() > 0) {
    const label_map_type& labels = proc->config(0)->ram().label_map();
    for (label_map_type::const_iterator it=labels.begin(); it != labels.end(); ++it) {
      ext_labels_[it->first] = it->second;
    }
  }
  if (proc->config_size() > 0) {
    const label_map_type& rom_labels = proc->config(0)->rom().label_map();
    for (label_map_type::const_iterator it=rom_labels.begin(); it != rom_labels.end(); ++it) {
      ext_labels_[it->first] = it->second;
    }
  }
  return true;
}

bool State::load(const std::string& src, bool is_rom, f18a::Processor_ptr target_proc)
{
  include_map_type::iterator it;
  f18a::Processor_ptr proc;
  f18a::Module_ptr module;
  std::string next_src;
  cell number, node;
  int scr_ind;
  size_t pos = 0;
  last_token_pos_ = 0;
  Opcode::Enum op;
  long par1, par2, res;

  while (scan_next(src, pos, last_token_pos_, token_)) {
    op = Opcode::from_string(token_);
    switch (op) {
		case Opcode::call:
		case Opcode::jump:
		//case Opcode::next:
		//case Opcode::_if:
 	//	case Opcode::mif:
      if (stack_empty()) {return false;}
      if (!append_call(op, stack_.back())) {return false;}
      stack_.pop_back();
      break;

		case Opcode::unext:
      // remove unused address
      if (stack_empty()) {return false;}
      stack_.pop_back();
    case Opcode::ret:
		case Opcode::fetch_p:
		case Opcode::fetch_plus:
		case Opcode::fetch_b:
		case Opcode::fetch:
		case Opcode::store_p:
		case Opcode::store_plus:
		case Opcode::store_b:
		case Opcode::store:
		case Opcode::plus_mul:
		case Opcode::mul2:
		case Opcode::div2:
		case Opcode::inv:
		case Opcode::add:
		case Opcode::_and:
		case Opcode::_xor:
		case Opcode::drop:
		case Opcode::dup:
		case Opcode::pop:
		case Opcode::over:
		case Opcode::a:
		case Opcode::nop:
		case Opcode::push:
		case Opcode::b_store:
		case Opcode::a_store:
      last_call_slot_ = F18A_NO_OF_SLOTS;
      // generate normal instruction
      if (!append_slot(op)) {return false;}
      break;

		case Opcode::ex: // ex skips the remaining slots
      last_call_slot_ = F18A_NO_OF_SLOTS;
      // generate normal instruction
      if (!append_slot(op)) {return false;}
      if (!flush()) {return false;}
      break;

    default: // opcode not found or not processed
      // label not found
      switch(Command::from_string(token_)) {

      case Command::abegin:
        if (single_processing_) {return failed("A[ cannot be nested");}
        stack_.push_back(slot_);
        stack_.push_back(instr_pos_);
        stack_.push_back(memory_pos_);
        stack_.push_back(curr_);
        stack_.push_back(last_token_pos_);
        single_ = curr_ = F18A_INITIAL_CELL;
        slot_ = 0;
        single_processing_ = true;
        break;

      case Command::setp:
        if (!single_processing_) {return failed("not in a A[");}
        if (stack_empty()) {return false;}
        instr_pos_ = stack_.back(); stack_.pop_back();
        memory_pos_ = Processor::inc_addr(instr_pos_);
        break;

      case Command::enda:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!single_processing_) {return failed("not in a A[");}
        if (slot_ != 0) {
          flush();
        }
        if (stack_empty()) {return false;}
        last_token_pos_ = stack_.back(); stack_.pop_back();
        if (stack_empty()) {return false;}
        curr_ = stack_.back(); stack_.pop_back();
        if (stack_empty()) {return false;}
        memory_pos_ = stack_.back(); stack_.pop_back();
        if (stack_empty()) {return false;}
        instr_pos_ = stack_.back(); stack_.pop_back();
        if (stack_empty()) {return false;}
        slot_ = stack_.back(); stack_.pop_back();
        stack_.push_back(single_);
        single_processing_ = false;
        break;

      case Command::colon:
        if (!flush()) {return false;}
        if (!scan_next(src, pos, last_token_pos_, token_)) {return failed(": without a name");}
        labels_[token_] = instr_pos_;
        break;

      case Command::semicolon:
        if (last_call_slot_ < F18A_NO_OF_SLOTS) {
          // optimize last call to jump
          if (single_processing_) {
            single_ = write_slot(single_, last_call_slot_, Opcode::jump);
          }
          else {
            if (!memory(last_call_pos_, write_slot(memory(last_call_pos_), last_call_slot_, Opcode::jump))) {return false;}
          }
          last_call_slot_ = F18A_NO_OF_SLOTS;
          continue;
        }
        if (!append_slot(Opcode::ret)) {return false;}
        if (!flush(false)) {return false;}
        break;

      case Command::number:
        if (!scan_next(src, pos, last_token_pos_, token_)) {return failed("# without a token");}
        if (label_to_number(token_, number)) { // try label
          stack_.push_back(number);
        }
        else if (convert_to_number(token_, number)) {
          stack_.push_back(number);
        }
        else if ((board_ != 0) && board_->find_screen(token_, scr_ind)) {
          stack_.push_back(scr_ind);
        }
        else {
          return failed("cannot convert to number");
        }
        break;

      case Command::dot:
        if (!append_slot(Opcode::nop)) {return false;}
        break;

      case Command::skip:
        if (!flush()) {return false;}
        break;

      case Command::lesssl:
        if (stack_empty()) {return false;}
        if (slot_ >= stack_.back()) {
          if (!flush()) {return false;}
        }
        stack_.pop_back();
        break;

      case Command::here:
        if (!here()) {return false;}
        break;

      case Command::begin:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!here()) {return false;}
        break;

      case Command::then:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!then()) {return false;}
        break;

      case Command::_if:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!forward(Opcode::_if)) {return false;}
        break;

      case Command::_while:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!forward(Opcode::_if)) {return false;}
        if (!swap()) {return false;}
        break;

      case Command::mif:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!forward(Opcode::mif)) {return false;}
        break;

      case Command::mwhile:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!forward(Opcode::mif)) {return false;}
        if (!swap()) {return false;}
        break;

      case Command::zif:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!forward(Opcode::next)) {return false;}
        break;

      case Command::avail:
        if (target_proc.is_null()) {return failed("avail failed");}
        stack_.push_back(target_proc->warm());
        break;

      case Command::ahead:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (slot_ == F18A_LAST_SLOT) {
          if (!flush()) {return false;}
        }
        stack_.push_back(sa());
        if (!append_slot(Opcode::jump)) {return false;}
        if (!flush()) {return false;}
        break;

      case Command::leap:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (slot_ == F18A_LAST_SLOT) {
          if (!flush()) {return false;}
        }
        stack_.push_back(sa());
        if (!append_slot(Opcode::call)) {return false;}
        if (!flush()) {return false;}
        break;

      case Command::comma:
        if (slot_ == 0) {
          curr_ = stack_.back();
          slot_ = 4;
          if (!flush()) {return false;}
        }
        else {
          if (stack_empty()) {return false;}
          if (!append_cell(stack_.back())) {return false;}
        }
        stack_.pop_back();
        break;

      case Command::org:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!flush()) {return false;}
        if (stack_empty()) {return false;}
        instr_pos_ = stack_.back();
        memory_pos_ = instr_pos_+1;
        stack_.pop_back();
        break;

      case Command::qorg:
        if (stack_empty()) {return false;}
        if (instr_pos_ != stack_.back()) {
          stack_.pop_back();
          std::string addr_txt;
          std::stringstream ss;
          ss << std::hex << instr_pos_;
          return failed("?org failed @"+ss.str()+"("+boost::lexical_cast<std::string>(instr_pos_)+")");
        }
        stack_.pop_back();
        break;

      case Command::nl:
      case Command::ndl:
        if (stack_empty()) {return false;}
        if (!scan_next(src, pos, last_token_pos_, token_)) {return failed("NL/NDL without a name");}
        if (is_rom) {rom_literals_[token_] = stack_.back();}
        else {literals_[token_] = stack_.back();}
        stack_.pop_back();
        break;

      case Command::equ:
        if (stack_empty()) {return false;}
        if (!scan_next(src, pos, last_token_pos_, token_)) {return failed("equ without a name");}
        //if (is_rom) {rom_labels_[token_] = stack_.back();}
        //else {labels_[token_] = stack_.back();}
        labels_[token_] = stack_.back();
        stack_.pop_back();
        break;

      case Command::its:
        // ( n) include
        if (stack_empty()) {return false;}
        node = stack_.back();
        stack_.pop_back();
        proc = board_->processor(node);
        if (proc.is_null()) {return failed("invalid node " + boost::lexical_cast<std::string>(node));}
        // lookup label in node
        if (!scan_next(src, pos, last_token_pos_, token_)) {return failed("missing label name");}
        module = proc->config_by_name(module_->name());
        if (module.is_null()) {
          return failed("module " + module_->name() + " not found in node " + boost::lexical_cast<std::string>(node));
        }
        if (!module->find_label(token_, number)) {
          return failed("label " + token_ + " not found in moule for node " + boost::lexical_cast<std::string>(node));
        }
        if (!append_call(Opcode::call, number)) {return false;}
        break;

      case Command::pcy:
        if (!flush()) {return false;}
        instr_pos_ |= F18A_EXT_ARITH_SIZE;
        memory_pos_ |= F18A_EXT_ARITH_SIZE;
        break;

      case Command::mcy:
        if (!flush()) {return false;}
        instr_pos_ &= ~F18A_EXT_ARITH_SIZE;
        memory_pos_ &= ~F18A_EXT_ARITH_SIZE;
        break;

      case Command::until:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (stack_empty()) {return false;}
        if (!append_call(Opcode::_if, sa_instr_addr(stack_.back()))) {return false;}
        stack_.pop_back();
        break;

      case Command::muntil:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (stack_empty()) {return false;}
        if (!append_call(Opcode::mif, sa_instr_addr(stack_.back()))) {return false;}
        stack_.pop_back();
        break;

      case Command::again:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (stack_empty()) {return false;}
        if (!append_call(Opcode::jump, sa_instr_addr(stack_.back()))) {return false;}
        stack_.pop_back();
        break;

      case Command::repeat:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        //if (!swap()) {return false;}
        if (stack_empty()) {return false;}
        if (!append_call(Opcode::jump, sa_instr_addr(stack_.back()))) {return false;}
        stack_.pop_back();
        if (!then()) {return false;}
        break;

      case Command::_else:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!forward(Opcode::jump)) {return false;}
        if (!swap()) {return false;}
        if (!then()) {return false;}
        break;

      case Command::_for:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!append_slot(Opcode::push)) {return false;}
        if (!here()) {return false;}
        break;

      case Command::next:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (stack_empty()) {return false;}
        if (!append_call(Opcode::next, sa_instr_addr(stack_.back()))) {return false;}
        stack_.pop_back();
        break;

      case Command::snext:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (!swap()) {return false;}
        if (!append_call(Opcode::next, sa_instr_addr(stack_.back()))) {return false;}
        stack_.pop_back();
        break;

      case Command::comment:
        if (!skip_next(src, pos, ')')) {
          return failed("( without closing )");
        }
        break;

      case Command::lcomment:
        if (!skip_line(src, pos)) {
          return failed("( invalid line comment");
        }
        break;

      case Command::swap:
        if (!swap()) {return false;}
        break;

      case Command::lit:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (stack_empty()) {return false;}
        if (!append_slot(Opcode::fetch_p)) {return false;}
        if (slot_ == 0) {
          curr_ = stack_.back();
          slot_ = 4;
          if (!flush()) {return false;}
        }
        else if (!append_cell(stack_.back())) {return false;}
        stack_.pop_back();
        break;

      case Command::alit:
        last_call_slot_ = F18A_NO_OF_SLOTS;
        if (stack_empty()) {return false;}
        if (!append_slot(Opcode::fetch_p)) {return false;}
        if (slot_ == 0) {
          curr_ = stack_.back();
          slot_ = 4;
          if (!flush()) {return false;}
        }
        else if (!append_cell(stack_.back())) {return false;}
        stack_.pop_back();
        break;

      case Command::await:
        if (!label_to_number(token_, number)) {
          return failed("cannot find await address");
        }
        if (!append_call(Opcode::call, number)) {return false;}
        break;

      case Command::load:
        node = stack_.back();
        if ((board_ != 0) && board_->find_screen_source(node, next_src)) {
          stack_.pop_back();
          if (!load(next_src, is_rom, board_->processor(node))) {return false;}
        }
        else {
          return failed("cannot find screen " + boost::lexical_cast<std::string>(stack_.back()));
        }
        break;

      case Command::assign_a:
        if (stack_empty()) {return false;}
        module_->reg().named_.A_ = stack_.back();
        stack_.pop_back();
        break;

      case Command::assign_b:
        if (stack_empty()) {return false;}
        module_->reg().named_.B_ = stack_.back();
        stack_.pop_back();
        break;

      case Command::assign_io:
        if (stack_empty()) {return false;}
        module_->reg().named_.IO_ = stack_.back();
        stack_.pop_back();
        break;

      case Command::assign_p:
        if (stack_empty()) {return false;}
        module_->reg().named_.P_ = stack_.back();
        stack_.pop_back();
        break;

      case Command::do_add:
        if (stack_empty()) {return false;}
        par2 = stack_.back();
        stack_.pop_back();
        if (stack_empty()) {return false;}
        par1 = stack_.back();
        stack_.pop_back();
        res = par1 + par2;
        stack_.push_back(res);
        break;

      case Command::do_sub:
        if (stack_empty()) {return false;}
        par2 = stack_.back();
        stack_.pop_back();
        if (stack_empty()) {return false;}
        par1 = stack_.back();
        stack_.pop_back();
        res = par1 - par2;
        stack_.push_back(res);
        break;

      case Command::do_mul:
        if (stack_empty()) {return false;}
        par2 = stack_.back();
        stack_.pop_back();
        if (stack_empty()) {return false;}
        par1 = stack_.back();
        stack_.pop_back();
        res = par1 * par2;
        stack_.push_back(res);
        break;

      case Command::do_div:
        if (stack_empty()) {return false;}
        par2 = stack_.back();
        stack_.pop_back();
        if (stack_empty()) {return false;}
        par1 = stack_.back();
        stack_.pop_back();
        res = par1 / par2;
        stack_.push_back(res);
        break;

      case Command::do_and:
        if (stack_empty()) {return false;}
        par2 = stack_.back();
        stack_.pop_back();
        if (stack_empty()) {return false;}
        par1 = stack_.back();
        stack_.pop_back();
        res = par1 & par2;
        stack_.push_back(res);
        break;

      case Command::do_or:
        if (stack_empty()) {return false;}
        par2 = stack_.back();
        stack_.pop_back();
        if (stack_empty()) {return false;}
        par1 = stack_.back();
        stack_.pop_back();
        res = par1 | par2;
        stack_.push_back(res);
        break;

      case Command::do_xor:
        if (stack_empty()) {return false;}
        par2 = stack_.back();
        stack_.pop_back();
        if (stack_empty()) {return false;}
        par1 = stack_.back();
        stack_.pop_back();
        res = par1 ^ par2;
        stack_.push_back(res);
        break;

      case Command::do_inv:
        if (stack_empty()) {return false;}
        par1 = stack_.back();
        stack_.pop_back();
        res = ~par1;
        stack_.push_back(res);
        break;

      case Command::include:
        if (stack_empty()) {return false;}
        it = include_map_.find(stack_.back());
        if ((it == include_map_.end()) || !it->second) {
          // labels from this not not included yet
          if (board_.is_null()) {return failed("cannot include any node");}
          proc = board_->processor(stack_.back());
          if (proc.is_null()) {return failed("invalid node");}
          if (!include_labels(*proc)) {return false;}
        }
        stack_.pop_back();
        break;

      //case Command:::
      //  break;

      default:
        if (label_to_number(token_, number)) { // try label
          // label found, generate call
          if (!append_call(Opcode::call, number)) {return false;}
        }
        else if (convert_to_number(token_, number)) { // try number
          if (!append_slot(Opcode::fetch_p, false)) {return false;}
          if (!append_cell(number)) {return false;}
          if (slot_ > F18A_LAST_SLOT) {finish();}
        }
        else {
          // last possibility is that the text is a screen
          if ((board_ != 0) && board_->find_screen(token_, scr_ind)) {
            stack_.push_back(scr_ind);
          }
          else {
            return failed("invalid ");
          }
        }
      }
    }
    last_token_pos_ = pos;
  }
  if (slot_ > 0) {
    // flush final word
    if (!flush()) {return false;}
  }
  return true;
}

bool State::disassemble(std::string& dest)
{
  return false;
}

void State::write(Processor_ptr dest, size_t pos, bool is_rom) const
{
  Module_ptr module = dest->assert_config(pos);
  if (is_rom) {
    module->rom().label_map() = labels_;
    dest->module()->rom().label_map() = labels_;
    module->rom().data(0, module_->rom().data(), F18A_ROM_SIZE);
    dest->module()->rom().data(0, module_->rom().data(), F18A_ROM_SIZE);
  }
  else {
    module->ram().label_map() = labels_;
    dest->module()->ram().label_map() = labels_;
    module->ram().data(0, module_->ram().data(), F18A_RAM_SIZE);
    dest->module()->ram().data(0, module_->ram().data(), F18A_RAM_SIZE);
    dest->module()->reg() = module_->reg();
  } 
  module->reg() = module_->reg();
}

cell State::assemble1(const std::string& src)
{
  std::string txt = "A[ ";
  txt += src;
  txt += " ]]";
  State tmp;
  tmp.preset();
  tmp.load(txt, false, 0);
  return tmp.single_;
}

cell State::focusing_call(f18a::Port::Enum p)
{
  std::string txt = "A[ ";
  txt += f18a::Port::to_call(p);
  txt += " ]]";
  State tmp;
  tmp.preset();
  tmp.load(txt, false, 0);
  return tmp.single_;
}

cell State::focusing_jump(f18a::Port::Enum p)
{
  std::string txt = "A[ ";
  txt += f18a::Port::to_call(p);
  txt += " ; ]]";
  State tmp;
  tmp.preset();
  tmp.load(txt, false, 0);
  return tmp.single_;
}

cell State::addr_to_call(long addr)
{
  return write_slot(addr, 0, Opcode::call);
}

cell State::addr_to_jump(long addr)
{
  return write_slot(addr, 0, Opcode::jump);
}

}
