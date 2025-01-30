#include "f64sim.hpp"

namespace f64sim {

Memory::Memory()
: io_(0)
, rom_locked_(false)
{
  size_t i;
  for (i=0; i<F64_ROM_SIZE; ++i) {
    rom_[i] = 0;
  }
  for (i=0; i<F64_RAM_SIZE; ++i) {
    ram_[i] = 0;
  }
  for (i=0; i<F64_IO_BITS; ++i) {
    port_[i] = 0;
  }
}

int Memory::read(int adr, long long& data)
{
  return -1;
}

int Memory::write(int adr, long long& data)
{
  return -1;
}
  


}
