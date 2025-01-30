#include <evb.hpp>
#include <connection.hpp>

#include <gtkmm.h>

#define READ_IIMEOUT  2000

//#define CHECK_ACTION

namespace evb {

NodeAccess::NodeAccess()
: umbilical_(0)
, kraken_(0)
, tentacle_(0)
, segment_(0)
, node_(0)
{
}

NodeAccess::NodeAccess(Umbilical_ptr u, Kraken_ptr k, Tentacle_ptr t, size_t s, size_t n)
: umbilical_(u)
, kraken_(k)
, tentacle_(t)
, segment_(s)
, node_(n)
{
}

bool NodeAccess::valid() const
{
  return umbilical_ != 0;
}

Umbilical_ptr NodeAccess::umbilical()
{
  if (umbilical_.is_null()) {
    throw exception("NodeAccess::umbilical invalid");
  }
  return umbilical_;
}

Kraken_ptr NodeAccess::kraken()
{
  if (kraken_.is_null()) {
    throw exception("NodeAccess::kraken invalid");
  }
  return kraken_;
}

Tentacle_ptr NodeAccess::tentacle()
{
  if (tentacle_.is_null()) {
    throw exception("NodeAccess::tentacle invalid");
  }
  return tentacle_;
}

bool NodeAccess::activate()
{
  if (!valid()) {
    return false;
  }
  return false;
}

bool NodeAccess::snd(long data) // operate instruction directly on the active Kraken node
{
  if (umbilical_ != 0) {
    return umbilical_->snd(data);
  }
  return false;
}

bool NodeAccess::rcv(long& data) // operate instruction directly on the active Kraken node
{
  if (umbilical_ != 0) {
    return umbilical_->rcv(data);
  }
  return false;
}

bool NodeAccess::w(size_t out_size, long* out_data)
{
  if (umbilical_ != 0) {
    return umbilical_->w(segment_, out_size, out_data);
  }
  return false;
}

bool NodeAccess::w1(long data)
{
  if (umbilical_ != 0) {
    return umbilical_->w1(segment_, data);
  }
  return false;
}

bool NodeAccess::w2(long data1, long data2)
{
  if (umbilical_ != 0) {
    return umbilical_->w2(segment_, data1, data2);
  }
  return false;
}

bool NodeAccess::wr(size_t out_size, long* out_data, size_t in_size, long* in_data)
{
  if (umbilical_ != 0) {
    return umbilical_->wr(segment_, out_size, out_data, in_size, in_data);
  }
  return false;
}

bool NodeAccess::wr1(long out_data, long& in_data)
{
  if (umbilical_ != 0) {
    return umbilical_->wr1(segment_, out_data, in_data);
  }
  return false;
}

// ======================================================================================

Umbilical::Umbilical(Manager_ptr m, connection::Serial_ptr s, size_t node)
: manager_(m)
, serial_(s)
, node_(node)
, active_kraken_(-1)
, delay_ms_(-1)
, async_seta_(-1)
, kraken_w_(-1)
, kraken_wr_(-1)
, kraken_w1_(-1)
, kraken_w2_(-1)
, kraken_wr1_(-1)
, async_snd_(-1)
, async_rcv_(-1)
, async_main_(-1)
, async_echo_(-1)
{
}

Umbilical::~Umbilical()
{
}

void Umbilical::wait()
{
  size_t delay = delay_ms();
  if (delay > 0) {
    connection::sleep(delay);
  }
}

bool Umbilical::connected() const
{
  if (serial_ != 0) {
    return serial_->connected();
  }
  return false;
}

bool Umbilical::access(size_t node, Kraken_ptr& k, Tentacle_ptr& t, size_t& s)
{
  size_t limit = kraken_list_.size();
  for (size_t i=0; i<limit; ++i) {
    k = kraken_list_[i];
    if ((k != 0) && k->access(node, t, s)) {
      return true;
    }
  }
  return false;
}

bool Umbilical::setup_async(const std::string module_name, bool send_code)
{
  //async_module_name_ = module_name;
  Manager_ptr man = manager();
  Board_ptr board = man->board();

  if (!write_boot_frame(module_name, send_code)) {
    man->add_error("Umbilical::setup_async cannot find configuration '"+module_name+"' in node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  //if (send_code) {
    long out = 0x12345;
    long in;
    if (!serial_->write(out)) {
      man->add_error("Umbilical::setup_async write out failed");
      return false;
    }
    //connection::sleep(1);
    if (!serial_->read_wait(in)) {
      man->add_error("Umbilical::setup_async read in failed");
      return false;
    }
  //}
  if (module_name == KRAKEN_MODULE_NAME) {
    f18a::Processor_ptr proc = board->processor(node_);
    // write boot frame to node 708
    f18a::Module_ptr module = proc->config_by_name(module_name);
    if (module.is_null()) {
      man->add_error("Umbilical::setup_async cannot find configuration '"+module_name+"' in node "+boost::lexical_cast<std::string>(node_));
      return false;
    }
    f18a::cell addr;
    if (!module->find_label("seta", addr)) {
      man->add_error("Umbilical::setup_async cannot find label 'seta'");
      return false;
    }
    async_seta_ = addr;

    if (!module->find_label("kraken-w", addr)) {
      man->add_error("Umbilical::setup_async cannot find label 'kraken-w'");
      return false;
    }
    kraken_w_ = addr;

    if (!module->find_label("kraken-wr", addr)) {
      man->add_error("Umbilical::setup_async cannot find label 'kraken-wr'");
      return false;
    }
    kraken_wr_ = addr;

    if (!module->find_label("kraken-w1", addr)) {
      man->add_error("Umbilical::setup_async cannot find label 'kraken-w1'");
      return false;
    }
    kraken_w1_ = addr;

    if (!module->find_label("kraken-w2", addr)) {
      man->add_error("Umbilical::setup_async cannot find label 'kraken-w2'");
      return false;
    }
    kraken_w2_ = addr;

    if (!module->find_label("kraken-wr1", addr)) {
      man->add_error("Umbilical::setup_async cannot find label 'kraken-wr1'");
      return false;
    }
    kraken_wr1_ = addr;

    if (!module->find_label("ksnd", addr)) {
      man->add_error("Umbilical::setup_async cannot find label 'ksnd'");
      return false;
    }
    async_snd_ = addr;

    if (!module->find_label("krcv", addr)) {
      man->add_error("Umbilical::setup_async cannot find label 'krcv'");
      return false;
    }
    async_rcv_ = addr;

    if (!module->find_label("main", addr)) {
      man->add_error("Umbilical::setup_async cannot find label 'main'");
      return false;
    }
    async_main_ = addr;

    if (!module->find_label("echo", addr)) {
      man->add_error("Umbilical::setup_async cannot find label 'echo'");
      return false;
    }
    async_echo_ = addr;

    //if (send_code) {
      if (!test_async()) {
        man->add_error("Umbilical::setup_async test failed");
        return false;
      //}
    }
  }
  return true;
}

void Umbilical::delay(size_t ms)
{
  delay_ms_ = ms;
}

void Umbilical::reset_delay()
{
  delay_ms_ = -1;
}

size_t Umbilical::delay_ms() const
{
  if (delay_ms_ >= 0) {
    return delay_ms_;
  }
  //return manager_.delay_ms();
  size_t res = 0;
  if (serial_ != 0) {
    res = serial_->delay_ms();
  }
  return res;
}

bool Umbilical::test_async()
{
  Manager_ptr man = manager();
  if (serial_.is_null()) {
    man->add_error("Umbilical::test_async no communication to node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (async_echo_ < 0) {
    man->add_error("Umbilical::test_async label 'echo' in node "+boost::lexical_cast<std::string>(node_)+" is not defined");
    return false;
  }
  long out = 0x12345;
  long in;
  if (!serial_->write(async_echo_)) {
    man->add_error("Umbilical::test_async write echo");
    return false;
  }
  if (!serial_->write(out)) {
    man->add_error("Umbilical::test_async write out failed");
    return false;
  }
  if (!serial_->read_wait(in)) {
    man->add_error("Umbilical::test_async read in failed");
    return false;
  }
  if (out != in) {
    man->add_error("Umbilical::test_async out != in");
    return false;
  }
  out = 0x23456;
  if (!serial_->write(async_echo_)) {
    man->add_error("Umbilical::test_async write echo");
    return false;
  }
  if (!serial_->write(out)) {
    man->add_error("Umbilical::test_async write out failed");
    return false;
  }
  if (!serial_->read_wait(in)) {
    man->add_error("Umbilical::test_async read in failed");
    return false;
  }
  if (out != in) {
    man->add_error("Umbilical::test_async out != in");
    return false;
  }
  return true;
}

bool Umbilical::w1(size_t segment, long data)
{
  Manager_ptr man = manager();
  if (active_kraken_ < 0) {
    man->add_error("Umbilical::w1 no active Kraken in node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (serial_.is_null()) {
    man->add_error("Umbilical::w1 no communication to node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  Board_ptr board = man->board();
  if (kraken_w1_ < 0) {
    man->add_error("Umbilical::activate_kraken label 'kraken_w1' in node "+boost::lexical_cast<std::string>(node_)+" is not defined");
    return false;
  }
  if (!serial_->write(kraken_w1_)) {
    man->add_error("Umbilical::w1 write address failed");
    return false;
  }
  if (!serial_->write(segment)) {
    man->add_error("Umbilical::w1 write segment failed");
    return false;
  }
  if (!serial_->write(data)) {
    man->add_error("Umbilical::w1 write data failed");
    return false;
  }
  size_t delay = delay_ms();
  if (delay > 0) {
    connection::sleep(delay);
  }
  return true;
}

bool Umbilical::w2(size_t segment, long data1, long data2)
{
  Manager_ptr man = manager();
  if (active_kraken_ < 0) {
    man->add_error("Umbilical::w2 no active Kraken in node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (serial_.is_null()) {
    man->add_error("Umbilical::w2 no communication to node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (!serial_->write(kraken_w2_)) {
    man->add_error("Umbilical::w2 write address failed");
    return false;
  }
  if (!serial_->write(segment)) {
    man->add_error("Umbilical::w2 write segment failed");
    return false;
  }
  if (!serial_->write(data1)) {
    man->add_error("Umbilical::w2 write data failed");
    return false;
  }
  if (!serial_->write(data2)) {
    man->add_error("Umbilical::w2 write data failed");
    return false;
  }
  size_t delay = delay_ms();
  if (delay > 0) {
    connection::sleep(delay);
  }
  return true;
}

bool Umbilical::snd(long data)
{
  Manager_ptr man = manager();
  if (serial_.is_null()) {
    man->add_error("Umbilical::snd no communication to node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (!serial_->write(async_snd_)) {
    man->add_error("Umbilical::snd write address failed");
    return false;
  }
  if (!serial_->write(data)) {
    man->add_error("Umbilical::snd write data failed");
    return false;
  }
  return true;
}

bool Umbilical::rcv(long& data)
{
  Manager_ptr man = manager();
  if (serial_.is_null()) {
    man->add_error("Umbilical::rcv no communication to node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (!serial_->write(async_rcv_)) {
    man->add_error("Umbilical::rcv write address failed");
    return false;
  }
  if (!serial_->read_wait(data)) {
    man->add_error("Umbilical::rcv read data failed");
    return false;
  }
  return true;
}

bool Umbilical::w(size_t segment, size_t out_size, long* out_data)
{
  if (out_size == 0) {return true;} // we can always send no data
  Manager_ptr man = manager();
  if (active_kraken_ < 0) {
    man->add_error("Umbilical::w no active Kraken in node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (serial_.is_null()) {
    man->add_error("Umbilical::w no communication to node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (!serial_->write(kraken_w_)) {
    man->add_error("Umbilical::w write address failed");
    return false;
  }
  if (!serial_->write(segment)) {
    man->add_error("Umbilical::w write segment failed");
    return false;
  }
  if (!serial_->write(out_size-1)) {
    man->add_error("Umbilical::w write m-1 failed");
    return false;
  }
  size_t delay = delay_ms();
  if (delay > 0) {
    connection::sleep(delay);
  }
  for (size_t i=0; i<out_size; ++i) {
    if (!serial_->write(out_data[i])) {
      man->add_error("Umbilical::w write data["+boost::lexical_cast<std::string>(i)+"] failed");
      return false;
    }
    if (delay > 0) {
      connection::sleep(delay);
    }
  }
  return true;
}

bool Umbilical::wr(size_t segment, size_t out_size, long* out_data, size_t in_size, long* in_data)
{
  if (out_size == 0) {return true;} // we can always send no data
  if (in_size == 0) {return w(segment, out_size, out_data);} // we can always send no data
  Manager_ptr man = manager();
  if (active_kraken_ < 0) {
    man->add_error("Umbilical::wr no active Kraken in node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (serial_.is_null()) {
    man->add_error("Umbilical::wr no communication to node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (!serial_->write(kraken_wr_)) {
    man->add_error("Umbilical::wr write address failed");
    return false;
  }
  if (!serial_->write(segment)) {
    man->add_error("Umbilical::wr write segment failed");
    return false;
  }
  if (!serial_->write(out_size-1)) {
    man->add_error("Umbilical::wr write m-1 failed");
    return false;
  }
  size_t delay = delay_ms();
  if (delay > 0) {
    connection::sleep(delay);
  }
  for (size_t i=0; i<out_size; ++i) {
    if (!serial_->write(out_data[i])) {
      man->add_error("Umbilical::wr write data["+boost::lexical_cast<std::string>(i)+"] failed");
      return false;
    }
    if (delay > 0) {
      connection::sleep(delay);
    }
  }
  if (!serial_->write(in_size-1)) {
    man->add_error("Umbilical::wr write k-1 failed");
    return false;
  }
  for (size_t i=0; i<in_size; ++i) {
    if (!serial_->read_wait(in_data[i], READ_IIMEOUT)) {
      man->add_error("Umbilical::wr read data["+boost::lexical_cast<std::string>(i)+"] failed");
      return false;
    }
  }
  return true;
}

bool Umbilical::wr1(size_t segment, long out_data, long& in_data)
{
  Manager_ptr man = manager();
  if (active_kraken_ < 0) {
    man->add_error("Umbilical::wr1 no active Kraken in node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (serial_.is_null()) {
    man->add_error("Umbilical::wr1 no communication to node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (!serial_->write(kraken_wr1_)) {
    man->add_error("Umbilical::wr1 write address failed");
    return false;
  }
  if (!serial_->write(segment)) {
    man->add_error("Umbilical::wr1 write segment failed");
    return false;
  }
  if (!serial_->write(out_data)) {
    man->add_error("Umbilical::wr1 write data failed");
    return false;
  }
  size_t delay = delay_ms();
  if (delay > 0) {
    connection::sleep(delay);
  }
  if (!serial_->read_wait(in_data, READ_IIMEOUT)) {
    man->add_error("Umbilical::wr1 read data failed");
    return false;
  }
  if (delay > 0) {
    connection::sleep(delay);
  }
  return true;
}

bool Umbilical::activate_kraken(size_t node, bool send_code)
{
  size_t limit = kraken_list_.size();
  for (size_t i=0; i<limit; ++i) {
    if ((kraken_list_[i] != 0) && (kraken_list_[i]->node() == node)) {
      return activate_kraken(*kraken_list_[i], send_code);
    }
  }
  Manager_ptr man = manager();
  man->add_error("Umbilical::activate_kraken cannot activate Kraken in node "+boost::lexical_cast<std::string>(node));
  return false;
}

void Umbilical::invalidate_active_kraken()
{
  active_kraken_ = -1;
}

bool Umbilical::activate_kraken(Kraken_ptr k, bool send_code)
{
  if ((active_kraken_ >= 0) && (kraken_list_[active_kraken_] == k)) {
    return true;
  }
  Manager_ptr man = manager();
  if (serial_.is_null()) {
    man->add_error("Umbilical::activate_kraken no communication to node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  Board_ptr board = man->board();
  size_t limit = kraken_list_.size();
  for (size_t i=0; i<limit; ++i) {
    if (kraken_list_[i] == k) {
      long port_addr = f18a::Port::to_addr(kraken_list_[i]->port());
      if (send_code) {
        if (async_seta_ < 0) {
          man->add_error("Umbilical::activate_kraken label 'seta' in node "+boost::lexical_cast<std::string>(node_)+" is not defined");
          return false;
        }
        if (!serial_->write(async_seta_)) {man->add_error("Umbilical::activate_kraken failed"); return false;}
        if (!serial_->write(port_addr)) {man->add_error("Umbilical::activate_kraken failed"); return false;}
      }
      Board_ptr board = manager()->board();
      if (active_kraken_ >= 0) {
        board->set_info(kraken_list_[active_kraken_]->node(), ga144::InfoBit::kraken);
      }
      active_kraken_ = i;
      board->set_info(kraken_list_[active_kraken_]->node(), ga144::InfoBit::active_kraken);

      return true;
    }
  }
  return false;
}

bool Umbilical::write_boot_stream(const f18a::cell_list_type& boot_stream)
{
  Manager_ptr man = manager();
  size_t limit = boot_stream.size();
  for (size_t i=0; i<limit; ++i) {
    if (!serial_->write(boot_stream[i])) {
      man->add_error("Umbilical::write_boot_stream failed");
      return false;
    }
    connection::sleep(1);
  }
  return true;
}

bool Umbilical::write_boot_frame(const std::string& module_name, bool send_code)
{
  Manager_ptr man = manager();
  Board_ptr board = man->board();
  f18a::Processor_ptr proc = board->processor(node_);
  if (proc.is_null()) {
    man->add_error("Umbilical::install_kraken invalid node "+boost::lexical_cast<std::string>(node_));
    return false;
  }

  f18a::Module_ptr module = proc->config_by_name(module_name);
  if (module.is_null()) {
    man->add_error("Umbilical::write_boot_frame cannot find configuration '"+module_name+"' in node "+boost::lexical_cast<std::string>(node_));
    return false;
  }

  if (module->reg().named_.P_ == F18_INVALID_VALUE) {
    man->add_error("Umbilical::write_boot_frame no entry point for node "+boost::lexical_cast<std::string>(node_));
    return false;
  }
  if (send_code) {
    size_t start_pos = module->reg().named_.P_;
    if (!serial_->write(start_pos)) {man->add_error("Umbilical::write_boot_frame failed"); return false;}  // start address of node
    if (!serial_->write(0)) {man->add_error("Umbilical::write_boot_frame failed"); return false;}  // address to write
    if (!serial_->write(F18A_RAM_SIZE)) {man->add_error("Umbilical::write_boot_frame failed"); return false;} // size of memory block to write
    for (size_t i=0; i<F18A_RAM_SIZE; ++i) {
      if (!serial_->write(module->ram().data(i))) {man->add_error("Umbilical::write_boot_frame failed"); return false;}
    }
  }
  board->set_info(node_, ga144::InfoBit::umbilical);
  return true;
}

bool Umbilical::test_kraken()
{
  Manager_ptr man = manager();
  long data;
  //if (!test_echo(true)) {return false;}

  if (!snd(man->remote_op(Manager::RemoteOp::read_a))) {
    man->add_error("Umbilical::test_kraken snd failed");
    return false;
  }

  if (!rcv(data)) {
    man->add_error("Umbilical::test_kraken rcv failed");
    return false;
  }
  return true;
}

bool Umbilical::test_tentacle(size_t index)
{
  long data;
  Manager_ptr man = manager();
  if (active_kraken_ >= 0) {
    if (!test_kraken()) {
      man->add_error("Umbilical::test_tentacle Kraken failed");
      return false;
    }
    Tentacle_ptr tentacle = kraken_list_[active_kraken_]->active_tentacle();
    if (index >= tentacle->size()) {
      man->add_error("Umbilical::test_tentacle invalid node index");
      return false;
    }
    if ((tentacle != 0) && (tentacle->size() > 0)) {
      if (!wr1(index, man->remote_op(Manager::RemoteOp::read_a), data)) {
        man->add_error("Umbilical::test_tentacle failed");
        return false;
      } 
    }
  }
  else {
    man->add_error("Umbilical::test_tentacle no active Kraken");
    return false;
  }
  return true;
}

bool Umbilical::test_tentacle_end()
{
  long data;
  Manager_ptr man = manager();
  if (active_kraken_ >= 0) {
    if (!test_kraken()) {
      man->add_error("Umbilical::test_tentacle_end Kraken failed");
      return false;
    }
    Tentacle_ptr tentacle = kraken_list_[active_kraken_]->active_tentacle();
    if ((tentacle != 0) && (tentacle->size() > 0)) {
      if (!wr1(tentacle->size()-1, man->remote_op(Manager::RemoteOp::read_a), data)) {
        man->add_error("Umbilical::test_tentacle_end failed");
        return false;
      } 
    }
  }
  return true;
}

bool Umbilical::install_kraken(size_t node, const std::string& module_name, bool send_code)
{
  Manager_ptr man = manager();
  Board_ptr board = man->board();
  long data;
  f18a::Processor_ptr proc = board->processor(node);
  if (proc.is_null()) {
    man->add_error("Umbilical::install_kraken invalid node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Port::Enum port;
  if (!man->port_to_kraken(node_, node, port)) {
    man->add_error("Umbilical::install_kraken cannot ind port from node "+boost::lexical_cast<std::string>(node_)+" to node "+boost::lexical_cast<std::string>(node));
    return false;
  }

  Kraken_ptr kraken = new Kraken(this, node, port);
  kraken_list_.push_back(kraken);
  //if (!activate_kraken(*kraken)) {
  //  man->add_error("Umbilical::install_kraken cannot activate Kraken in node "+boost::lexical_cast<std::string>(node));
  //  return false;
  //}
  long focus = f18a::State::focusing_jump(port);
  f18a::Module_ptr module = proc->config_by_name(module_name);
  if (module.is_null()) {
    man->add_error("Umbilical::install_kraken cannot find configuration '"+module_name+"' in node "+boost::lexical_cast<std::string>(node));
    return false;
  }

  if (send_code) {
    //if (!first) {
    if (!serial_->write(async_main_)) {man->add_error("Umbilical::install_kraken failed"); return false;}
    //}

    // set A in Kraken node
    if (!serial_->write(f18a::Port::to_addr(port))) {man->add_error("Umbilical::install_kraken failed"); return false;}

    // set # of words - 1 of remaining message
    if (!serial_->write(1+1+2+1+F18A_RAM_SIZE+2  -1)) {man->add_error("Umbilical::install_kraken failed"); return false;}
    // focus Kraken node (1)
    if (!serial_->write(focus)) {man->add_error("Umbilical::install_kraken failed"); return false;}
    // set A to 0 (1)
    if (!serial_->write(man->remote_op(Manager::RemoteOp::set_a_to_0))) {man->add_error("Umbilical::install_kraken failed"); return false;}
    // set count for loop (2)
    if (!serial_->write(man->remote_op(Manager::RemoteOp::write_rstack))) {man->add_error("Umbilical::install_kraken failed"); return false;}
    if (!serial_->write(F18A_RAM_SIZE  -1)) {man->add_error("Umbilical::install_kraken failed"); return false;}
    // move ram data (F18A_RAM_SIZE+1)
    if (!serial_->write(man->remote_op(Manager::RemoteOp::write_mem_loop))) {man->add_error("Umbilical::install_kraken failed"); return false;}
    for (size_t i=0; i<F18A_RAM_SIZE; ++i) {
      data = module->ram().data(i);
      if (data == F18_INVALID_VALUE) {
        data = F18A_INITIAL_CELL;
      }
      if (!serial_->write(data)) {man->add_error("Umbilical::install_kraken failed"); return false;}
    }
    // set A to port (2)
    size_t port_addr = f18a::Port::to_addr(port);
    if (!serial_->write(man->remote_op(Manager::RemoteOp::write_a))) {man->add_error("Umbilical::install_kraken failed"); return false;}
    if (!serial_->write(port_addr)) {man->add_error("Umbilical::install_kraken failed"); return false;}
#ifdef CHECK_ACTION
    if (!test_kraken()) {return false;}
#endif
  }
  if (active_kraken_ >= 0) {
    board->set_info(kraken_list_[active_kraken_]->node(), ga144::InfoBit::kraken);
  }
  board->set_info(node, ga144::InfoBit::active_kraken);
  active_kraken_ = kraken_list_.size()-1;
  return true;
}

bool Umbilical::add_tentacle_to(size_t to_node, bool send_code)
{
  if (active_kraken_ >= 0) {
    return kraken_list_[active_kraken_]->add_tentacle_to(to_node, send_code);
  }
  return false;
}

bool Umbilical::reduce_tentacle_to(size_t to_node)
{
  if (active_kraken_ >= 0) {
    return kraken_list_[active_kraken_]->reduce_tentacle_to(to_node);
  }
  return false;
}

bool Umbilical::destroy_tentacle()
{
  if (active_kraken_ >= 0) {
    return kraken_list_[active_kraken_]->destroy_tentacle();
  }
  return false;
}

bool Umbilical::remove_all_tentacles()
{
  if (active_kraken_ >= 0) {
    return kraken_list_[active_kraken_]->remove_all_tentacles();
  }
  return false;
}

//bool Umbilical::reset_target()
//{
//  size_t segment;
//  if (!activate_node(500, segment)) {return false;}
//  long data[4];
//  data[0] = ops_[RemoteOp::write_a];
//  data[1] = F18A_IO;
//  data[2] = ops_[RemoteOp::store_a];
//  data[3] = F18A_PIN17_LO;
//  if (!w(segment, 4, data)) {return false;}
//  connection::sleep(50);
//  if (!w2(segment, ops_[RemoteOp::store_a], F18A_PIN17_HI)) {return false;}
//  connection::sleep(20);
//  return true;
//}
// ======================================================================================

Kraken::Kraken(Umbilical_ptr u, size_t node, f18a::Port::Enum port)
: umbilical_(u)
, port_(port)
, node_(node)
, active_tentacle_(-1)
, delay_ms_(-1)
{
}

Kraken::~Kraken()
{
}

Manager_ptr Kraken::manager() {return umbilical_->manager();}
const Manager_ptr Kraken::manager() const {return umbilical_->manager();}

bool Kraken::access(size_t node, Tentacle_ptr& t, size_t& s)
{
  size_t limit = tentacles_.size();
  for (size_t i=0; i<limit; ++i) {
    t = tentacles_[i];
    if ((t != 0) && t->access(node, s)) {
      return true; 
    }
  }
  return false;
}

Tentacle_ptr Kraken::active_tentacle()
{
  if (active_tentacle_ >= 0) {
    return tentacles_[active_tentacle_];
  }
  return Tentacle_ptr();
}

void Kraken::delay(size_t ms)
{
  delay_ms_ = ms;
}

void Kraken::reset_delay()
{
  delay_ms_ = -1;
}

size_t Kraken::delay_ms() const
{
  if (delay_ms_ >= 0) {
    return delay_ms_;
  }
  return umbilical_->delay_ms();
}

bool Kraken::activate()
{
  return umbilical_->activate_kraken(this, true);
}

bool Kraken::activate_tentacle(Tentacle_ptr act_t)
{
  size_t limit = tentacles_.size();
  for (size_t i=0; i<limit; ++i) {
    Tentacle_ptr t = tentacles_[i];
    if (t == act_t) {
      if (i != active_tentacle_) {
        Manager_ptr man = manager();
        Board_ptr board = man->board();

        size_t node = node_;

        ga144::Chip_ptr chip = board->split_node(node);
        if (chip.is_null()) {
          man->add_error("Kraken::activate_tentacle invalid node");
          return false;
        }

        f18a::Port::Enum port;
        if (!chip->find_port(node_, act_t->front(), port)) {
          man->add_error("Kraken::activate_tentacle no port");
          return false;
        }
        size_t port_addr = f18a::Port::to_addr(port);
        if (!umbilical()->snd(man->remote_op(Manager::RemoteOp::write_b))) {
          man->add_error("Kraken::activate_tentacle write command failed");
          return false;
        }
        if (!umbilical()->snd(port_addr)) {
          man->add_error("Kraken::activate_tentacle write port failed");
          return false;
        }
        active_tentacle_ = i;
      }
      return true; 
    }
  }
  return false;
}

bool Kraken::add_tentacle_to(size_t to_node, bool send_code)
{
  if (tentacles_.empty()) {
    Tentacle_ptr t = new Tentacle(this);
    tentacles_.push_back(t);
    active_tentacle_ = 0;
  }
  if (active_tentacle_ >= 0) {
    return tentacles_[active_tentacle_]->add_to(to_node, send_code);
  }
  return false;
}

bool Kraken::reduce_tentacle_to(size_t to_node)
{
  if (active_tentacle_ >= 0) {
    return tentacles_[active_tentacle_]->reduce_to(to_node);
  }
  return false;
}

bool Kraken::destroy_tentacle()
{
  if (active_tentacle_ >= 0) {
    return tentacles_[active_tentacle_]->destroy();
  }
  return false;
}

bool Kraken::remove_all_tentacles()
{
  active_tentacle_ = -1;
  size_t limit = tentacles_.size();
  for (size_t i=0; i<limit; ++i) {
    Tentacle_ptr t = tentacles_[i];
    if (t != 0) {
      if (!t->activate()) {
        return false;
      }
      if (!t->destroy()) {
        return false;
      }
      tentacles_[i].reset();
    }
  }
  tentacles_.clear();
  return true;
}

// ======================================================================================


Tentacle::Tentacle(Kraken_ptr k)
: kraken_(k)
, delay_ms_(-1)
{
}

Tentacle::~Tentacle()
{
}

Manager_ptr Tentacle::manager() {return kraken_->manager();}
const Manager_ptr Tentacle::manager() const {return kraken_->manager();}

Umbilical_ptr Tentacle::umbilical() {return kraken_->umbilical();}
const Umbilical_ptr Tentacle::umbilical() const {return kraken_->umbilical();}

void Tentacle::delay(size_t ms)
{
  delay_ms_ = ms;
}

void Tentacle::reset_delay()
{
  delay_ms_ = -1;
}

size_t Tentacle::delay_ms() const
{
  if (delay_ms_ >= 0) {
    return delay_ms_;
  }
  return kraken_->delay_ms();
}

void Tentacle::push_back(size_t node)
{
  path_.push_back(node);
}

bool Tentacle::node_to_tentacle_index(size_t node, size_t& tentacle_index) const
{
  size_t limit = size();
  for (size_t i=0; i<limit; ++i) {
    if (path_[i] == node) {
      tentacle_index = i;
      return true;
    }
  }
  return false;
}

bool Tentacle::has_node(size_t node) const
{
  size_t limit = size();
  for (size_t i=0; i<limit; ++i) {
    if (path_[i] == node) {
      return true;
    }
  }
  return false;
}

void Tentacle::reset()
{
  path_.clear();
}

bool Tentacle::activate()
{
  if (!kraken_->activate()) {
    return false;
  }
  return kraken_->activate_tentacle(this);
}

bool Tentacle::access(size_t node, size_t& segment)
{
  size_t limit = size();
  for (size_t i=0; i<limit; ++i) {
    if (path_[i] == node) {
      segment = i;
      return true;
    }
  }
  return false;
}

bool Tentacle::destroy()
{
  while (size() > 0) {
    if (!remove_last_segment()) {
      return false;
    }
  }
  return true;
}

bool Tentacle::remove_last_segment()
{
  if (size() > 0) {
    Manager_ptr man = manager();
    Board_ptr board = man->board();
    if (!activate()) {
      man->add_error("Tentacle::remove_last_segment cannot activate tentacle");
      return false;
    }
#ifdef CHECK_ACTION
    if (!umbilical().test_kraken()) {
      man->add_error("Tentacle::remove_last_segment no Kraken available");
      return false;
    }
#endif
    if (empty()) {
      man->add_error("Tentacle::remove_last_segment tentacle is empty");
      return false;
    }
    size_t segment = size()-1;
    size_t node = back();

    ga144::Chip_ptr chip = board->split_node(node);
    if (chip.is_null()) {
      man->add_error("Tentacle::remove_last_segment invalid end segment");
      return false;
    }
    size_t ind = chip->index(node);
    f18a::Processor_ptr proc = chip->processor(ind);
    long adr = proc->warm();
    long warm = f18a::State::addr_to_call(adr);
    if (!umbilical()->w1(segment, warm)) {
      man->add_error("Tentacle::remove_last_segment failed");
      return false;
    }
    pop_back();
    proc->reset_tentacle_index();
    chip->set_info(ind, ga144::InfoBit::warm);
    proc->changed();
    return true;
  }
  return false;
}

bool Tentacle::add_to(size_t to_node, bool send_code)
{
  Manager_ptr man = manager();
  Board_ptr board = man->board();
  if (!activate()) {
    man->add_error("Tentacle::add_to cannot activate tentacle");
    return false;
  }
#ifdef CHECK_ACTION
  if (!umbilical().test_tentacle_end()) {
    man->add_error("Tentacle::add_to end segment failed on node "+boost::lexical_cast<std::string>(back()));
    return false;
  }
#endif
  size_t src, src_ind, dest_ind, cnt;
  if (empty()) {
    // first segment
    src = kraken()->node();;
  }
  else {
    src = back();
  }
  ga144::Chip_ptr src_chip = board->split_node(src);
  if (src_chip.is_null()) {
    man->add_error("Tentacle::add_to invalid source node");
    return false;
  }
  size_t dest = to_node;
  ga144::Chip_ptr dest_chip = board->split_node(dest);
  if (dest_chip.is_null()) {
    man->add_error("Tentacle::add_to invalid destination chip");
    return false;
  }
  if (src_chip != dest_chip) {
    man->add_error("Tentacle::add_to source and destination nodes are on different chips");
    return false;
  }
  src_ind = src_chip->index(src);
  dest_ind = dest_chip->index(dest);
  f18a::Processor_ptr src_proc = src_chip->processor(src_ind);
  f18a::Processor_ptr dest_proc = dest_chip->processor(dest_ind);
  if (src_proc == dest_proc) {
    // tentacle is already there
    return true;
  }
  ga144::Neighbour::Enum dir;
  if (src_proc->row() == dest_proc->row()) {
    // horizontal movement
    if (src_proc->column() < dest_proc->column()) {
      // move to east
      cnt = dest_proc->column() - src_proc->column();
      dir = ga144::Neighbour::east;
    }
    else { // move to west
      cnt = src_proc->column() - dest_proc->column();
      dir = ga144::Neighbour::west;
    }
  }
  else if (src_proc->column() == dest_proc->column()) {
    // vertical movement
    if (src_proc->row() < dest_proc->row()) {
      // move to north
      cnt = dest_proc->row() - src_proc->row();
      dir = ga144::Neighbour::north;
    }
    else { // move to south
      cnt = src_proc->row() - dest_proc->row();
      dir = ga144::Neighbour::south;
    }
  }
  else {
    man->add_error("Tentacle::add_to source ("+boost::lexical_cast<std::string>(src)+") and destination ("+boost::lexical_cast<std::string>(dest)+") must lie on same row or column");
    return false;
  }
  while (cnt > 0) {
    if (send_code) {
      if (!add_segment_code(dir)) {
        man->add_error("Tentacle::add_to path from source ("+boost::lexical_cast<std::string>(src)+") to destination ("+boost::lexical_cast<std::string>(dest)+") failed");
        return false;
      }
    }
    if (!add_segment_data(dir)) {
      man->add_error("Tentacle::add_to path from source ("+boost::lexical_cast<std::string>(src)+") to destination ("+boost::lexical_cast<std::string>(dest)+") is blocked");
      return false;
    }
    --cnt;
  }
  return true;
}

bool Tentacle::reduce_to(size_t to_node)
{
  if (!has_node(to_node)) {
    return false;
  }
  while (back() != to_node) {
    if (!remove_last_segment()) {
      return false;
    }
  }
  if (!remove_last_segment()) {
    return false;
  }
  return true;
}

bool Tentacle::add_segment_code(ga144::Neighbour::Enum dir)
{
  Manager_ptr man = manager();
  Board_ptr board = man->board();
  if (!activate()) {
    man->add_error("Tentacle::add_segment_code cannot activate tentacle");
    return false;
  }
#ifdef CHECK_ACTION
  if (!umbilical().test_tentacle_end()) {
    man->add_error("Tentacle::add_segment_code end segment failed on node "+boost::lexical_cast<std::string>(back()));
    return false;
  }
#endif
  size_t tentacle_length = size();
  size_t src, dest, src_ind, dest_ind;
  if (empty()) {
    // first segment
    src = kraken()->node();;
  }
  else {
    src = back();
  }
  ga144::Chip_ptr chip = board->split_node(src);
  if (chip.is_null()) {
    man->add_error("Tentacle::add_segment_code invalid end segment");
    return false;
  }
  src_ind = chip->index(src);
  if (!chip->neighbour(src_ind, dir, dest_ind)) {
    man->add_error("Tentacle::add_segment_code invalid neighbour");
    return false;
  }
  if (!chip->can_be_tentacle_segment(dest_ind)) {
    man->add_error("Tentacle::add_segment_code node cannot be tentacle segment");
    return false;
  }
  dest = chip->node(dest_ind);
  // install new tentacle segment
  f18a::Port::Enum port;
  if (!chip->neighbour_to_port(src_ind, dir, port)) {
    man->add_error("Tentacle::add_segment_code no port");
    return false;
  }
  size_t port_addr = f18a::Port::to_addr(port);
  long focus = f18a::State::focusing_jump(port);
  // set B to next segment
  if (tentacle_length == 0) {
    if (!umbilical()->snd(man->remote_op(Manager::RemoteOp::write_b))) {return false;}
    if (!umbilical()->snd(port_addr)) {return false;}
    if (!umbilical()->w1(tentacle_length, focus)) {return false;}
  }
  else {
    long data[4];
    data[0] = man->remote_op(Manager::RemoteOp::write_b);
    data[1] = port_addr;
    data[2] = man->remote_op(Manager::RemoteOp::store_b);
    data[3] = focus;
    if (!umbilical()->w(tentacle_length-1, 4, data)) {return false;}
  }
  return true;
}

bool Tentacle::add_segment_data(ga144::Neighbour::Enum dir)
{
  Manager_ptr man = manager();
  Board_ptr board = man->board();
  size_t tentacle_length = size();
  size_t src, dest, src_ind, dest_ind;
  if (empty()) {
    // first segment
    src = kraken()->node();;
  }
  else {
    src = back();
  }
  ga144::Chip_ptr chip = board->split_node(src);
  if (chip.is_null()) {
    man->add_error("Tentacle::add_segment_data invalid end segment");
    return false;
  }
  src_ind = chip->index(src);
  if (!chip->neighbour(src_ind, dir, dest_ind)) {
    man->add_error("Tentacle::add_segment_data invalid neighbour");
    return false;
  }
  if (!chip->can_be_tentacle_segment(dest_ind)) {
    man->add_error("Tentacle::add_segment_data node cannot be tentacle segment");
    return false;
  }
  dest = chip->node(dest_ind);
  f18a::Port::Enum port;
  if (!chip->neighbour_to_port(src_ind, dir, port)) {
    man->add_error("Tentacle::add_segment_data no port");
    return false;
  }
  size_t port_addr = f18a::Port::to_addr(port);

  chip->processor(src_ind)->set_b(port_addr);
  dest = board->join_node(chip, dest);
  push_back(dest);
  chip->processor(dest_ind)->tentacle_index(tentacle_length);
  chip->set_info(dest_ind, ga144::InfoBit::tentacle);
  return true;
}

bool Tentacle::test(size_t index)
{
  Manager_ptr man = manager();
  if (!activate()) {
    man->add_error("Tentacle::test cannot activate tentacle");
    return false;
  }
  if (!umbilical()->test_tentacle(index)) {
    man->add_error("Tentacle::test tentacle end test failed for segment " +boost::lexical_cast<std::string>(size()));
    return false;
  }
  return true;
}

bool Tentacle::add_segment(ga144::Neighbour::Enum dir, bool send_code)
{
  //long tb = GetTickCount();

  Manager_ptr man = manager();
  Board_ptr board = man->board();
  if (send_code) {
    if (!activate()) {
      man->add_error("Tentacle::add_segment cannot activate tentacle");
      return false;
    }
#ifdef CHECK_ACTION
    if (!umbilical().test_tentacle_end()) {
      man->add_error("Tentacle::add_segment end segment failed on node "+boost::lexical_cast<std::string>(back()));
      return false;
    }
#endif
  }
  size_t tentacle_length = size();
  size_t src, dest, src_ind, dest_ind;
  if (empty()) {
    // first segment
    src = kraken()->node();;
  }
  else {
    src = back();
  }
  ga144::Chip_ptr chip = board->split_node(src);
  if (chip.is_null()) {
    man->add_error("Tentacle::add_segment invalid end segment");
    return false;
  }
  src_ind = chip->index(src);
  if (!chip->neighbour(src_ind, dir, dest_ind)) {
    man->add_error("Tentacle::add_segment invalid neighbour");
    return false;
  }
  if (!chip->can_be_tentacle_segment(dest_ind)) {
    man->add_error("Tentacle::add_segment node cannot be tentacle segment");
    return false;
  }
  dest = chip->node(dest_ind);
  // install new tentacle segment
  f18a::Port::Enum port;
  if (!chip->neighbour_to_port(src_ind, dir, port)) {
    man->add_error("Tentacle::add_segment no port");
    return false;
  }
  size_t port_addr = f18a::Port::to_addr(port);

  //long t1 = GetTickCount() - tb;

  if (send_code) {
    long focus = f18a::State::focusing_jump(port);
    // set B to next segment
    if (tentacle_length == 0) {
      //if (!snd(focus)) {return false;}
      if (!umbilical()->snd(man->remote_op(Manager::RemoteOp::write_b))) {return false;}
      if (!umbilical()->snd(port_addr)) {return false;}
      if (!umbilical()->w1(tentacle_length, focus)) {return false;}
      //umbilical_->write(kraken_write_);
    }
    else {
      long data[4];
      data[0] = man->remote_op(Manager::RemoteOp::write_b);
      data[1] = port_addr;
      data[2] = man->remote_op(Manager::RemoteOp::store_b);
      data[3] = focus;
      if (!umbilical()->w(tentacle_length-1, 4, data)) {return false;}
      //if (!w2(tentacle_length-1, ops_[RemoteOp::write_b], port_addr)) {return false;}
      //if (!w1(tentacle_length, focus)) {return false;}

    }
  }
  //long t2 = GetTickCount() - tb;

  //
  chip->processor(src_ind)->set_b(port_addr);
  dest = board->join_node(chip, dest);
  push_back(dest);
  chip->processor(dest_ind)->tentacle_index(tentacle_length);
  chip->set_info(dest_ind, ga144::InfoBit::tentacle);
  //

  //long t3 = GetTickCount() - tb;

  if (send_code) {
#ifdef CHECK_ACTION
    if (!umbilical().test_tentacle_end()) {
      man->add_error("Tentacle::add_segment tentacle end test failed for segment " +boost::lexical_cast<std::string>(tentacle_length));
      return false;
    }
#endif
  }
  //long t4 = GetTickCount() - tb;

  return true;
}

// ======================================================================================



// ======================================================================================

Manager::Manager(Board_ptr b/*, connection::Serial& u*/)
: board_(b)
, thread_(0)
, is_active_(true)
, is_running_(false)
{
  umbilical_list_.resize(Port::limit);

  thread_ = new boost::thread(sigc::mem_fun(*this, &Manager::run_thread));
}

Manager::~Manager()
{
  shutdown();
}

f18a::cell Manager::remote_op(RemoteOp::Enum op)
{
  static f18a::cell* ops = 0;
  if (ops == 0) {
    ops = new f18a::cell[RemoteOp::limit];

    ops[RemoteOp::write_a] = f18a::State::assemble1("@p a!");
    ops[RemoteOp::write_b] = f18a::State::assemble1("@p b!");
    ops[RemoteOp::write_mem] = f18a::State::assemble1("@p !+");
    ops[RemoteOp::write_mem_loop] = f18a::State::assemble1("begin @p !+ unext");
    ops[RemoteOp::write_mem_loop_b] = f18a::State::assemble1("begin @p !b unext");
    ops[RemoteOp::write_stack] = f18a::State::assemble1("@p");
    ops[RemoteOp::write_rstack] = f18a::State::assemble1("@p >r");
    ops[RemoteOp::write_stack4] = f18a::State::assemble1("@p @p @p @p");
    ops[RemoteOp::write_stack3] = f18a::State::assemble1("@p @p @p");
    ops[RemoteOp::write_stack2] = f18a::State::assemble1("@p @p");

    ops[RemoteOp::read_a] = f18a::State::assemble1("a !p");
    ops[RemoteOp::read_mem] = f18a::State::assemble1("@+ !p");
    ops[RemoteOp::read_mem_loop] = f18a::State::assemble1("begin @+ !p unext");
    ops[RemoteOp::read_stack] = f18a::State::assemble1("!p");
    ops[RemoteOp::read_rstack] = f18a::State::assemble1("r> !p");
    ops[RemoteOp::read_stack4] = f18a::State::assemble1("!p !p !p !p");
    ops[RemoteOp::read_stack3] = f18a::State::assemble1("!p !p !p");
    ops[RemoteOp::read_stack2] = f18a::State::assemble1("!p !p");
    ops[RemoteOp::read_rstack2] = f18a::State::assemble1("r> !p r> !p");

    ops[RemoteOp::set_a_to_0] = f18a::State::assemble1("dup xor a!");
    ops[RemoteOp::push_rstack3] = f18a::State::assemble1(">r >r >r");
    ops[RemoteOp::store_a] = f18a::State::assemble1("@p !");
    ops[RemoteOp::store_b] = f18a::State::assemble1("@p !b");
    ops[RemoteOp::fetch_a] = f18a::State::assemble1("@ !p");
    ops[RemoteOp::fetch_b] = f18a::State::assemble1("@b !p");

  }
  return ops[op];
}

void Manager::error()
{
  board_->error();
}

void Manager::success()
{
  board_->success();
}

void Manager::shutdown()
{
  if (thread_) {
    is_active_ = false;
    while (is_running_) {
      cond_.notify_one();
      connection::sleep(10);
    }
    thread_ = 0;
  }
}

void Manager::run_thread()
{
  is_running_ = true;
  while (is_active_) {
    try {
      Action action;
      {
        boost::mutex::scoped_lock lock(mutex_);
        if (action_queue_.empty()) {
          boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
          time += boost::posix_time::milliseconds(100);
          cond_.timed_wait(lock, time);
        }
        if (!action_queue_.empty()) {
          action = action_queue_.front();
          action_queue_.pop();
        }
      }
      if (is_active_) {
        action.action(*this);
      }
    }
    catch (...) {
    }
  }
  is_running_ = false;
}

void Manager::append_action(const Action& a, bool with_delimiter)
{
  {
    boost::mutex::scoped_lock lock(mutex_);
    action_queue_.push(a);
    if (with_delimiter) {
      action_queue_.push(Action(Action::delimiter));
    }
  }
  cond_.notify_one();
}

void Manager::cancel_current_action()
{
  boost::mutex::scoped_lock lock(mutex_);
  if (!action_queue_.empty()) {
    action_queue_.pop();
  }
}

void Manager::cancel_action_group()
{
  boost::mutex::scoped_lock lock(mutex_);
  while (!action_queue_.empty() && (action_queue_.front().op() != Action::delimiter)) {
    action_queue_.pop();
  }
  if (!action_queue_.empty()) { // remove delimiter action
    action_queue_.pop();
  }
}

void Manager::cancel_all_actions()
{
  boost::mutex::scoped_lock lock(mutex_);
  while (!action_queue_.empty()) {
    action_queue_.pop();
  }
}

void Manager::do_expand_tentacle_to(size_t node)
{
  append_action(Action(Action::Enum::expand_tentacle_to, node), false);
}

void Manager::do_reduce_tentacle_to(size_t node)
{
  append_action(Action(Action::Enum::reduce_tentacle_to, node), false);
}

void Manager::do_reset()
{
  append_action(Action(Action::Enum::reset), false);
}

void Manager::do_boot(BootMethod::Enum bm)
{
  append_action(Action(Action::Enum::boot, bm), false);
}

void Manager::do_span_host()
{
  append_action(Action(Action::Enum::span_host), false);
}

void Manager::do_span_target()
{
  append_action(Action(Action::Enum::span_target), false);
}

void Manager::do_span_host_target()
{
  append_action(Action(Action::Enum::span_host_target), false);
}

void Manager::do_remove_all_tentacles()
{
  append_action(Action(Action::Enum::remove_all_tentacles), false);
}


void Manager::do_compile_module(Module_ptr mod)
{
  append_action(Action(Action::Enum::compile_module, mod), false);
}

void Manager::do_write_module(Module_ptr mod)
{
  append_action(Action(Action::Enum::write_module, mod), false);
}

void Manager::do_start_module(Module_ptr mod)
{
  append_action(Action(Action::Enum::start_module, mod), false);
}

bool Manager::reset()
{
  event_list_.clear();
  size_t limit = umbilical_list_.size();
  for (size_t i=0; i<limit; ++i) {
    umbilical_list_[i].reset();
  }
  board_->reset();

  if (!board_->connected(Port::A)) {
    event_list_.push_back("Manager::reset port A is not connected");
    return false;
  }
  if (!board_->assert_connection(Port::A)->reset_evb()) {
    event_list_.push_back("Manager::reset reset failed");
    return false;
  }
  connection::sleep(5);
  return true;
}

bool Manager::boot_async(size_t node, const std::string& module_name, bool send_code)
{
  if (send_code) {
    if (!reset()) {return false;}
    umbilical_list_[Port::A] = new Umbilical(this, board_->assert_connection(Port::A), node);
  }
  if (!umbilical_list_[Port::A]->setup_async(module_name, send_code)) {
    event_list_.push_back("Manager::boot_async umbilical setup failed");
    return false;
  }
  return true;
}

bool Manager::boot_async_708_kraken(const std::string& module_name, bool kraken_in_707, bool kraken_in_709, bool kraken_in_608, bool send_code)
{
  if (!boot_async(708, module_name, send_code)) {
    event_list_.push_back("Manager::boot_async_708_kraken boot async failed");
    return false;
  }
  //bool first = true;
  if (kraken_in_707) {
    if (!install_kraken(707, module_name, send_code)) {
      event_list_.push_back("Manager::boot_async_708_host install Kraken n 707 failed");
      return false;
    }
    //first = false;
  }
  if (kraken_in_709) {
    if (!install_kraken(709, module_name, send_code)) {
      event_list_.push_back("Manager::boot_async_708_host install Kraken n 709 failed");
      return false;
    }
    //first = false;
  }
  if (kraken_in_608) {
    if (!install_kraken(608, module_name, send_code)) {
      event_list_.push_back("Manager::boot_async_708_host install Kraken n 608 failed");
      return false;
    }
    //first = false;
  }
  return true;
}

bool Manager::boot_async_708_host(const std::string& module_name, bool send_code)
{
  if (!boot_async_708_kraken(module_name, true, false, false, send_code)) {
    event_list_.push_back("Manager::boot_async_708_host boot async failed");
    return false;
  }
  if (!span_host(true)) {
    event_list_.push_back("Manager::boot_async_708_host span host failed");
    return false;
  }
  return true;
}

bool Manager::boot_async_708_host_target(const std::string& module_name, bool send_code)
{
  if (!boot_async_708_kraken(module_name, true, true, false, send_code)) {
    event_list_.push_back("Manager::boot_async_708_host_target boot async failed");
    return false;
  }
  if (!span_host_target(true)) {
    event_list_.push_back("Manager::boot_async_708_host span host & target failed");
    return false;
  }
  return true;
}


bool Manager::boot_fast_708_host(const std::string& package_name)
{
  boot_708(package_name);

  //Package_ptr pack;
  //f18a::cell_list_type empty, boot_chip, boot_708;
  //board_->create_boot_frame(Board::BootFramePath::via_708, Board::BootCodeType::chain_via_b, 0, 0, pack, empty, boot_chip);

  //if (!boot_async_708_kraken(module_name, true, false, false)) {
  //  event_list_.push_back("Manager::boot_async_708_host boot async failed");
  //  return false;
  //}
  //if (!span_host()) {
  //  event_list_.push_back("Manager::boot_async_708_host span host failed");
  //  return false;
  //}
  return true;
}

bool Manager::boot_fast_708_host_target(const std::string& package_name)
{
  boot_708_both(package_name);
  //if (!boot_async_708_kraken(module_name, true, true, false)) {
  //  event_list_.push_back("Manager::boot_async_708_host_target boot async failed");
  //  return false;
  //}
  //if (!span_host_target()) {
  //  event_list_.push_back("Manager::boot_async_708_host span host & target failed");
  //  return false;
  //}
  return true;
}


bool Manager::port_to_kraken(size_t umbilical_node, size_t kraken_node, f18a::Port::Enum& port)
{
  switch (umbilical_node) {
  case 708:
    switch (kraken_node) {
    case 707: port = f18a::Port::left; break;
    case 709: port = f18a::Port::right; break;
    case 608: port = f18a::Port::down; break;
    default:
      event_list_.push_back("Manager::install_kraken invalid kraken node "+boost::lexical_cast<std::string>(kraken_node));
      return false;
    }
    break;

  default:
    event_list_.push_back("Manager::port_to_kraken invalid umbilical node "+boost::lexical_cast<std::string>(umbilical_node));
    return false;
  }
  return true;
}


bool Manager::install_kraken(size_t node, const std::string& module_name, bool send_code)
{
  if (umbilical_list_[Port::A].is_null()) {
    event_list_.push_back("Manager::install_kraken no umbilical to node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  return umbilical_list_[Port::A]->install_kraken(node, module_name, send_code);
}

void Manager::do_activate_kraken(size_t node)
{
  append_action(Action(Action::Enum::activate_kraken, node), false);
}

void Manager::invalidate_active_kraken()
{
  if (umbilical_list_[Port::A].is_null()) {
    event_list_.push_back("Manager::invalidate_active_kraken no umbilical");
    return;
  }
  umbilical_list_[Port::A]->invalidate_active_kraken();
}

bool Manager::activate_kraken(size_t node, bool send_code)
{
  if (umbilical_list_[Port::A].is_null()) {
    event_list_.push_back("Manager::activate_kraken no umbilical to node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  return umbilical_list_[Port::A]->activate_kraken(node, send_code);
}

bool Manager::access(size_t node, Umbilical_ptr& u, Kraken_ptr& k, Tentacle_ptr& t, size_t& s)
{
  size_t limit = umbilical_list_.size();
  for (size_t i=0; i<limit; ++i) {
    u = umbilical_list_[i];
    if ((u != 0) && u->access(node, k, t, s)) {
      return true;
    }
  }
  return false;
}

NodeAccess Manager::access(size_t node)
{
  Umbilical_ptr u;
  Kraken_ptr k;
  Tentacle_ptr t;
  size_t s;
  if (access(node, u, k, t, s)) {
    return NodeAccess(u, k, t, s, node);
  }
  return NodeAccess();
}

bool Manager::activate_node(size_t node, NodeAccess& acc)
{
  acc = access(node);
  if (!acc.valid()) {
    event_list_.push_back("Manager::activate_node node "+boost::lexical_cast<std::string>(node)+" not reachable");
    return false;
  }
  if (!acc.tentacle()->activate()) {
    event_list_.push_back("Manager::activate_node cannot activate tentacle to node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  return true;
}

bool Manager::boot_async_708_kraken_707(const std::string& module_name, bool send_code)
{
  if (!boot_async(708, module_name, send_code)) {return false;}
  if (!install_kraken(707, module_name, send_code)) {return false;}
  return true;
}


bool Manager::boot_via(BootMethod::Enum bm, const f18a::cell_list_type& boot_stream)
{
  switch (bm) {
  case  BootMethod::fast_708_host: if (!boot_via_708(boot_stream)) {return false;}; return true;
  default: ;
  }
  return false;
}

bool Manager::boot(BootMethod::Enum bm)
{
  switch (bm) {
  case  BootMethod::async_708: if (!boot_async(708, KRAKEN_MODULE_NAME, true)) {return false;}; break;
  case  BootMethod::async_708_kraken_707: if (!boot_async_708_kraken(KRAKEN_MODULE_NAME, true, false, false, true)) {return false;}; break;
  case  BootMethod::async_708_kraken_709: if (!boot_async_708_kraken(KRAKEN_MODULE_NAME, false, true, false, true)) {return false;}; break;
  case  BootMethod::async_708_kraken_707_709: if (!boot_async_708_kraken(KRAKEN_MODULE_NAME, true, true, false, true)) {return false;}; break;
  case  BootMethod::async_708_host: if (!boot_async_708_host(KRAKEN_MODULE_NAME, true)) {return false;}; break;
  case  BootMethod::async_708_host_target: if (!boot_async_708_host_target(KRAKEN_MODULE_NAME, true)) {return false;}; break;
  case  BootMethod::fast_708_host: if (!boot_fast_708_host("Boot 708")) {return false;}; break;
  case  BootMethod::fast_708_host_target: if (!boot_fast_708_host_target("Boot 708 both")) {return false;}; break;
  default: if (!reset()) {return false;}; break;
  }
  //boot_method_ = bm;
  return false;
}

bool Manager::connected(Port::Enum p) const
{
  if(umbilical_list_[p] != 0) {
    if (umbilical_list_[p]->connected()) {
    }
  }
  return false;
}

bool Manager::connect(Port::Enum p)
{
  if (board_->connect(p)) {
  }
  umbilical_list_[p] = 0;
  return false;
}

bool Manager::recycle(size_t tentacle_index)
{
  event_list_.push_back("Manager::recycle not implemented");
  return false;
}

bool Manager::reach_target()
{
  event_list_.push_back("Manager::reach_target not implemented");
  return false;
}

bool Manager::span_host(bool send_code)
{
  if (!activate_kraken(707, send_code)) {return false;}
  //if (!destroy_tentacle()) {return false;}
  if (!add_tentacle_to(700, send_code)) {return false;}
  if (!add_tentacle_to(600, send_code)) {return false;}
  if (!add_tentacle_to(616, send_code)) {return false;}
  if (!add_tentacle_to(516, send_code)) {return false;}
  if (!add_tentacle_to(500, send_code)) {return false;}
  if (!add_tentacle_to(400, send_code)) {return false;}
  if (!add_tentacle_to(416, send_code)) {return false;}
  if (!add_tentacle_to(316, send_code)) {return false;}
  if (!add_tentacle_to(300, send_code)) {return false;}
  if (!add_tentacle_to(200, send_code)) {return false;}
  if (!add_tentacle_to(216, send_code)) {return false;}
  if (!add_tentacle_to(116, send_code)) {return false;}
  if (!add_tentacle_to(100, send_code)) {return false;}
  if (!add_tentacle_to(0, send_code)) {return false;}
  if (!add_tentacle_to(17, send_code)) {return false;}
  if (!add_tentacle_to(717, send_code)) {return false;}
  if (!add_tentacle_to(709, send_code)) {return false;}
  if (send_code) {
    if (!silence_all_io()) {return false;}
    if ((umbilical_list_[Port::A] != 0) && !umbilical_list_[Port::A]->test_tentacle_end()) {
      event_list_.push_back("Manager::span_host tentacle end test failed");
      return false;
    }
  }
  return true;
}

bool Manager::span_target(bool send_code)
{
  if (!activate_kraken(707, send_code)) {return false;}
  if (!add_tentacle_to(700, send_code)) {return false;}
  if (!add_tentacle_to(300, send_code)) {return false;}
  if (send_code) {
    if (!build_sync_bridge(300, 300+GA144_CHIP_OFFSET, 400+GA144_CHIP_OFFSET)) {return false;}
  }
  if (!add_tentacle_to(10700, send_code)) {return false;}
  if (!add_tentacle_to(10717, send_code)) {return false;}
  if (!add_tentacle_to(10617, send_code)) {return false;}
  if (!add_tentacle_to(10601, send_code)) {return false;}
  if (!add_tentacle_to(10501, send_code)) {return false;}
  if (!add_tentacle_to(10517, send_code)) {return false;}
  if (!add_tentacle_to(10417, send_code)) {return false;}
  if (!add_tentacle_to(10401, send_code)) {return false;}
  if (!add_tentacle_to(10301, send_code)) {return false;}
  if (!add_tentacle_to(10317, send_code)) {return false;}
  if (!add_tentacle_to(10217, send_code)) {return false;}
  if (!add_tentacle_to(10200, send_code)) {return false;}
  if (!add_tentacle_to(10100, send_code)) {return false;}
  if (!add_tentacle_to(10117, send_code)) {return false;}
  if (!add_tentacle_to(10017, send_code)) {return false;}
  if (!add_tentacle_to(10000, send_code)) {return false;}
  return true;
}

bool Manager::span_host_target(bool send_code)
{
  if (!span_target(send_code)) {return false;}
  //if (!activate_kraken(707)) {return false;}
  //if (!add_tentacle_to(700)) {return false;}
  //if (!add_tentacle_to(300)) {return false;}
  if (!activate_kraken(709, send_code)) {return false;}
  if (!add_tentacle_to(717, send_code)) {return false;}
  if (!add_tentacle_to(617, send_code)) {return false;}
  if (!add_tentacle_to(601, send_code)) {return false;}
  if (!add_tentacle_to(501, send_code)) {return false;}
  if (!add_tentacle_to(517, send_code)) {return false;}
  if (!add_tentacle_to(417, send_code)) {return false;}
  if (!add_tentacle_to(401, send_code)) {return false;}
  if (!add_tentacle_to(301, send_code)) {return false;}
  if (!add_tentacle_to(317, send_code)) {return false;}
  if (!add_tentacle_to(217, send_code)) {return false;}
  if (!add_tentacle_to(200, send_code)) {return false;}
  if (!add_tentacle_to(100, send_code)) {return false;}
  if (!add_tentacle_to(117, send_code)) {return false;}
  if (!add_tentacle_to(17, send_code)) {return false;}
  if (!add_tentacle_to(0, send_code)) {return false;}
  return true;
}

bool Manager::silence_sram()
{
  if (!write_io(8, 0x3557f)) {return false;}
  if (!write_io(9, 0x15555)) {return false;}
  if (!write_data(9, 0)) {return false;}
  if (!write_io(7, 0x15555)) {return false;}
  if (!write_data(7, 0)) {return false;}
  return true;
}

bool Manager::silence_spi()
{
  if (!write_io(705, 0x3ffff)) {return false;}
  return true;
}

bool Manager::silence_all_io()
{
  if (!silence_spi()) {return false;}
  if (!silence_sram()) {return false;}
  return true;
}

bool Manager::reset_target()
{
  NodeAccess acc;
  if (!activate_node(500, acc)) {
    event_list_.push_back("Manager::reset_target cannot activate node 500");
    return false;
  }
  long data[4];
  data[0] = remote_op(RemoteOp::write_a);
  data[1] = F18A_IO;
  data[2] = remote_op(RemoteOp::store_a);
  data[3] = F18A_PIN17_LO;
  if (!acc.w(4, data)) {
    event_list_.push_back("Manager::reset_target failed");
    return false;
  }
  connection::sleep(50);
  if (!acc.w2(remote_op(RemoteOp::store_a), F18A_PIN17_HI)) {
    event_list_.push_back("Manager::reset_target failed");
    return false;
  }
  //connection::sleep(20);
  return true;
}

bool Manager::build_sync_bridge(size_t host_node, size_t target_node, size_t next_node)
{
  if (!reset_target()) {return false;}

  f18a::cell addr;
  size_t pos;

  NodeAccess acc;
  if (!activate_node(host_node, acc)) {
    event_list_.push_back("Manager::reset_target cannot activate node "+boost::lexical_cast<std::string>(host_node));
    return false;
  }


  //if (acc.umbilical().test_tentacle_end()) {
  //  event_list_.push_back("Manager::reset_target cannot activate node "+boost::lexical_cast<std::string>(host_node));
  //  return false;
  //}

  size_t src_node = host_node;
  ga144::Chip_ptr src_chip = board_->split_node(src_node);
  if (src_chip.is_null()) {
    event_list_.push_back("Manager::build_sync_bridge invalid source node");
    return false;
  }
  size_t src_ind = src_chip->index(src_node);
  f18a::Processor_ptr src_proc = src_chip->processor(src_ind);
  if (src_ind == 0) {
    event_list_.push_back("Manager::build_sync_bridge source node to close to Kraken");
    return false;
  }
  size_t prev_node = acc.tentacle()->at(acc.segment()-1);

  size_t dest_node = target_node;
  ga144::Chip_ptr dest_chip = board_->split_node(dest_node);
  if (dest_chip.is_null()) {
    event_list_.push_back("Manager::build_sync_bridge invalid destination chip");
    return false;
  }
  size_t dest_ind = dest_chip->index(dest_node);
  f18a::Processor_ptr dest_proc = dest_chip->processor(dest_ind);
  size_t post_node = next_node;
  if (board_->split_node(post_node) != dest_chip) {
    event_list_.push_back("Manager::build_sync_bridge invalid target post node "+ boost::lexical_cast<std::string>(next_node));
    return false;
  }
  size_t post_ind = dest_chip->index(post_node);
  f18a::Processor_ptr post_proc = dest_chip->processor(post_ind);

  f18a::Module_ptr src_module = src_proc->config_by_name(BRIDGE_MODULE_NAME);
  if (src_module.is_null()) {
    src_module = dest_proc->config_by_name(BRIDGE_MODULE_NAME);
  }
  f18a::Module_ptr dest_module = dest_proc->config_by_name(BRIDGE_MODULE_NAME);
  if (src_module.is_null()) {
    event_list_.push_back("Manager::build_sync_bridge cannot find configuration '" BRIDGE_MODULE_NAME "' in node "+boost::lexical_cast<std::string>(src_node)+" or node "+boost::lexical_cast<std::string>(dest_node));
    return false;
  }
  if (dest_module.is_null()) {
    dest_module = src_module;
  }
  f18a::Module_ptr boot_module = src_proc->config_by_name(BOOTLOADER_MODULE_NAME);
  if (boot_module.is_null()) {
    event_list_.push_back("Manager::build_sync_bridge cannot find configuration '" BOOTLOADER_MODULE_NAME "' in node "+boost::lexical_cast<std::string>(src_node));
    return false;
  }

  long data[F18A_RAM_SIZE+5];

  // write ram of node 300
  pos = 0;
  data[pos++] = remote_op(RemoteOp::set_a_to_0);
  data[pos++] = remote_op(RemoteOp::write_rstack);
  data[pos++] = F18A_RAM_SIZE-1;
  data[pos++] = remote_op(RemoteOp::write_mem_loop);
  for (size_t i=0; i<F18A_RAM_SIZE; ++i) {
    data[pos++] = boot_module->ram().data(i);
  }
  if (!boot_module->find_label("trns", addr)) {
    event_list_.push_back("Manager::build_sync_bridge cannot find label 'trns' in node "+ boost::lexical_cast<std::string>(host_node));
    return false;
  }
  data[pos++] = f18a::State::addr_to_jump(addr);
  if (!acc.w(pos, data)) {
    event_list_.push_back("Manager::build_sync_bridge cannot write ram of node "+ boost::lexical_cast<std::string>(host_node));
    return false;
  }

  pos = 1;
  // send boot frame to node 10300 via node 300
  // boot frame conatins bridge code for node 10300
  if (!dest_module->find_label("ent", addr)) {
    event_list_.push_back("Manager::build_sync_bridge cannot find label 'ent' in node "+ boost::lexical_cast<std::string>(target_node));
    return false;
  }
  data[pos++] = addr;  // start address of node
  data[pos++] = 0;  // address to write
  data[pos++] = F18A_RAM_SIZE; // size of memory block to write
  for (size_t i=0; i<F18A_RAM_SIZE; ++i) {
    data[pos++] = dest_module->ram().data(i);
  }
  data[0] = pos-2; // size of data transfered to target node (minus 1)
  if (!acc.w(pos, data)) {
    event_list_.push_back("Manager::build_sync_bridge cannot send data to node "+ boost::lexical_cast<std::string>(host_node));
    return false;
  }

  // install bridge code in node 300
  pos = 0;
  data[pos++] = remote_op(RemoteOp::set_a_to_0);
  data[pos++] = remote_op(RemoteOp::write_rstack);
  data[pos++] = F18A_RAM_SIZE-1;
  data[pos++] = remote_op(RemoteOp::write_mem_loop);
  for (size_t i=0; i<F18A_RAM_SIZE; ++i) {
    data[pos++] = src_module->ram().data(i);
  }
  if (!src_module->find_label("ent", addr)) {
    event_list_.push_back("Manager::build_sync_bridge cannot find label 'ent' in node "+ boost::lexical_cast<std::string>(host_node));
    return false;
  }
  data[pos++] = f18a::State::addr_to_jump(addr);
  if (!acc.w(pos, data)) {
    return false;
  }

  acc.tentacle()->pop_back();
  src_chip->set_info(src_ind, ga144::InfoBit::bridge);
  dest_chip->set_info(dest_ind, ga144::InfoBit::bridge);
  // make a focusing call to target cont
  f18a::Port::Enum port;
  ga144::Neighbour::Enum dir;
  if (!dest_chip->neighbour_enum(dest_ind, post_ind, dir)) {
    event_list_.push_back("Manager::build_sync_bridge no neighbour");
    return false;
  }
  if (!dest_chip->neighbour_to_port(dest_ind, dir, port)) {
    event_list_.push_back("Manager::build_sync_bridge no port");
    return false;
  }
  size_t port_addr = f18a::Port::to_addr(port);
  dest_chip->processor(dest_ind)->set_b(port_addr);
  long focus = f18a::State::focusing_jump(port);
 
  if (!acc.w1(focus)) {
    event_list_.push_back("Manager::build_sync_bridge cannot focus node "+ boost::lexical_cast<std::string>(next_node));
    return false;
  }
  post_proc->tentacle_index(acc.tentacle()->size());
  acc.tentacle()->push_back(next_node);
  dest_chip->set_info(post_ind, ga144::InfoBit::tentacle);

#ifdef CHECK_ACTION
  if (!acc.umbilical().test_tentacle_end()) {
    return false;
  }
#endif
  return true;
}

bool Manager::read_tentacle()
{
  event_list_.push_back("Manager::test_tentacle not implemented");
  return false;
}

void Manager::do_destroy_tentacle()
{
  append_action(Action(Action::Enum::destroy_tentacle), false);
}

bool Manager::add_tentacle_to(size_t node, bool send_code)
{
  if (umbilical_list_[Port::A] != 0) {
    return umbilical_list_[Port::A]->add_tentacle_to(node, send_code);
  }
  return false;
}

bool Manager::reduce_tentacle_to(size_t node)
{
  if (umbilical_list_[Port::A] != 0) {
    return umbilical_list_[Port::A]->reduce_tentacle_to(node);
  }
  return false;
}

bool Manager::destroy_tentacle()
{
  if (umbilical_list_[Port::A] != 0) {
    return umbilical_list_[Port::A]->destroy_tentacle();
  }
  return false;
}

bool Manager::remove_all_tentacles()
{
  if (umbilical_list_[Port::A] != 0) {
    return umbilical_list_[Port::A]->remove_all_tentacles();
  }
  return false;
}

bool Manager::read_sram(size_t adr)
{
  //short data = board_->sram().read(adr);
  long adr1 = ((adr << 2) & 0x3fffc) | ((adr >> 16) & 3);
  // write lower 18 address in node 009
  if (!write_data(9, adr1)) {return false;}
  // set io to input
  if (!write_io(7, 0x14555)) {return false;}
  // calculate control word
  long control = 0;
  switch ((adr >> 18) & 3) {
  case 0: control = 0x2556E; break;
  case 1: control = 0x2557E; break;
  case 2: control = 0x3556E; break;
  case 3: control = 0x3557E; break;
  }
  if (!write_io(8, control)) {return false;}
  // read data
  long data;
  if (!read_data(7, data)) {return false;}
  // write to SRAM object
  board_->sram().write(adr, static_cast<short>(data));
  // reset control lines
  if (!write_io(8, 0x3557F)) {return false;}
  return true;
}

bool Manager::write_sram(size_t adr)
{
  short data = board_->sram().read(adr);
  long adr1 = ((adr << 2) & 0x3fffc) | ((adr >> 16) & 3);
  // write lower 18 address in node 009
  if (!write_data(9, adr1)) {return false;}
  // set io to output
  if (!write_io(7, 0x15555)) {return false;}
  // write data
  if (!write_data(7, static_cast<long>(data) & 0xffff)) {return false;}
  // calculate control word
  long control = 0;
  switch ((adr >> 18) & 3) {
  case 0: control = 0x2556A; break;
  case 1: control = 0x2557A; break;
  case 2: control = 0x3556A; break;
  case 3: control = 0x3557A; break;
  }
  if (!write_io(8, control)) {return false;}
  // reset control lines
  if (!write_io(8, 0x3557F)) {return false;}
  return true;
}

void Manager::do_read_ram(size_t node)
{
  append_action(Action(Action::Enum::read_ram, node), false);
}

bool Manager::read_ram(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::read_ram cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::read_ram cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long out[4];
  long in[F18A_RAM_SIZE];
  out[0] = remote_op(RemoteOp::set_a_to_0);
  out[1] = remote_op(RemoteOp::write_rstack);
  out[2] = F18A_RAM_SIZE-1;
  out[3] = remote_op(RemoteOp::read_mem_loop);
  // to read RAM we need to set A to 0
  if (!acc.wr(4, out, F18A_RAM_SIZE, in)) {
    return false;
  }
  for (size_t i=0; i<F18A_RAM_SIZE; ++i) {
    proc->module()->ram().data(i, in[i]);
  }
  proc->changed();
  return true;
}

void Manager::do_read_rom(size_t node)
{
  append_action(Action(Action::Enum::read_rom, node), false);
}

void Manager::do_read_and_compare_rom(size_t node)
{
  append_action(Action(Action::Enum::read_and_compare_rom, node), false);
}

bool Manager::read_and_compare_rom(size_t node)
{
  bool res = true;
  bool has_uncomparable_data = false;
  f18a::Memory_ptr rom = new f18a::Memory();
  if (!read_rom(node, rom)) {
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  // compare rom with compiled rom
  int event_cnt = 0;
  f18a::Memory& mem = proc->module()->rom();
  for (size_t i=0; i<F18A_ROM_SIZE; ++i) {
    if (mem.data(i) == F18_INVALID_VALUE) {
      has_uncomparable_data = true;
    }
    else if (mem.data(i) != rom->data(i)) {
      if (++event_cnt > 4) {
        event_list_.push_back("too many mismatch between chip ROM and memory");
        return false;
      }
      std::string txt = "ROM mismatch at ";
      utils::append_hex(i+F18A_ROM_START, 3, txt);
      txt += " on chip rom ";
      utils::append_cell(rom->data(i), txt);
      txt += " on memory ";
      utils::append_cell(mem.data(i), txt);
      event_list_.push_back(txt);
      res = false;
    }
  }
  if (res) {
    if (has_uncomparable_data) {
      event_list_.push_back("ROM and memory are identical (except for some uncompiled places)");
    }
    else {
      event_list_.push_back("ROM and memory are identical");
    }
  }
  return res;
}

bool Manager::read_rom(size_t node, f18a::Memory_ptr m)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::read_rom cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::read_rom cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long out[5];
  long in[F18A_ROM_SIZE];
  out[0] = remote_op(RemoteOp::write_a);
  out[1] = F18A_ROM_START;
  out[2] = remote_op(RemoteOp::write_rstack);
  out[3] = F18A_ROM_SIZE-1;
  out[4] = remote_op(RemoteOp::read_mem_loop);
  // to read RAM we need to set A to 0
  if (!acc.wr(5, out, F18A_ROM_SIZE, in)) {
    return false;
  }
  f18a::Memory* mem = m.get();
  if (mem == 0) {
    mem = &proc->module()->rom();
  }
  for (size_t i=0; i<F18A_ROM_SIZE; ++i) {
    mem->data(i, in[i]);
  }
  proc->changed();
  return true;
}

void Manager::do_read_ram_and_rom(size_t node)
{
  append_action(Action(Action::Enum::read_ram_and_rom, node), false);
}

bool Manager::read_ram_and_rom(size_t node)
{
  if (!read_ram(node)) {return false;}
  if (!read_rom(node)) {return false;}
  return true;
}

void Manager::do_write_ram(size_t node)
{
  append_action(Action(Action::Enum::write_ram, node), false);
}

bool Manager::write_ram(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::write_ram cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::write_ram cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long data;
  long out[F18A_RAM_SIZE+4];
  out[0] = remote_op(RemoteOp::set_a_to_0);
  out[1] = remote_op(RemoteOp::write_rstack);
  out[2] = F18A_RAM_SIZE-1;
  out[3] = remote_op(RemoteOp::write_mem_loop);
  for (size_t i=0; i<F18A_RAM_SIZE; ++i) {
    data = proc->module()->ram().data(i);
    if (data == F18_INVALID_VALUE) {
      out[i+4] = F18A_INITIAL_CELL;
    }
    else {
      out[i+4] = data;
    }
  }
  if (!acc.w(F18A_RAM_SIZE+4, out)) {
    return false;
  }
  proc->changed();
  return true;
}

void Manager::do_read_stack(size_t node)
{
  append_action(Action(Action::Enum::read_stack, node), false);
}

bool Manager::read_stack(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::read_stack cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::read_stack cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long data;
  if (!acc.wr1(remote_op(RemoteOp::read_stack), data)) {return false;}
  proc->module()->reg().named_.T_ = data;
  if (!acc.wr1(remote_op(RemoteOp::read_stack), data)) {return false;}
  proc->module()->reg().named_.S_ = data;
  f18a::Stack& stack = proc->module()->ps();
  for (size_t i=0; i<F18A_PARAMETER_STACK_SIZE; ++i) {
    if (!acc.wr1(remote_op(RemoteOp::read_rstack), data)) {return false;}
    stack.data(i, data);
  }
  proc->changed();
  return true;
}

void Manager::do_write_stack(size_t node)
{
  append_action(Action(Action::Enum::write_stack, node), false);
}

bool Manager::write_stack(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::write_stack cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::write_stack cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long out[13];
  f18a::Stack& stack = proc->module()->ps();
  out[0] = remote_op(RemoteOp::write_stack4);
  out[1] = stack.at(7);
  out[2] = stack.at(6);
  out[3] = stack.at(5);
  out[4] = stack.at(4);
  out[5] = remote_op(RemoteOp::write_stack4);
  out[6] = stack.at(3);
  out[7] = stack.at(2);
  out[8] = stack.at(1);
  out[9] = stack.at(0);
  out[10] = remote_op(RemoteOp::write_stack2);
  out[11] = proc->module()->reg().named_.S_;
  out[12] = proc->module()->reg().named_.T_;
  if (!acc.w(13, out)) {
    error();
  }
  return true;
}

void Manager::do_read_return_stack(size_t node)
{
  append_action(Action(Action::Enum::read_return_stack, node), false);
}

bool Manager::read_return_stack(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::read_return_stack cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::read_return_stack cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long data;
  if (!acc.wr1(remote_op(RemoteOp::read_rstack), data)) {return false;}
  proc->module()->reg().named_.R_ = data;
  f18a::Stack& stack = proc->module()->rs();
  for (size_t i=0; i<F18A_RETURN_STACK_SIZE; ++i) {
    if (!acc.wr1(remote_op(RemoteOp::read_rstack), data)) {return false;}
    stack.data(i, data);
  }
  proc->changed();
  return true;
}

void Manager::do_write_return_stack(size_t node)
{
  append_action(Action(Action::Enum::write_return_stack, node), false);
}

bool Manager::write_return_stack(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::write_return_stack cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::write_return_stack cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long out[15];
  f18a::Stack& stack = proc->module()->rs();
  out[0] = remote_op(RemoteOp::write_stack4);
  out[1] = proc->module()->reg().named_.R_;
  out[2] = stack.at(0);
  out[3] = stack.at(1);
  out[4] = stack.at(2);
  out[5] = remote_op(RemoteOp::write_stack4);
  out[6] = stack.at(3);
  out[7] = stack.at(4);
  out[8] = stack.at(5);
  out[9] = stack.at(6);
  out[10] = remote_op(RemoteOp::write_stack);
  out[11] = stack.at(7);
  out[12] = remote_op(RemoteOp::push_rstack3);
  out[13] = remote_op(RemoteOp::push_rstack3);
  out[14] = remote_op(RemoteOp::push_rstack3);
  if (!acc.w(15, out)) {
    error();
  }
  return true;
}

void Manager::do_read_a(size_t node)
{
  append_action(Action(Action::Enum::read_a, node), false);
}


bool Manager::read_a(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::read_a cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::read_a cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long data;
  if (!acc.wr1(remote_op(RemoteOp::read_a), data)) {
    return false;
  }
  proc->module()->reg().named_.A_ = data;
  proc->changed();
  return true;
}

void Manager::do_write_a(size_t node)
{
  append_action(Action(Action::Enum::write_a, node), false);
}

bool Manager::write_a(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::write_a cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::write_a cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long data = proc->module()->reg().named_.A_;
  if (data != F18_INVALID_VALUE) {
    if (!acc.w2(remote_op(RemoteOp::write_a), data)) {
      return false;
    }
  }
  return true;
}

void Manager::do_write_b(size_t node)
{
  append_action(Action(Action::Enum::write_b, node), false);
}

bool Manager::write_b(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::write_b cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::write_b cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long data = proc->module()->reg().named_.B_;
  if (data != F18_INVALID_VALUE) {
    if (!acc.w2(remote_op(RemoteOp::write_b), data)) {
      return false;
    }
  }
  return true;
}

void Manager::do_read_io(size_t node)
{
  append_action(Action(Action::Enum::read_io, node), false);
}

bool Manager::read_data(size_t node, long& data)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::read_io cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::read_io cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long out[3];
  long in[1];
  out[0] = remote_op(RemoteOp::write_a);
  out[1] = F18A_DATA;
  out[2] = remote_op(RemoteOp::fetch_a);
  if (!acc.wr(3, out, 1, in)) {
    return false;
  }
  data = in[0];
  return true;
}

bool Manager::read_io(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::read_io cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::read_io cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long out[3];
  long in[1];
  out[0] = remote_op(RemoteOp::write_a);
  out[1] = F18A_IO;
  out[2] = remote_op(RemoteOp::fetch_a);
  if (!acc.wr(3, out, 1, in)) {
    return false;
  }
  proc->module()->reg().named_.IO_ = in[0];
  proc->changed();
  return true;
}

void Manager::do_write_io(size_t node)
{
  append_action(Action(Action::Enum::write_io, node), false);
}

bool Manager::write_io(size_t node, long value)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::write_io cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::write_io cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long data = value;
  if (data == F18_INVALID_VALUE) {
    data = proc->module()->reg().named_.IO_;
  }
  if (data != F18_INVALID_VALUE) {
    long out[4];
    out[0] = remote_op(RemoteOp::write_a);
    out[1] = F18A_IO;
    out[2] = remote_op(RemoteOp::store_a);
    out[3] = data;
    if (!acc.w(4, out)) {
      return false;
    }
    proc->write_io(data);
  }
  return true;
}

bool Manager::write_data(size_t node, long value)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::write_data cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::write_data cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long data = value;
  if (data == F18_INVALID_VALUE) {
    data = proc->module()->reg().named_.IO_;
  }
  if (data != F18_INVALID_VALUE) {
    long out[4];
    out[0] = remote_op(RemoteOp::write_a);
    out[1] = F18A_DATA;
    out[2] = remote_op(RemoteOp::store_a);
    out[3] = data;
    if (!acc.w(4, out)) {
      return false;
    }
    proc->write_io(data);
  }
  return true;
}

void Manager::do_call(size_t node)
{
  append_action(Action(Action::Enum::call, node), false);
}

bool Manager::call(size_t node)
{
  return true;
}

void Manager::do_jump(size_t node)
{
  append_action(Action(Action::Enum::jump, node), false);
}

bool Manager::jump(size_t node)
{
  return true;
}

void Manager::do_read(size_t node)
{
  append_action(Action(Action::Enum::read, node), false);
}

bool Manager::read(size_t node)
{
  if (!read_stack(node)) {return false;}
  if (!read_return_stack(node)) {return false;}
  if (!read_a(node)) {return false;}
  if (!read_io(node)) {return false;}
  if (!read_ram(node)) {return false;}
  if (!read_rom(node)) {return false;}
  return true;
}

void Manager::do_write(size_t node)
{
  append_action(Action(Action::Enum::write, node), false);
}

bool Manager::write(size_t node)
{
  if (!write_ram(node)) {return false;}
  if (!write_io(node, F18_INVALID_VALUE)) {return false;}
  if (!write_a(node)) {return false;}
  if (!write_return_stack(node)) {return false;}
  if (!write_stack(node)) {return false;}
  return true;
}

void Manager::do_start(size_t node)
{
  append_action(Action(Action::Enum::start, node), false);
}

bool Manager::start(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::start cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::start cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long start_addr = proc->module()->reg().named_.P_;
  if (start_addr == F18_INVALID_VALUE) {
    event_list_.push_back("Manager::start node "+boost::lexical_cast<std::string>(node)+" has no starting address set in P");
    return false;
  }
  if (!reduce_tentacle_to(node)) {return false;}
  //if (!write_b(node)) {return false;}
  long start_instr = f18a::State::addr_to_jump(start_addr);
  if (!acc.w1(start_instr)) {return false;}

  ga144::Chip_ptr chip = board_->split_node(node);
  if (chip.is_null()) {
    event_list_.push_back("Manager::start cannot find chip");
    return false;
  }
  size_t chip_ind = chip->index(node);


  chip->processor(chip_ind)->set_b(F18_INVALID_VALUE);
  acc.tentacle()->pop_back();
  proc->tentacle_index(EVB_INVALID_TENTACLE);
  chip->set_info(chip_ind, ga144::InfoBit::running);

  proc->changed();
  return true;
}

void Manager::do_test(size_t node)
{
  append_action(Action(Action::Enum::test, node), false);
}

bool Manager::test(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::test cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::test cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long data;
  if (!acc.wr1(remote_op(RemoteOp::read_a), data)) {
    return false;
  }
  event_list_.push_back("Test ok for node "+boost::lexical_cast<std::string>(node));
  return true;
}

void Manager::do_run(size_t node)
{
  append_action(Action(Action::Enum::run, node), false);
}

bool Manager::run(size_t node)
{
  NodeAccess acc;
  if (!activate_node(node, acc)) {
    event_list_.push_back("Manager::run cannot activate node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::run cannot open processor for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  long start_addr = proc->module()->reg().named_.P_;
  if (start_addr == F18_INVALID_VALUE) {
    event_list_.push_back("Manager::start node "+boost::lexical_cast<std::string>(node)+" has no staring address set in P");
    return false;
  }
  if (!reduce_tentacle_to(node)) {return false;}
  if (!write(node)) {return false;}
  if (!write_b(node)) {return false;}
  long start_instr = f18a::State::addr_to_jump(start_addr);
  if (!acc.w1(start_instr)) {return false;}

  ga144::Chip_ptr chip = board_->split_node(node);
  if (chip.is_null()) {
    event_list_.push_back("Manager::start cannot find chip");
    return false;
  }
  size_t chip_ind = chip->index(node);

  chip->processor(chip_ind)->set_b(F18_INVALID_VALUE);
  acc.tentacle()->pop_back();
  proc->tentacle_index(EVB_INVALID_TENTACLE);
  chip->set_info(chip_ind, ga144::InfoBit::running);

  proc->changed();
  return true;
}

bool Manager::get_pin(Pin::Enum, PinStatus::Enum&, long& value)
{
  return false;
}

bool Manager::set_pin(Pin::Enum pin, long value)
{
  size_t node = evb::Pin::node(pin);
  //size_t index;
  long mask = 0;
  //if (!activate_node(node, index)) {
  //  event_list_.push_back("Manager::set_pin "+boost::lexical_cast<std::string>(node)+" is not reachable");
  //  return false;
  //}
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    event_list_.push_back("Manager::set_pin cannot access node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  evb::PinType::Enum type = evb::Pin::type(pin);
  switch (type) {
  case evb::PinType::io_1:
    value &= 3;
    mask = 3;
    break;

  case evb::PinType::io_3:
    value &= 3;
    value <<= 2;
    mask = 3 << 2;
    break;

  case evb::PinType::io_5:
    value &= 3;
    value <<= 4;
    mask = 3 << 4;
    break;

  case evb::PinType::io_17:
    value &= 3;
    value <<= 16;
    mask = 3 << 16;
    break;

  case evb::PinType::analog_out:
    value &= F18A_AOUT_MASK;
    value ^= F18A_AOUT_INV;
    mask = F18A_AOUT_MASK;
    break;

  default:
    event_list_.push_back("Manager::set_pin invalid pin "+boost::lexical_cast<std::string>(pin));
    return false;
  }
  if (proc->io_is_written()) {
    value |= proc->io_written() & ~mask;
  }
  proc->write_io(value);
  if (!write_io(node, value)) {
    event_list_.push_back("Manager::set_pin failed for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  return true;
}

void Manager::do_set_pin(Pin::Enum pin, long value)
{
  append_action(Action(Action::Enum::write_pin, pin, value), false);
}


bool Manager::compile_module(Module_ptr mod)
{
  size_t limit = mod->size();
  size_t ok = 0;
  size_t failed = 0;
  for (size_t i=0; i<limit; ++i) {
    size_t nd = mod->node(i);
    f18a::State state;
    ga144::Chip_ptr chip = board_->split_node(nd);
    f18a::Processor_ptr proc = chip->processor(chip->index(nd));
    f18a::Module_ptr pmod = proc->config_by_name(mod->name());
    std::string src;
    if (pmod != 0) {
      src = pmod->ram().source();
    }
    if (state.assemble(
        mod->name(),
        src,
        false,
        proc,
        board_
      )
    ) {
      ++ok;
    }
    else {
      ++failed;
    }
  }
  if (failed > 0) {
    event_list_.push_back("Manager::compile_module ERROR ok("+boost::lexical_cast<std::string>(ok)+") failed("+ boost::lexical_cast<std::string>(failed)  +")");
    return false;
  }
  return true;
}

bool Manager::write_module(Module_ptr mod)
{
  std::map<size_t, size_t> node_map;
  size_t nd, ond;
  size_t limit = mod->size();
  for (size_t i=0; i<limit; ++i) {
    ond = nd = mod->node(i);
    ga144::Chip_ptr chip = board_->split_node(nd);
    size_t pos = chip->index(nd) + chip->offset();
    node_map[pos] = ond;
  }
  for (std::map<size_t, size_t>::reverse_iterator it = node_map.rbegin(); it != node_map.rend(); ++it) {
    do_write(it->second);
    //if (!write(it->second)) {
    //  event_list_.push_back("Manager::write_module ERROR on node("+boost::lexical_cast<std::string>(it->second)  +")");
    //  return false;
    //}
  }
  return true;
}

bool Manager::start_module(Module_ptr mod)
{
  std::map<size_t, size_t> node_map;
  size_t nd, ond;
  size_t limit = mod->size();
  for (size_t i=0; i<limit; ++i) {
    ond = nd = mod->node(i);
    ga144::Chip_ptr chip = board_->split_node(nd);
    size_t pos = chip->index(nd) + chip->offset();
    node_map[pos] = ond;
  }
  for (std::map<size_t, size_t>::reverse_iterator it = node_map.rbegin(); it != node_map.rend(); ++it) {
    do_run(it->second);
    //if (!do_run(it->second)) {
    //  event_list_.push_back("Manager::write_module ERROR on node("+boost::lexical_cast<std::string>(it->second)  +")");
    //  return false;
    //}
  }
  return true;
}

void Manager::do_boot_via_708(const f18a::cell_list_type& boot_stream)
{
  append_action(Action(Action::Enum::boot_via_708, boot_stream), false);
}

bool Manager::boot_via_708(const f18a::cell_list_type& boot_stream)
{
  if (!reset()) {
    event_list_.push_back("Manager::boot_via_708 reset failed");
    return false;
  }
  umbilical_list_[Port::A] = new Umbilical(this, board_->assert_connection(Port::A), 708);
  if (!umbilical_list_[Port::A]->write_boot_stream(boot_stream)) {
    event_list_.push_back("Manager::boot_via_708 write boot stream failed");
    return false;
  }
  return true;
}

void Manager::do_boot_708(const f18a::cell_list_type& boot_stream)
{
  append_action(Action(Action::Enum::boot_708, boot_stream), false);
}

bool Manager::boot_708(const std::string& package_name)
{
  Package_ptr package = board_->find_package(package_name);
  if (package.is_null()) {
    event_list_.push_back("Manager::boot_708 cannot find package '"+package_name+"'");
    return false;
  }
  f18a::cell_list_type empty, frame1, frame2;
  size_t node = 708;
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    add_error("Umbilical::boot_708 invalid node "+boost::lexical_cast<std::string>(node));
    return false;
  }

  f18a::Module_ptr module = proc->config_by_name(KRAKEN_MODULE_NAME);
  if (module.is_null()) {
    add_error("Umbilical::boot_708 cannot find configuration '" KRAKEN_MODULE_NAME "' in node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  if (module->reg().named_.P_ == F18_INVALID_VALUE) {
    add_error("Umbilical::boot_708 no entry point for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::cell frame2_jump_adr = module->reg().named_.P_;
  f18a::cell frame1_jump_adr = 0xaa;

  board_->create_boot_frame(Board::BootFramePath::via_708, Board::BootCodeType::chain_via_b, frame1_jump_adr, 0, package, empty, frame1);
  board_->create_boot_frame(Board::BootFramePath::for_708, Board::BootCodeType::chain_via_b, frame2_jump_adr, 0, package, empty, frame2);
  if (!reset()) {return false;}
  umbilical_list_[Port::A] = new Umbilical(this, board_->assert_connection(Port::A), 708);
//#if defined(_WIN32)
//  ::Sleep(5000);
//#else
//  ::usleep(5000 * 1000);
//#endif
  
  if (!umbilical_list_[Port::A]->write_boot_stream(frame1)) {
    return false;
  }
  if (!umbilical_list_[Port::A]->write_boot_stream(frame2)) {
    return false;
  }

  board_->set_info(708, ga144::InfoBit::umbilical);

  if (!boot_async_708_kraken(KRAKEN_MODULE_NAME, true, false, false, false)
    || !span_host(false)
  ) {
    event_list_.push_back("Manager::boot_708 boot_async_708_kraken failed");
    return false;
  }
  invalidate_active_kraken();

  if (!activate_kraken(707, true)) {
    event_list_.push_back("Manager::boot_708 activate_kraken failed");
    return false;
  }

  //f18a::port_list_type port_list;
  //node_list_type node_list;
  //board_->get_path(Board::BootFramePath::via_708, 0, port_list, node_list);

  //board_->set_info(node_list[0], ga144::InfoBit::umbilical);
  //board_->set_info(node_list[1], ga144::InfoBit::active_kraken);
  //size_t i, limit = node_list.size();
  //for (i=2; i<limit; ++i) {
  //  board_->set_info(node_list[i], ga144::InfoBit::tentacle);
  //}

  //event_list_.push_back("Manager::boot_708 boot with no errors");
  return true;
}

void Manager::do_boot_708_both(const f18a::cell_list_type& boot_stream)
{
  append_action(Action(Action::Enum::boot_708_both, boot_stream), false);
}

bool Manager::boot_708_both(const std::string& package_name)
{
  // to boot both we must build 4 bootframes:
  // 1) boot frame to build path for host chip via node 709
  // 2) boot frame for target chip via node 707
  // 3) boot frame for host chip via existing path
  // 4) boot frame for node 708
  Package_ptr package = board_->find_package(package_name);
  if (package.is_null()) {
    event_list_.push_back("Manager::boot_708_both cannot find package '"+package_name+"'");
    return false;
  }
  f18a::cell_list_type empty, frame1, frame2, frame3, frame4;
  size_t node = 708;
  f18a::Processor_ptr proc = board_->processor(node);
  if (proc.is_null()) {
    add_error("Umbilical::boot_708_both invalid node "+boost::lexical_cast<std::string>(node));
    return false;
  }

  f18a::Module_ptr kraken_module = proc->config_by_name(KRAKEN_MODULE_NAME);
  if (kraken_module.is_null()) {
    add_error("Umbilical::boot_708_both cannot find configuration '" KRAKEN_MODULE_NAME "' in node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  if (kraken_module->reg().named_.P_ == F18_INVALID_VALUE) {
    add_error("Umbilical::boot_708_both no entry point for node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Module_ptr bridge_module = proc->config_by_name(BRIDGE_MODULE_NAME);
  if (bridge_module.is_null()) {
    add_error("Umbilical::boot_708_both cannot find configuration '" BRIDGE_MODULE_NAME "' in node "+boost::lexical_cast<std::string>(node));
    return false;
  }
  f18a::Module_ptr bootloader_module = proc->config_by_name(BOOTLOADER_MODULE_NAME);
  if (bootloader_module.is_null()) {
    add_error("Umbilical::boot_708_both cannot find configuration '" BOOTLOADER_MODULE_NAME "' in node "+boost::lexical_cast<std::string>(node));
    return false;
  }


  f18a::cell frame4_jump_adr = kraken_module->reg().named_.P_;
  f18a::cell frame3_jump_adr = 0xaa;
  f18a::cell frame2_jump_adr = 0xaa;
  f18a::cell frame1_jump_adr = 0xaa;

  // now the chip is booted, we must setup the user inteface
  return true;
}

}
