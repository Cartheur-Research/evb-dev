#include <boot.hpp>
#include <evb.hpp>

namespace boot {



// ======================================================================================

PathHelper::PathHelper()
: current_node_()
{
}

PathHelper::PathHelper(size_t node)
: current_node_(node)
{
}

void PathHelper::begin(size_t n)
{
  current_node_ = n;
}

f18a::Port::Enum PathHelper::step(ga144::Neighbour::Enum n)
{
  size_t node = current_node_;
  f18a::Port::Enum port = ga144::Chip::calc_neighbour(node, n);
  if (port != f18a::Port::limit) {
    current_node_ = node;
  }
  return port;
}


// ======================================================================================

Stream::Stream(Path::Enum p)
: path_(p)
{
}


void Stream::append_pump(size_t position, size_t len)
{
  while (position > 0) {
    stream_.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
    stream_.push_back(3 * --position + len-1);
    stream_.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop));
  }
}

void Stream::append_data_pump(size_t position, const f18a::cell_list_type& data)
{
  append_pump(position, data.size());
  stream_.insert(stream_.end(), data.begin(), data.end());
}

void Stream::append(const f18a::cell_list_type& data)
{
  stream_.insert(stream_.end(), data.begin(), data.end());
}



}
