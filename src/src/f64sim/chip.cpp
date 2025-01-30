#include "f64sim.hpp"

namespace f64sim {


void Chip::reset()
{
  size_t i;
  for (i=0; i<F64_PROCESSOR_SIZE; ++i) {
    proc_list()[i].reset();
  }
}

void Chip::cold_reset()
{
  size_t i;
  for (i=0; i<F64_PROCESSOR_SIZE; ++i) {
    proc_list()[i].cold_reset();
  }
}

}
