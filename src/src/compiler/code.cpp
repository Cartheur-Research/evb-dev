#include "cvmc.hpp"
#include <iomanip>

namespace cvmc {

#define CODE_CALL			0x0000
	//	0x0000 .. 0x7fff
#define CODE_BR				0x9000
	//	-0x400 .. 0x3ff
#define CODE_CBR			0x9800
	//	-0x400 .. 0x3ff
#define CODE_GLOBAL			0xb800
	//	0x000 .. 0x1ff
#define CODE_STO			0xba00
	//	0x000 .. 0x1ff
#define CODE_LDO			0xbc00
	//	0x000 .. 0x1ff
#define CODE_LA				0xbe00
//#define CODE_LA_LIMIT		0x1ff
	//	0x000 .. 0x1ff
#define CODE_LIT			0xd000
	//	-0x800 .. 0x7ff



#define CODE_PLIT			0x800e
#define CODE_LLIT			0x8014
#define CODE_POP			0x8018
#define CODE_PUSH			0x801a
#define CODE_RET			0x802e
#define CODE_XS				0x8030
#define CODE_XP				0x8032
#define CODE_TJMP			0x8034
#define CODE_PC				0x8037

#define CODE_XF				0xa033
#define CODE_TERM			0xa038
#define CODE_WAIT			0xa03c

#define CODE_ENTER			0xa800
	// 0x00 .. 0xff
#define CODE_EXIT			0xa900
	// 0x00 .. 0xff
#define CODE_STLOC			0xaa00
	// 0x00 .. 0xff
#define CODE_STPAR			0xab00
	// 0x00 .. 0xff
#define CODE_LDLOC			0xac00
	// 0x00 .. 0xff
#define CODE_LDPAR			0xad00
	// 0x00 .. 0xff
#define CODE_LALOC			0xae00
	// 0x00 .. 0xff
#define CODE_LAPAR			0xaf00
	// 0x00 .. 0xff

#define CODE_LOAD			  0xb008
#define CODE_STORE			0xb00c
#define CODE_LOAD_PUSH	0xb010
#define CODE_XG				  0xb029
#define CODE_LOAD_EXT		0xb02e
#define CODE_STORE_EXT	0xb035
#define CODE_STORE_MK		0xb03d

//#define CODE_SWAP			0xc015

#define CODE_SL     	0xc82d
#define CODE_SR     	0xc82f
#define CODE_USR			0xc831
#define CODE_ADD			0xc833
#define CODE_AND			0xc834
#define CODE_XOR			0xc835
#define CODE_OR				0xc836
#define CODE_CX				0xc838

#define CODE_ZEXT			0xe00f
#define CODE_ADDC			0xe013
#define CODE_LDD			0xe01d
#define CODE_STD			0xe01e
#define CODE_XD				0xe020
#define CODE_LSHC			0xe022
#define CODE_RSHC			0xe026
#define CODE_SEXT			0xe02b
#define CODE_UMUL			0xe030

#define CODE_XPT			0xe812
#define CODE_OUT			0xe814
#define CODE_IN				0xe815
#define CODE_LDHI			0xe80f
#define CODE_LDLO			0xe816
#define CODE_STHI			0xe817
#define CODE_STLO			0xe81c

#define CODE_SUB			0xf006
#define CODE_INC			0xf00a
#define CODE_DEC			0xf00c
#define CODE_EQ 			0xf00e
#define CODE_EQ0			0xf00f
#define CODE_FALSE		0xf010
#define CODE_TRUE			0xf011
#define CODE_NE 			0xf013
#define CODE_NE0			0xf014
#define CODE_UGT			0xf017
#define CODE_GT 			0xf01b
#define CODE_GT0 			0xf01c
#define CODE_GE 			0xf01d
#define CODE_GE0 			0xf01e
#define CODE_ULE			0xf01f
#define CODE_LE 			0xf023
#define CODE_LE0 			0xf024
#define CODE_LT 			0xf025
#define CODE_LT0 			0xf026
#define CODE_ULT			0xf027
#define CODE_UGE			0xf029

#define CODE_MUL2			0xf02b
#define CODE_UDIV2		0xf02c
#define CODE_DIV2			0xf02d
#define CODE_ABS			0xf02f
#define CODE_NEG			0xf031
#define CODE_NOP			0xf032
#define CODE_NOT			0xf033

#define CODE_XT				0xf034
#define CODE_LDT			0xf035
#define CODE_STT			0xf036

#define CODE_BITCNT		0xf037


//*************************************************************************************

class StringAssembler : Expression {
protected:
	const std::string&	txt_;
	size_t				      pos_;
	size_t				      len_;
	string_map_type		  label_map_;
  typedef std::vector<std::pair<std::string, size_t> > fixup_type;
  fixup_type          fixup_list_;

public:
	StringAssembler(const std::string& txt)
	: txt_(txt)
	, pos_(0)
	, len_(0)
	{
	}

	void label(CompilationUnit& cu, const std::string& name)
	{
    size_t pos = cu.code().size();
		label_map_[name] = pos;
    size_t limit = fixup_list_.size();
    if (limit > 0) {
      size_t i = 0;
      while (i<limit) {
        std::pair<std::string, size_t>& pair = fixup_list_[i];
        if (pair.first == name) {
          fixup(cu, pair.second, pos);
          fixup_list_.erase(fixup_list_.begin()+i);
          --limit;
        }
        else {++i;}
      }
    }
	}

	bool find_label(const std::string& name, size_t& loc) const
	{
		string_map_type::const_iterator it = label_map_.find(name);
		if (it == label_map_.end()) {
			loc = 0;
			return false;
		}
		loc = it->second;
		return true;
	}

	virtual void instr0(CompilationUnit&, const std::string& op)
  {
  }

	virtual void instr1(CompilationUnit&, const std::string& op, const std::string& arg0)
  {
  }

	virtual void fixup(CompilationUnit&, size_t position, size_t target)
  {
  }

	void add_fixup(const std::string& name, size_t pos)
  {
    fixup_list_.push_back(std::make_pair(name, pos));
  }

	virtual int compile(CompilationUnit& cu, bool push_result)
	{
		std::string op, arg0, arg1;
		int state = 0;
		pos_ = len_ = 0;
		while ((pos_+len_) < txt_.size()) {
			char ch = txt_.at(pos_+len_);
			if (ch <= ' ') {
				// whitespace
				if (len_ == 0) {++pos_;}
				else {
					switch (state) {
					case 0: op = txt_.substr(pos_, len_); break;
					case 1: arg0 = txt_.substr(pos_, len_); break;
					case 2: arg1 = txt_.substr(pos_, len_); break;
					default: throw exception("StringAssembler::compile too many arguments");
					}
					pos_ += len_;
					len_ = 0;
					++state;
				}
			}
			else if (ch == ':') { // label
        if ((state != 0) || (len_ == 0)) {
          throw exception("StringAssembler::compile invalid label");
        }
        label(cu, txt_.substr(pos_, len_));
				pos_ += len_+1;
        len_ = 0;
			}
			else if (ch == ';') {
				// 
				if (len_ > 0) {
					switch (state) {
					case 0: op = txt_.substr(pos_, len_); break;
					case 1: arg0 = txt_.substr(pos_, len_); break;
					case 2: arg1 = txt_.substr(pos_, len_); break;
					default: throw exception("StringAssembler::compile too many arguments");
					}
					++state;
					pos_ += len_;
					len_ = 0;
				}
				if (state > 0) {
					switch (state) {
					case 1: instr0(cu, op); break;
					case 2: instr1(cu, op, arg0); break;
					//case 3: instr2(cu.code(), op, arg0, arg1); break;
					default: throw exception("StringAssembler::compile invalid # of arguments");
					}
					state = 0;
				}
				op.clear();
				arg0.clear();
				arg1.clear();
				++pos_;
			}
			else {
				++len_;
			}
		}
		if (len_ > 0) {
			switch (state) {
			case 0: op = txt_.substr(pos_, len_); break;
			case 1: arg0 = txt_.substr(pos_, len_); break;
			case 2: arg1 = txt_.substr(pos_, len_); break;
			default: throw exception("StringAssembler::compile too many arguments");
			}
			++state;
			len_ = 0;
		}
		if (state > 0) {
			switch (state) {
			case 1: instr0(cu, op); break;
			case 2: instr1(cu, op, arg0); break;
			//case 3: instr2(cu.code(), op, arg0, arg1); break;
			default: throw exception("StringAssembler::compile invalid # of arguments");
			}
		}
    if (fixup_list_.size() != 0) {
      throw exception("StringAssembler::compile unresolved fixups");
    }
		return 0;
	}

};

//*************************************************************************************

#define F18A_P_STACK_SIZE 10
#define F18A_R_STACK_SIZE 9
#define F18A_MEMORY_SIZE 64
#define F18A_SLOT_SIZE 5
#define F18A_SLOT_MASK ((1 << F18A_SLOT_SIZE)-1)

class Node {
private:
  std::vector<int>  memory_;
  //std::vector<int>  p_reg_;
  //std::vector<int>  a_reg_;
  //std::vector<int>  b_reg_;
  //std::vector<int>  p_stack_;
  //std::vector<int>  r_stack_;

public:
  Node()
  {
    memory_.resize(F18A_MEMORY_SIZE, 0);
  }

  void set_memory(int offset, int value)
  {
    memory_[offset] = value;
  }

  int get_memory(int offset) const
  {
    return memory_[offset];
  }

  size_t size() const
  {
    return memory_.size();
  }

  size_t size16() const // size in 16 bit words
  {
    return 64+8;
  }


  //void finalize()
  //{
  //  for (size_t i=0; i<F18A_MEMORY_SIZE; ++i) {
  //    memory_[i] ^= 0x15555;
  //  }
  //}

};

class Word {
private:
  int   offset_;
  bool  immediate_;
  bool  system_;
  bool  is_valid_;

public:
  Word(int offset=0, bool immediate=false, bool system=false)
  : offset_(offset)
  , immediate_(immediate)
  , system_(system)
  , is_valid_(true)
  {
  }

  void invalidate() {is_valid_ = false;}

  int offset() const {return offset_;}
  int location() const {return offset_ << 2;}
  bool immediate() const {return immediate_;}
  bool system() const {return system_;}
  bool is_valid() const {return is_valid_;}

};



class F18AAssembler : public StringAssembler {
public:
  enum Opcode {
    ret,
    ex,
    jump,
    call,
    unext,
    next,
    if0,
    mif,
    fetchp,
    fetchinc,
    fetchb,
    fetch,
    storep,
    storeinc,
    storeb,
    store,
    addmul,
    mul2,
    div2,
    not,
    add,
    and,
    xor,
    drop,
    dup,
    pop,
    over,
    a,
    nop,
    push,
    bstore,
    astore
  };

  enum Target {
    undefined,
    instructions,
    packed,
    normal
  };

private:
  typedef std::map<std::string, Word> map_type;
  map_type          opcode_;
  map_type          dictionary_;
  map_type          named_literal_;
  std::vector<int>  stack_;
  std::vector<Node> nodes_;
  Target            target_;
  int               state_;
  int               ip_; //holds the address of the next word to store into target memory, corresponding with p register during
                    // execution. If negative, bits 10 and up are set, and the value of p is unknown for inline assembly.
                    // When an address is specified for inline assembly, the address is stored into the low order 10 bits of
                    // both 'IP and 'IW , and bits 10 and up of 'IP are zero.
  int               slot_; // The next unused slot [0..3] of the instruction in IWD . If 4, further ops are forbidden (inline).
  int               remembered_;
  int               iwd_; // 18-bit instruction word being built
  int               iw_; // holds the F18 address at which the instruction in IWD will be stored. If negative, bits 10 and up are
                    // set, the low order bits [0..9] are the same as they are in 'IP . In this case the instruction is being
                    // generated inline and will not be stored into a bin directly by the Assembler.
  int               cl_; //holds the slot number of call opcode in preceding word. Negative if there isn't one. Used in tail
                    // optimization. Second volatile cell is slot of last opcode stored.
  bool              vol_; // Volatile flag set zero by # and true after parsing a number, a named literal, or a named call. When
                    // true these things perform their normal behaviors of generating literals or calls. When false, each of
                    // them leaves a number on the Assembler's stack.
  bool              last_was_call_;
  bool              fill_node_; // code fills up entire nodes
                      // a node contains 
                      // 64+8 = 72  memory
                      // 8+2  = 10  stack
                      // 8+1  = 9   return stack
                      // 6+1  = 7   2 stack + 1 return stack + A + B + P + 1 for extra bits
                      // total  98


public:
	F18AAssembler(Target t, const std::string& txt)
	: StringAssembler(txt)
  , target_(t)
  , state_(0)
  , ip_(0)
  , slot_(0)
  , remembered_(-1)
  , iwd_(0)
  , iw_(0)
  , cl_(-1)
  , vol_(false)
  , last_was_call_(false)
	{
    nodes_.resize(1);
    opcode_["ex"] = Word(ex);
    opcode_["unext"] = Word(unext);
    opcode_["@p"] = Word(fetchp);
    opcode_["@+"] = Word(fetchinc);
    opcode_["@b"] = Word(fetchb);
    opcode_["@"] = Word(fetch);
    opcode_["!p"] = Word(storep);
    opcode_["!+"] = Word(storeinc);
    opcode_["!b"] = Word(storeb);
    opcode_["!"] = Word(store);
    opcode_["+*"] = Word(addmul);
    opcode_["2*"] = Word(mul2);
    opcode_["2/"] = Word(div2);
    opcode_["-"] = Word(not);
    opcode_["inv"] = Word(not);
    opcode_["+"] = Word(add);
    opcode_["and"] = Word(and);
    opcode_["or"] = Word(xor);
    opcode_["xor"] = Word(xor);
    opcode_["drop"] = Word(drop);
    opcode_["dup"] = Word(dup);
    opcode_["pop"] = Word(pop);
    opcode_["r>"] = Word(pop);
    opcode_["over"] = Word(over);
    opcode_["a"] = Word(a);
    opcode_["."] = Word(nop);
    opcode_["push"] = Word(push);
    opcode_[">r"] = Word(push);
    opcode_["b!"] = Word(bstore);
    opcode_["a!"] = Word(astore);
    // do not directly compile 'unext'
    opcode_["unext"].invalidate();
  }

	virtual void instr0(CompilationUnit&, const std::string& op) {throw exception("F18AAssembler::instr0 not implemented");}
	virtual void instr1(CompilationUnit&, const std::string& op, const std::string& arg0) {throw exception("F18AAssembler::instr1 not implemented");}
	virtual void fixup(CompilationUnit&, size_t position, size_t target) {throw exception("F18AAssembler::fixup not implemented");}

  static int get_shift(int slot)
  {
    switch (slot) {
    case 0: return 13;
    case 1: return 8;
    case 2: return 3;
    case 3: return -2;
    }
    return 0;
  }

  static int shift_value(int slot, int value)
  {
    switch (slot) {
    case 0: return value <<= 13;
    case 1: return value <<= 8;
    case 2: return value <<= 3;
    case 3:
      if ((value & 3) != 0) {
        IssueList::warning("F18AAssembler::shift_value mask out values in slot 3");
      }
      return value >>= 2;
    }
    return value;
  }


  Node& current()
  {
    return nodes_.back();
  }

  static size_t slot(int location)
  {
    return location & 3;
  }

  static size_t h(int location)
  {
    return (location >> 2) & 0x3ff;
  }

  static size_t ip(int location)
  {
    return location >> 12;
  }

  int to_location(int offset)
  {
    return offset << 2;
  }

  int location()
  {
    return (ip_ << 12) | (iw_ << 2) | slot_;
  }

  void finish_cell()
  {
    current().set_memory(iw_, iwd_ ^ 0x15555);
    slot_ = 0;
    iwd_ = 0;
    if (++ip_ >= F18A_MEMORY_SIZE) {
      ip_ = 0;
    }
    iw_ = ip_;
  }


  void flush_cell()
  {
    if (slot_ > 0) {
      while (!add_slot(nop)) {}
    }
  }

  void here()
  {
    flush_cell();
    push_stack(location());
  }

  void append_memory(int value)
  {
    if (++ip_ >= F18A_MEMORY_SIZE) {
      //IssueList::warning("F18AAssembler::append_memory memory overflow");
      ip_ = 0;
    }
    current().set_memory(ip_, value);
  }

  int read_slot(size_t offset, int slot)
  {
    int value = current().get_memory(offset);
    switch (slot) {
    case 0: return (value >> 13) & F18A_SLOT_MASK;
    case 1: return (value >> 8) & F18A_SLOT_MASK;
    case 2: return (value >> 3) & F18A_SLOT_MASK;
    case 3: return (value << 2) & F18A_SLOT_MASK;
    }
    return -1;
  }

  void write_slot(size_t offset, int slot, int val)
  {
    int value = current().get_memory(offset);
    switch (slot) {
    case 0: value &= ~(F18A_SLOT_MASK << 12); value |= (val & F18A_SLOT_MASK) << 13; break;
    case 1: value &= ~(F18A_SLOT_MASK << 7); value |= (val & F18A_SLOT_MASK) << 8; break;
    case 2: value &= ~(F18A_SLOT_MASK << 2); value |= (val & F18A_SLOT_MASK) << 3; break;
    case 3: value &= ~(F18A_SLOT_MASK >> 3); value |= (val & F18A_SLOT_MASK) >> 2; break;
    }
    current().set_memory(offset, value);
  }

  bool add_slot(int value, bool remember=false) // return true if cell is finished
  {
    value &= F18A_SLOT_MASK; // only 5 bits allowed
    last_was_call_ = false;
    cl_ = -1;
    if (remember) {
      remembered_ = location();
    }
    else {
      remembered_ = -1;
    }
    iwd_ |= shift_value(slot_, value);
    if (slot_ < 3) {
      ++slot_;
      return false;
    }
    finish_cell();
    return true;
  }

  //void add_literal(int value)
  //{
  //  if (state_ == 0) {
  //    push_stack(value);
  //  }
  // else {
  //   append_memory(value);
  //   add_slot(fetchp);
  //  }
  //}

  void push_stack(int value)
  {
    stack_.push_back(value);
  }

  int pop_stack()
  {
    int value = stack_.back();
    stack_.pop_back();
    return value;
  }

  bool parse_integer(const std::string& txt, int& val)
	{
    size_t pos, begin, size = txt.size();
    int ch;
    bool is_neg;
    begin = 0;
    val = 0;
    pos = 0;
    is_neg = false;
    if (txt[begin] == '-') {
      is_neg = true;
      ++begin;
    }
    if (size > begin) {
      if ((txt[begin] >= '0') && (txt[begin] <= '9')) {
        // try a number
        if (size > (pos+2)) {
          if ((txt[begin] == '0') && ((txt[begin+1] == 'x') || (txt[begin+1] == 'X'))) { // hex number
            pos = begin+2;
            while (pos < txt.size()) {
              ch = txt[pos];
              if ((ch >= '0') && (ch <= '9')) {ch = ch - '0';}
              else if ((ch >= 'a') && (ch <= 'f')) {ch = ch - 'a' + 10;}
              else if ((ch >= 'A') && (ch <= 'F')) {ch = ch - 'A' + 10;}
              else {break;}
              val <<= 4;
              val += ch;
              ++pos;
            }
            if (pos == size) {
              if (is_neg) {val = -val;}
              return true;
            }
          }
          if ((txt[begin] == '0') && ((txt[begin+1] == 'b') || (txt[begin+1] == 'B'))) { // binary number
            pos = begin+2;
            while (pos < size) {
              ch = txt[pos];
              if ((ch >= '0') && (ch <= '1')) {ch = ch - '0';}
              else {break;}
              val <<= 1;
              val += ch;
              ++pos;
            }
            if (pos == size) {
              if (is_neg) {val = -val;}
              return true;
            }
          }
        }
        // decimal number
        pos = begin;
        while (pos < size) {
          ch = txt[pos];
          if ((ch >= '0') && (ch <= '9')) {ch = ch - '0';}
          else {break;}
          val *= 10;
          val += ch;
          ++pos;
        }
        if (pos == size) {
          if (is_neg) {val = -val;}
          return true;
        }
      }
    }
    return false;
  }

  //void ahead(Opcode op)
  //{
  //  if (slot_ > 2) {flush_cell();}
  //  push_stack(location());
  //  add_slot(op);
  //  flush_cell();
  //}

  void compile_jump(Opcode op, size_t target_location)
  {
    int target = ip(target_location);
    int diff = ip(location()) ^ target;
    if (slot_ == 3) {flush_cell();}
    else if (slot_ == 2) {
      if (diff >= 8) {flush_cell();}
    }
    else if (slot_ == 1) {
      if (diff >= 256) {flush_cell();}
    }
    add_slot(op);
    int mem = ip(location());
    //int cell = current().get_memory(mem);
    switch (slot_) {
    case 1: iwd_ ^= target ^ 0x155; break;
    case 2: iwd_ ^= target ^ 0x55; break;
    case 3: iwd_ ^= target ^ 0x5; break;
    default: throw exception("F18AAssembler::compile_jump invalid slot");
    }
    //current().set_memory(mem, cell);
    finish_cell();
  }

  void ahead(int opcode)
  {
    if (slot_ >= 3) {flush_cell();};
    add_slot(opcode);
    push_stack(location());
    finish_cell();
  }

  void begin()
  {
    flush_cell();
    push_stack(location());
  }

  void swap()
  {
    int a = pop_stack();
    int b = pop_stack();
    push_stack(a);
    push_stack(b);
  }

  bool fit_value(int value)
  {
    if (slot_ == 3) {
      if ((value & 3) != 0) {
        return false;
      }
    }
    return true;
  }

  void fixup(int location, int dest_location)
  {
    int dest = ip(dest_location);
    int mem = ip(location);
    int slot_no = slot(location);
    int diff = mem ^ dest;
    int cell = current().get_memory(mem);
    switch (slot_no) {
    case 1: cell &= 0x3e000; break;
    case 2:
      if (diff >= 0x100) {
        IssueList::error("F18AAssembler::fixup offset does not fit in slots 2+3");
        diff &= 0xff;
      };
      cell &= 0x3ff00;
      break;
    case 3:
      if (diff >= 0x8) {
        IssueList::error("F18AAssembler::fixup offset does not fit in slot 3");
        diff &= 0x7;
      };
      cell &= 0x3fff8;
      break;
    default: throw exception("F18AAssembler::fixup invalid slot");
    }
    cell ^= dest;
    current().set_memory(mem, cell);
  }

  void then()
  {
    if (slot_ >= 3) {flush_cell();};
    fixup(pop_stack(), location());
  }

  void compile_branch(int opcode, int dest, bool remember)
  {
    if (slot_ == 0) {
      add_slot(opcode, remember);
      iwd_ |= dest ^ 0x155;
      finish_cell();
      return;
    }
    int src = ip_;
    int diff = src ^ dest;
    if ((slot_ == 1) && (diff < 0x100)) {
      add_slot(opcode, remember);
      iwd_ |= dest ^ 0x55;
      finish_cell();
      return;
    }
    if ((slot_ == 2) && (diff < 0x8)) {
      add_slot(opcode, remember);
      iwd_ |= dest ^ 0x5;
      finish_cell();
      return;
    }
    flush_cell();
    compile_branch(opcode, dest, remember);
  }

  void compile_word(const std::string& txt)
	{
    if (state_ == 0) { // ordinary word
      map_type::iterator it;
      it = dictionary_.find(txt);
      if (it != dictionary_.end()) { // word found in dictionary
        // compile call
        int call_loc = location();
        compile_branch(call, it->second.offset(), true);
        cl_ = call_loc;
        last_was_call_ = true;
        return;
      }
      it = opcode_.find(txt);
      if ((it != opcode_.end()) && it->second.is_valid()) { // word is an opcode
        if (!fit_value(it->second.offset())) { // flush cell if opcode does not fit
          flush_cell();
        }
        add_slot(it->second.offset());
        return;
      }
      int value;
      if (parse_integer(txt, value)) {
        push_stack(value);
        return;
      }
      // no word & no number -> special word
      if (txt.size() == 1) {
        if (txt == ":") {
          state_ = 1;
        }
        else if (txt == ";") {
          if (cl_ >= 0) { // convert call to a jump
            int op = call ^ jump;
            op <<= get_shift(slot(cl_));
            current().set_memory(h(cl_), current().get_memory(h(cl_)) ^ op);
            cl_ = -1;
          }
          else if (last_was_call_) { // convert call to a jump
            int op = call ^ jump;
            op <<= get_shift(slot(remembered_));
            current().set_memory(h(remembered_), current().get_memory(h(remembered_)) ^ op);
            last_was_call_ = false;
          }
          else {
            add_slot(ret);
            if (slot_ > 0) {finish_cell();}
          }
        }
        else {
          throw exception("F18AAssembler::compile_word invalid word " + txt);
        }
      }
      else {
        if (txt == "begin") {
          begin();
        }
        else if (txt == "if") {
          // ( - a ) if T is nonzero, program flow continues; otherwise jumps to matching then
          ahead(if0);
        }
        else if (txt == "while") {
          // ( x - ax ) if T is nonzero, program flow continues; otherwise jumps to matching then
          ahead(if0);
          swap();
        }
        else if (txt == "until") {
          // ( a - ) if T is nonzero, program flow continues; otherwise jumps to a
          compile_jump(if0, pop_stack());
        }
        else if (txt == "-if") {
          // ( - a ) if T is negative, program flow continues; otherwise jumps to matching then
          ahead(mif);
        }
        else if (txt == "-while") {
          // ( x - ax ) if T is negative, program flow continues; otherwise jumps to matching then
          ahead(mif);
          swap();
        }
        else if (txt == "-until") {
          // ( a - )  if T is negative, program flow continues; otherwise jumps to matching then
          compile_jump(mif, pop_stack());
        }
        else if (txt == "zif") {
          // if R is zero, pops the return stack and program flow contimues; otherwise decrements R and jumps to matching then
          ahead(next);
        }
        else if (txt == "end") {
          // ( a - )  unconditionally jumps to a
          compile_jump(jump, pop_stack());
        }
        else if (txt == "..") {
          flush_cell();
        }
        else if (txt == "lit") { // literal
          add_slot(fetchp);
          append_memory(pop_stack());
        }
        else if (txt == "org") {
          flush_cell();
          iw_ = pop_stack();
        }
        else if (txt == "ahead") {
          // ( - a )
          // jumps to matching then
          ahead(jump);
        }
        else if (txt == "leap") {
          // ( - a )
          // call to matching then
          ahead(call);
        }
        else if (txt == "then") {
          // ( a - )
          // forces word alignment and resolves a forward transfer
          then();
        }
        else if (txt == "for") {
          // ( - a )
          add_slot(push);
          flush_cell();
          push_stack(location());
        }
        else if (txt == "next") {
          // ( a - )
          compile_jump(next, pop_stack());
        }
        else if (txt == "unext") {
          // ( a - )
          if (h(location()) != h(pop_stack())) {
            IssueList::warning("F18AAssembler::compile_word unext destination not in same word");
          }
          add_slot(unext);
        }
        else if (txt == "*next") {
          // ( ax - x )
          swap();
          compile_jump(next, pop_stack());
        }
        else if (txt == "swap") {
          swap();
        }
        else if (txt == "node") { // start a new node
          flush_cell();
          Node n;
          nodes_.push_back(n);
        }
        else {
          throw exception("F18AAssembler::compile_word invalid word " + txt);
        }
      }
    }
    else if (state_ == 1) { // defining word
      state_ = 0;
      flush_cell();
      dictionary_[txt] = Word(iw_);
    }
  }

	virtual int compile(CompilationUnit& cu, bool push_result)
	{
    try {
		  std::string op, arg0, arg1;
		  pos_ = len_ = 0;
		  while ((pos_+len_) < txt_.size()) {
			  char ch = txt_.at(pos_+len_);
			  if (ch <= ' ') {
				  // whitespace
				  if (len_ == 0) {++pos_;}
				  else { // there is a word to compile
					  compile_word(txt_.substr(pos_, len_));
					  pos_ += len_;
					  len_ = 0;
				  }
			  }
			  else {
				  ++len_;
			  }
		  }
      if (len_ > 0) { // compile last word
  		  compile_word(txt_.substr(pos_, len_));
      }
      flush_cell();
      if (stack_.size() != 0) {
        IssueList::warning("F18AAssembler::compile parameter stack not empty after compilation");
      }
      //for (std::vector<Node>::iterator it=nodes_.begin(); it != nodes_.end(); ++it) {
      //  it->finalize();
      //}
    }
    catch (...) {
      IssueList::error("F18AAssembler::compile failed");
    }
		return 0;
  }
};

//*************************************************************************************

class CVM2Assembler : public StringAssembler {
private:
static string_map_type	instr0_map_g;
static string_map_type	instr1_map_g;

public:
	CVM2Assembler(const std::string& txt)
	: StringAssembler(txt)
	{
		if (instr0_map_g.empty()) {
			instr0_map_g["abs"]		= CODE_ABS;
			instr0_map_g["add"]		= CODE_ADD;
			instr0_map_g["addc"]	= CODE_ADDC;
			instr0_map_g["and"]		= CODE_AND;
			instr0_map_g["bcnt"]	= CODE_BITCNT;
			instr0_map_g["dec"]		= CODE_DEC;
			instr0_map_g["cx"]		= CODE_CX;
			instr0_map_g["div2"]	= CODE_DIV2;
			instr0_map_g["eq"]		= CODE_EQ;
			instr0_map_g["eq0"]		= CODE_EQ0;
			instr0_map_g["false"]	= CODE_FALSE;
			instr0_map_g["ge"]		= CODE_GE;
			instr0_map_g["ge0"]		= CODE_GE0;
			instr0_map_g["gt"]		= CODE_GT;
			instr0_map_g["gt0"]		= CODE_GT0;
			instr0_map_g["in"]		= CODE_IN;
			instr0_map_g["inc"]		= CODE_INC;
			instr0_map_g["ldhi"]	= CODE_LDHI;
			instr0_map_g["ld"]		= CODE_LOAD;
			instr0_map_g["ldd"]		= CODE_LDD;
			instr0_map_g["ldph"]	= CODE_LOAD_PUSH;
			instr0_map_g["ldlo"]	= CODE_LDLO;
			instr0_map_g["ldt"]		= CODE_LDT;
			instr0_map_g["ldx"]		= CODE_LOAD_EXT;
			instr0_map_g["le"]		= CODE_LE;
			instr0_map_g["le0"]		= CODE_LE0;
			instr0_map_g["lshc"]	= CODE_LSHC;
			instr0_map_g["lt"]		= CODE_LT;
			instr0_map_g["lt0"]		= CODE_LT0;
			instr0_map_g["mul2"]	= CODE_MUL2;
			instr0_map_g["ne"]		= CODE_NE;
			instr0_map_g["ne0"]		= CODE_NE0;
			instr0_map_g["neg"]		= CODE_NEG;
			instr0_map_g["nop"]		= CODE_NOP;
			instr0_map_g["not"]		= CODE_NOT;
			instr0_map_g["or"]		= CODE_OR;
			instr0_map_g["out"]		= CODE_OUT;
			instr0_map_g["pc"]		= CODE_PC;
			instr0_map_g["pop"]		= CODE_POP;
			instr0_map_g["push"]	= CODE_PUSH;
			instr0_map_g["ret"]		= CODE_RET;
			instr0_map_g["rshc"]	= CODE_RSHC;
			instr0_map_g["sext"]	= CODE_SEXT;
			instr0_map_g["sl"]		= CODE_SL;
			instr0_map_g["sr"]		= CODE_SR;
			instr0_map_g["st"]		= CODE_STORE;
			instr0_map_g["std"]		= CODE_STD;
			instr0_map_g["sthi"]	= CODE_STHI;
			instr0_map_g["stlo"]	= CODE_STLO;
			instr0_map_g["stmk"]	= CODE_STORE_MK;
			instr0_map_g["stt"]		= CODE_STT;
			instr0_map_g["stx"]		= CODE_STORE_EXT;
			instr0_map_g["sub"]		= CODE_SUB;
			instr0_map_g["term"]	= CODE_TERM;
			instr0_map_g["tjmp"]	= CODE_TJMP;
			instr0_map_g["true"]	= CODE_TRUE;
			instr0_map_g["udiv2"]	= CODE_UDIV2;
			instr0_map_g["uge"]		= CODE_UGE;
			instr0_map_g["ugt"]		= CODE_UGT;
			instr0_map_g["ule"]		= CODE_ULE;
			instr0_map_g["ult"]		= CODE_ULT;
			instr0_map_g["umul"]	= CODE_UMUL;
			instr0_map_g["usr"]		= CODE_USR;
			instr0_map_g["wait"]	= CODE_WAIT;
			instr0_map_g["xd"]		= CODE_XD;
			instr0_map_g["xf"]		= CODE_XF;
			instr0_map_g["xg"]		= CODE_XG;
			instr0_map_g["xor"]		= CODE_XOR;
			instr0_map_g["xp"]		= CODE_XP;
			instr0_map_g["xpt"]		= CODE_XPT;
			instr0_map_g["xs"]		= CODE_XS;
			instr0_map_g["xt"]		= CODE_XT;
			instr0_map_g["zext"]	= CODE_ZEXT;
		}
		if (instr1_map_g.empty()) {
			instr1_map_g["br"]		= CODE_BR;
			instr1_map_g["call"]	= CODE_CALL;
			instr1_map_g["cbr"]		= CODE_CBR;
			instr1_map_g["enter"]	= CODE_ENTER;
			instr1_map_g["exit"]	= CODE_EXIT;
			instr1_map_g["glob"]	= CODE_GLOBAL;
			instr1_map_g["la"]		= CODE_LA;
			instr1_map_g["lal"]	= CODE_LALOC;
			instr1_map_g["lap"]	= CODE_LAPAR;
			instr1_map_g["ldl"]	= CODE_LDLOC;
			instr1_map_g["ldo"]		= CODE_LDO;
			instr1_map_g["ldp"]	= CODE_LDPAR;
			instr1_map_g["lit"]		= CODE_LIT;
			instr1_map_g["llit"]	= CODE_LLIT;
			instr1_map_g["plit"]	= CODE_PLIT;
			instr1_map_g["stl"]	= CODE_STLOC;
			instr1_map_g["sto"]		= CODE_STO;
			instr1_map_g["stp"]	= CODE_STPAR;
		}
	}

	virtual void instr0(CompilationUnit& cu, const std::string& op)
	{
		string_map_type::iterator it = instr0_map_g.find(op);
		if (it == instr0_map_g.end()) {
			if (!op.empty() && (op.back() == ':')) {
				std::string name = op;
				name.pop_back();
				label(cu, name);
			}
			else {
				IssueList::error("CVM2Assembler::instr0 invalid instruction "+op);
			}
		}
		cu.code().append_data(it->second);
	}

	virtual void instr1(CompilationUnit& cu, const std::string& op, const std::string& arg0)
	{
		string_map_type::iterator it = instr1_map_g.find(op);
		if (it == instr1_map_g.end()) {
			IssueList::error("CVM2Assembler::instr1 invalid instruction "+op);
      return;
		}
    cvmc::ObjPtr c;
    Constant* cons;
    const char* warn;
		codepoint_type cp = it->second;
		size_t loc = 0;
		int offset = 0;
		int pos = 0;
		std::string name;
		switch (cp) {
		case CODE_CALL:
			if (find_label(arg0, loc)) {cu.code().compile_call(loc);}
			else {
        if (Parser::parse_number_constant(warn, arg0, c, true)) {
          cons = dynamic_cast<Constant*>(c.get());
          if ((cons == 0) || !cons->get_int(pos)) {
    			  IssueList::error("CVM2Assembler::instr1 invalid call address constant " + arg0);
          }
          cu.code().compile_call(static_cast<size_t>(pos));
  				IssueList::error("CVM2Assembler::instr1 invalid call address " + arg0);
        }
        else { // unresolved reference
	        XRefManager* xrefm = cu.code().manager();
	        if (xrefm == 0) {
		        throw exception("Call::compile no xref manager");
	        }
	        xrefm->add_call(arg0, cu.code().size());
	        cu.code().compile_call(0);
        }
      }
			break;

		case CODE_BR:
		case CODE_CBR:
			if (arg0.at(0) == '@') { // branch to absolut address
				name = arg0.substr(1);
        if (!Parser::parse_number_constant(warn, name, c, false)) {
  				IssueList::error("CVM2Assembler::instr1 invalid absolute address " + name);
        }
        cons = dynamic_cast<Constant*>(c.get());
        if ((cons == 0) || !cons->get_int(offset)) {
    			IssueList::error("CVM2Assembler::instr1 invalid absolute address constant " + name);
        }
				offset -= cu.code().size()+1;
			}
			else if (find_label(arg0, loc)) {
				offset = loc;
				offset -= cu.code().size()+1;
			}
			else {
        if (Parser::parse_number_constant(warn, arg0, c, true)) {
          cons = dynamic_cast<Constant*>(c.get());
          if ((cons == 0) || !cons->get_int(offset)) {
    			  IssueList::error("CVM2Assembler::instr1 invalid branch constant " + arg0);
          }
  				//IssueList::error("CVM2Assembler::instr1 invalid branch " + arg0);
        }
        else {
          add_fixup(arg0, cu.code().size());
          offset = 0;
        }
			}
			if ((offset < -0x400) || (offset > 0x3ff)) {
				IssueList::warning("CVM2Assembler::instr1 invalid branch " + boost::lexical_cast<std::string>(offset));
			}
			cu.code().append_data(cp | (offset & 0x7ff));
			break;

		case CODE_LAPAR:
		case CODE_LDPAR:
		case CODE_STPAR:
			offset = 2;
		case CODE_EXIT:
		case CODE_ENTER:
		case CODE_LALOC:
		case CODE_LDLOC:
		case CODE_STLOC:
      if (!Parser::parse_number_constant(warn, arg0, c, true)) {
  			IssueList::error("CVM2Assembler::instr1 invalid offset " + arg0);
      }
      cons = dynamic_cast<Constant*>(c.get());
      if ((cons == 0) || !cons->get_int(pos)) {
    		IssueList::error("CVM2Assembler::instr1 invalid offset constant " + arg0);
      }
			pos += offset;
			if ((pos < 0) || (pos > PAR_LIMIT)) {
				IssueList::warning("CVM2Assembler::instr1 invalid offset " + boost::lexical_cast<std::string>(pos));
			}
			cu.code().append_data(cp | (pos & PAR_LIMIT));
			break;

		case CODE_LIT:
      if (!Parser::parse_number_constant(warn, arg0, c, true)) {
  			IssueList::error("CVM2Assembler::instr1 invalid short literal " + arg0);
      }
      cons = dynamic_cast<Constant*>(c.get());
      if ((cons == 0) || !cons->get_int(offset)) {
    		IssueList::error("CVM2Assembler::instr1 invalid short literal constant " + arg0);
      }
			if ((pos < -0x800) || (pos > 0x7ff)) {
				IssueList::warning("CVM2Assembler::instr1 invalid short literal " + boost::lexical_cast<std::string>(pos));
			}
			cu.code().append_data(cp | (offset & 0xfff));
			break;

		case CODE_LLIT:
		case CODE_PLIT:
      if (!Parser::parse_number_constant(warn, arg0, c, true)) {
  			IssueList::error("CVM2Assembler::instr1 invalid literal " + arg0);
      }
      cons = dynamic_cast<Constant*>(c.get());
      if ((cons == 0) || !cons->get_int(offset)) {
    		IssueList::error("CVM2Assembler::instr1 invalid literal constant " + arg0);
      }
			cu.code().append_data(cp);
			cu.code().append_data(offset);
			break;

		case CODE_GLOBAL:
		case CODE_LA:
		case CODE_LDO:
		case CODE_STO:
      if (!Parser::parse_number_constant(warn, arg0, c, false)) {
  			IssueList::error("CVM2Assembler::instr1 invalid index " + arg0);
      }
      cons = dynamic_cast<Constant*>(c.get());
      if ((cons == 0) || !cons->get_int(pos)) {
    		IssueList::error("CVM2Assembler::instr1 invalid index literal " + arg0);
      }
			if ((pos < 0) || (pos > OFFSET_LIMIT)) {
				IssueList::warning("CVM2Assembler::instr1 invalid index " + boost::lexical_cast<std::string>(pos));
			}
			loc = static_cast<size_t>(pos);
			cu.code().append_data(cp | (loc & OFFSET_LIMIT));
			break;


		}
	}

	//virtual void instr2(Code& code, const std::string& op, const std::string& arg0, const std::string& arg1)
	//{
	//	string_map_type::iterator it = instr2_map_g.find(op);
	//	if (it == instr2_map_g.end()) {
	//		IssueList::error("CVM2Assembler::instr2 invalid instruction "+op);
	//	}
 //   cvmc::ObjPtr c;
 //   Constant* cons;
 //   const char* warn;
	//	codepoint_type cp = it->second;
	//	int val0, val1;
	//	switch (cp) {
	//	case CODE_PLIT2:
 //     if (!Parser::parse_number_constant(warn, arg0, c, true)) {
 // 			IssueList::error("CVM2Assembler::instr2 invalid argument[0] " + arg0);
 //     }
 //     cons = dynamic_cast<Constant*>(c.get());
 //     if ((cons == 0) || !cons->get_int(val0)) {
 //   		IssueList::error("CVM2Assembler::instr2 invalid argument[0] constant " + arg0);
 //     }
 //     if (!Parser::parse_number_constant(warn, arg1, c, true)) {
 // 			IssueList::error("CVM2Assembler::instr2 invalid argument[1] " + arg0);
 //     }
 //     cons = dynamic_cast<Constant*>(c.get());
 //     if ((cons == 0) || !cons->get_int(val1)) {
 //   		IssueList::error("CVM2Assembler::instr2 invalid argument[1] constant " + arg0);
 //     }
	//		code.append_data(cp);
	//		code.append_data(val0);
	//		code.append_data(val1);
	//		break;
	//	}
	//}

	void fixup(CompilationUnit& cu, size_t position, size_t target)
  {
    codepoint_type cp = cu.code().data(position);
    int offset = target - position - 1;
		if ((offset < -0x400) || (offset > 0x3ff)) {
			IssueList::warning("CVM2Assembler::fixup invalid branch " + boost::lexical_cast<std::string>(offset));
		}
		cu.code().data(position,  (cp & ~0x7ff) | (offset & 0x7ff));
  }


};

string_map_type	CVM2Assembler::instr0_map_g;
string_map_type	CVM2Assembler::instr1_map_g;
//string_map_type	CVM2Assembler::instr2_map_g;


//*************************************************************************************

Assembler::Assembler()
: ass_(0)
{
}

Assembler::Assembler(const context_type::position_type& w)
: Expression(w)
, ass_(0)
{
}

Assembler::Assembler(const context_type::position_type& w, Type* t)
: Expression(w, t)
, ass_(0)
{
}

void Assembler::build_type()
{
	if (target() == "cvm2") {
  	set_type(new Type(T_VOID));
  }
	else if (target() == "f18a") {
    set_type(new ReferenceType(T_POINTER, Type::basic(T_INT)));
  }
	else if (target() == "f18ai") {
    // single instructions
		ass_ = new F18AAssembler(F18AAssembler::instructions, source());
    CompilationUnit cu;
		ass_->compile(cu, false);
    // either of type long (= 1 word)
    // or long[] (> 1 word)
    size_t len = cu.code().data().size();
    if (len == 1) {
    	set_type(new Type(T_LONG));
    }
    else {
    }
  }
	else {
  	set_type(new Type(T_VOID));
		throw exception("Assembler::build_type invalid target "+target());
	}
}

void Assembler::clear()
{
	if (ass_) {
		delete ass_;
		ass_ = 0;
	}
}

Assembler::~Assembler()
{
	clear();
}

void Assembler::target(const std::string& t)
{
  target_ = t;
  if (t == "f18a") {
    set_type(new ReferenceType(T_POINTER, Type::basic(T_INT)));
  }
}

void Assembler::optimize(ObjPtr& obj, int mask)
{
}

int Assembler::compile(CompilationUnit& cu, bool push_result)
{
	clear();
	if (target() == "cvm2") {
		ass_ = new CVM2Assembler(source());
		ass_->compile(cu, push_result);
	}
	else if (target() == "f18ai") {
    // 
    //if (ass_ == 0) {
		  //ass_ = new F18AAssembler(source());
		  //ass_->compile(cu, push_result);
    //}
	}
	else {
		throw exception("Assembler::compile unknown target "+target());
	}
	return 0;
}

//void Code::assemble(const std::string& txt)
//{
//}

//*************************************************************************************

//#define CODE_		0x

Code::Code()
: RefObj(OT_CODE)
, opt_pos_(0)
, opt_value_(0)
, is_data_(false)
, optimize_(true)
, optimize_space_(false)
, last_was_literal_(false)
, last_was_load_(false)
{
}

XRefManager* Code::manager()
{
	return static_cast<XRefManager*>(manager_.get());
}

void Code::manager(XRefManager* t)
{
	manager_ = t;
}

void Code::fixup_string_literal(size_t pos, size_t string_pos)
{
	if (string_pos > 0x3ff) {
		throw exception("Code::fixup_string_literal not reachable");
	}
	codepoint_type cp = data_.at(pos) & 0xf000;
	if (cp != CODE_LIT) {
		throw exception("Code::fixup_string_literal invalid fixup");
	}
	cp |= string_pos;
	data_.at(pos) = cp;
}

void Code::fixup_global(size_t pos, size_t global_pos)
{
	codepoint_type cp = data_.at(pos);
	global_pos += cp & GLOB_LIMIT;
	if (global_pos > GLOB_LIMIT) {
		throw exception("Code::fixup_global not reachable");
	}
	cp &= 0xf800;
	if (cp != CODE_GLOBAL) {
		throw exception("Code::fixup_global invalid fixup");
	}
	cp |= global_pos;
	data_.at(pos) = cp;
}

void Code::fixup_call(size_t pos, size_t function_pos)
{
	if (function_pos > 0x7fff) {
		throw exception("Code::fixup_call not reachable");
	}
	codepoint_type cp = data_.at(pos) & 0x8000;
	if (cp != 0) {
		throw exception("Code::fixup_call invalid fixup");
	}
	data_.at(pos) = function_pos;
}

void Code::no_last()
{
	last_was_load_ = last_was_literal_ = false;
}

void Code::clear()
{
	data_.clear();
}

void Code::append_data(codepoint_type cp)
{
	data_.push_back(cp);
}

void Code::append(codepoint_type cp)
{
	//if (!data_.empty()) {
	//	if ((cp == CODE_PUSH) && (data_.back() == (CODE_CLEANUP + 1))) {
	//		data_.back() = CODE_SWAP;
	//		return;
	//	}
	//}
	data_.push_back(cp);
}

void Code::append(const Code& c)
{
	data_.insert(data_.end(), c.data_.begin(), c.data_.end());
}

void Code::adjust(size_t pos, int value)
{
	data_[pos] += value;
}

void Code::data(size_t pos, codepoint_type cp)
{
	data_[pos] = cp;
}

codepoint_type Code::data(size_t pos) const
{
	return data_[pos];
}

int Code::compile_push()				// push R onto the stack, return the change in stack size
{
	append(CODE_PUSH);
	no_last();
	return 1;
}

int Code::compile_load_push()				// load word from mem[r] and push it on the stack. r is unchangeds
{
	append(CODE_LOAD_PUSH);
	no_last();
	return 1;
}

int Code::compile_pop()				// pop R from the stack, return the change in stack size
{
	append(CODE_POP);
	no_last();
	return -1;
}

int Code::compile_push_opt()
{	// pop R from the stack
	if (optimize_ && (opt_pos_ < data_.size())) {
		// we may have a possibility to optimize
		if (data_.back() == CODE_POP) {
			data_.pop_back(); // remove last operation
			return -1;
		}
	}
	if (optimize_ && last_was_literal_) {
		data_.resize(opt_pos_);
		append(CODE_PLIT);
		append(opt_value_);
		no_last();
		return 1;
	}
	if (optimize_ && last_was_load_) {
		data_.resize(opt_pos_);
		append(CODE_LOAD_PUSH);
		no_last();
		return 1;
	}
	append(CODE_PUSH);
	no_last();
	return 1;
}

int Code::compile_pop_opt()
{	// pop R from the stack
	if (optimize_ && (opt_pos_ < data_.size())) {
		// we may have a possibility to optimize
		if (data_.back() == CODE_PUSH) {
			data_.pop_back(); // remove last operation
			return 1;
		}
	}
	append(CODE_POP);
	no_last();
	return -1;
}

int Code::compile_inc()
{	// R += 1
	if (optimize_ && (opt_pos_ < data_.size())) {
		// we may have a possibility to optimize
		if (data_.back() == CODE_DEC) {
			data_.pop_back(); // remove last operation
			return 0;
		}
	}
	append(CODE_INC);
	no_last();
	return 0;
}

int Code::compile_dec()
{	// R -= 1
	if (optimize_ && (opt_pos_ < data_.size())) {
		// we may have a possibility to optimize
		if (data_.back() == CODE_INC) {
			data_.pop_back(); // remove last operation
			return 0;
		}
	}
	append(CODE_DEC);
	no_last();
	return 0;
}

int Code::compile_true()
{
  append(CODE_TRUE); no_last(); return -1;
}

int Code::compile_false()
{
  append(CODE_FALSE); no_last(); return -1;
}

int Code::compile_terminate() {append(CODE_TERM); no_last(); return 0;}				// set p to 1 and wait for stimulus

int Code::compile_wait() {append(CODE_WAIT); no_last(); return 0;}				// wait for stimulus

int Code::compile_return() {append(CODE_RET); no_last(); return 0;}				// return from call

int Code::compile_tjmp() {append(CODE_TJMP); no_last(); return 0;}				// table jump

int Code::compile_add() {append(CODE_ADD); no_last(); return -1;}					// R = pop() + R

int Code::compile_add_opt(int val)
{
  if (val < 0) {
    val = -val;
    if (val > 0) {return compile_sub_opt(val);}
  }
	if (val != 0) {
    if (val == 1) {
      compile_inc();
    }
		else if ((val >= 0) && (val <= OFFSET_LIMIT)) {
			compile_adr_offset(val);
		}
		else {
			compile_push_literal(val);
			append(CODE_ADD);
		}
	}
	return 0;
}

int Code::compile_sub_opt(int val)
{
  if (val < 0) {return compile_add_opt(-val);}
	if (val != 0) {
    if (val == 1) {
      compile_dec();
    }
		else {
      compile_push_opt();
			compile_literal(val);
			append(CODE_SUB);
		}
	}
	return 0;
}

int Code::compile_mul_opt(int val)
{
	if (val != 0) {
		if ((((val-1) & val) == 0) && (val > 0)) { // factor is power of 2
			val >>= 1;
			while (val) {
				val >>= 1;
				compile_mul2();
			}
		}
		else { // any other value
			compile_push();
			compile_literal(val);
			compile_mul();
		}
	}
	else {
		compile_literal_short(0);
	}
	return 0;
}

int Code::compile_sub() {append(CODE_SUB); no_last(); return -1;}					// R = pop() - R
int Code::compile_and() {append(CODE_AND); no_last(); return -1;}					// R = pop() & R
int Code::compile_or() {append(CODE_OR); no_last(); return -1;}					// R = pop() | R
int Code::compile_xor() {append(CODE_XOR); no_last(); return -1;}					// R = pop() ^ R
int Code::compile_negate() {append(CODE_NEG); no_last(); return 0;}				// R = -R
int Code::compile_not() {append(CODE_NOT); no_last(); return 0;}					// R = ~R
int Code::compile_eq0() {append(CODE_EQ0); no_last(); return 0;}					// R = !R (or R = R == 0)
int Code::compile_ne0() {append(CODE_NE0); no_last(); return 0;}					// R = !!R (or R = R != 0)
int Code::compile_mul2() {append(CODE_MUL2); no_last(); return 0;}				// R <<= 1
int Code::compile_div2() {append(CODE_DIV2); no_last(); return 0;}				// R >>= 1
int Code::compile_udiv2() {append(CODE_UDIV2); no_last(); return 0;}				// R >>= 1, unsigned
int Code::compile_load() {opt_pos_ = data_.size(); append(CODE_LOAD); no_last(); last_was_load_ = true; return 0;}				// R = mem(R)
int Code::compile_store() {append(CODE_STORE); no_last(); return -1;}				// mem(R) = pop()

int Code::compile_xs() {append(CODE_XS); no_last(); return 0;}					// R <-> S
int Code::compile_xf() {append(CODE_XF); no_last(); return 0;}					// R <-> F
int Code::compile_xp() {append(CODE_XP); no_last(); return 0;}					// R <-> P
//int Code::compile_xn() {append(CODE_XN); no_last(); return 0;}					// R <-> N
int Code::compile_xg() {append(CODE_XG); no_last(); return 0;}					// R <-> G
int Code::compile_xd() {append(CODE_XD); no_last(); return 0;}					// R <-> D
int Code::compile_xt() {append(CODE_XT); no_last(); return 0;}					// R <-> T
int Code::compile_xpt() {append(CODE_XPT); no_last(); return 0;}				// R <-> PT

int Code::compile_cx() {append(CODE_CX); no_last(); return -4;}				// compare and exchange

int Code::compile_bitcnt() {append(CODE_BITCNT); no_last(); return 0;}				// count 1 bits

int Code::compile_sl()						// R = pop() << R
{
	append(CODE_SL);
	no_last();
	return -1;
}

int Code::compile_sr()						// R = pop() << R
{
	append(CODE_SR);
	no_last();
	return -1;
}

int Code::compile_usr()						// R = pop() << R
{
	append(CODE_USR);
	no_last();
	return -1;
}

int Code::compile_eq()						// R = pop() == R
{
	append(CODE_EQ);
	no_last();
	return -1;
}

int Code::compile_ne()						// R = pop() != R
{
	append(CODE_NE);
	no_last();
	return -1;
}

int Code::compile_gt()						// R = pop() > R
{
	append(CODE_GT);
	no_last();
	return -1;
}

int Code::compile_ge()						// R = pop() >= R
{
	append(CODE_GE);
	no_last();
	return -1;
}

int Code::compile_lt()						// R = pop() < R
{
	append(CODE_LT);
	no_last();
	return -1;
}

int Code::compile_le()						// R = pop() <= R
{
	append(CODE_LE);
	no_last();
	return -1;
}

int Code::compile_ugt()						// R = pop() > R (unsigned)
{
	append(CODE_UGT);
	no_last();
	return -1;
}

int Code::compile_uge()						// R = pop() >= R (unsigned)
{
	append(CODE_UGE);
	no_last();
	return -1;
}

int Code::compile_ult()						// R = pop() < R (unsigned)
{
	append(CODE_ULT);
	no_last();
	return -1;
}

int Code::compile_ule()						// R = pop() <= R (unsigned)
{
	append(CODE_ULE);
	no_last();
	return -1;
}

int Code::compile_gt0()						// R = R > 0
{
	append(CODE_GT0);
	no_last();
	return 0;
}

int Code::compile_ge0()						// R = R >= 0
{
	append(CODE_GE0);
	no_last();
	return 0;
}

int Code::compile_lt0()						// R = R < 0
{
	append(CODE_LT0);
	no_last();
	return 0;
}

int Code::compile_le0()						// R = R <= 0
{
	append(CODE_LE0);
	no_last();
	return 0;
}

int Code::compile_mul()
{
	append(CODE_UMUL);
	no_last();
	return -1;
}

int Code::compile_umul()
{
	append(CODE_UMUL);
	no_last();
	return -1;
}

int Code::compile_sext()
{
	append(CODE_SEXT);
	no_last();
	return 0;
}

int Code::compile_zext()
{
	append(CODE_ZEXT);
	no_last();
	return 0;
}

int Code::compile_addc()
{
	append(CODE_ADDC);
	no_last();
	return 0;
}

int Code::compile_lshc()
{
	append(CODE_LSHC);
	no_last();
	return 0;
}

int Code::compile_rshc()
{
	append(CODE_RSHC);
	no_last();
	return 0;
}

int Code::compile_literal(int val, bool optimze) // 16 bit signed literal
{
	opt_pos_ = data_.size();
	opt_value_ = val;
	if ((val >= -0x800) && (val <= 0x7ff) && optimze) {
		append(CODE_LIT | (val & 0xfff));
	}
	else {
		append(CODE_LLIT);
		append(static_cast<codepoint_type>(val));
	}
	no_last();
	last_was_literal_ = true;
	return 0;
}

int Code::compile_literal_short(int val)
{ // 16 bit short literal
	append(CODE_LIT | (val & 0xfff));
	return 0;
}

int Code::compile_push_long(long val)
{
  int res = compile_push_literal(static_cast<short>(val));
  res += compile_push_literal(static_cast<short>(val >> 16));
  return res;
}

int Code::compile_literal_long(long val)
{ // 32 bit literal, hi in d, lo in r
	int res = 0;
	if (static_cast<short>(val) == val) {
		// constant fits into 16 bits
		res += compile_literal(static_cast<int>(val));
		append(CODE_SEXT);
	}
	else {
		res += compile_literal(static_cast<int>(val >> BIT_PER_INT));
		append(CODE_XD);
		res += compile_literal(static_cast<int>(val & INT_MASK));
	}
	return res;
}

int Code::compile_push_literal(int val)
{
	append(CODE_PLIT);
	append(static_cast<codepoint_type>(val));
	return 1;
}

int Code::compile_push_long()
{	// push D, R onto the stack
	int res = compile_push_opt();
	append(CODE_XD);
	res += compile_push_opt();
	return res;
}

int Code::compile_pop_long()
{	// pop D, R from the stack
	int res = compile_pop();
	append(CODE_XD);
	res += compile_pop();
	return res;
}

int Code::compile_push_long_opt(bool final_xd)
{	// push D, R onto the stack
	int res = compile_push_opt();
	append(CODE_XD);
	res += compile_push_opt();
	if (final_xd) {append(CODE_XD);}
	no_last();
	return res;
}

int Code::compile_pop_long_opt()
{	// pop D, R from the stack
	int res = compile_pop_opt();
	append(CODE_XD);
	res += compile_pop();
	append(CODE_XD);
	no_last();
	return res;
}

int Code::compile_stt() {append(CODE_STT); no_last(); return 0;}					// 
int Code::compile_ldt() {append(CODE_LDT); no_last(); return 0;}					// 

int Code::compile_std() {append(CODE_STD); no_last(); return 0;}					// 
int Code::compile_ldd() {append(CODE_LDD); no_last(); return 0;}					// 

int Code::compile_ldx() {append(CODE_LOAD_EXT); no_last(); return 0;}				//
int Code::compile_stx() {append(CODE_STORE_EXT); no_last(); return 0;}				//
int Code::compile_stmk() {append(CODE_STORE_MK); no_last(); return 0;}				//

int Code::compile_load_local(size_t offset)
{
	if (offset > LOC_LIMIT) {
		IssueList::error("Code::compile_load_local invalid index " + boost::lexical_cast<std::string>(offset));
	}
	append(CODE_LDLOC | (offset & LOC_LIMIT));
	no_last();
	return 0;
}

int Code::compile_store_local(size_t offset)
{
	if (offset > LOC_LIMIT) {
		IssueList::error("Code::compile_store_local invalid index " + boost::lexical_cast<std::string>(offset));
	}
	append(CODE_STLOC | (offset & LOC_LIMIT));
	no_last();
	return 0;
}

int Code::compile_adr_local(size_t offset)
{
	if (offset > LOC_LIMIT) {
		IssueList::error("Code::compile_adr_local invalid index " + boost::lexical_cast<std::string>(offset));
	}
	append(CODE_LALOC | (offset & LOC_LIMIT));
	no_last();
	return 0;
}

int Code::compile_load_parameter(size_t offset)
{
	if (offset > PAR_LIMIT) {
		IssueList::error("Code::compile_load_parameter invalid index " + boost::lexical_cast<std::string>(offset));
	}
	append(CODE_LDPAR | ((offset+2) & PAR_LIMIT));
	no_last();
	return 0;
}

int Code::compile_store_parameter(size_t offset)
{
	if (offset > PAR_LIMIT) {
		IssueList::error("Code::compile_store_parameter invalid index " + boost::lexical_cast<std::string>(offset));
	}
	append(CODE_STPAR | ((offset+2) & PAR_LIMIT));
	no_last();
	return 0;
}

int Code::compile_adr_parameter(size_t offset)
{
	if (offset > PAR_LIMIT) {
		IssueList::error("Code::compile_adr_parameter invalid index " + boost::lexical_cast<std::string>(offset));
	}
	append(CODE_LAPAR | ((offset+2) & PAR_LIMIT));
	no_last();
	return 0;
}

//int Code::compile_load_global(size_t offset)
//{
//	if (offset > 0x1ff) {
//		IssueList::error("Code::compile_store_local invalid index " + boost::lexical_cast<std::string>(offset));
//	}
//	append(CODE_GLOBAL | (offset & 0x1ff));
//	no_last();
//	return 0;
//}
//
//int Code::compile_store_global(size_t offset)
//{
//	if (offset > 0x1ff) {
//		IssueList::error("Code::compile_store_local invalid index " + boost::lexical_cast<std::string>(offset));
//	}
//	append(CODE_GLOBAL | (offset & 0x1ff));
//	no_last();
//	return 0;
//}

int Code::compile_adr_global(size_t offset)
{
	if (offset > GLOB_LIMIT) {
		IssueList::error("Code::compile_adr_global invalid index " + boost::lexical_cast<std::string>(offset));
	}
	append(CODE_GLOBAL | (offset & GLOB_LIMIT));
	no_last();
	return 0;
}

int Code::compile_load_offset(size_t offset)
{
	if (offset > OFFSET_LIMIT) {
		IssueList::error("Code::compile_load_offset invalid index " + boost::lexical_cast<std::string>(offset));
	}
	//append(CODE_LA | (offset & 0x1ff));
	//append(CODE_LOAD);
	append(CODE_LDO | (offset & OFFSET_LIMIT));
	no_last();
	return 1;
}

int Code::compile_store_offset(size_t offset)
{
	if (offset > OFFSET_LIMIT) {
		IssueList::error("Code::compile_store_offset invalid index " + boost::lexical_cast<std::string>(offset));
	}
	//append(CODE_LA | (offset & 0x1ff));
	//append(CODE_STORE);
	append(CODE_STO | (offset & OFFSET_LIMIT));
	no_last();
	return -1;
}

int Code::compile_adr_offset(size_t offset)
{
	if (offset > OFFSET_LIMIT) {
		IssueList::error("Code::compile_adr_offset invalid index " + boost::lexical_cast<std::string>(offset));
	}
	append(CODE_LA | (offset & OFFSET_LIMIT));
	no_last();
	return 0;
}

int Code::compile_reserve(size_t words_on_stack_to_be_reserved)
{
  int res = 0;
  switch (words_on_stack_to_be_reserved) {
  case 0: break;
  case 5:
    res += compile_push();
  case 4:
    res += compile_push();
  case 3:
    res += compile_push();
  case 2:
    res += compile_push();
  case 1:
    res += compile_push();
    break;
  default:
    res += compile_xs();
    res += compile_not();
    while (words_on_stack_to_be_reserved > 0) {
      if (words_on_stack_to_be_reserved > OFFSET_LIMIT) {
        res += compile_adr_offset(OFFSET_LIMIT);
        words_on_stack_to_be_reserved -= OFFSET_LIMIT;
      }
      else {
        res += compile_adr_offset(words_on_stack_to_be_reserved);
        words_on_stack_to_be_reserved = 0;
      }
    }
    res += compile_not();
    res += compile_xs();
  }
  return res;
}

int Code::compile_enter(size_t local_space)
{
	if (local_space > 0) {--local_space;}
	if (local_space > LOCALS_LIMIT) {
		IssueList::error("Code::compile_store_local invalid index " + boost::lexical_cast<std::string>(local_space));
	}
	append(CODE_ENTER | (local_space & LOCALS_LIMIT));
	no_last();
	return local_space;
}

int Code::compile_exit(size_t parameter_space)
{
	if (parameter_space > PARAMETER_LIMIT) {
		IssueList::error("Code::compile_exit invalid index " + boost::lexical_cast<std::string>(parameter_space));
	}
	append(CODE_EXIT | (parameter_space & PARAMETER_LIMIT));
	no_last();
	return -static_cast<int>(parameter_space);
}

int Code::compile_branch()
{
	append(CODE_BR);
	no_last();
	return 0;
}

int Code::compile_cond_branch()
{
	append(CODE_CBR);
	no_last();
	return 0;
}

void Code::fixup_branch(size_t pos, size_t target_pos)
{
	codepoint_type cp = data_.at(pos) & 0xf800;
	int offset = target_pos;
	offset -= pos+1;
	if ((offset < -0x400) || (offset > 0x3ff)) {
		throw generator_exception("Code::fixup_branch too far away");
	}
	data_[pos] = cp | (offset & 0x7ff);
}

int Code::compile_call(size_t adr)
{
	if (adr >= 0x8000) {
		throw generator_exception("Code::compile_call invalid address");
	}
	append(CODE_CALL | adr);
	no_last();
	return 0;
}


void Code::print(std::ostream& out)
{
	size_t pos = 0;
	size_t limit = data_.size();
	disassemble_code(out, pos, limit, 0);
	out << std::endl;
}

void Code::disassemble_line(std::ostream& out, size_t pos)
{
	out << std::hex << std::setw(4) << pos << "\t" << std::setw(4) << data_.at(pos);
}

void Code::disassemble_code(std::ostream& out, size_t& pos, size_t len, Linker* lnk)
{
	std::string name;
	size_t offset;
	ExtRefType rt;
	short val;
	while (len > 0) {
		--len;
		if (lnk) {
			xref_map_type::iterator it = lnk->function_xref_map().find(pos);
			if (it != lnk->function_xref_map().end()) {
				out  << it->second<< "\n";
			}
		}
		codepoint_type cp = data_.at(pos);
		disassemble_line(out, pos);
		out << "\t";
		if (cp <= 0x7fff) {
			out << "call";
			if (lnk) {
				xref_map_type::iterator it = lnk->function_xref_map().find(cp);
				if (it == lnk->function_xref_map().end()) {
					out << "\t" << std::setw(4) << cp;
				}
				else {
					out << "\t" << it->second;
				}
			}
			else {
				out << "\t" << std::setw(4) << cp;
			}
		}
		else {
			switch (cp >> 12) {
			case 0x8:
				if (cp == CODE_RET) {out << "ret";}
				else if (cp == CODE_LLIT) {
					cp = data_.at(++pos);
					out << "llit\t" << std::dec << static_cast<short>(cp) << " (0x" << std::hex << cp << ")";
					if (len > 0) {--len;}
				}
				else if (cp == CODE_PLIT) {
					cp = data_.at(++pos);
					out << "plit\t" << std::dec << static_cast<short>(cp) << " (0x" << std::hex << cp << ")";
					if (len > 0) {--len;}
				}
				//else if (cp == CODE_PLIT2) {
				//	cp = data_.at(++pos);
				//	out << "plit2\t" << std::dec << static_cast<short>(cp) << " (0x" << std::hex << cp << "), ";
				//	cp = data_.at(++pos);
				//	out << std::dec << static_cast<short>(cp) << " (0x" << std::hex << cp << ")";
				//	if (len > 0) {--len;}
				//	if (len > 0) {--len;}
				//}
				else if (cp == CODE_POP) {out << "pop";}
				else if (cp == CODE_PUSH) {out << "push";}
				else if (cp == CODE_PC) {out << "pc";}
				else if (cp == CODE_XP) {out << "xp";}
				else if (cp == CODE_XS) {out << "xs";}
				else if (cp == CODE_TJMP) {
					out << "tjmp";
					while (data_.at(++pos)) {
						disassemble_line(out, pos);
						cp = data_.at(pos);
						out << "@" << (cp+pos) << "\n";
						if (len > 0) {--len;}
					}
					disassemble_line(out, pos);
					out << "default";
					if (len > 0) {--len;}
				}
				else {out << "?";}
				break;
			case 0x9:
				if (cp <= 0x97ff) {out << "br\t\t@";}
				else {out << "cbr\t\t@";}
				val = static_cast<short>(cp & 0x7ff);
				if (val > 0x3ff) {
					val |= 0xfc00;
				}
				val += pos+1;
				out << val;
				break;
			case 0xa:
				if (cp >= 0xa800) {
					int offset = 0;
					if (cp >= CODE_LAPAR) {out << "lap\t\t"; offset = -2;}
					else if (cp >= CODE_LALOC) {out << "lal\t\t";}
					else if (cp >= CODE_LDPAR) {out << "ldp\t\t"; offset = -2;}
					else if (cp >= CODE_LDLOC) {out << "ldl\t\t";}
					else if (cp >= CODE_STPAR) {out << "stp\t\t"; offset = -2;}
					else if (cp >= CODE_STLOC) {out << "stl\t\t";}
					else if (cp >= CODE_EXIT) {out << "exit\t";}
					else if (cp >= CODE_ENTER) {out << "enter\t"; offset = 1;}
					else {out << "?"; break;}
					val = static_cast<short>(cp & 0xff);
					val += offset;
					out << std::dec << val;
				}
				else if (cp == CODE_TERM) {out << "term";}
				else if (cp == CODE_XF) {out << "xf";}
				else if (cp == CODE_WAIT) {out << "wait";}
				else {out << "?";}
				break;
			case 0xb:
				if (cp < 0xb800) {
					if (cp == CODE_LOAD) {out << "ld";}
					else if (cp == CODE_LOAD_PUSH) {out << "ldph";}
					else if (cp == CODE_STORE) {out << "st";}
					else if (cp == CODE_XG) {out << "xg";}
					else if (cp == CODE_LOAD_EXT) {out << "ldx";}
					else if (cp == CODE_STORE_EXT) {out << "stx";}
					else if (cp == CODE_STORE_MK) {out << "stmk";}
					else {out << "?";}
				}
				else {
					bool use_adr_info = false;
					if (cp >= CODE_LA) {
						out << "la\t\t";
					}
					else if (cp >= CODE_LDO) {
						out << "ldo\t\t";
					}
					else if (cp >= CODE_STO) {
						out << "sto\t\t";
					}
					else {
						out << "glob\t";
						use_adr_info = true;
					}
					cp &= GLOB_LIMIT;
					if (use_adr_info && lnk) {
						rt = lnk->get_adr_info(cp+lnk->data_base(), name, offset);
						if (rt == EXR_STATIC) {
							out << name;
							if (offset > 0) {
								out << " + " << std::dec << offset;
							}
						}
						else {
							out << "invalid global address";
						}
					}
					else {
						out << "+" << std::dec << cp;
					}
				}
				break;
			case 0xc:
				if (cp == CODE_ADD) {out << "add";}
				else if (cp == CODE_SL) {out << "sl";}
				else if (cp == CODE_SR) {out << "sr";}
				else if (cp == CODE_USR) {out << "usr";}
				else if (cp == CODE_AND) {out << "and";}
				else if (cp == CODE_XOR) {out << "xor";}
				else if (cp == CODE_OR) {out << "or";}
				else if (cp == CODE_XS) {out << "xs";}
				else if (cp == CODE_CX) {out << "cx";}
				else {out << "?";}
				break;
			case 0xd:
				val = static_cast<short>(cp & 0xfff);
				if (val > 0x7ff) {val |= 0xf800;}
				out << "lit\t\t" << std::dec << val << " (0x" << std::hex << val << ")";
				break;
			case 0xe:
				if (cp == CODE_STLO) {out << "stlo";}
				else if (cp == CODE_STHI) {out << "sthi";}
				else if (cp == CODE_LDLO) {out << "ldlo";}
				else if (cp == CODE_LDHI) {out << "ldhi";}
				else if (cp == CODE_OUT) {out << "out";}
				else if (cp == CODE_IN) {out << "in";}
				else if (cp == CODE_XD) {out << "xd";}
				else if (cp == CODE_XPT) {out << "xpt";}
				else if (cp == CODE_ADDC) {out << "addc";}
				else if (cp == CODE_LSHC) {out << "lshc";}
				else if (cp == CODE_RSHC) {out << "rshc";}
				else if (cp == CODE_UMUL) {out << "umul";}
				else if (cp == CODE_SEXT) {out << "sext";}
				else if (cp == CODE_ZEXT) {out << "zext";}
				else if (cp == CODE_STD) {out << "std";}
				else if (cp == CODE_LDD) {out << "ldd";}
				else {out << "?";}
				break;
			case 0xf:
				if (cp == CODE_SUB) {out << "sub";}
				else if (cp == CODE_EQ) {out << "eq";}
				else if (cp == CODE_EQ0) {out << "eq0";}
				else if (cp == CODE_NE) {out << "ne";}
				else if (cp == CODE_NE0) {out << "ne0";}
				else if (cp == CODE_GE) {out << "ge";}
				else if (cp == CODE_LT) {out << "lt";}
				else if (cp == CODE_GT) {out << "gt";}
				else if (cp == CODE_LE) {out << "le";}
				else if (cp == CODE_UGE) {out << "uge";}
				else if (cp == CODE_ULT) {out << "ult";}
				else if (cp == CODE_UGT) {out << "ugt";}
				else if (cp == CODE_ULE) {out << "ule";}
				else if (cp == CODE_GE0) {out << "ge0";}
				else if (cp == CODE_GT0) {out << "gt0";}
				else if (cp == CODE_LE0) {out << "le0";}
				else if (cp == CODE_LT0) {out << "lt0";}
				else if (cp == CODE_NE0) {out << "ne0";}
				else if (cp == CODE_INC) {out << "inc";}
				else if (cp == CODE_DEC) {out << "dec";}
				else if (cp == CODE_NOT) {out << "not";}
				else if (cp == CODE_ABS) {out << "abs";}
				else if (cp == CODE_NEG) {out << "neg";}
				else if (cp == CODE_TRUE) {out << "true";}
				else if (cp == CODE_FALSE) {out << "false";}
				else if (cp == CODE_MUL2) {out << "mul2";}
				else if (cp == CODE_DIV2) {out << "div2";}
				else if (cp == CODE_UDIV2) {out << "udiv2";}
				else if (cp == CODE_XT) {out << "xt";}
				else if (cp == CODE_STT) {out << "stt";}
				else if (cp == CODE_LDT) {out << "ldt";}
				else if (cp == CODE_BITCNT) {out << "bcnt";}
				else {out << "?";}
				break;
			default:;
			}
		}
		if (len > 0) {out << "\n";}
		++pos;
	}
}

void Code::disassemble_string_literal(std::ostream& out, size_t& pos, size_t len, Linker* lnk)
{
	while (len > 0) {
		--len;
		codepoint_type cp = data_.at(pos);
		disassemble_line(out, pos);
		if (cp <= 0x1f) {
			out << "\t\\x" << cp;
		}
		else {
			out << "\t" << static_cast<wchar_t>(cp);
		}
		if (lnk) {
			xref_map_type::iterator it = lnk->string_xref_map().find(pos);
			if (it != lnk->string_xref_map().end()) {
				out << "\t\"" << it->second << "\"";
			}
		}
		if (len > 0) {out << "\n";}
		++pos;
	}
}

void Code::disassemble_global(std::ostream& out, size_t& pos, size_t len, Linker* lnk)
{
	while (len > 0) {
		--len;
		disassemble_line(out, pos);
		if (lnk) {
			xref_map_type::iterator it = lnk->static_xref_map().find(pos - lnk->data_base());
			if (it != lnk->static_xref_map().end()) {
				out << "\t" << it->second;
			}
		}
		if (len > 0) {out << "\n";}
		++pos;
	}
}

void Code::disassemble_data(std::ostream& out, size_t& pos, size_t len)
{
	while (len > 0) {
		--len;
		codepoint_type cp = data_.at(pos);
		disassemble_line(out, pos);
		if (len > 0) {out << "\n";}
		++pos;
	}
}

void Code::write(std::ostream& out, bool big_endian)
{
	char hi, lo;
	codepoint_type val;
	size_t limit = data_.size();
	for (size_t i=0; i<limit; ++i) {
		val = data_.at(i);
		hi = static_cast<char>(val >> 8);
		lo = static_cast<char>(val);
		if (big_endian) {
			out << hi << lo;
		}
		else {
			out << lo << hi;
		}
	}
}

void Code::write(xml::Writer& out)
{
	//out.newline();
	out.out() << "<" << (is_data_ ? "data" : "code") << " value='";
	size_t sz = size();
	if (sz > 0) {
		out.hex();
		out.out() << data().at(0);
		for (size_t i=1; i<sz; ++i) {
			out.out() << " " << data().at(i);
		}
		out.dec();
	}
	out.out() << "'/>";
}

void Code::read(xml::Iterator& it)
{
	if (it.current() != xml::Element::tag) {throw exception("Code::read invalid file");}
	std::string tag = it.tag();
	is_data_ = tag == "data";
	it.next();
	while (it.current() == xml::Element::attribute) {
		if (it.key() == "value") {
			std::stringstream ss(it.value());
			codepoint_type cp;
			ss >> std::hex;
			while (ss >> cp) {
				append_data(cp);
			}
		}
		it.next();
	}
	if ((it.current() != xml::Element::empty) || (it.tag() != tag)) {throw exception("Code::read invalid file");}
}

}
