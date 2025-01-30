#include "f64sim.hpp"

namespace f64sim {

void Task::write(Register::Enum r, long long val)
{
  if (r < Register::P) {
    reg_.list_[r] = val;
  }
  else if (r == Register::P) {
    from_ret_adr(val);
  }
}

long long Task::read(Register::Enum r)
{
  if (r < Register::P) {
    return reg_.list_[r];
  }
  if (r == Register::P) {
    return ret_adr();
  }
  return 0;
}

void Task::reset()
{
  size_t i;
  for (i=0; i<Register::limit; ++i) {
    reg_.list_[i] = 0;
  }
  ip_ = 0;
  slot_ = 0;
  sp_ = 0;
  rp_ = 0;
  task_ = 0;
}

long long Task::ret_adr() const
{
  long long val = slot_;
  val <<= F64_TASK_BITS;
  val |= task_;
  val <<= F64_TASK_SHIFT - F64_ADDR_BITS;
  val |= next_ip_;
  val <<= F64_ADDR_BITS;
  val |= ip_;
  return val;
}

void Task::from_ret_adr(long long val)
{
  ip_ = static_cast<int>(val) & F64_ADDR_MASK;
  val >>= F64_ADDR_BITS;
  next_ip_ = static_cast<int>(val) & F64_ADDR_MASK;
  val >>= F64_TASK_SHIFT - F64_ADDR_BITS;
  task_ = static_cast<int>(val) & F64_TASK_MASK;
  val >>= F64_TASK_BITS;
  slot_ = static_cast<int>(val) & F64_SLOT_INDEX_MASK;
}


bool Task::next_instr(int& instr)
{
  if (slot_ < F64_SLOT_INDEX_SIZE) {
    instr = static_cast<int>(reg_.named_.I_ >> ((7 - slot_)*F64_SLOT_BITS)) & F64_SLOT_MASK;
    ++slot_;
    return true;
  }
  instr = 0;
  return false;
}

int Task::next_ip()
{
  ip_ = next_ip_;
  if ((next_ip_ >> F64_PAGE_SHIFT) != F64_IO_PAGE) {
    ++next_ip_;
  }
  return ip_;
}
  
void Task::load_instr_word(long long val)
{
}

void Task::push_p(long long val)
{
  if (++sp_ >= F64_PARAMETER_STACK_SIZE) {sp_ = 0;}
  p_stack_[sp_] = val;
}

long long Task::pop_p()
{
  long long val = p_stack_[sp_];
  if (--sp_ < 0) {sp_ = F64_PARAMETER_STACK_SIZE-1;}
  return val;
}

void Task::push_r(long long val)
{
  if (++rp_ >= F64_RETURN_STACK_SIZE) {rp_ = 0;}
  r_stack_[rp_] = val;
}

long long Task::pop_r()
{
  long long val = r_stack_[rp_];
  if (--rp_ < 0) {rp_ = F64_RETURN_STACK_SIZE-1;}
  return val;
}

void Task::literal(long long val)
{
  dup();
  reg_.named_.T_ = val;
}

void Task::dup()
{
  push_p(reg_.named_.S_);
  reg_.named_.S_ = reg_.named_.T_;
}

void Task::over()
{
  long long val = reg_.named_.S_;
  push_p(val);
  reg_.named_.S_ = reg_.named_.T_;
  reg_.named_.T_ = val;
}

void Task::under()
{
  push_p(reg_.named_.S_);
}

void Task::swap()
{
  long long val = reg_.named_.S_;
  reg_.named_.S_ = reg_.named_.T_;
  reg_.named_.T_ = val;
}

long long Task::drop()
{
  long long val = reg_.named_.T_;
  reg_.named_.T_ = nip();
  return val;
}

long long Task::nip()
{
  long long val = reg_.named_.S_;
  reg_.named_.S_ = pop_p();
  return val;
}


}
