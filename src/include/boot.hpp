#ifndef BOOT_HPP
#define BOOT_HPP

#include <ga144.hpp>


namespace boot {

struct Path {
  enum Enum {
    undefined,
    chip_via_300,
    chip_via_705,
    chip_via_708,
    host_target_via_705_host,
    host_target_via_705_target,
    host_target_via_708_host,
    host_target_via_708_target,
    limit
  };
};

class Stream : public utils::RefCntObj {
private:
  Path::Enum            path_;
  f18a::cell_list_type  stream_;
  f18a::cell            port_address_;

  void append_pump(size_t position, size_t len);

public:
  Stream(Path::Enum);
  
  void port_address(f18a::cell val) {port_address_ = val;}

  void append(const f18a::cell_list_type& data);

  Path::Enum path() const {return path_;}
  const f18a::cell_list_type& data() const {return stream_;}
  f18a::cell port_address() {return port_address_;}

  void append_data_pump(size_t position, const f18a::cell_list_type&);

};

class PathHelper : public utils::RefCntObj {
private:
  size_t    current_node_;

public:
  PathHelper();
  PathHelper(size_t node);

  void begin(size_t node);

  f18a::Port::Enum step(ga144::Neighbour::Enum);
  size_t current_node() const {return current_node_;}
};

}

#endif
