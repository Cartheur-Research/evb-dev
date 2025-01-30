#ifndef PFVM_HPP
#define PFVM_HPP

#ifdef _WIN32
#pragma warning (disable: 4251) // http://msdn2.microsoft.com/en-gb/library/esew7y1w.aspx
#pragma warning (disable: 4996)
#endif 

#include <string>
#include <vector>
#include <queue>
#include <list>
#include <map>
#include <utils.hpp>

typedef unsigned short cell;

namespace evb {
class Manager;
}

namespace host {

class VirtualMachine;

struct Primitive {
  enum Enum {
    undefined,
    dup,
    drop,
    swap
  };
};

class Word : public utils::RefCntObj {
public:

  struct Type {
    enum Enum {
      undefined,
      primitive,
      secondary,
      literal_
    };
  };


private:
  std::string         name_;
  Type::Enum          type_;
  Primitive::Enum     primitive_;
  std::vector<Word*>  secondary_list_;
  int                 literal_value_;

public:
  Word(const std::string&, Primitive::Enum); // primitive
  Word(const std::string&); // secondary
  Word(const std::string&, int); // literal

  void execute(VirtualMachine& vm);

};

typedef std::map<std::string, Word*> word_map_type;

class Dictionary {
private:
  word_map_type     interpreter_map_;
  word_map_type     compiler_map_;
  bool              is_compiling_;
};

class VirtualMachine : public utils::RefCntObj {
};

}

namespace pF { // polyForth

typedef std::vector<cell> memory_type;
typedef std::list<cell> stack_type;
typedef std::map<std::string, cell> dictionary_type;



class VirtualMachine : public utils::RefCntObj { // polyForth target virtual machine
public:

  struct Primitive {
    enum Enum {
      undefined,
      nop,
      inc,
      dec,
      add,
      sub,
      umadd,
      div2,
      mul2,
      lsh8,
      rsh8,
      min,
      max,
      neg,
      inv,
      dup,
      qdup,
      swap,
      drop,
      over,
      under,
      dip,
      nip,
      or,
      xor,
      and,
      zeq,
      zlt,
      lit,
      con,
      var,
      exit,
      exec,
      fetch,
      store,
      sp_fetch,
      sp_store,
      rp_fetch,
      rp_store,
      tor,
      r_fetch,
      rfrom,
      rdup,
      rdrop,
      rxq,
      txq,
      if_,
      else_,
      next,
      xm_fetch,
      xm_store,
      xcx,
      mk_store,
      suspend,
      u,
      u_fetch,
      u_store,
      usr
    };
    static void define(Enum, cell, const std::string&);
    static cell to_code(Enum);
    static Enum from_code(cell);
    static Enum from_name(const std::string&);

    static std::string initialize(evb::Manager&); // return error string
  };

private:
  memory_type     ram_;
  cell            ip_;    // instruction pointer
  cell            sp_;    // parameter stack pointer
  cell            rp_;    // return stack pointer
  cell            t_;     // top of parameter stack
  cell            s_;     // second of parameter stack
  cell            r_;     // top of return stack
  cell            u_;     // user variable pointer

  void do_umadd();
  void do_swap();
  void do_serial_in();
  void do_serial_out();
  void do_();

public:
  VirtualMachine();
  virtual ~VirtualMachine();

  void push_p(cell);
  void push_r(cell);
  cell pop_p();
  cell pop_r();
  cell fetch(cell addr);
  void store(cell addr, cell val);
  cell ext_fetch(cell hi_addr, cell lo_addr);
  void ext_store(cell hi_addr, cell lo_addr, cell val);
  cell long_fetch(long addr);
  void long_fetch(long addr, cell);
  // byte memory access
  cell to_byte_addr(cell addr);
  unsigned char fetch_byte(cell byte_addr);
  void store_byte(cell byte_addr, unsigned char val);

  void execute(Primitive::Enum);
  void step();
};

class Compiler : public utils::RefCntObj {
public:

  struct HostWord {
    enum Enum {
      undefined,

    };
   };


  struct Error {
    size_t        offset_;    // file offset
    size_t        line_;      // line in file
    size_t        position_;  // position in file
    std::string   name_;
    std::string   reason_;
  };
  typedef std::list<Error> error_list_type;

private:

  VirtualMachine*   vm_;
  size_t            h_; // next dictionary location
  stack_type        stack_;
  error_list_type   error_list_;
  dictionary_type   target_dictionary_;
  const char*       text_;
  size_t            text_len_;
  size_t            offset_;
  size_t            line_;
  size_t            pos_;
  bool              compiling_;

public:
  Compiler();
  virtual ~Compiler();

  void error(const std::string& name, const std::string& reason);

  size_t skip(bool match, char del=' ');
  std::string word(char del=' ');

  void compile_word(cell);
  void compile_next_word();

  error_list_type compile(const char*, size_t);
};

class Interpreter : public utils::RefCntObj {
};

}

namespace target {

class Error : public utils::RefCntObj {
public:
  struct Type {
    enum Enum {
      no_error,
      invalid_word
    };
   };

private:
  Type::Enum    type;
  std::string   text_;

public:
  Error();
};

class Word : public utils::RefCntObj {
public:

  struct Type {
    enum Enum {
      undefined,
      primitive,
      secondary,
      literal_
    };
  };

private:
  std::string   name_;
  Type::Enum    type_;
  cell          opcode_; // for primitives
  cell          rom_nfa_;
  cell          rom_cfa_;


public:
  Word(const std::string&, pF::VirtualMachine::Primitive::Enum); // primitive
  Word(const std::string&, pF::VirtualMachine&); // secondary
  Word(const std::string&, cell); // literal


};
typedef std::map<std::string, Word*> word_map_type;

class Compiler : public utils::RefCntObj { // target compiler
private:
  pF::VirtualMachine&     vm_; // target vm holds RAM
  size_t                  line_;
  size_t                  line_pos_;
  bool                    compiling_;

public:
  Compiler(pF::VirtualMachine&);
  ~Compiler();

  void skip_whitespace(const char*, size_t len, size_t& pos);

  std::string next(const char*, size_t len, char del, size_t& pos);

  void compile(const char*, size_t len); // compiler source

};



}

#endif
