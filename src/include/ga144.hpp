#ifndef GA144_HPP
#define GA144_HPP

#include <node_type.hpp>
#include <f18a.hpp>
#include <bitset>

#define GA144_PROCESSOR_ROWS 8
#define GA144_PROCESSOR_COLS 18
#define GA144_CHIP_OFFSET 10000

#define GA144_PROCESSOR_SIZE (GA144_PROCESSOR_ROWS * GA144_PROCESSOR_COLS)

namespace Glib {
  class Dispatcher;
}

namespace chip {
  class Device : public utils::RefCntObj {
  };
}

namespace ga144 {

//typedef std::bitset<GA144_PROCESSOR_SIZE*f18a::Port::limit> chip_port_bitvector_type;

struct Neighbour {
  enum Enum {
    north,
    west,
    south,
    east,
    limit
  };
};

struct InfoBit {
  enum Enum {
    reset,              // node is in reset configuration
    warm,               // node is in warm configuration
    umbilical,          // node contains umbilical interface to studio
    kraken,             // node is kraken
    active_kraken,      // node is an active kraken
    tentacle,           // node is in tentacle
    bridge,             // node is bridge node
    running,            // node running some program
    limit
  };

};

class Location : public utils::RefCntObj {
  size_t  x_;
  size_t  y_;
  bool    valid_;

public:
  Location();
  Location(size_t, size_t);
  ~Location();

  bool valid() const {return valid_;}

  Location north() const;
  Location south() const;
  Location east() const;
  Location west() const;

  f18a::Port::Enum north_port() const;
  f18a::Port::Enum south_port() const;
  f18a::Port::Enum east_port() const;
  f18a::Port::Enum west_port() const;
};

class State : public utils::RefCntObj {
private:
  //chip_port_bitvector_type  read_pending_;
  //chip_port_bitvector_type  write_pending_;
public:
};

class Chip : public chip::Device {
  boost::mutex                mutex_;
  f18a::Processor_ptr         proc_[GA144_PROCESSOR_SIZE];
  NodeType_ptr                types_[GA144_PROCESSOR_SIZE];
  long                        info_[GA144_PROCESSOR_SIZE];
  size_t                      offset_;

  Glib::Dispatcher*               modify_dispatcher_;
  boost::mutex                    modify_mutex_;
  std::vector<Glib::Dispatcher*>  modify_dispatcher_list_;

  Glib::Dispatcher*               change_dispatcher_;
  boost::mutex                    change_mutex_;
  std::vector<Glib::Dispatcher*>  change_dispatcher_list_;

public:
  Chip(size_t offset);
  ~Chip();

  static const size_t invalid_index = static_cast<size_t>(-1);

  void reset();
  //void pre();
  //void post();
  void execute();

  void read(xml::Iterator&);
  void write(xml::Writer&);

  //f18a::Processor*  proc_list() {return proc_;}
  //const f18a::Processor*  proc_list() const {return proc_;}

  size_t offset() const {return offset_;}

  size_t size() const {return GA144_PROCESSOR_SIZE;}
  size_t index(size_t col, size_t row) const;
  size_t index(size_t node) const;

  size_t node(size_t index) const; // without chip offser

  size_t port(size_t index, f18a::Port::Enum port) const;

  f18a::Processor_ptr processor_by_node(size_t node);
  const f18a::Processor_ptr processor_by_node(size_t node) const;

  f18a::Processor_ptr processor(size_t index);
  const f18a::Processor_ptr processor(size_t index) const;

  NodeType_ptr node_type(size_t index) {return types_[index];}
  void node_type(size_t index, NodeType_ptr val);
  void reset_node_type(size_t index);

  long info(size_t index) {return info_[index];}
  void info(size_t index, long val);
  void set_info(size_t index, InfoBit::Enum val) {info(index, 1 << val);}

  boost::mutex& mutex() {return mutex_;}

  void on_modify();
  void modify();
  void register_modify_dispatcher(Glib::Dispatcher&);
  void deregister_modify_dispatcher(Glib::Dispatcher&);

  void on_changed();
  void changed();
  void register_change_dispatcher(Glib::Dispatcher&);
  void deregister_change_dispatcher(Glib::Dispatcher&);

  bool can_be_tentacle_segment(size_t index) const;
  bool neighbour(size_t index, Neighbour::Enum, size_t& neighbour_ind) const;
  bool neighbour_to_port(size_t index, Neighbour::Enum, f18a::Port::Enum& port_to_neighbour) const;
  bool neighbour_enum(size_t index, size_t neighbour_ind, Neighbour::Enum&) const;
  bool find_port(size_t from_node, size_t to_node, f18a::Port::Enum& port) const;

  static f18a::Port::Enum calc_neighbour(size_t& node, Neighbour::Enum); // returns address of port to next node (or 0 if not possible)
  static f18a::Port::Enum port_between(size_t from_node, size_t to_node);

  void update(evb::Board&);

  // initialization
  void compile_rom();

  // simulation
  void tic(); // first half of step
  void toc(); // second half of step
};
typedef utils::RefCntPtr<Chip> Chip_ptr;

}

#endif
