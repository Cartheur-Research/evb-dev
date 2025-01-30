#ifndef F64SIM_HPP
#define F64SIM_HPP

#ifdef _WIN32
#pragma warning (disable: 4251) // http://msdn2.microsoft.com/en-gb/library/esew7y1w.aspx
#pragma warning (disable: 4996)
#endif 

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <exception>

#include <boost/lexical_cast.hpp>

#define F64_PARAMETER_STACK_SIZE 8
#define F64_RETURN_STACK_SIZE 8

#define F64_TASK_BITS 1
#define F64_TASK_SIZE (1 << F64_TASK_BITS)
#define F64_TASK_MASK (F64_TASK_SIZE - 1)

#define F64_SLOT_INDEX_BITS 3
#define F64_SLOT_INDEX_SIZE (1 << F64_SLOT_INDEX_BITS)
#define F64_SLOT_INDEX_MASK (F64_SLOT_INDEX_SIZE - 1)
#define F64_SLOT_INDEX_SHIFT (64 - F64_SLOT_INDEX_BITS)

#define F64_SLOT_BITS F64_SLOT_INDEX_SIZE
#define F64_SLOT_SIZE (1 << F64_SLOT_BITS)
#define F64_SLOT_MASK (F64_SLOT_SIZE - 1)

#define F64_TASK_SHIFT (F64_SLOT_INDEX_SHIFT - F64_TASK_BITS)

#define F64_REGISTER_BITS 2
#define F64_REGISTER_SIZE (1 << F64_REGISTER_BITS)
#define F64_REGISTER_MASK (F64_REGISTER_SIZE - 1)

#define F64_IO_BITS 8
#define F64_IO_SIZE (1 << F64_IO_BITS)
#define F64_IO_MASK (F64_IO_SIZE - 1)

#define F64_ADDR_BITS 16
#define F64_ADDR_SIZE (1 << F64_ADDR_BITS)
#define F64_ADDR_MASK (F64_ADDR_SIZE - 1)

#define F64_ROM_SIZE 0x100

#define F64_MEMORY_SIZE (F64_ADDR_SIZE - F64_IO_SIZE)
#define F64_RAM_SIZE (F64_MEMORY_SIZE - F64_ROM_SIZE)

#define F64_PAGE_SHIFT 8
#define F64_PAGE_SIZE (1 << F64_PAGE_SHIFT)
#define F64_PAGE_MASK (F64_PAGE_SIZE - 1)
#define F64_NO_OF_PAGES (F64_MEMORY_SIZE >> F64_PAGE_SHIFT)

#define F64_ROM_PAGE 0
#define F64_RAM_START_PAGE 1
#define F64_IO_PAGE 0xff

#define F64_PROCESSOR_ROWS 4
#define F64_PROCESSOR_COLS 4

#define F64_PROCESSOR_SIZE (F64_PROCESSOR_ROWS * F64_PROCESSOR_COLS)

#define F64_PORT_FACES 2
#define F64_PORT_ROWS (F64_PROCESSOR_ROWS-1)
#define F64_PORT_COLS (F64_PROCESSOR_COLS-1)

#define F64_PORT_SIZE (F64_PORT_ROWS * F64_PORT_COLS)

namespace f64sim {

typedef long long cell;

struct Processor;

struct Direction {
  enum Enum {
    RIGHT,
    LEFT,
    UP,
    DOWN,
    FRONT,
    BACK,
    FUTURE,
    PAST,
    limit
  };
};

struct Register {
  enum Enum {
    A,
    B,
    C,
    D,
    T,
    S,
    R,
    I,
    P,
    F,
    limit
  };
};


struct PadType {
  enum Enum {
    digital,
    ananlog,
    limit
  };
};

struct PortOp {
  enum Enum {
    idle,
    read,
    write,
    limit
  };
};

struct RegisterSet {
  cell  A_;
  cell  B_;
  cell  C_;
  cell  D_;
  cell  T_;  // top of stack
  cell  S_;  // second of stack
  cell  R_;  // top of return stack
  cell  I_;  // instruction register
  cell  P_;  // prefix
  cell  F_;  // flags
};

union Registers {
  RegisterSet named_;
  cell   list_[Register::limit];
};

struct Task {
  Registers   reg_;
  cell        p_stack_[F64_PARAMETER_STACK_SIZE];
  cell        r_stack_[F64_RETURN_STACK_SIZE];
  int         ip_;        // current instruction pointer
  int         next_ip_;   // next instruction pointer
  int         slot_;      // current slot (slot that will be used in next instruction)
  int         sp_;        // stack pointer
  int         rp_;        // return stack pointer
  int         task_;      // task number

  void write(Register::Enum, cell);
  cell read(Register::Enum);

  cell ret_adr() const;
  void from_ret_adr(cell val);

  bool next_instr(int& instr); // false if no more instruction
  int next_ip(); // get next instruction address
  void load_instr_word(cell);

  void push_p(cell);
  cell pop_p();

  void push_r(cell);
  cell pop_r();

  void literal(cell);
  void dup();
  void over();
  void under();
  void swap();
  cell drop();
  cell nip();

  void reset();
};

struct Port {
  cell        value_[F64_PORT_FACES];
  Processor*  owner_[F64_PORT_FACES];
  PortOp      status_[F64_PORT_FACES];

  void reset();

};

struct Pad {
  cell   value_;
  Processor*  owner_;
  PadType     type_;
  int         pin_;

  void reset();

};

struct I_O {
  cell        port_[F64_IO_BITS];
  int         read_handshake_mask_;
  int         write_handshake_mask_;
};


struct Memory {
  cell        rom_[F64_ROM_SIZE];
  cell        ram_[F64_RAM_SIZE];
  Port*       port_[F64_IO_BITS];
  I_O*        io_;
  bool        rom_locked_;

  Memory();

  void lock_rom() {rom_locked_ = true;}

  int read(int adr, cell& data); // == 0 -> no read, > 0 delay
  int write(int adr, cell& data); // == 0 -> no read, > 0 delay
  
};

struct Processor {
  I_O         io_;
  Task        task_[F64_TASK_SIZE];
  Memory      mem_;
  int         curr_;      // task selector
  int         delta_;     // current delta
  int         adr_;     // current address
  cell        data_;    // current data
  bool        is_running_;
  bool        wait_memory_;
  bool        wait_is_read_;
  bool        wait_is_data_;

  Processor();

  cell read(int adr);
  void write(int adr, cell);

  int execute(int instr);
  int step(int elapsed); // return next delta

  void reset();
  void cold_reset();
};

struct Chip {
  Processor   proc_[F64_PROCESSOR_COLS][F64_PROCESSOR_ROWS];
  Port        port_[F64_PORT_SIZE];

  void reset();
  void cold_reset();

  Processor*  proc_list() {return reinterpret_cast<Processor*>(proc_);}
  const Processor*  proc_list() const {return reinterpret_cast<const Processor*>(proc_);}

};

}

#endif F64SIM_HPP