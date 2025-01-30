#ifndef F18A_HPP
#define F18A_HPP

#include <exception>
#include <string>
#include <map>
#include <xml.hpp>
#include <utils.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

namespace evb {
  class Board;
  class Module;
  typedef utils::RefCntPtr<Board> Board_ptr;
}

namespace Glib {
  class Dispatcher;
}


namespace processor {
  class Device : public utils::RefCntObj {
  };
}

namespace f18a {

typedef long cell;
typedef std::vector<cell> cell_list_type;

#define F18_INVALID_VALUE  0x80000000

#define F18A_PARAMETER_STACK_SIZE 8
#define F18A_RETURN_STACK_SIZE 8
#define F18A_STACK_SIZE 8

#define F18A_ROM_BITS  6
#define F18A_ROM_SIZE (1 << F18A_ROM_BITS)
#define F18A_ROM_MASK  (F18A_ROM_SIZE-1)

#define F18A_RAM_BITS  6
#define F18A_RAM_SIZE (1 << F18A_RAM_BITS)
#define F18A_RAM_MASK  (F18A_ROM_SIZE-1)

#define F18A_MEM_BITS  6
#define F18A_MEM_SIZE (1 << F18A_MEM_BITS)
#define F18A_MEM_MASK  (F18A_ROM_SIZE-1)

#define F18A_INC_BITS  7
#define F18A_INC_SIZE (1 << F18A_INC_BITS)
#define F18A_INC_MASK  (F18A_INC_SIZE-1)

#define F18A_MEMORY_BITS  8
#define F18A_MEMORY_SIZE (1 << F18A_MEMORY_BITS)
#define F18A_MEMORY_MASK  (F18A_MEMORY_SIZE-1)

#define F18A_CELL_BITS  18
#define F18A_CELL_SIZE (1 << F18A_CELL_BITS)
#define F18A_CELL_MASK  (F18A_CELL_SIZE-1)

#define F18A_INITIAL_CELL    0x15555

#define F18A_RAM_START 0
#define F18A_ROM_START 0x80
#define F18A_IO_START 0x100

#define F18A_RAM_SHADOW_OFFSET  0x40
#define F18A_ROM_SHADOW_OFFSET  0x40

#define F18A_EXT_ARITH_BIT  9
#define F18A_EXT_ARITH_SIZE  (1 << F18A_EXT_ARITH_BIT)
#define F18A_EXT_ARITH_MASK  (F18A_EXT_ARITH_SIZE-1)

#define F18A_ADDR_BITS  10
#define F18A_ADDR_SIZE  (1 << F18A_ADDR_BITS)
#define F18A_ADDR_MASK  (F18A_ADDR_SIZE-1)
#define F18A_ADDR_UNDEF_BITS  0x1400
#define F18A_ADDR_UNDEF_MASK  0x1c00

#define F18A_NO_OF_SLOTS  4
#define F18A_LAST_SLOT    (F18A_NO_OF_SLOTS - 1)

#define F18A_SLOT_BITS  5
#define F18A_SLOT_SIZE  (1 << F18A_SLOT_BITS)
#define F18A_SLOT_MASK  (F18A_SLOT_SIZE-1)

#define F18A_FINAL_SLOT_BITS  3
#define F18A_FINAL_SLOT_SIZE  (1 << F18A_FINAL_SLOT_BITS)
#define F18A_FINAL_SLOT_MASK  (F18A_FINAL_SLOT_SIZE-1)

#define F18A_SKIP_SLOT_BITS  (F18A_SLOT_BITS - F18A_FINAL_SLOT_BITS)
#define F18A_SKIP_SLOT_SIZE  (1 << F18A_SKIP_SLOT_BITS)
#define F18A_SKIP_SLOT_MASK  (F18A_SKIP_SLOT_SIZE-1)

#define F18A_SLOT23_BITS  (F18A_SLOT_BITS + F18A_FINAL_SLOT_BITS)
#define F18A_SLOT23_SIZE  (1 << F18A_SLOT23_BITS)
#define F18A_SLOT23_MASK  (F18A_SLOT23_SIZE-1)

#define F18A_SLOT123_BITS  (F18A_SLOT_BITS + F18A_SLOT23_BITS)
#define F18A_SLOT123_SIZE  (1 << F18A_SLOT123_BITS)
#define F18A_SLOT123_MASK  (F18A_SLOT123_SIZE-1)

#define F18A_AOUT_BITS  9
#define F18A_AOUT_SIZE  (1 << F18A_AOUT_BITS)
#define F18A_AOUT_MASK  (F18A_AOUT_SIZE-1)
#define F18A_AOUT_INV  0x155

#define F18A_COLD  0xaa
#define F18A_WARM  0xa9

#define F18A_IO         0x15d
#define F18A_RIGHT      0x1d5
#define F18A_DOWN       0x115
#define F18A_LEFT       0x175
#define F18A_UP         0x145
#define F18A_LDATA      0x171
#define F18A_DATA       0x141
#define F18A_WARP       0x157
#define F18A_CENTER     0x1a5
#define F18A_TOP        0x1b5
#define F18A_SIDE       0x185
#define F18A_CORNER     0x195

#define F18A_PIN17_HI   0x30000
#define F18A_PIN17_LO   0x20000
#define F18A_PIN17_WK   0x10000
#define F18A_PIN17_HZ   0x00000

#define F18A_PIN1_HI    0x00003
#define F18A_PIN1_LO    0x00002
#define F18A_PIN1_WK    0x00001
#define F18A_PIN1_HZ    0x00000

#define F18A_PIN3_HI    0x0000c
#define F18A_PIN3_LO    0x00008
#define F18A_PIN3_WK    0x00004
#define F18A_PIN3_HZ    0x00000

#define F18A_PIN5_HI    0x00030
#define F18A_PIN5_LO    0x00020
#define F18A_PIN5_WK    0x00010
#define F18A_PIN5_HZ    0x00000

#define F18A_WAKE_ON_LOW    0x00800
#define F18A_WAKE_ON_HIGH   0x00000

#define F18A_VCO_HZ_IN      0x00000
#define F18A_VCO_VDD_CALIB  0x02000
#define F18A_VCO_DISABLED   0x04000
#define F18A_VCO_VSS_CALIB  0x06000

#define F18A_SERDES_SND     0x20000
#define F18A_SERDES_RCV     0x00000

#define F18A_DATA_OUT       0x01000
#define F18A_DATA_IN        0x00000

#define F18A_READ1          0x00002
#define F18A_READ3          0x00008
#define F18A_READ5          0x00020
#define F18A_READ17         0x20000

#define F18A_RIGHT_MASK     0x18000
#define F18A_DOWN_MASK      0x06000
#define F18A_LEFT_MASK      0x01800
#define F18A_UP_MASK        0x00600

#define F18A_RIGHT_READ     0x00000
#define F18A_DOWN_READ      0x00000
#define F18A_LEFT_READ      0x00000
#define F18A_UP_READ        0x00000

#define F18A_RIGHT_WRITE    0x08000
#define F18A_DOWN_WRITE     0x02000
#define F18A_LEFT_WRITE     0x00800
#define F18A_UP_WRITE       0x00200

typedef std::map<std::string, cell> label_map_type;
typedef std::map<cell, std::string> addr_map_type;
typedef std::map<size_t, bool> include_map_type;

extern size_t read_slot(cell data, size_t slot);
extern cell write_slot(cell data, size_t slot, size_t);
extern int to_digit(unsigned char, size_t base);
extern bool scan_next(const std::string&, size_t& pos, size_t& start_pos, std::string&, char del = ' ');
extern bool skip_next(const std::string&, size_t& pos, char del = ' ');
extern bool skip_line(const std::string&, size_t& pos);
extern bool convert_to_number(const std::string&, cell&);
extern bool convert_io_to_name(size_t addr, std::string&);

class exception : public std::runtime_error {
public:
	explicit exception(const char* msg) : std::runtime_error(msg) {}
	explicit exception(const std::string& msg) : std::runtime_error(msg.c_str()) {}
};

class fit_exception : public exception {
public:
	explicit fit_exception(const char* msg) : exception(msg) {}
	explicit fit_exception(const std::string& msg) : exception(msg.c_str()) {}
};

struct Opcode {
	enum Enum {
		ret,
		ex,
		jump,
		call,
		unext,
		next,
		_if,
		mif,
		fetch_p,
		fetch_plus,
		fetch_b,
		fetch,
		store_p,
		store_plus,
		store_b,
		store,
		plus_mul,
		mul2,
		div2,
		inv,
		add,
		_and,
		_xor,
		drop,
		dup,
		pop,
		over,
		a,
		nop,
		push,
		b_store,
		a_store,
		limit
	};

  static void to_string(Enum, std::string&, size_t size=0); // if size > 0 then the result is set to this size, right adjusted
  static Enum from_string(const std::string&);
  static bool fit_slot3(Enum);
  static bool has_addr_field(Enum);
  static size_t max_size();
};

struct Command {
	enum Enum {
    undefined,
    number,           // #
    colon,            // :
    dot,              // .
    skip,             // ..
    lesssl,           // <sl
    comma,            // ,
    semicolon,        // ;
    comment,          // (
    pcy,              // +cy
    mcy,              // -cy
    abegin,           // A[
    setp,             // =P
    enda,             // ]]
    lcomment,         // line comment //
    here,
    begin,
    _else,
    then,
    _if,
    _while,
    until,
    mif,
    mwhile,
    muntil,
    zif,
    ahead,
    leap,
    again,
    repeat,
    _for,
    next,
    snext,
    org,
    qorg,             // ?org ( n-) error if org is not equal to expected value
    nl,
    ndl,
    equ,              // label definition
    its,
    swap,
    lit,
    alit,
    await,
    avail,
    load,
    assign_a,         // /A
    assign_b,         // /B
    assign_io,        // /IO
    assign_p,         // /P
    assign_stack,     // /STACK
    assign_rstack,    // /RSTACK
    do_add,           // [+]
    do_sub,           // [-]
    do_mul,           // [*]
    do_div,           // [/]
    do_and,           // AND
    do_or,            // OR
    do_xor,           // XOR
    do_inv,           // INV
    include,
    limit
  };
  static Enum from_string(const std::string&);
  static void to_string(Enum, std::string&, size_t size=0); // if size > 0 then the result is set to this size, right adjusted
};

struct Register {
	enum Enum {
		T,
		S,
		R,
		P,
		I,
		A,
		B,
		IO,
		UP,     // up port
		LT,     // left port
		DN,     // down port
		RT,     // right port
		limit
	};

  static void to_string(Enum, std::string&, size_t size=0);
  static size_t max_size();

};

struct Port {
	enum Enum {
		up,
		left,
		down,
		right,
		limit
	};

  static cell to_addr(Enum);
  static Enum from_addr(cell);
  static Enum opposite(Enum);
  static const std::string& to_call(Enum);
};
typedef std::vector<Port::Enum> port_list_type;

struct IOBitCtrl {
  enum Enum {
    tristate,
    weak_pulldown,
    low,
    high,
    limit
  };
};

struct IOBitWrite {
  enum Enum {
    pin1_ctl = 0,
    pin1_value = 1,
    pin3_ctl = 2,
    pin3_value = 3,
    pin5_ctl = 4,
    pin5_value = 5,
    wd = 11,
    pin17_ctl = 16,
    pin17_value = 17,
    limit
  };
};

struct IOBitAltWrite {
  enum Enum {
    da_lsb = 0,
    da_msb = 11,
    db = 12,
    ctl = 13,
    vco = 14,
    sr = 17,
    limit
  };
};

struct IOBitRead {
  enum Enum {
    pin1 = 1,
    pin3 = 3,
    pin5 = 5,
    uw = 9,
    ur = 10,
    lw = 11,
    lr = 12,
    dw = 13,
    dr = 14,
    rw = 15,
    rr = 16,
    pin17 = 17,
    limit
  };
};

struct IOPortAddr {
  enum Enum {
    io = 0x08,
    data = 0x14,
    ___u = 0x10,
    __l_ = 0x20,
    __lu = 0x30,
    _d__ = 0x40,
    _d_u = 0x50,
    _dl_ = 0x60,
    _dlu = 0x70,
    r___ = 0x80,
    r__u = 0x90,
    r_l_ = 0xa0,
    r_lu = 0xb0,
    rd__ = 0xc0,
    rd_u = 0xd0,
    rdl_ = 0xe0,
    rdlu = 0xf0
  };
};

struct ExecutionState {
  enum Enum {
    undefined,
    running,
    wait_completing,
    wait_reading,
    wait_writing,
    wait_phantom,
    limit
  };
};

struct RomType {
  enum Enum {
    undefined,
    basic,
    analog,
    digital,
    serdes_boot,
    sync_boot,
    async_boot,
    spi_boot,
    wire1,
    limit
  };
};
 

struct RegisterSet {
	cell	T_;   // top of parameter stack
	cell	S_;   // second of parameter stack
	cell	R_;   // top of return stack
	cell	P_;   // program counter
	cell	I_;   // instruction register
	cell	A_;   // register A
	cell	B_;   // register B
	cell	IO_;  // I/O register
	cell	UP_;  // up port
	cell	LT_;  // left port
	cell	DN_;  // down port
	cell	RT_;  // right port

  static void write(xml::Writer&, const char* name, cell val);

  void reset();

  bool writeable() const;
  void read(xml::Iterator&);
  void write(xml::Writer&);
};

union Registers {
  RegisterSet named_;
  cell        list_[Register::limit];
};

class Stack : public utils::RefCntObj {
  cell        data_[F18A_STACK_SIZE];
  size_t      limit_;
  size_t      pos_; // next push position
public:
  Stack();
  Stack(size_t limit);
  Stack(const Stack&);
  Stack& operator=(const Stack&);
  ~Stack();
  
  void reset();

  void push(cell);
  cell pop();

  cell top() const;

  void data(size_t pos, cell val);

  cell& at(size_t pos);
  cell at(size_t pos) const;

  bool writeable() const;
  void read(xml::Iterator&);
  void write(xml::Writer&);

};
typedef utils::RefCntPtr<Stack> Stack_ptr;

class Memory : public utils::RefCntObj {
  cell            data_[F18A_MEM_SIZE];
  size_t          limit_;
  std::string     source_;
  std::string     comment_;
  label_map_type  label_map_;

public:
  Memory();
  Memory(size_t limit);
  Memory(const Memory&);
  Memory& operator=(const Memory&);
  ~Memory();

  void reset();

  label_map_type& label_map() {return label_map_;}
  const label_map_type& label_map() const {return label_map_;}

  void source(const std::string& val);
  const std::string& source() const {return source_;}

  void comment(const std::string& val);
  const std::string& comment() const {return comment_;}

  const cell* data() const {return data_;}
  void data(size_t begin, const cell* addr, size_t size); // set ram
  cell data(size_t pos) const {return data_[pos];}
  void data(size_t pos, cell val) {data_[pos] = val;}
  void insert_ram(size_t pos, cell val);
  void remove_ram(size_t pos);

  bool writeable() const;
  void read(xml::Iterator&);
  void write(xml::Writer&);
};
typedef utils::RefCntPtr<Memory> Memory_ptr;

class Module : public utils::RefCntObj {
  evb::Module* owner_;
  std::string     name_;
  Registers       reg_;
  Stack           ps_;
  Stack           rs_;
  Memory          ram_;
  Memory          rom_;

  Module(const Module&);
  Module& operator=(const Module&);
public:
  Module();

  void reset();

  bool find_label(const std::string& name, cell& addr);

  void owner(evb::Module&);
  bool update(evb::Board&);

  void name(const std::string& val) {name_ = val;}
  const std::string& name() const;

  Registers& reg() {return reg_;}
  Stack& ps() {return ps_;}
  Stack& rs() {return rs_;}
  Memory& ram() {return ram_;}
  Memory& rom() {return rom_;}

  const Registers& reg() const {return reg_;}
  const Stack& ps() const {return ps_;}
  const Stack& rs() const {return rs_;}
  const Memory& ram() const {return ram_;}
  const Memory& rom() const {return rom_;}

  bool writeable() const;
  void read(xml::Iterator&);
  void write(xml::Writer&);

  size_t append_init_node(cell_list_type&, bool is_boot_frame, cell destination_adr = F18_INVALID_VALUE); // append code for booting this node to cell list
  size_t append_boot_stream(cell_list_type&); // append boot stream for this node

  // simulation
  void tic(); // first half of step
  void toc(); // second half of step
};
typedef utils::RefCntPtr<Module> Module_ptr;

class Processor : public processor::Device {
  //friend struct State;
  boost::mutex                    modify_mutex_;
  std::vector<Glib::Dispatcher*>  modify_dispatcher_list_;
  boost::mutex                    change_mutex_;
  std::vector<Glib::Dispatcher*>  change_dispatcher_list_;

  Module_ptr              module_;
  std::vector<Module_ptr> config_;  // configuration(s) for this processor
  Module_ptr          rom_module_;
  Registers           post_reg_;	// register after execution
  size_t              slot_;

  cell              io_written_;
  size_t            row_;
  size_t            column_;
  size_t            current_; // current configuration
  int               tentacle_index_;
  bool              io_is_written_;

  include_map_type  include_map_;

public:
  Processor();
  ~Processor();

  bool io_is_written() const {return io_is_written_;}
  cell io_written() const {return io_written_;}
  void write_io(cell);

  bool is_tentacle() const {return tentacle_index_ >= 0;}
  int tentacle_index() const {return tentacle_index_;}
  void tentacle_index(int val) {tentacle_index_ = val;}
  void reset_tentacle_index() {tentacle_index_ = -1;}

  size_t row() const {return row_;}
  size_t column() const {return column_;}

  Module_ptr module() {return module_;}
  const  Module_ptr module() const {return module_;}

  Module_ptr rom_module();
  const  Module_ptr rom_module() const {return rom_module_;}

  size_t current() const {return current_;}
  void current(size_t val) {current_ = val;}

  bool has_config() const {return !config_.empty();}
  size_t config_size() const {return config_.size();}
  Module_ptr config(size_t pos) {return config_[pos];}
  const Module_ptr config(size_t pos) const {return config_[pos];}

  void append_config();
  Module_ptr assert_config(size_t pos);
  Module_ptr current_config() {return config_[current_];}
  const Module_ptr current_config() const {return config_[current_];}

  //Module_ptr assert_config_by_name(const std::string& name);
  Module_ptr config_by_name(const std::string& name); // return 0 if nonextistend

  //const label_map_type& labels() const {return label_map_;}
  //const label_map_type& rom_labels() const {return rom_label_map_;}
  bool startup_p(size_t&) const;

  size_t node() const; // return the decimal node number (without chip offset)

  size_t warm() const; // return call address to multiport wait

  bool find_label(const std::string& name, size_t config_pos, cell& addr);

  void reset_includes() {include_map_.clear();}
  void include(size_t node) {include_map_[node] = true;}
  const include_map_type& include_map() const {return include_map_;}
  bool includes(size_t node) const;

  bool is_h_border() const;
  bool is_v_border() const;
  bool is_corner() const;

  void row(size_t val) {row_ = val;}
  void column(size_t val) {column_ = val;}

  void addr_map(addr_map_type&);

  void reset();
  void execute();
  void memory(size_t addr, cell val);
  cell memory(size_t addr) const;

  void set_b(cell);

  void register_modify_dispatcher(Glib::Dispatcher&);
  void deregister_modify_dispatcher(Glib::Dispatcher&);
  void modify(); // signaling the content of a processor has changed
  void on_modify(); // signaling the content of a processor has changed

  void register_change_dispatcher(Glib::Dispatcher&);
  void deregister_change_dispatcher(Glib::Dispatcher&);
  void changed(); // signaling the appearance of a processor has changed
  void on_changed(); // signaling the appearance of a processor has changed

  static long instruction4(Opcode::Enum slot0, Opcode::Enum slot1, Opcode::Enum slot2, Opcode::Enum slot3);
  static long instruction3(Opcode::Enum slot0, Opcode::Enum slot1, Opcode::Enum slot2, int value);
  static long instruction2(Opcode::Enum slot0, Opcode::Enum slot1, int value);
  static long instruction1(Opcode::Enum slot0, int value);
  static long instruction0(int value);

  static int instruction_slots(cell);

  //static bool has_data(Opcode::Enum);
  static Opcode::Enum get_instruction(cell code, int slot);
  static size_t get_data(cell, int slot);
  static cell get_data_mask(int slot);
  static size_t inc_addr(size_t addr);

  void read(xml::Iterator&);
  void write(xml::Writer&);

  void update(evb::Board&);

  // simulation
  void tic(); // first half of step
  void toc(); // second half of step

  RomType::Enum rom_type() const;
  std::string rom_source() const;
};
typedef utils::RefCntPtr<Processor> Processor_ptr;


struct State : public utils::RefCntObj {
  // static variables
  typedef std::map<std::string, cell> label_map_type;
  //std::string       source_;
  evb::Board_ptr board_;
  std::string       error_;
  std::string       token_;
  //Registers         reg_;
  Module_ptr        module_;
  //Task              startup_;
  //cell              p_stack_[F18A_PARAMETER_STACK_SIZE];
  //cell              r_stack_[F18A_RETURN_STACK_SIZE];
  //cell              ram_[F18A_RAM_SIZE];
  //cell              rom_[F18A_RAM_SIZE];
  cell              single_;
  label_map_type    labels_;
  label_map_type    rom_labels_;
  label_map_type    ext_labels_;
  label_map_type    literals_;
  label_map_type    rom_literals_;
  include_map_type  include_map_;
  // assembly variables
  //size_t            pstack_size_;
  //size_t            rstack_size_;
  size_t            last_token_pos_;
  size_t            instr_pos_;
  size_t            memory_pos_;
  cell              curr_;
  size_t            slot_;
  size_t            last_call_slot_;
  size_t            last_call_pos_;
  std::deque<cell>  stack_;
  bool              single_processing_; // indicates that a A[ ]] sequence is active

  static size_t sa_instr_addr(long sa);
  static size_t sa_mem_addr(long sa);
  static size_t sa_slot(long sa);
  static size_t sa_offset(long sa);

  State();

  const std::string& token() const {return token_;}

  long sa(); // generates an address containing instruction address, slot and delta to address
  bool fit(long sa); // return true if a jump can be generated
  bool failed(const char*);
  bool failed(const std::string&);

  bool finish(); // finish current cell
  bool convert_to_number(const std::string&, cell&);
  bool label_to_number(const std::string&, cell&);
  bool flush(bool fill_nop = true);
  bool swap();
  bool forward(Opcode::Enum);
  bool append_slot(size_t, bool do_finish = true);
  bool append_cell(cell);
  bool append_call(Opcode::Enum, size_t destination);
  bool stack_empty();
  bool here(); // force word alignment and leave address on the stack
  bool less_slot(size_t no); // forces a flush if slot_ >= no
  bool then();
  bool memory(size_t addr, cell val);
  cell memory(size_t addr);
  bool include_labels(Processor_ptr);
  size_t last_token_pos() const {return last_token_pos_;}

  bool load(const std::string&, bool is_rom, f18a::Processor_ptr);

  void preset();
  bool assemble(const std::string& module, const std::string& src, bool is_rom, Processor_ptr, evb::Board_ptr);
  bool disassemble(std::string&);

  void write(Processor_ptr, size_t pos, bool is_rom) const;

  static cell assemble1(const std::string&); // assemble 1 instruction cell
  static cell focusing_call(f18a::Port::Enum);
  static cell focusing_jump(f18a::Port::Enum);
  static cell addr_to_call(long addr);
  static cell addr_to_jump(long addr);
};
typedef utils::RefCntPtr<State> State_ptr;


}

#endif
