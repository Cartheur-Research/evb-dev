#include "f64sim.hpp"

namespace f64sim {

void Processor::reset()
{
}

void Processor::cold_reset()
{
  reset();
}

int Processor::execute(int instr)
{
  return -1;
}

int Processor::step(int elapsed)
{
  if (is_running_) {
    if (elapsed < delta_) {
      delta_ -= elapsed;
      return delta_;
    }
    delta_ = 0;
    // process next instruction
    int instr;
    if (!task_[curr_].next_instr(instr)) { // 
      adr_ = task_[curr_].next_ip();
      delta_ = mem_.read(adr_, data_);
      if (delta_ > 0) {
        task_[curr_].load_instr_word(data_);
        return delta_;
      }
      is_running_ = false;
      wait_memory_ = true;
      wait_is_read_ = true;
      wait_is_data_ = false;
      return -1;
    }
    delta_ = execute(instr);
    return delta_;
  }
  if (wait_memory_) {
    if (wait_is_read_) {
      delta_ = mem_.read(adr_, data_);
      if (delta_ > 0) {
        is_running_ = true;
        wait_memory_ = false;
        if (wait_is_data_) {
          task_[curr_].literal(data_);
        }
        else {
          task_[curr_].load_instr_word(data_);
        }
        return delta_;
      }
    }
    else {
      delta_ = mem_.write(adr_, data_);
      if (delta_ > 0) {
        is_running_ = true;
        wait_memory_ = false;
        return delta_;
      }
    }
  }
  return -1;
}


}
