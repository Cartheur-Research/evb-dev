#include "pf.hpp"


namespace host {
};

//=====================================================================================================================

namespace pF {

#define RAM_SIZE  0x100000

VirtualMachine::VirtualMachine()
: ip_(0)
, sp_(0)
, rp_(0)
, t_(0)
, s_(0)
, r_(0)
, u_(0)
{
  ram_.resize(RAM_SIZE);
}

VirtualMachine::~VirtualMachine()
{
}

void VirtualMachine::push_p(cell val)
{
  ram_[--sp_] = val;
}

void VirtualMachine::push_r(cell val)
{
  ram_[--rp_] = val;
}

cell VirtualMachine::pop_p()
{
  cell res = ram_[sp_++];
  return res;
}

cell VirtualMachine::pop_r()
{
  cell res = ram_[rp_++];
  return res;
}

cell VirtualMachine::fetch(cell addr)
{
  return ram_[static_cast<unsigned short>(addr)];
}

void VirtualMachine::store(cell addr, cell val)
{
  ram_[static_cast<unsigned short>(addr)] = val;
}

cell VirtualMachine::to_byte_addr(cell addr)
{
  return addr * 2;
}

unsigned char VirtualMachine::fetch_byte(cell byte_addr)
{
  return 0;
}

void VirtualMachine::store_byte(cell byte_addr, unsigned char val)
{
}

cell VirtualMachine::ext_fetch(cell hi_addr, cell lo_addr)
{
  unsigned long addr = static_cast<unsigned short>(hi_addr);
  addr <<= 16;
  addr |= static_cast<unsigned short>(lo_addr);
  return ram_[addr];
}

void VirtualMachine::ext_store(cell hi_addr, cell lo_addr, cell val)
{
  unsigned long addr = static_cast<unsigned short>(hi_addr);
  addr <<= 16;
  addr |= static_cast<unsigned short>(lo_addr);
  ram_[addr] = val;
}

void VirtualMachine::do_umadd()
{
  unsigned long t = static_cast<unsigned short>(t_);
  unsigned long s = static_cast<unsigned short>(s_);
  unsigned long sum = t + s;
  s_ = static_cast<cell>(sum);
  t_ = static_cast<cell>(sum >> 16);
}

void VirtualMachine::do_swap()
{
  cell tmp = t_;
  t_ = s_;
  s_ = tmp;
}

void VirtualMachine::do_serial_in()
{
}

void VirtualMachine::do_serial_out()
{
}

void VirtualMachine::do_()
{
  throw "not implemented";
}

void VirtualMachine::execute(Primitive::Enum opcode)
{
  switch (opcode) {
  case Primitive::nop: break;
  case Primitive::inc: ++t_; break;
  case Primitive::dec: --t_; break;
  case Primitive::add: t_ = s_ + t_; s_ = pop_p(); break;
  case Primitive::sub: t_ = s_ - t_; s_ = pop_p(); break;
  case Primitive::umadd: do_umadd(); break;
  case Primitive::div2: t_ >>= 1;  break;
  case Primitive::mul2: t_ <<= 1; break;
  case Primitive::rsh8: t_ >>= 8;  break;
  case Primitive::lsh8: t_ <<= 8; break;
  case Primitive::min: if (s_ < t_) {t_ = s_;} s_ = pop_p(); break;
  case Primitive::max: if (s_ > t_) {t_ = s_;} s_ = pop_p(); break;
  case Primitive::neg: t_ = -t_; break;
  case Primitive::inv: t_ = ~t_; break;
  case Primitive::dup: push_p(s_); s_ = t_; break;
  case Primitive::qdup: if (t_ != 0) {push_p(s_); s_ = t_;} break;
  case Primitive::swap: do_swap(); break;
  case Primitive::drop: t_ = s_; s_ = pop_p(); break;
  case Primitive::over: push_p(s_); do_swap(); break;
  case Primitive::under: push_p(t_); break;
  case Primitive::dip: push_p(s_); break;
  case Primitive::nip: s_ = pop_p(); break;
  case Primitive::or: t_ = s_ | t_; s_ = pop_p(); break;
  case Primitive::xor: t_ = s_ ^ t_; s_ = pop_p(); break;
  case Primitive::and: t_ = s_ & t_; s_ = pop_p(); break;
  case Primitive::zeq: if (t_ == 0) {t_ = -1;} else {t_ = 0;} break;
  case Primitive::zlt: if (t_ < 0) {t_ = -1;} else {t_ = 0;} break;
  case Primitive::lit: push_p(s_); s_ = t_; t_ = ram_[ip_++]; break;
  case Primitive::con: push_p(s_); s_ = t_; t_ = ram_[ip_]; ip_ = r_; r_ = pop_r(); break;
  case Primitive::var: push_p(s_); s_ = t_; t_ = ip_; ip_ = r_; r_ = pop_r(); break;
  case Primitive::exit: ip_ = r_; r_ = pop_r(); break;
  case Primitive::exec: push_r(r_); r_ = ip_; ip_ = t_; t_ = s_; s_ = pop_p(); break;
  case Primitive::fetch: t_ = fetch(t_); break;
  case Primitive::store: store(t_, s_); t_ = pop_p(); s_ = pop_p(); break;
  case Primitive::sp_fetch: push_p(s_); s_ = t_; t_ = sp_ - 2; break;
  case Primitive::sp_store: push_p(s_); sp_ = t_; t_ = pop_p(); s_ = pop_p(); break;
  case Primitive::rp_fetch: push_p(s_); s_ = t_; t_ = rp_ - 1; break;
  case Primitive::rp_store: push_r(r_); rp_ = t_; r_ = pop_r(); t_ = s_; s_ = pop_p(); break;
  case Primitive::tor: push_r(r_); r_ = t_; t_ = s_; s_ = pop_p(); break;
  case Primitive::r_fetch: push_p(s_); s_ = t_; t_ = r_; break;
  case Primitive::rfrom: push_p(s_); s_ = t_; t_ = r_; r_ = pop_r(); break;
  case Primitive::rdup: push_r(r_); break;
  case Primitive::rdrop: r_ = pop_r(); break;
  case Primitive::rxq: do_serial_in(); break;
  case Primitive::txq: do_serial_out(); break;
  case Primitive::if_: if (t_ == 0) {ip_ = ram_[ip_];} else {++ip_;} t_ = s_; s_ = pop_p(); break;
  case Primitive::else_: ip_ = ram_[ip_]; break;
  case Primitive::next: if (r_ != 0) {--r_; ip_ = ram_[ip_];} else {r_ = pop_r(); ++ip_;} break;
  case Primitive::xm_fetch: t_ = ext_fetch(t_, s_); s_ = pop_p(); break;
  case Primitive::xm_store: ext_store(t_, s_, pop_p()); t_ = pop_p(); s_ = pop_p(); break;
  case Primitive::xcx: do_(); break;
  case Primitive::mk_store: do_(); break;
  case Primitive::suspend: break; // do nothing
  case Primitive::u: t_ += u_; break;
  case Primitive::u_fetch: push_p(s_); s_ = t_; t_ = u_; break;
  case Primitive::u_store: u_ = t_; t_ = s_; s_ = pop_p(); break;
  case Primitive::usr: push_p(s_); s_ = t_; t_ = u_ + ram_[ip_]; ip_ = r_; r_ = pop_r(); break;
  case Primitive::undefined: throw "invalid opcode";
  default:;
  }
}

void VirtualMachine::step()
{
  cell curr = ram_[ip_++];
  execute(Primitive::from_code(curr));
}


}
