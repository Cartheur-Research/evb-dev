#include <f18a.hpp>
#include <boost/lexical_cast.hpp>

namespace f18a {

//Task::Task()
//: slot_(0)
//, sp_(0)
//, rp_(0)
//, startup_sp_(0)
//, startup_rp_(0)
//{
//  reset();
//  reset_startup();
//}
//
//void Task::reset()
//{
//  for (size_t i=0; i<Register::limit; ++i) {
//    reg_.list_[i] = F18_INVALID_REGISTER_VALUE;
//  }
//  for (size_t i=0; i<F18A_PARAMETER_STACK_SIZE; ++i) {
//    p_stack_[i] = F18_INVALID_REGISTER_VALUE;
//  }
//  for (size_t i=0; i<F18A_RETURN_STACK_SIZE; ++i) {
//    r_stack_[i] = F18_INVALID_REGISTER_VALUE;
//  }
//  sp_ = 0;
//  rp_ = 0;
//  slot_ = 0;
//}
//
//void Task::reset_startup()
//{
//  for (size_t i=0; i<Register::limit; ++i) {
//    startup_reg_.list_[i] = F18_INVALID_REGISTER_VALUE;
//  }
//  for (size_t i=0; i<F18A_PARAMETER_STACK_SIZE; ++i) {
//    startup_p_stack_[i] = F18_INVALID_REGISTER_VALUE;
//  }
//  for (size_t i=0; i<F18A_RETURN_STACK_SIZE; ++i) {
//    startup_r_stack_[i] = F18_INVALID_REGISTER_VALUE;
//  }
//  startup_sp_ = 0;
//  startup_rp_ = 0;
//}
//
//void Task::set_startup(const Task& t)
//{
//  for (size_t i=0; i<Register::limit; ++i) {
//    startup_reg_.list_[i] = t.startup_reg_.list_[i];
//  }
//  for (size_t i=0; i<F18A_PARAMETER_STACK_SIZE; ++i) {
//    startup_p_stack_[i] = t.startup_p_stack_[i];
//  }
//  for (size_t i=0; i<F18A_RETURN_STACK_SIZE; ++i) {
//    startup_r_stack_[i] = t. startup_r_stack_[i];
//  }
//  startup_sp_ = t.startup_sp_;
//  startup_rp_ = t.startup_rp_;
//}
//
//void Task::pre()
//{
//  post_reg_ = reg_;
//}
//
//void Task::post()
//{
//  reg_ = post_reg_;
//}
//
//cell Task::pstack(size_t pos) const
//{
//  pos += sp_;
//  if (pos >= F18A_PARAMETER_STACK_SIZE) {pos -= F18A_PARAMETER_STACK_SIZE;}
//  return p_stack_[pos];
//}
//
//void Task::pstack(size_t pos, cell val)
//{
//  pos += sp_;
//  if (pos >= F18A_PARAMETER_STACK_SIZE) {pos -= F18A_PARAMETER_STACK_SIZE;}
//  p_stack_[pos] = val;
//}
//
//cell Task::rstack(size_t pos) const
//{
//  pos += rp_;
//  if (pos >= F18A_RETURN_STACK_SIZE) {pos -= F18A_RETURN_STACK_SIZE;}
//  return r_stack_[pos];
//}
//
//void Task::rstack(size_t pos, cell val)
//{
//  pos += rp_;
//  if (pos >= F18A_RETURN_STACK_SIZE) {pos -= F18A_RETURN_STACK_SIZE;}
//  r_stack_[pos] = val;
//}
//
//cell Task::startup_pstack(size_t pos) const
//{
//  pos += startup_sp_;
//  if (pos >= F18A_PARAMETER_STACK_SIZE) {pos -= F18A_PARAMETER_STACK_SIZE;}
//  return startup_p_stack_[pos];
//}
//
//void Task::startup_pstack(size_t pos, cell val)
//{
//  pos += startup_sp_;
//  if (pos >= F18A_PARAMETER_STACK_SIZE) {pos -= F18A_PARAMETER_STACK_SIZE;}
//  startup_p_stack_[pos] = val;
//}
//
//cell Task::startup_rstack(size_t pos) const
//{
//  pos += startup_rp_;
//  if (pos >= F18A_RETURN_STACK_SIZE) {pos -= F18A_RETURN_STACK_SIZE;}
//  return startup_r_stack_[pos];
//}
//
//void Task::startup_rstack(size_t pos, cell val)
//{
//  pos += startup_rp_;
//  if (pos >= F18A_RETURN_STACK_SIZE) {pos -= F18A_RETURN_STACK_SIZE;}
//  startup_r_stack_[pos] = val;
//}
//
//void Task::push(cell val)
//{
//  ++sp_;
//  if (sp_ >= F18A_PARAMETER_STACK_SIZE) {sp_ = 0;}
//  p_stack_[sp_] = val;
//}
//
//cell Task::pop()
//{
//  cell val = p_stack_[sp_];
//  if (sp_ ) {--sp_;}
//  else {sp_ = F18A_PARAMETER_STACK_SIZE-1;}
//  return val;
//}
//
//void Task::rpush(cell val)
//{
//  ++rp_;
//  if (rp_ >= F18A_RETURN_STACK_SIZE) {rp_ = 0;}
//  p_stack_[rp_] = val;
//}
//
//cell Task::rpop()
//{
//  cell val = p_stack_[rp_];
//  if (rp_ ) {--rp_;}
//  else {rp_ = F18A_RETURN_STACK_SIZE-1;}
//  return val;
//}
//
//void Task::startup_push(cell val)
//{
//  ++startup_sp_;
//  if (startup_sp_ >= F18A_PARAMETER_STACK_SIZE) {startup_sp_ = 0;}
//  startup_p_stack_[startup_sp_] = val;
//}
//
//cell Task::startup_pop()
//{
//  cell val = startup_p_stack_[startup_sp_];
//  if (startup_sp_ ) {--startup_sp_;}
//  else {startup_sp_ = F18A_PARAMETER_STACK_SIZE-1;}
//  return val;
//}
//
//void Task::startup_rpush(cell val)
//{
//  ++startup_rp_;
//  if (startup_rp_ >= F18A_RETURN_STACK_SIZE) {startup_rp_ = 0;}
//  startup_p_stack_[startup_rp_] = val;
//}
//
//cell Task::startup_rpop()
//{
//  cell val = startup_p_stack_[startup_rp_];
//  if (startup_rp_ ) {--startup_rp_;}
//  else {startup_rp_ = F18A_RETURN_STACK_SIZE-1;}
//  return val;
//}
//
//bool Task::writeable() const
//{
//  if (startup_reg_.named_.writeable()) {return true;}
//  return false;
//}
//
//void Task::read(xml::Iterator& it)
//{
//	if ((it.current() == xml::Element::tag) && (it.tag() == "regs")) {
//	  it.next_tag();
//    startup_reg_.named_.read(it);
//  	if ((it.current() == xml::Element::end) && (it.tag() == "regs")) {
//      it.next_tag();
//    }
//  };
//	if ((it.current() == xml::Element::tag) && (it.tag() == "pstack")) {
//    if ((it.next() == xml::Element::attribute) && (it.key() == "ind")) {
//      try {
//        startup_sp_ = boost::lexical_cast<int>(it.value());
//      }
//      catch (...) {}
//	    it.next();
//    }
//  	if (it.current() == xml::Element::content) {
//      std::vector<long> data;
//      it.read(data);
//      size_t limit = data.size();
//      for (size_t i=0; i<limit; ++i) {
//        startup_p_stack_[i] = data[i];
//      }
//  	  it.next_tag();
//    }
//  	if ((it.current() == xml::Element::end) && (it.tag() == "pstack")) {
//      it.next_tag();
//    }
//  }
//	if ((it.current() == xml::Element::tag) && (it.tag() == "rstack")) {
//    if ((it.next() == xml::Element::attribute) && (it.key() == "ind")) {
//      try {
//        startup_rp_ = boost::lexical_cast<int>(it.value());
//      }
//      catch (...) {}
//	    it.next();
//    }
//  	if (it.current() == xml::Element::content) {
//      std::vector<long> data;
//      it.read(data);
//      size_t limit = data.size();
//      for (size_t i=0; i<limit; ++i) {
//        startup_r_stack_[i] = data[i];
//      }
//  	  it.next_tag();
//    }
//  	if ((it.current() == xml::Element::end) && (it.tag() == "rstack")) {
//      it.next_tag();
//    }
//  }
//}
//
//void Task::write(xml::Writer& w)
//{
//  size_t limit;
//  if (startup_reg_.named_.writeable()) {
//    w.newline();
//    w.open("regs", false, true);
//    w.indent(1);
//    startup_reg_.named_.write(w);
//    w.newline(-1);
//    w.open("regs", true, true);
//  }
//  {
//    limit = F18A_PARAMETER_STACK_SIZE;
//    while ((limit > 0) && (startup_p_stack_[limit-1] == F18_INVALID_REGISTER_VALUE)) {--limit;}
//    if (limit > 0) {
//      std::vector<long> data;
//      data.reserve(limit);
//      for (size_t i=0; i<limit; ++i) {
//        data.push_back(startup_p_stack_[i]);
//      }
//      w.newline();
//      w.open("pstack", false);
//      w.attribute("ind", boost::lexical_cast<std::string>(startup_sp_));
//      w.close(false);
//      w.write(data);
//      w.open("pstack", true, true);
//    }
//  }
//  {
//    limit = F18A_RETURN_STACK_SIZE;
//    while ((limit > 0) && (startup_r_stack_[limit-1] == F18_INVALID_REGISTER_VALUE)) {--limit;}
//    if (limit > 0) {
//      std::vector<long> data;
//      data.reserve(limit);
//      for (size_t i=0; i<limit; ++i) {
//        data.push_back(startup_p_stack_[i]);
//      }
//      w.newline();
//      w.open("rstack", false);
//      w.attribute("ind", boost::lexical_cast<std::string>(startup_rp_));
//      w.close(false);
//      w.write(data);
//      w.open("rstack", true, true);
//    }
//  }
//}

}
