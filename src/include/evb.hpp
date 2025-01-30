#ifndef EVB_HPP
#define EVB_HPP

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
#include <queue>
#include <exception>

#include <boost/lexical_cast.hpp>

#include <ga144.hpp>
#include <boot.hpp>
#include <xml.hpp>
#include <connection.hpp>
#include <utils.hpp>

#define KRAKEN_MODULE_NAME  "!Async Kraken"
#define KRAKEN_MODULE_BOTH_NAME  "!Async Kraken Both"
#define BRIDGE_MODULE_NAME  "!Bridge"
#define BOOTLOADER_MODULE_NAME  "!Bootloader"

namespace Glib {
  class Dispatcher;
}

namespace evb {

typedef std::vector<size_t> node_list_type;

class exception : public std::runtime_error {
public:
	explicit exception(const char* msg) : std::runtime_error(msg) {}
	explicit exception(const std::string& msg) : std::runtime_error(msg.c_str()) {}
};

class Board;
class Kraken;
class Manager;
class Module;
class NodeAccess;
class Tentacle;
class Umbilical;
class Kraken;
class Package;

#define EVB_ASYNC_FEEDBACK 0x600D
#define EVB_INVALID_TENTACLE -1
#define EVB_SRAM_SIZE  0x200000

typedef utils::RefCntPtr<Module> Module_ptr;
typedef utils::RefCntPtr<Board> Board_ptr;
typedef utils::RefCntPtr<Manager> Manager_ptr;
typedef utils::RefCntPtr<Kraken> Kraken_ptr;
typedef utils::RefCntPtr<Umbilical> Umbilical_ptr;
typedef utils::RefCntPtr<Tentacle> Tentacle_ptr;
typedef utils::RefCntPtr<Package> Package_ptr;

struct Port {
  enum Enum {
    A,
    B,
    C,
    limit
  };
};

struct Chip {
  enum Enum {
    host,
    target,
    limit
  };
};

struct PinStatus {
  enum Enum {
    undefined,
    // digital
    hi_z,
    weak_lo,
    lo,
    hi,
    // analog input
    analog_input,
    vdd_calibration,
    analog_disabled,
    vss_calibration,
    // analog output
    limit
  };

  static bool to_string(Enum, std::string&);
  static bool to_description(Enum, std::string&);
};

struct PinType {
  enum Enum {
    undefined,
    io_1,
    io_3,
    io_5,
    io_17,
    analog_in,
    analog_out,
    data,
    address,
    limit
  };

  static bool to_string(Enum, std::string&);
  static bool to_description(Enum, std::string&);
};

struct Pin {
  enum Enum {
    h_001_1,      // SERDES data
    h_001_17,     // SERDES clock
    h_100_17,     // 1-pin GPIO node
    h_117_ai,     // analog input
    h_117_ao,     // analog output
    h_200_17,     // 1-pin GPIO node
    h_217_17,     // 1-pin GPIO node
    h_300_1,      // sync data
    h_300_17,     // sync clock
    h_317_17,     // 1-pin GPIO node
    h_417_17,     // 1-pin GPIO node
    h_500_17,     // 1-pin GPIO node
    h_517_17,     // 1-pin GPIO node
    h_517_ai,     // analog input
    h_517_ao,     // analog output
    h_600_17,     // 1-pin GPIO node
    h_617_ai,     // analog input
    h_617_ao,     // analog output
    h_701_1,      // SERDES data
    h_701_17,     // SERDES clock
    h_705_1,      // SPI clock
    h_705_3,      // SPI chip enable
    h_705_5,      // SPI data out
    h_705_17,     // SPI data in
    h_708_1,      // async out
    h_708_17,     // async in
    h_709_ai,     // analog input
    h_709_ao,     // analog output
    h_713_ai,     // analog input
    h_713_ao,     // analog output
    h_715_17,     // 1-pin GPIO node
    h_717_ai,     // analog input
    h_717_ao,     // analog output
    t_001_1,      // SERDES data
    t_001_17,     // SERDES clock
    t_100_17,     // 1-pin GPIO node
    t_117_ai,     // analog input
    t_117_ao,     // analog output
    t_200_17,     // 1-pin GPIO node
    t_217_17,     // 1-pin GPIO node
    t_300_1,      // sync data
    t_300_17,     // sync clock
    t_317_17,     // 1-pin GPIO node
    t_417_17,     // 1-pin GPIO node
    t_500_17,     // 1-pin GPIO node
    t_517_17,     // 1-pin GPIO node
    t_517_ai,     // analog input
    t_517_ao,     // analog output
    t_600_17,     // 1-pin GPIO node
    t_617_ai,     // analog input
    t_617_ao,     // analog output
    t_701_1,      // SERDES data
    t_701_17,     // SERDES clock
    t_705_1,      // SPI clock
    t_705_3,      // SPI chip enable
    t_705_5,      // SPI data out
    t_705_17,     // SPI data in
    t_708_1,      // async out
    t_708_17,     // async in
    t_709_ai,     // analog input
    t_709_ao,     // analog output
    t_713_ai,     // analog input
    t_713_ao,     // analog output
    t_715_17,     // 1-pin GPIO node
    t_717_ai,     // analog input
    t_717_ao,     // analog output
    // evb pins
    //
    t_008_1,      // GPIO 1
    t_008_3,      // GPIO 3
    t_008_5,      // GPIO 5
    t_008_17,     // GPIO 17
    t_bus,        // marker for separating bus pins
    //
    t_007_0,      // data 0
    t_007_1,      // data 1
    t_007_2,      // data 2
    t_007_3,      // data 3
    t_007_4,      // data 4
    t_007_5,      // data 5
    t_007_6,      // data 6
    t_007_7,      // data 7
    t_007_8,      // data 8
    t_007_9,      // data 9
    t_007_10,     // data 10
    t_007_11,     // data 11
    t_007_12,     // data 12
    t_007_13,     // data 13
    t_007_14,     // data 14
    t_007_15,     // data 15
    t_007_16,     // data 16
    t_007_17,     // data 17
    t_009_0,      // address 0
    t_009_1,      // address 1
    t_009_2,      // address 2
    t_009_3,      // address 3
    t_009_4,      // address 4
    t_009_5,      // address 5
    t_009_6,      // address 6
    t_009_7,      // address 7
    t_009_8,      // address 8
    t_009_9,      // address 9
    t_009_10,     // address 10
    t_009_11,     // address 11
    t_009_12,     // address 12
    t_009_13,     // address 13
    t_009_14,     // address 14
    t_009_15,     // address 15
    t_009_16,     // address 16
    t_009_17,     // address 17
    limit
  };

  static PinType::Enum type(Enum);
  static bool host(Enum);
  static bool target(Enum);
  static size_t node(Enum);

  static bool to_string(Enum, std::string&);
  static bool to_description(Enum, std::string&);
};

struct Connector {
  enum Enum {
    J1,   // external power
    J2,   // power connector
    J3,   // USB port A
    J4,   // user supply J1.9
    J5,   // user supply J1.7
    J6,   // user supply J1.5
    J7,   // port A Access
    J8,   // port A Access
    J9,   // USB port B
    J10,   // host power select
    J11,   // host power select
    J12,   // port B Access
    J13,   // port B Access
    J14,   // target power select
    J15,   // target power select
    J16,   // target power select
    J17,   // port C Access
    J18,   // USB port C
    J19,   // port C Access
    J20,   // reset and boot
    J21,   // uncommitted host pins
    J22,   // reset and host communication
    J23,   // port data connections to host and target
    J24,   // port B Access
    J25,   // reset and boot
    J26,   // reset and boot
    J27,   // uncommitted host pins
    J28,   // uncommitted target pins
    J29,   // ground pins
    J30,   // uncommitted target pins
    J31,   // uncommitted target pins
    J32,   // uncommitted target pins
    J33,   // SD socket signals
    J34,   // reset and host communication
    J35,   // reset and host communication
    J36,   // uncommitted target pins
    J37,   // SPI bus expansion
    J38,   // SD/MMC socket signals
    J39,   // SPI bus expansion
    J40,   // SD/MMC socket signals
    J41,   // 
    J42,   // 
    J43,   // 
    J44,   // 
    J45,   // 
    J46,   // 
    J47,   // 
    J48,   // 
    J49,   // 
    J50,   // 
    J51,   // 
    J52,   // 
    J53,   // 
    J54,   // 
    J55,   // 
    J56,   // 
    J57,   // 
    J58,   // 
    J59,   // 
    J60,   // 
    J61,   // 
    J62,   // 
    J63,   // 
    J64,   // 
    J65,   // 
    J66,   // 
    J67,   // 
    J68,   // 
    J69,   // 
    J70,   // 
    J71,   // 
    J72,   // 
    J73,   // 
    J74,   // 
    J75,   // 
    J76,   // 
    JP1,   // 
    limit
  };

  static bool has_pin1(Enum);
  static bool to_string(Enum, std::string&);
  static bool to_description(Enum, std::string&);
};

struct Jumper {
  enum Enum {
    J10_1_2,      // host VddC on J1.3
    J10_2_3,      // host VddC on main 1.8V bus
    J11_1_2,      // host VddI/A on J1.3
    J11_2_3,       // host VddI/A on main 1.8V bus
    J14_1_2,      // target VddC on J1.1
    J14_2_3,      // target VddC on main 1.8V bus
    J15_1_2,      // target VddI on J1.1
    J15_2_3,      // target VddI on main 1.8V bus
    J16_1_2,      // target VddA on J1.1
    J16_2_3,      // target VddA on main 1.8V bus
    J20_1_3,      // USB port A RTS signal connected to host RESET
    J20_2_4,      // host chip reset circuit / button
    J22_1_2,      // host 500.17 is connected to the target RESET
    J22_3_4,      // USB port C RTS signal connected to target RESET
    J22_5_6,      // power on circuit and pushbiutton is connected to the target RESET
    J23_1_2,      // USB port A Rx to host 708.17
    J23_3_4,      // USB port A Tx from host 708.1
    J23_5_6,      // USB port B Rx to host 200.17
    J23_7_8,      // USB port B Tx from host 100.1
    J23_9_10,     // USB port C Rx to host 708.17
    J23_11_12,    // USB port C Tx from host 708.1
    J25_1_2,      // flash chip is held in reset so that host 705's pins are available
    J25_2_3,      // flash chip is reset by host chip's reset and (if both J20 jumpers are inserted) by USB port A RTS
    J26_1_2,      // pulls host 705 pins high. Host 705 does not attempt to boot nor does it drive any of its pins
    J34_1_2,      // Host 300.1 <-> Target 300.1
    J35_1_2,      // Host 300.17 <-> Target 300.17
    J37_1_2,      // 
    J37_3_4,      // 
    J38_1_J40_1,  // CLK/SCLK <-> SPI CLK MMC
    J38_2_J40_2,  // DAT3/CS- <-> SPI CS- MMC
    J38_3_J40_3,  // CMD/SI <-> SPI DO
    J38_4_J40_4,  // DAT0/SO <-> SPI DI
    J38_5_J40_5,  // VDD <-> 1.8V
    J39_1_2,      // host 600 selects SPI flash when its pin is low (reset condition), or MMC card when the pin is high.
    J39_2_3,      // SPI flash is always selected. Host 600 unused.
    limit
  };

  static bool to_string(Enum, std::string&);
  static bool to_description(Enum, std::string&);
};

struct BootMethod {
  enum Enum {
    undefined,
    async_708,
    async_708_kraken_707,
    async_708_kraken_709,
    async_708_kraken_707_709,
    async_708_host,
    async_708_host_target,
    fast_708_host,
    fast_708_host_target,
    limit
  };
};

class Action {
public:
  enum Enum {
    nop,
    delimiter,
    reset,
    boot,
    activate_kraken, // node
    span_host,
    span_target,
    span_host_target,
    remove_all_tentacles,
    destroy_tentacle,
    //add_tentacle_segment, // ga144::Neighbour::Enum
    //remove_last_tentacle_segment,
    expand_tentacle_to, // node
    reduce_tentacle_to, // node
    read_ram, // node
    read_rom, // node
    read_and_compare_rom, // node
    read_ram_and_rom, // node
    write_ram, // node
    read_sram, // memory, adr
    write_sram, // memory, adr
    read_stack, // node
    write_stack, // node
    read_return_stack, // node
    write_return_stack, // node
    read_a, // node
    write_a, // node
    write_b, // node
    read_io, // node
    write_io, // node
    call, // node
    jump, // node
    run, // node
    start, // node
    read, // node
    write, // node
    test, // node
    write_pin, // pin, value
    compile_module, // module
    write_module, // module
    start_module, // module
    boot_708, // boot_stream (boot chip for interaction)
    boot_via_708, // boot_stream (boot chip & load application)
    boot_708_both, // boot_stream for host & target chip (boot chip for interaction)
    boot_via_708_both, // boot_stream for host & target chip (boot chip & load application)
    limit
  };

private:
  Enum                  op_;
  std::vector<long>     parameter_;
  Module_ptr            module_;
  f18a::cell_list_type  data_;

public:
  Action();
  Action(Enum);
  Action(Enum, long);
  Action(Enum, long, long);
  Action(Enum, Module_ptr);
  Action(Enum, const f18a::cell_list_type&);

  Enum op() const {return op_;}

  void signal_error(Manager&);
  void signal_success(Manager&);

  void action(Manager&);

};
typedef std::queue<Action> action_queue_type_;


class Tentacle : public utils::RefCntObj {
public:
  typedef std::vector<size_t>   tentacle_type_;

private:
  Kraken_ptr                    kraken_;
  tentacle_type_                path_;
  int                           delay_ms_;

public:
  Tentacle(Kraken_ptr);
  ~Tentacle();

  Umbilical_ptr umbilical();
  const Umbilical_ptr umbilical() const;

  Kraken_ptr kraken() {return kraken_;}
  const Kraken_ptr kraken() const {return kraken_;}

  Manager_ptr manager();
  const Manager_ptr manager() const;

  void reset();
  bool empty() const {return path_.empty();}
  size_t size() const {return path_.size();}
  size_t front() const {return path_.front();}
  size_t back() const {return path_.back();}

  void push_back(size_t node);
  void pop_back() {path_.pop_back();}

  bool node_to_tentacle_index(size_t node, size_t& tentacle_index) const;
  bool has_node(size_t node) const;

  size_t at(size_t ind) const {return path_[ind];}

  bool activate();
  void delay(size_t ms);
  void reset_delay();
  size_t delay_ms() const;

  bool access(size_t node, size_t&);

  bool destroy(); // read all nodes in a tentacle
  bool remove_last_segment(); // remove last node in the current tentacle
  bool add_segment_code(ga144::Neighbour::Enum); // add another node to the current tentacle
  bool add_segment_data(ga144::Neighbour::Enum); // add another node to the current tentacle
  bool add_segment(ga144::Neighbour::Enum, bool send_code); // add another node to the current tentacle
  bool add_to(size_t to_node, bool send_code);
  bool reduce_to(size_t to_node);
  bool test(size_t index);
};

class Kraken : public utils::RefCntObj {
public:
  typedef std::vector<Tentacle_ptr>  tentacle_list_type_;

private:
  Umbilical_ptr         umbilical_;
  tentacle_list_type_   tentacles_;
  f18a::Port::Enum      port_;
  size_t                node_;
  int                   active_tentacle_;
  int                   delay_ms_;

public:
  Kraken(Umbilical_ptr, size_t node, f18a::Port::Enum port);
  ~Kraken();

  f18a::Port::Enum port() const {return port_;}

  Umbilical_ptr umbilical() {return umbilical_;}
  const Umbilical_ptr umbilical() const {return umbilical_;}

  Manager_ptr manager();
  const Manager_ptr manager() const;

  Tentacle_ptr active_tentacle();
  bool activate_tentacle(Tentacle_ptr);

  bool access(size_t node, Tentacle_ptr&, size_t&);
  //Tentacle* access(size_t node);

  size_t node() const {return node_;}

  bool activate();
  void delay(size_t ms);
  void reset_delay();
  size_t delay_ms() const;

  bool add_tentacle_to(size_t to_node, bool send_code);
  bool reduce_tentacle_to(size_t to_node);
  bool destroy_tentacle(); // read all nodes in a tentacle
  bool remove_all_tentacles();
};

class Umbilical : public utils::RefCntObj {
public:
  typedef std::vector<Kraken_ptr>  kraken_list_type_;

private:
  Manager_ptr         manager_;
  //std::string         async_module_name_;
  connection::Serial_ptr serial_;
  kraken_list_type_   kraken_list_;
  size_t              node_;
  int                 active_kraken_;
  int                 delay_ms_;

  long                async_seta_;
  long                kraken_w_;
  long                kraken_wr_;
  long                kraken_w1_;
  long                kraken_w2_;
  long                kraken_wr1_;
  long                async_snd_;
  long                async_rcv_;
  long                async_main_;
  long                async_echo_;


public:
  Umbilical(Manager_ptr, connection::Serial_ptr, size_t node);
  ~Umbilical();

  Manager_ptr manager() {return manager_;}
  const Manager_ptr manager() const {return manager_;}

  bool connected() const;

  size_t node() const {return node_;}

  void invalidate_active_kraken();
  bool setup_async(const std::string module_name, bool send_code);

  bool activate_kraken(Kraken_ptr, bool send_code);
  bool activate_kraken(size_t node, bool send_code);

  void delay(size_t ms);
  void reset_delay();
  size_t delay_ms() const;
  void wait();

  bool test_async();
  bool test_kraken();
  bool test_tentacle_end();
  bool test_tentacle(size_t index);

  bool access(size_t node, Kraken_ptr&, Tentacle_ptr&, size_t&);
  //Kraken* access(size_t node);

  bool snd(long data); // operate instruction directly on the active Kraken node
  bool rcv(long& data); // operate instruction directly on the active Kraken node
  bool w(size_t segment, size_t out_size, long* out_data);
  bool w1(size_t segment, long data);
  bool w2(size_t segment, long data1, long data2);
  bool wr(size_t segment, size_t out_size, long* out_data, size_t in_size, long* in_data);
  bool wr1(size_t segment, long out_data, long& in_data);

  bool write_boot_stream(const f18a::cell_list_type& boot_stream);
  bool write_boot_frame(const std::string& module_name, bool send_code);
  bool install_kraken(size_t node, const std::string& module_name, bool send_code);
  //bool reset_target();
  bool add_tentacle_to(size_t to_node, bool send_code);
  bool reduce_tentacle_to(size_t to_node);
  bool destroy_tentacle(); // read all nodes in a tentacle
  bool remove_all_tentacles();
};

class NodeAccess : public utils::RefCntObj {
private:
  Umbilical_ptr   umbilical_;
  Kraken_ptr      kraken_;
  Tentacle_ptr    tentacle_;
  size_t          segment_;
  size_t          node_;

public:
  NodeAccess();
  NodeAccess(Umbilical_ptr, Kraken_ptr, Tentacle_ptr, size_t segment, size_t node);

  bool valid() const;

  Umbilical_ptr umbilical();
  Kraken_ptr kraken();
  Tentacle_ptr tentacle();
  size_t segment() const {return segment_;}
  size_t node() const {return node_;}

  bool snd(long data); // operate instruction directly on the active Kraken node
  bool rcv(long& data); // operate instruction directly on the active Kraken node
  bool w(size_t out_size, long* out_data);
  bool w1(long data);
  bool w2(long data1, long data2);
  bool wr(size_t out_size, long* out_data, size_t in_size, long* in_data);
  bool wr1(long out_data, long& in_data);

  bool activate();
};
typedef utils::RefCntPtr<NodeAccess> NodeAccess_ptr;

class SRAM : public utils::RefCntObj {
private:
  std::vector<short>    data_;

public:
  SRAM();
  ~SRAM();

  short read(size_t adr) const;
  void write(size_t adr, short);
};
typedef utils::RefCntPtr<SRAM> SRAM_ptr;

class Manager : public utils::RefCntObj {
public:
  typedef std::vector<Umbilical_ptr>  umbilical_list_type_;

  struct RemoteOp {
    enum Enum {
      write_a,
      write_b,
      write_stack,
      write_rstack,
      write_mem,
      write_mem_loop,
      write_mem_loop_b,
      write_stack4,
      write_stack3,
      write_stack2,
      write_stack1,
      read_a,
      read_stack,
      read_rstack,
      read_mem,
      read_mem_loop,
      read_stack4,
      read_stack3,
      read_stack2,
      read_stack1,
      read_rstack2,
      set_a_to_0,
      push_rstack3,
      //expand_tentacle_to,
      //reduce_tentacle_to,
      store_a,
      store_b,
      fetch_a,
      fetch_b,
      //set_pin,
      limit,
    };
  };

private:

  Board_ptr                     board_;
  //connection::Serial&           umbilical_;
  mutable std::vector<std::string>  event_list_;
  umbilical_list_type_          umbilical_list_;
  
  boost::mutex                  mutex_;
  boost::condition_variable     cond_;
  boost::thread*                thread_;
  action_queue_type_            action_queue_;

  //long                          ops_[RemoteOp::limit];

  //std::vector<Tentacle>         tentacles_;

  //size_t                        umbilical_node_;
  //size_t                        async_write_delay_ms_;
  //int                           active_kraken_;
  //std::vector<size_t>           kraken_nodes_;

  //BootMethod::Enum              boot_method_;

  volatile bool                 is_active_;
  volatile bool                 is_running_;

public:
  Manager(Board_ptr/*, connection::Serial&*/);
  ~Manager();

  static f18a::cell remote_op(RemoteOp::Enum op); // {return ops_[op];}

  Board_ptr board() {return board_;}
  const Board_ptr board() const {return board_;}

  bool access(size_t node, Umbilical_ptr&, Kraken_ptr&, Tentacle_ptr&, size_t&);
  NodeAccess access(size_t node);

  bool boot_async(size_t node, const std::string& module_name, bool send_code);
  bool boot_async_708_kraken(const std::string& module_name, bool kraken_in_707, bool kraken_in_709, bool kraken_in_608, bool send_code);
  bool boot_async_708_kraken_707(const std::string& module_name, bool send_code);
  bool boot_async_708_host(const std::string& module_name, bool send_code);
  bool boot_async_708_host_target(const std::string& module_name, bool send_code);
  bool boot_fast_708_host(const std::string& module_name);
  bool boot_fast_708_host_target(const std::string& module_name);
  bool install_kraken(size_t node, const std::string& module_name, bool send_code);

  const std::vector<std::string>& event_list() const {return event_list_;}

  bool port_to_kraken(size_t umbilical_node, size_t kraken_node, f18a::Port::Enum&);

  void reset_error_list() {event_list_.clear();}
  bool has_error() {return !event_list_.empty();}
  void add_error(const std::string& val) {event_list_.push_back(val);}
  void error();
  void success();

  //bool node_to_tentacle_index(size_t node, size_t& tentacle_index) const;
  //bool node_is_tentacle(size_t node, bool is_active) const;
  void invalidate_active_kraken();
  bool reset();
  bool reset_target();
  bool boot(BootMethod::Enum);
  bool boot_via(BootMethod::Enum, const f18a::cell_list_type& boot_stream);
  bool connected(Port::Enum) const;
  bool connect(Port::Enum);
  //bool kraken_ready();
  //bool kraken_ok() const;
  bool activate_kraken(size_t node, bool send_code);
  bool activate_node(size_t node, NodeAccess& acc);
  bool silence_sram();
  bool silence_spi();
  bool silence_all_io();

  bool add_tentacle_to(size_t to_node, bool send_code);
  bool reduce_tentacle_to(size_t to_node);
  bool destroy_tentacle(); // read all nodes in a tentacle
  //bool remove_last_tentacle_segment(); // remove last node in the current tentacle
  bool remove_all_tentacles();
  bool read(size_t node);
  bool write(size_t node);
  bool start(size_t node);
  bool run(size_t node);
  bool recycle(size_t node);  // test if a node is waiting on a part and if so sends a call to warm
  bool reach_target(); // build a connection via SERDES to target chip
  bool span_host(bool send_code); // set a tentacle crossing the whole host chip
  bool span_target(bool send_code); // set a tentacle crossing the whole target chip
  bool span_host_target(bool send_code); // set a tentacle crossing the whole host+target chips
  bool read_tentacle(); // read all nodes in a tentacle 
  //bool add_tentacle_segment(ga144::Neighbour::Enum); // add another node to the current tentacle
  bool read_ram(size_t node);
  bool read_rom(size_t node, f18a::Memory_ptr mem = 0);
  bool read_and_compare_rom(size_t node);
  bool read_ram_and_rom(size_t node);
  bool write_ram(size_t node);
  bool read_stack(size_t node);
  bool write_stack(size_t node);
  bool read_return_stack(size_t node);
  bool write_return_stack(size_t node);
  bool read_a(size_t node);
  bool write_a(size_t node);
  bool write_b(size_t node);
  bool read_io(size_t node);
  bool read_data(size_t node, long& data);
  bool write_io(size_t node, long value);
  bool write_data(size_t node, long value);
  bool call(size_t node);
  bool jump(size_t node);
  bool test(size_t node);

  bool read_sram(size_t adr);
  bool write_sram(size_t adr);

  bool build_sync_bridge(size_t host_node, size_t target_node, size_t next_node);

  bool compile_module(Module_ptr);
  bool write_module(Module_ptr);
  bool start_module(Module_ptr);
  bool boot_708(const std::string& package_name);
  bool boot_via_708(const f18a::cell_list_type& boot_stream);
  bool boot_708_both(const std::string& package_name);
  bool boot_via_708_both(const f18a::cell_list_type& boot_stream);

  bool test_kraken();
  //bool test_tentacle();
  //bool test_tentacle_end();


  // kraken transactions
  //bool echo(long out, long& in); // operate instruction directly on the communication node
  //bool snd(long data); // operate instruction directly on the Kraken node
  //bool rcv(long& data); // operate instruction directly on the Kraken node
  //bool w(size_t segment, size_t out_size, long* out_data);
  ////bool w_slow(size_t segment, size_t out_size, long* out_data, long ms_delay);
  //bool w1(size_t segment, long data);
  //bool w2(size_t segment, long data1, long data2);
  //bool wr(size_t segment, size_t out_size, long* out_data, size_t in_size, long* in_data);
  //bool wr1(size_t segment, long out_data, long& in_data);
  //bool feedback();

  void append_action(const Action&, bool with_delimiter);
  void run_thread();
  void shutdown();

  void cancel_current_action();
  void cancel_action_group();
  void cancel_all_actions();

  void do_reset();
  void do_boot(BootMethod::Enum);
  void do_destroy_tentacle();
  void do_span_host();
  void do_span_target();
  void do_span_host_target();
  void do_remove_all_tentacles();
  void do_activate_kraken(size_t node);

  void do_read_ram(size_t node);
  void do_read_rom(size_t node);
  void do_read_and_compare_rom(size_t node);
  void do_read_ram_and_rom(size_t node);
  void do_write_ram(size_t node);
  void do_read_stack(size_t node);
  void do_write_stack(size_t node);
  void do_read_return_stack(size_t node);
  void do_write_return_stack(size_t node);
  void do_read_a(size_t node);
  void do_write_a(size_t node);
  void do_write_b(size_t node);
  void do_read_io(size_t node);
  void do_write_io(size_t node);
  void do_call(size_t node);
  void do_jump(size_t node);
  void do_read(size_t node);
  void do_write(size_t node);
  void do_start(size_t node);
  void do_run(size_t node);
  void do_test(size_t node);
  void do_expand_tentacle_to(size_t node);
  void do_reduce_tentacle_to(size_t node);

  void do_compile_module(Module_ptr);
  void do_write_module(Module_ptr);
  void do_start_module(Module_ptr);
  void do_boot_708(const f18a::cell_list_type& boot_stream);
  void do_boot_via_708(const f18a::cell_list_type& boot_stream);
  void do_boot_708_both(const f18a::cell_list_type& boot_stream);
  void do_boot_via_708_both(const f18a::cell_list_type& boot_stream);

  bool get_pin(Pin::Enum, PinStatus::Enum&, long& value);
  bool set_pin(Pin::Enum, long value);
  void do_set_pin(Pin::Enum, long value);

};

class Module : public utils::RefCntObj {
private:
  Board&                          board_;
  std::string                     name_;
  std::vector<f18a::Module_ptr>   module_list_;
  std::vector<size_t>             node_list_;

public:
  Module(Board&, const std::string&);
  ~Module();

  const std::string& name() const {return name_;}
  void rename(const std::string& val) {name_ = val;}

  size_t size() const {return node_list_.size();}
  size_t node(size_t pos) const {return node_list_[pos];}
  f18a::Module_ptr module(size_t pos) {return module_list_[pos];}
  const f18a::Module_ptr module(size_t pos) const {return module_list_[pos];}

  void add(size_t node, f18a::Module_ptr);
  void remove(size_t node);

  void read(xml::Iterator&);
  void write(xml::Writer&);

  //bool compile();
  //bool write();
  //bool start();

};

class Package : public utils::RefCntObj {
private:
  Board&                    board_;
  std::string               name_;
  std::vector<Module_ptr>   module_list_;
  node_list_type            exclude_list_;

public:
  Package(Board&, const std::string&);
  ~Package();

  const std::string& name() const {return name_;}
  void rename(const std::string& val) {name_ = val;}
  void exclude_list(const node_list_type& val) {exclude_list_ = val;}

  size_t size() const {return module_list_.size();}
  Module_ptr module(size_t pos) {return module_list_[pos];}
  const Module_ptr module(size_t pos) const {return module_list_[pos];}
  const node_list_type& exclude_list() const {return exclude_list_;}

  void clear();
  void append(Module_ptr);
  void remove(size_t pos);
  bool move_up(size_t pos);
  bool move_down(size_t pos);
  bool find(const std::string& name, size_t& pos);
  Package_ptr clone();

  void read(xml::Iterator&);
  void write(xml::Writer&);

};

class Board : public utils::RefCntObj {
public:
  typedef std::map<std::string, std::string> screen_map_type;
  typedef std::map<std::string, int> screen_index_map_type;
  typedef std::map<int, std::string> screen_lookup_map_type;
  typedef std::map<std::string, Module_ptr> module_map_type;
  typedef std::map<std::string, Package_ptr> package_map_type;

private:
  std::vector<connection::Serial_ptr>  connection_list_;
  //std::vector<ChipPtr>    chip_list_;

  Glib::Dispatcher*               modify_dispatcher_;
  boost::mutex                    modify_mutex_;
  std::vector<Glib::Dispatcher*>  modify_dispatcher_list_;

  boost::mutex                    error_mutex_;
  std::vector<Glib::Dispatcher*>  error_dispatcher_list_;
  std::vector<Glib::Dispatcher*>  success_dispatcher_list_;

  std::string             name_;
  ga144::Chip_ptr   	    host_;
  ga144::Chip_ptr   	    target_;
  module_map_type         module_map_;
  package_map_type        package_map_;
  screen_index_map_type   screen_to_index_;
  screen_lookup_map_type  screen_index_to_source_;
  long long               jumpers_;
  SRAM                    sram_;

  Board(const Board&);
  Board& operator=(const Board&);

  void load_node_modules(ga144::Chip_ptr chip, f18a::Processor_ptr, module_map_type&);
  void load_chip_modules(ga144::Chip_ptr, module_map_type&);

public:
  Board();
  ~Board();

  static const size_t invalid_node = 999999;

  virtual std::string picture_file() const;
  virtual std::string version() const;

  const SRAM& sram() const {return sram_;}
  SRAM& sram() {return sram_;}

  const std::string& name() const {return name_;}
  void name(const std::string& val) {name_ = val;}

  void clean();
  void update_modules();
  module_map_type& modules() {return module_map_;}
  Module_ptr find_module(const std::string&);
  Module_ptr add_module(const std::string&);
  Module_ptr rename_module(const std::string& old_name, const std::string& new_name);

  package_map_type& packages() {return package_map_;}
  Package_ptr find_package(const std::string&);
  Package_ptr add_package(const std::string&);
  bool remove_package(const std::string&);
  Package_ptr rename_package(const std::string& old_name, const std::string& new_name);
  void set_package(Package_ptr);

  void reset();
  //bool boot_async(Port::Enum);
  //bool boot_async_kraken(Port::Enum);
  bool connect(Port::Enum);
  bool connected(Port::Enum) const;
  void test_echo();

  //connection::Serial& serial(size_t pos) {return *connection_list_[pos];}
  ga144::Chip_ptr chip(size_t pos); // {return *chip_list_[pos];}
  ga144::Chip_ptr host() {return host_;}
  ga144::Chip_ptr target() {return target_;}

  bool jumper(Jumper::Enum) const;
  void jumper(Jumper::Enum, bool val);

  virtual void read(xml::Iterator&);
  virtual void write(xml::Writer&);

  bool has_connection(Port::Enum p) const {return connection_list_[p] != 0;}
  connection::Serial_ptr connection(Port::Enum p) {return connection_list_[p];}
  const connection::Serial_ptr connection(Port::Enum p) const {return connection_list_[p];}
  void connection(Port::Enum, connection::Serial_ptr);
  connection::Serial_ptr assert_connection(Port::Enum p);
  connection::Serial_ptr new_connection(Port::Enum p);

  bool find_screen(const std::string&, int&);
  bool find_screen_source(int, std::string&);

  f18a::Processor_ptr processor(size_t node);
  const f18a::Processor_ptr processor(size_t node) const;
  const ga144::Chip_ptr split_node(size_t& node) const;
  ga144::Chip_ptr split_node(size_t& node);
  size_t join_node(const ga144::Chip_ptr, size_t node) const;

  long info(size_t node);
  void info(size_t node, long val);
  void set_info(size_t node, ga144::InfoBit::Enum val);

  void on_modify();
  void modify();
  void register_modify_dispatcher(Glib::Dispatcher&);
  void deregister_modify_dispatcher(Glib::Dispatcher&);

  void error();
  void register_error_dispatcher(Glib::Dispatcher&);
  void deregister_error_dispatcher(Glib::Dispatcher&);

  void success();
  void register_success_dispatcher(Glib::Dispatcher&);
  void deregister_success_dispatcher(Glib::Dispatcher&);

  // simulation
  void step(); // simulate 1 step
  void tic(); // first half of step, all io-lines to the neighbours are evaluated and set
  void toc(); // second half of step, execute current code and perform port operations if any

  // boot stream
  struct BootFramePath {
    enum Enum {
      via_300, // boot frame loaded via node 300 into target chip
      for_300, // boot frame for node 300 on the target chip
      for_300_bridge, // boot frame containing bridge code for node 10300
      via_705, // load host/target chip via node 705
      via_705_to_300_bridge, // build path to node 300 for sending one or several boot frames to node 10300
      via_705_remaining_chip, // build path to remaining nodes on host chip
      for_705, // boot frame for node 705 on the host chip
      via_708, // load host/target chip via node 708
      via_708_to_300_bridge, // build path to node 300 for sending one or several boot frames to node 10300
      via_708_remaining_chip, // build path to remaining nodes on host chip
      for_708 // boot frame for node 708 on the host chip
    };
  };

  void get_path(
    BootFramePath::Enum,
    size_t chip_offset,
    f18a::port_list_type&,
    node_list_type&
  );

  struct BootCodeType {
    enum Enum {
      default_code,
      chain_via_a,
      chain_via_b
    };
  };


  void create_boot_frame(
    BootFramePath::Enum,
    BootCodeType::Enum,
    f18a::cell final_jump_adr,
    size_t chip_offset,
    Package_ptr package,
    const f18a::cell_list_type& additional_data,
    f18a::cell_list_type& frame);

  void create_build_path_stream(boot::Stream&);
  void create_boot_stream(boot::Stream&, Package_ptr);
  void append_default_init_code(ga144::Chip_ptr, size_t node, f18a::cell_list_type&, bool is_boot_frame);
  //void append_default_boot_stream(ga144::Chip_ptr, size_t node, f18a::cell_list_type&);

};

}

#endif
