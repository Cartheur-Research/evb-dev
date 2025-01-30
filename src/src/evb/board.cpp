#include <evb.hpp>
#include <connection.hpp>

#include <gtkmm.h>

/**

pf 2281 contains code for async writing back data.

*/

namespace evb {

SRAM::SRAM()
{
  data_.resize(EVB_SRAM_SIZE);
}

SRAM::~SRAM()
{
}

short SRAM::read(size_t adr) const
{
  if (adr < EVB_SRAM_SIZE) {
    return data_[adr];
  }
  return 0;
}

void SRAM::write(size_t adr, short val)
{
  if (adr < EVB_SRAM_SIZE) {
    data_[adr] = val;
  }
}

// ======================================================================================


Module::Module(Board& b, const std::string& name)
: board_(b)
, name_(name)
{
}

Module::~Module()
{
}

void Module::add(size_t node, f18a::Module_ptr mod)
{
  module_list_.push_back(mod);
  node_list_.push_back(node);
  mod->name(name_);
}

void Module::remove(size_t node)
{
  size_t limit = node_list_.size();
  for (size_t i=0; i<limit; ++i) {
    if (node_list_[i] == node) {
      module_list_.erase(module_list_.begin() + i);
      node_list_.erase(node_list_.begin() + i);
      return;
    }
  }
}


void Module::read(xml::Iterator& it)
{
	if ((it.current() == xml::Element::tag) && (it.tag() == "name")) {
    it.next();
    if (it.current() == xml::Element::content) {
      name_ = it.value();
  	  it.next_tag();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "name")) {
      it.next_tag();
    }
  }
	if ((it.current() == xml::Element::tag) && (it.tag() == "list")) {
    it.next();
    if (it.current() == xml::Element::content) {
      std::vector<long> list;
      it.read(list);
      size_t limit = list.size();
      for (size_t i=0; i<limit; ++i) {
        f18a::Processor_ptr proc = board_.processor(list[i]);
        if (proc != 0) {
          f18a::Module_ptr mod = proc->config_by_name(name_);
          if (mod.is_not_null()) {
            node_list_.push_back(list[i]);
            module_list_.push_back(mod);
          }
          //else {
          //  Gtk::MessageDialog dialog(0, "Module load failed");
          //  std::string err = "Cannot find Module '";
          //  err += name_;
          //  err += "' in node ";
          //  err += boost::lexical_cast<std::string>(list[i]);
          //  dialog.set_secondary_text(err);
          //  dialog.show();
          //  dialog.run();
          //}
        }
      }
  	  it.next_tag();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "list")) {
      it.next_tag();
    }
  }

}

void Module::write(xml::Writer& w)
{
  if (!name_.empty()) {
    w.newline();
    w.open("name", false, true);
    w.write(name_);
    w.open("name", true, true);
  }

  std::vector<long> list;
  size_t limit = node_list_.size();
  for (size_t i=0; i<limit; ++i) {
    list.push_back(node_list_[i]);
  }
  if (!list.empty()) {
    w.newline();
    w.open("list", false, true);
    w.write(list);
    w.open("list", true, true);
  }
}

// ======================================================================================


Package::Package(Board& b, const std::string& name)
: board_(b)
, name_(name)
{
}

Package::~Package()
{
}

Package_ptr Package::clone()
{
  Package_ptr res = new Package(board_, name_);
  res->module_list_ = module_list_;
  res->exclude_list_ = exclude_list_;
  return res;
}

void Package::clear()
{
  name_.clear();
  module_list_.clear();
}

void Package::append(Module_ptr mod)
{
  module_list_.push_back(mod);
}

void Package::remove(size_t pos)
{
  module_list_.erase(module_list_.begin() + pos);
}

bool Package::move_up(size_t pos)
{
  if (pos > 0) {
    module_list_[pos-1].swap(module_list_[pos]);
    return true;
  }
  return false;
}

bool Package::move_down(size_t pos)
{
  if (pos+1 < module_list_.size()) {
    module_list_[pos+1].swap(module_list_[pos]);
    return true;
  }
  return false;
}

bool Package::find(const std::string& name, size_t& pos)
{
  for (size_t i=0; i<module_list_.size(); ++i) {
    if (module_list_[i]->name() == name) {
      pos = i;
      return true;
    }
  }
  return false;
}


void Package::read(xml::Iterator& it)
{
	if ((it.current() == xml::Element::tag) && (it.tag() == "name")) {
    it.next();
    if (it.current() == xml::Element::content) {
      name_ = it.value();
  	  it.next_tag();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "name")) {
      it.next_tag();
    }
  }
	if ((it.current() == xml::Element::tag) && (it.tag() == "list")) {
    it.next();
  	while ((it.current() == xml::Element::tag) && (it.tag() == "m")) {
      it.next();
      if (it.current() == xml::Element::content) {
        std::string mod_name = it.value();
        Module_ptr mod = board_.find_module(mod_name);
        if (mod.is_not_null()) {
          append(mod);
        }
  	    it.next_tag();
      }
  	  if ((it.current() == xml::Element::end) && (it.tag() == "m")) {
        it.next_tag();
      }
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "list")) {
      it.next_tag();
    }
  }
	if ((it.current() == xml::Element::tag) && (it.tag() == "exclude")) {
    it.next();
    if (it.current() == xml::Element::content) {
      std::vector<long> list;
      it.read(list);

      evb::node_list_type excl_list;
      size_t limit = list.size();
      for (size_t i=0; i<limit; ++i) {
        excl_list.push_back(list[i]);
      }
      exclude_list(excl_list);

      it.next_tag();
    }
  	if ((it.current() == xml::Element::end) && (it.tag() == "exclude")) {
      it.next_tag();
    }
  }
}

void Package::write(xml::Writer& w)
{
  if (!name_.empty()) {
    w.newline();
    w.open("name", false, true);
    w.write(name_);
    w.open("name", true, true);
  }

  size_t i, limit = module_list_.size();
  if (limit > 0) {
    w.newline();
    w.open("list", false, true);
    for (i=0; i<limit; ++i) {
      w.open("m", false, true);
      w.write(module_list_[i]->name());
      w.open("m", true, true);
    }
    w.open("list", true, true);
  }
  const evb::node_list_type& excl_list = exclude_list();
  limit = excl_list.size();
  std::vector<long> list;
  for (i=0; i<limit; ++i) {
    list.push_back(excl_list[i]);
  }
  if (!list.empty()) {
    w.newline();
    w.open("exclude", false, true);
    w.write(list);
    w.open("exclude", true, true);
  }
}



// ======================================================================================

Board::Board()
: modify_dispatcher_(0)
, jumpers_(0)
{
  host_ = new ga144::Chip(0);
  target_ = new ga144::Chip(GA144_CHIP_OFFSET);

  modify_dispatcher_ = new Glib::Dispatcher();
  connection_list_.resize(Port::limit);
  //for (size_t i=0; i<Port::limit; ++i) {
  //  connection_list_[i] = new connection::Serial();
  //}

  //chip_list_.resize(Chip::limit);
  //for (size_t i=0; i<Chip::limit; ++i) {
  //  chip_list_[i] = new ga144::Chip();
  //}

  jumper(Jumper::J10_2_3, true);
  jumper(Jumper::J11_2_3, true);
  jumper(Jumper::J14_2_3, true);
  jumper(Jumper::J15_2_3, true);
  jumper(Jumper::J16_2_3, true);

  jumper(Jumper::J20_1_3, true);
  jumper(Jumper::J20_2_4, true);

  jumper(Jumper::J22_1_2, true);
  jumper(Jumper::J22_3_4, true);
  jumper(Jumper::J22_5_6, true);

  jumper(Jumper::J23_1_2, true);
  jumper(Jumper::J23_3_4, true);
  jumper(Jumper::J23_5_6, true);
  jumper(Jumper::J23_7_8, true);
  jumper(Jumper::J23_9_10, true);
  jumper(Jumper::J23_11_12, true);

  jumper(Jumper::J25_2_3, true);

  jumper(Jumper::J26_1_2, true);

  jumper(Jumper::J34_1_2, true);

  jumper(Jumper::J35_1_2, true);

  jumper(Jumper::J37_1_2, true);
  jumper(Jumper::J37_3_4, true);

  jumper(Jumper::J38_1_J40_1, true);
  jumper(Jumper::J38_2_J40_2, true);
  jumper(Jumper::J38_3_J40_3, true);
  jumper(Jumper::J38_4_J40_4, true);
  jumper(Jumper::J38_5_J40_5, true);

  jumper(Jumper::J39_1_2, true);

  host_->register_modify_dispatcher(*modify_dispatcher_);
  target_->register_modify_dispatcher(*modify_dispatcher_);
  modify_dispatcher_->connect(sigc::mem_fun(*this, &Board::on_modify));

}

Board::~Board()
{
  delete modify_dispatcher_;
}

std::string Board::picture_file() const
{
  return "pict/evb001_board.png";
}

std::string Board::version() const
{
  return "";
}

void Board::register_modify_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(modify_mutex_);
  modify_dispatcher_list_.push_back(&d);
}

void Board::deregister_modify_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(modify_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = modify_dispatcher_list_.begin(); it != modify_dispatcher_list_.end(); ++it) {
    if (*it == &d) {
      modify_dispatcher_list_.erase(it);
      return;
    }
  }
}

void Board::on_modify()
{
  modify();
}

void Board::modify()
{
  boost::mutex::scoped_lock lock(modify_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = modify_dispatcher_list_.begin(); it != modify_dispatcher_list_.end(); ++it) {
    (*it)->emit();
  }
}


void Board::error()
{
  boost::mutex::scoped_lock lock(error_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = error_dispatcher_list_.begin(); it != error_dispatcher_list_.end(); ++it) {
    (*it)->emit();
  }
}

void Board::register_error_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(error_mutex_);
  error_dispatcher_list_.push_back(&d);
}

void Board::deregister_error_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(error_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = error_dispatcher_list_.begin(); it != error_dispatcher_list_.end(); ++it) {
    if (*it == &d) {
      error_dispatcher_list_.erase(it);
      return;
    }
  }
}

void Board::success()
{
  boost::mutex::scoped_lock lock(error_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = success_dispatcher_list_.begin(); it != success_dispatcher_list_.end(); ++it) {
    (*it)->emit();
  }
}

void Board::register_success_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(error_mutex_);
  success_dispatcher_list_.push_back(&d);
}

void Board::deregister_success_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(error_mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = error_dispatcher_list_.begin(); it != success_dispatcher_list_.end(); ++it) {
    if (*it == &d) {
      success_dispatcher_list_.erase(it);
      return;
    }
  }
}

void Board::reset()
{
  host_->reset();
  target_->reset();
  //if (has_connection(Port::A) && connection(Port::A)->connected()) {
  //  if (!connection(Port::A)->rts(false)) {return false;}
  //  connection::sleep(5);
  //  if (!connection(Port::A)->rts(true)) {return false;}
  //  host_->reset();
  //  target_->reset();
  //  return true;
  //}
  //return false;
}

//bool Board::boot_async(Port::Enum p)
//{
//  if (!has_connection(p) || !connection(Port::A)->connected()) {return false;}
//  if (!connection(Port::A)->rts(false)) {return false;}
//  connection::sleep(50);
//  if (!connection(Port::A)->rts(true)) {return false;}
//  connection::sleep(20);
//
//  connection::Serial* serial = connection(p);
//  if ((serial == 0) || !serial->connected()) {return false;}
//  f18a::Processor& proc_708 = *host_->processor(host_->index(708));
//  f18a::Module& module_708 = proc_708.assert_config(0);
//  //if (!reset()) {return false;}
//  serial->reset_evb001();
//  // write boot frame for node 708
//  connection(Port::A)->write(module_708.reg().named_.P_);
//  connection(Port::A)->write(0);
//  connection(Port::A)->write(F18A_RAM_SIZE);
//  for (size_t i=0; i<F18A_RAM_SIZE; ++i) {
//    connection(Port::A)->write(module_708.ram().data(i));
//  }
//  return true;
//}

//bool Board::boot_async_kraken(Port::Enum p)
//{
//  if (!boot_async(p)) {return false;}
//  f18a::Processor& proc_708 = *host_->processor(host_->index(708));
//  f18a::Processor& proc_707 = *host_->processor(host_->index(707));
//
//  return true;
//}

bool Board::connect(Port::Enum p)
{
  if (has_connection(p)) {
    return connection(p)->connect();
  }
  return false;
}

bool Board::jumper(Jumper::Enum j) const
{
  return (jumpers_ & (1LL << j)) != 0;
}

void Board::jumper(Jumper::Enum j, bool val)
{
  if (val) {
    jumpers_ |= (1LL << j);
  }
  else {
    jumpers_ &= ~(1LL << j);
  }
}

void Board::connection(Port::Enum p, connection::Serial_ptr s)
{
  connection_list_[p] = s;
}

connection::Serial_ptr Board::new_connection(Port::Enum p)
{
  connection::Serial_ptr res = new connection::Serial();
  connection(p, res);
  return res;
}

connection::Serial_ptr Board::assert_connection(Port::Enum p)
{
  if (connection_list_[p].is_null()) {
    connection_list_[p] = new connection::Serial();
  }
  return connection_list_[p];
}

bool Board::find_screen(const std::string& name, int& pos)
{
  screen_index_map_type::iterator it = screen_to_index_.find(name);
  if (it == screen_to_index_.end()) {
    return false;
  }
  pos = it->second;
  return true;
}

bool Board::find_screen_source(int pos, std::string& src)
{
  screen_lookup_map_type::iterator it = screen_index_to_source_.find(pos);
  if (it == screen_index_to_source_.end()) {
    return false;
  }
  src = it->second;
  return true;
}

void Board::read(xml::Iterator& it)
{
	if (it.tag() == "name") {
  	if (it.next() == xml::Element::content) {
      name_ = it.value();
      if ((it.next_tag() == xml::Element::end) && (it.tag() == "name")) {
        it.next_tag();
      }
    }
  }
	if (it.tag() == "host") {
    it.next_tag();
    host_->read(it);
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "host")) {
      it.next();
    }
  }
	if (it.tag() == "target") {
    it.next_tag();
    target_->read(it);
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "target")) {
      it.next();
    }
  }
	if (it.tag() == "a") {
    it.next_tag();
    new_connection(Port::A)->read(it);
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "a")) {
      it.next_tag();
    }
  }
	if (it.tag() == "b") {
    it.next_tag();
    new_connection(Port::B)->read(it);
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "b")) {
      it.next_tag();
    }
  }
	if (it.tag() == "c") {
    it.next_tag();
    new_connection(Port::C)->read(it);
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "c")) {
      it.next_tag();
    }
  }
	if (it.tag() == "jumpers") {
  	if (it.next() == xml::Element::content) {
      try {
        std::stringstream ss(it.value());
        ss >> std::hex >> jumpers_;
      }
      catch (...) {
        jumpers_ = 0;
      }
      if ((it.next_tag() == xml::Element::end) && (it.tag() == "jumpers")) {
        it.next_tag();
      }
    }
  }
	if (it.tag() == "screens") {
    it.next_tag();
    while ((it.current() == xml::Element::tag) && (it.tag() == "screen")) {
      if ((it.next() == xml::Element::attribute) && (it.key() == "name")) {
        std::string name = it.value();
        std::string source;
        if (it.next() == xml::Element::content) {
          source = it.value();
          it.next_tag();
        }
        size_t pos = screen_to_index_.size();
        screen_to_index_[name] = pos;
        if (!source.empty()) {
          screen_index_to_source_[pos] = source;
        }
      }
      if ((it.current() == xml::Element::empty) && (it.tag() == "screen")) {
        it.next_tag();
      }
      else if ((it.current() == xml::Element::end) && (it.tag() == "screen")) {
        it.next_tag();
      }
    }
    if ((it.current() == xml::Element::end) && (it.tag() == "screens")) {
      it.next_tag();
    }
  }
	if (it.tag() == "modules") {
    it.next_tag();
    while ((it.current() == xml::Element::tag) && (it.tag() == "module")) {
      it.next_tag();
      evb::Module_ptr mod = new evb::Module(*this, "");
      mod->read(it);
      module_map_[mod->name()] = mod;
      if ((it.current() == xml::Element::end) && (it.tag() == "module")) {
        it.next_tag();
      }
    }
    if ((it.current() == xml::Element::end) && (it.tag() == "modules")) {
      it.next_tag();
    }
  }
	if (it.tag() == "packages") {
    it.next_tag();
    while ((it.current() == xml::Element::tag) && (it.tag() == "package")) {
      it.next_tag();
      evb::Package_ptr pack = new evb::Package(*this, "");
      pack->read(it);
      package_map_[pack->name()] = pack;
      if ((it.current() == xml::Element::end) && (it.tag() == "package")) {
        it.next_tag();
      }
    }
    if ((it.current() == xml::Element::end) && (it.tag() == "packages")) {
      it.next_tag();
    }
  }
}

void Board::write(xml::Writer& w)
{
  w.newline();
  w.open("version", false, true);
  w.write(version());
  w.open("version", true, true);

  w.newline();
  w.open("name", false, true);
  w.write(name_);
  w.open("name", true, true);

  w.newline();
  w.open("host", false, true);
  w.indent(1);
  host_->write(w);
  w.newline(-1);
  w.open("host", true, true);

  w.newline();
  w.open("target", false, true);
  w.indent(1);
  target_->write(w);
  w.newline(-1);
  w.open("target", true, true);

  if (has_connection(Port::A)) {
    w.newline();
    w.open("a", false, true);
    w.newline(1);
    connection(Port::A)->write(w);
    w.newline(-1);
    w.open("a", true, true);
  }
  if (has_connection(Port::B)) {
    w.newline();
    w.open("b", false, true);
    w.newline(1);
    connection(Port::A)->write(w);
    w.newline(-1);
    w.open("b", true, true);
  }
  if (has_connection(Port::C)) {
    w.newline();
    w.open("c", false, true);
    w.newline(1);
    connection(Port::A)->write(w);
    w.newline(-1);
    w.open("c", true, true);
  }
  {
    w.newline();
    w.open("jumpers", false, true);
    {
      std::stringstream ss;
      ss << std::hex << jumpers_;
      w.write(ss.str());
    }
    w.open("jumpers", true, true);
  }
  if (!screen_to_index_.empty()) {
    w.newline();
    w.open("screens", false, true);
    w.indent(1);
    for (screen_index_map_type::iterator it=screen_to_index_.begin(); it != screen_to_index_.end(); ++it) {
      w.newline();
      screen_lookup_map_type::iterator it2 = screen_index_to_source_.find(it->second);
      if (it2 == screen_index_to_source_.end()) {
        w.open("screen", false, false);
        w.attribute("name", it->first);
        w.close(true);
      }
      else {
        w.open("screen", false, false);
        w.attribute("name", it->first);
        w.close(false);
        w.write(it2->second);
        w.open("screen", true, true);
      }
    }
    w.newline(-1);
    w.open("screens", true, true);
  }
  if (!module_map_.empty()) {
    w.newline();
    w.open("modules", false, true);
    w.indent(1);
    for (module_map_type::iterator it = module_map_.begin(); it != module_map_.end(); ++it) {
      w.newline();
      w.open("module", false, true);
      w.indent(1);
      it->second->write(w);
      w.newline(-1);
      w.open("module", true, true);
    }
    w.newline(-1);
    w.open("modules", true, true);
  }
  if (!package_map_.empty()) {
    w.newline();
    w.open("packages", false, true);
    w.indent(1);
    for (package_map_type::iterator it = package_map_.begin(); it != package_map_.end(); ++it) {
      w.newline();
      w.open("package", false, true);
      w.indent(1);
      it->second->write(w);
      w.newline(-1);
      w.open("package", true, true);
    }
    w.newline(-1);
    w.open("packages", true, true);
  }
  host_->update(*this);
  target_->update(*this);
}

ga144::Chip_ptr Board::chip(size_t pos)
{
  if (pos == 0) {return host_;}
  return target_;
  //return *chip_list_[pos];
}

f18a::Processor_ptr Board::processor(size_t node)
{
  if (node < GA144_CHIP_OFFSET) {
    return host_->processor(host_->index(node));
  }
  node %= GA144_CHIP_OFFSET;
  return target_->processor(target_->index(node));
}

const f18a::Processor_ptr Board::processor(size_t node) const
{
  if (node < GA144_CHIP_OFFSET) {
    return host_->processor(host_->index(node));
  }
  node %= GA144_CHIP_OFFSET;
  return target_->processor(target_->index(node));
}

const ga144::Chip_ptr Board::split_node(size_t& node) const
{
  if (node < GA144_CHIP_OFFSET) {
    return host_;
  }
  if (node < (GA144_CHIP_OFFSET*2)) {
    node -= GA144_CHIP_OFFSET;
    return target_;
  }
  return 0;
}


long Board::info(size_t node)
{
  ga144::Chip_ptr chip = split_node(node);
  if (chip.is_not_null()) {
    return chip->info(chip->index(node));
  }
  return 0;
}

void Board::info(size_t node, long val)
{
  ga144::Chip_ptr chip = split_node(node);
  if (chip.is_not_null()) {
    chip->info(chip->index(node), val);
  }
}

void Board::set_info(size_t node, ga144::InfoBit::Enum val)
{
  ga144::Chip_ptr chip = split_node(node);
  if (chip.is_not_null()) {
    chip->set_info(chip->index(node), val);
  }
}

ga144::Chip_ptr Board::split_node(size_t& node)
{
  if (node < GA144_CHIP_OFFSET) {
    return host_;
  }
  if (node < (GA144_CHIP_OFFSET*2)) {
    node -= GA144_CHIP_OFFSET;
    return target_;
  }
  return 0;
}

size_t Board::join_node(const ga144::Chip_ptr chip, size_t node) const
{
  if (chip == host_) {return node;}
  if (chip == target_) {return node+GA144_CHIP_OFFSET;}
  return invalid_node;
}

void Board::test_echo()
{
}

void Board::clean()
{
}

bool Board::connected(Port::Enum p) const
{
  if (has_connection(p)) {
    return connection(p)->connected();
  }
  return false;
}

void Board::load_node_modules(ga144::Chip_ptr chip, f18a::Processor_ptr proc, module_map_type& map)
{
  size_t limit = proc->config_size();
  for (size_t i=0; i<limit; ++i) {
    f18a::Module_ptr mod = proc->config(i);
    if (!mod->name().empty()) {
      module_map_type::iterator it = map.find(mod->name());
      evb::Module_ptr module;
      if (it == map.end()) {
        // new module
        module = new evb::Module(*this, mod->name());
        map[mod->name()] = module;
      }
      else {
        module = it->second;
      }
      size_t node = proc->node() + chip->offset();
      module->add(node, mod);
    }
  }
}

void Board::load_chip_modules(ga144::Chip_ptr chip, module_map_type& map)
{
  size_t limit = chip->size();
  for (size_t i=0; i<limit; ++i) {
    load_node_modules(chip, chip->processor(i), map);
  }
}

void Board::update_modules()
{
  module_map_type map;
  load_chip_modules(host_, map);
  load_chip_modules(target_, map);
  map.swap(module_map_);
  modify();
}

evb::Module_ptr Board::find_module(const std::string& name)
{
  module_map_type::iterator it = module_map_.find(name);
  if (it != module_map_.end()) {
    return it->second;
  }
  return Module_ptr();
}

evb::Module_ptr Board::add_module(const std::string& name)
{
  module_map_type::iterator it = module_map_.find(name);
  if (it != module_map_.end()) {
    return it->second;
  }
  Module_ptr res = new Module(*this, name);
  module_map_[name] = res;
  return res;
}


evb::Module_ptr Board::rename_module(const std::string& old_name, const std::string& new_name)
{
  Module_ptr res;
  module_map_type::iterator it = module_map_.find(old_name);
  if (it != module_map_.end()) {
    it->second->rename(new_name);
    res = it->second;
    module_map_.erase(it);
    module_map_[new_name] = res;
  }
  return res;
}

evb::Package_ptr Board::find_package(const std::string& name)
{
  package_map_type::iterator it = package_map_.find(name);
  if (it != package_map_.end()) {
    return it->second;
  }
  return Package_ptr();
}

evb::Package_ptr Board::add_package(const std::string& name)
{
  package_map_type::iterator it = package_map_.find(name);
  if (it != package_map_.end()) {
    return it->second;
  }
  Package_ptr res = new Package(*this, name);
  package_map_[name] = res;
  return res;
}

bool Board::remove_package(const std::string& name)
{
  package_map_type::iterator it = package_map_.find(name);
  if (it != package_map_.end()) {
    package_map_.erase(it);
    return true;
  }
  return false;
}

void Board::set_package(Package_ptr ptr)
{
  package_map_[ptr->name()] = ptr;
}


evb::Package_ptr Board::rename_package(const std::string& old_name, const std::string& new_name)
{
  Package_ptr res;
  package_map_type::iterator it = package_map_.find(old_name);
  if (it != package_map_.end()) {
    it->second->rename(new_name);
    res = it->second;
    package_map_.erase(it);
    package_map_[new_name] = res;
  }
  return res;
}

void Board::step()
{
  tic();
  toc();
}

void Board::tic()
{
  host_->tic();
  target_->tic();
}

void Board::toc()
{
  host_->toc();
  target_->toc();
}

void Board::get_path(
  BootFramePath::Enum p,
  size_t chip_offset,
  f18a::port_list_type& pl,
  node_list_type& path
)
{
  size_t curr;
  pl.clear();
  path.clear();

  switch (p) {
  case BootFramePath::via_300: // boot frame loaded via node 300 into target chip
    path.push_back(chip_offset+300);
    curr = 400; while (curr <= 700) {path.push_back(chip_offset+curr); curr += 100;}
    curr = 701; while (curr <= 717) {path.push_back(chip_offset+curr++);}
    curr = 617; while (curr >= 601) {path.push_back(chip_offset+curr--);}
    curr = 501; while (curr <= 517) {path.push_back(chip_offset+curr++);}
    curr = 417; while (curr >= 401) {path.push_back(chip_offset+curr--);}
    curr = 301; while (curr <= 317) {path.push_back(chip_offset+curr++);}
    curr = 217; while (curr >= 200) {path.push_back(chip_offset+curr--);}
    curr = 100; while (curr <= 117) {path.push_back(chip_offset+curr++);}
    curr = 17; while (curr >= 0) {path.push_back(chip_offset+curr--);}
    break;

  case BootFramePath::for_300: // boot frame for node 300 on the target chip
    path.push_back(300 + chip_offset);
    break;

  case BootFramePath::for_300_bridge: // boot frame containing bridge code for node 10300
    path.push_back(300 + GA144_CHIP_OFFSET);
    break;

  case BootFramePath::via_705:
    path.push_back(chip_offset+705);
    curr = 704; while (curr >= 700) {path.push_back(chip_offset+curr--);}
    curr = 600; while (curr <= 616) {path.push_back(chip_offset+curr++);}
    curr = 516; while (curr >= 500) {path.push_back(chip_offset+curr--);}
    curr = 400; while (curr <= 416) {path.push_back(chip_offset+curr++);}
    curr = 316; while (curr >= 300) {path.push_back(chip_offset+curr--);}
    curr = 200; while (curr <= 216) {path.push_back(chip_offset+curr++);}
    curr = 116; while (curr >= 100) {path.push_back(chip_offset+curr--);}
    curr =   0; while (curr <=  16) {path.push_back(chip_offset+curr++);}
    curr =  17; while (curr <= 617) {path.push_back(chip_offset+curr); curr += 100;}
    curr = 717; while (curr >= 706) {path.push_back(chip_offset+curr--);}
    break;

  case BootFramePath::via_705_to_300_bridge:
    path.push_back(chip_offset+705);
    curr = 704; while (curr >= 700) {path.push_back(chip_offset+curr--);}
    curr = 600; while (curr >= 500) {path.push_back(chip_offset+curr); curr -= 100;}
    break;

  case BootFramePath::via_705_remaining_chip:
    path.push_back(chip_offset+705);
    curr = 706; while (curr <= 717) {path.push_back(chip_offset+curr++);}
    curr = 617; while (curr >= 601) {path.push_back(chip_offset+curr--);}
    curr = 501; while (curr <= 517) {path.push_back(chip_offset+curr++);}
    curr = 417; while (curr >= 401) {path.push_back(chip_offset+curr--);}
    curr = 301; while (curr <= 317) {path.push_back(chip_offset+curr++);}
    curr = 217; while (curr >= 200) {path.push_back(chip_offset+curr--);}
    curr = 100; while (curr <= 117) {path.push_back(chip_offset+curr++);}
    curr = 17; while (curr >= 0) {path.push_back(chip_offset+curr--);}
    break;

  case BootFramePath::for_705:
    path.push_back(chip_offset+705);
    break;

  case BootFramePath::via_708:
    path.push_back(chip_offset+708);
    //curr = 707; while (curr >= 706) {path.push_back(chip_offset+curr--);}
    curr = 707; while (curr >= 700) {path.push_back(chip_offset+curr--);}
    curr = 600; while (curr <= 616) {path.push_back(chip_offset+curr++);}
    curr = 516; while (curr >= 500) {path.push_back(chip_offset+curr--);}
    curr = 400; while (curr <= 416) {path.push_back(chip_offset+curr++);}
    curr = 316; while (curr >= 300) {path.push_back(chip_offset+curr--);}
    curr = 200; while (curr <= 216) {path.push_back(chip_offset+curr++);}
    curr = 116; while (curr >= 100) {path.push_back(chip_offset+curr--);}
    curr =   0; while (curr <=  16) {path.push_back(chip_offset+curr++);}
    curr =  17; while (curr <= 617) {path.push_back(chip_offset+curr); curr += 100;}
    curr = 717; while (curr >= 709) {path.push_back(chip_offset+curr--);}
    break;

  case BootFramePath::via_708_to_300_bridge:
    path.push_back(chip_offset+708);
    curr = 707; while (curr >= 700) {path.push_back(chip_offset+curr--);}
    curr = 600; while (curr >= 300) {path.push_back(chip_offset+curr); curr -= 100;}
    break;

  case BootFramePath::via_708_remaining_chip:
    path.push_back(chip_offset+708);
    curr = 709; while (curr <= 717) {path.push_back(chip_offset+curr++);}
    curr = 617; while (curr >= 601) {path.push_back(chip_offset+curr--);}
    curr = 501; while (curr <= 517) {path.push_back(chip_offset+curr++);}
    curr = 417; while (curr >= 401) {path.push_back(chip_offset+curr--);}
    curr = 301; while (curr <= 317) {path.push_back(chip_offset+curr++);}
    curr = 217; while (curr >= 200) {path.push_back(chip_offset+curr--);}
    curr = 100; while (curr <= 117) {path.push_back(chip_offset+curr++);}
    curr = 17; while (curr >= 0) {path.push_back(chip_offset+curr--);}
    break;

  case BootFramePath::for_708:
    path.push_back(chip_offset+708);
    break;
  }

  if (path.size() > 1) {
    for (size_t i=1; i<path.size(); ++i) {
      pl.push_back(ga144::Chip::port_between(path[i-1], path[i]));
    }
  }

}


void Board::create_boot_frame(
  BootFramePath::Enum path_type,
  BootCodeType::Enum code_type,
  f18a::cell final_jump_adr,
  size_t chip_offset,
  Package_ptr package,
  const f18a::cell_list_type& additional_data,
  f18a::cell_list_type& frame)
{
  node_list_type path;
  f18a::port_list_type port_list;
  size_t curr;
  ga144::Chip_ptr chip;
  bool use_bridge = (path_type == BootFramePath::via_705_to_300_bridge) || (path_type == BootFramePath::via_708_to_300_bridge);
  bool special_code_500_to_300 = (path_type == BootFramePath::via_705_to_300_bridge) || (path_type == BootFramePath::via_708_to_300_bridge);

  get_path(path_type, chip_offset, port_list, path);
  if (path.empty()) {
    return;
  }

  switch (path_type) {
  case BootFramePath::via_300: // boot frame loaded via node 300 into target chip
    break;

  case BootFramePath::for_300: // boot frame for node 300 on the target chip
    break;

  case BootFramePath::for_300_bridge: // boot frame containing bridge code for node 10300
    break;

  case BootFramePath::via_705:
    break;

  case BootFramePath::via_705_to_300_bridge:
    use_bridge = true;
    special_code_500_to_300 = true;
    break;

  case BootFramePath::via_705_remaining_chip:
    break;

  case BootFramePath::for_705:
    break;

  case BootFramePath::via_708:
    break;

  case BootFramePath::via_708_to_300_bridge:
    use_bridge = true;
    special_code_500_to_300 = true;
    break;

  case BootFramePath::via_708_remaining_chip:
    break;

  case BootFramePath::for_708:
    break;
  }

  // build package node map
  typedef std::map<size_t, f18a::Module_ptr> node_module_map_type;
  node_module_map_type module_map;
  node_module_map_type::iterator it;
  if (package.is_not_null()) {
    // read all modules
    size_t i, limit = package->size();
    for (i=0; i<limit; ++i) {
      Module_ptr module = package->module(i);
      if (module.is_not_null()) {
        size_t module_size = module->size();
        for (size_t j=0; j<module_size; ++j) {
          f18a::Module_ptr mod = module->module(j);
          if (mod.is_not_null()) {
            module_map[module->node(j)] = mod;
          }
        }
      }
    }
    // remove excluded nodes
    const evb::node_list_type& excl_list = package->exclude_list();
    limit = excl_list.size();
    for (i=0; i<limit; ++i) {
      it = module_map.find(excl_list[i]);
      if (it != module_map.end()) {
        module_map.erase(it);
      }
    }
  }
  size_t path_size = path.size();
  if (path_size == 1) {
    curr = path[0];
    it = module_map.find(curr);
    if (it == module_map.end()) {
      frame.push_back(final_jump_adr);
      frame.push_back(0);
      frame.push_back(0);
    }
    else {
      it->second->append_init_node(frame, true, final_jump_adr);
    }
    return;
  }
  //path_size = 3;

#if 1

  f18a::cell_list_type curr_code;
  f18a::cell_list_type next_code;
  size_t i;

  switch (code_type) {
  case BootCodeType::default_code:
    break;

  case BootCodeType::chain_via_a:
    curr_code.push_back(f18a::State::focusing_jump(port_list[path_size-2]));
    it = module_map.find(path[path_size-1]);
    if (it != module_map.end()) {
      it->second->append_init_node(curr_code, false);
    }
    for (i=path_size-2; i>0; --i) {
      next_code.clear();
      if (special_code_500_to_300 &&
        (
          path[i] == 400
          || path[i] == 300
          || path[i] == 10300
        )
      ) {
        if (path[i] == 400) {
          // drive reset line low
          // initialize node 300
          // drive reset line high
          // initialize bridge

        }
        else { // ignore code for 300 and 10300
        }
      }
      else {
        next_code.push_back(f18a::State::focusing_jump(port_list[i-1]));
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_a));
        next_code.push_back(f18a::Port::to_addr(port_list[i]));
        //
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
        next_code.push_back(curr_code.size()-1);
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop));
        next_code.insert(next_code.end(), curr_code.begin(), curr_code.end());
        //
        it = module_map.find(path[i]);
        if (it != module_map.end()) {
          it->second->append_init_node(next_code, false);
        }
        curr_code = next_code;
      }
    }
    // construct frame for final node
    frame.push_back(final_jump_adr);
    frame.push_back(f18a::Port::to_addr(port_list[0]));
    frame.push_back(curr_code.size());
    frame.insert(frame.end(), curr_code.begin(), curr_code.end());
    break;

  case BootCodeType::chain_via_b:
    curr_code.push_back(f18a::State::focusing_jump(port_list[path_size-2]));
    it = module_map.find(path[path_size-1]);
    if (it != module_map.end()) {
      it->second->append_init_node(curr_code, false);
    }
    for (i=path_size-2; i>0; --i) {
      next_code.clear();
      next_code.push_back(f18a::State::focusing_jump(port_list[i-1]));
      next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_b));
      next_code.push_back(f18a::Port::to_addr(port_list[i]));
      //
      next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
      next_code.push_back(curr_code.size()-1);
      next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop_b));
      next_code.insert(next_code.end(), curr_code.begin(), curr_code.end());
      //
      it = module_map.find(path[i]);
      if (it != module_map.end()) {
        it->second->append_init_node(next_code, false);
      }
      //
      curr_code = next_code;
    }
    // construct frame for final node
    frame.push_back(final_jump_adr);
    frame.push_back(f18a::Port::to_addr(port_list[0]));
    frame.push_back(curr_code.size());
    frame.insert(frame.end(), curr_code.begin(), curr_code.end());
    break;
  }

#else
  size_t pos = path.size() - 1;
  f18a::Port::Enum a_port;
  f18a::Port::Enum p_port;
  f18a::cell_list_type curr_code;
  f18a::cell_list_type next_code;
  curr = path[pos];
  chip = split_node(curr);
  it = module_map.find(path[pos]);





  if (special_code_500_to_300) {
    // build special code for node 500
    f18a::cell_list_type node_300_prepare_code;
    f18a::cell_list_type node_300_boot_code;
    f18a::cell_list_type node_300_transfer_code;
    f18a::cell_list_type node_400_prepare_code;
    f18a::cell_list_type node_400_boot_code;
      // prepare code for node 300
        // focus
    node_300_prepare_code.push_back(f18a::State::focusing_jump(f18a::Port::up));
       // finish focus
    node_300_prepare_code.push_back(f18a::State::addr_to_jump(chip->processor(chip->index(300))->warm()));

        // boot code to be transmitted to target chip
    node_300_boot_code.push_back(additional_data.size()-1);
    node_300_boot_code.insert(node_300_boot_code.end(), additional_data.begin(), additional_data.end());

      // prepare code for node 400
        // focus
    node_400_prepare_code.push_back(f18a::State::focusing_jump(f18a::Port::down));
        // to node 300
    node_400_prepare_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_a));
    node_400_prepare_code.push_back(f18a::Port::to_addr(f18a::Port::up));
        // send prepare code to node 300
    node_400_prepare_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
    node_400_prepare_code.push_back(node_300_prepare_code.size()-1);
    node_400_prepare_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop));
    node_400_prepare_code.insert(node_400_prepare_code.end(), node_300_prepare_code.begin(), node_300_prepare_code.end());
       // finish focus
    node_400_prepare_code.push_back(f18a::State::addr_to_jump(chip->processor(chip->index(400))->warm()));


      // boot code for node 400
        // focus
    node_400_boot_code.push_back(f18a::State::focusing_jump(f18a::Port::down));
        // to node 300
    node_400_boot_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_a));
    node_400_boot_code.push_back(f18a::Port::to_addr(f18a::Port::up));
        // send code to node 300
    node_400_boot_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
    node_400_boot_code.push_back(node_300_prepare_code.size()-1);
    node_400_boot_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop));
    node_400_boot_code.insert(node_400_boot_code.end(), node_300_boot_code.begin(), node_300_boot_code.end());
       // finish focus
    node_400_boot_code.push_back(f18a::State::addr_to_jump(chip->processor(chip->index(400))->warm()));

      // focusing jump for node 500 (must be the first action)
    next_code.push_back(f18a::State::focusing_jump(f18a::Port::up));
      // reset target chip
    next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::store_b));
    next_code.push_back(F18A_PIN17_LO);
      // prepare node 400
    next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_a));
    next_code.push_back(f18a::Port::to_addr(f18a::Port::down));
      // send prepare code to node 400
    next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
    next_code.push_back(node_400_prepare_code.size()-1);
    next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop));
    next_code.insert(next_code.end(), node_400_prepare_code.begin(), node_400_prepare_code.end());
      // return reset line to high
    next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::store_b));
    next_code.push_back(F18A_PIN17_HI);
      // send boot code to node 400
    next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
    next_code.push_back(node_400_boot_code.size()-1);
    next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop));
    next_code.insert(next_code.end(), node_400_boot_code.begin(), node_400_boot_code.end());
      // finish focus
    next_code.push_back(f18a::State::addr_to_jump(chip->processor(chip->index(500))->warm()));
  }
  else {
    if (it == module_map.end()) {
      append_default_init_code(chip, path[pos], curr_code, false);
    } // default initialisation
    else {
      it->second->append_init_node(curr_code, false);
    }
  }
  if (pos > 1) {
    --pos;
    size_t prev_node;
    size_t curr_node = path[pos];
    size_t next_node = path[pos+1];
    while (pos > 0) {
      prev_node = path[pos-1];
      // path buildup
      a_port = ga144::Chip::port_between(next_node, curr_node);
      p_port = ga144::Chip::port_between(curr_node, prev_node);
      switch (code_type) {

      case BootCodeType::default_code:
          // focusing jump  (must be the first action)
        next_code.push_back(f18a::State::focusing_jump(p_port));
          // set a
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_a));
        next_code.push_back(f18a::Port::to_addr(a_port));
        // data pump
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
        next_code.push_back(curr_code.size()-1);
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop));
        next_code.insert(next_code.end(), curr_code.begin(), curr_code.end());
        // node initialization
        it = module_map.find(curr_node);
        if (it == module_map.end()) {
          // default initialisation
          append_default_init_code(chip, path[pos], next_code, false);
        }
        else {
          // module initialisation
          it->second->append_init_node(next_code, false);
        }
        break;

      case BootCodeType::chain_via_a:
          // focusing jump  (must be the first action)
        next_code.push_back(f18a::State::focusing_jump(p_port));
          // set a
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_a));
        next_code.push_back(f18a::Port::to_addr(a_port));
        // data pump
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
        next_code.push_back(curr_code.size()-1);
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop));
        next_code.insert(next_code.end(), curr_code.begin(), curr_code.end());
        it = module_map.find(curr_node);
        if (it != module_map.end()) {
          it->second->append_init_node(next_code, false);
        }
        break;

      case BootCodeType::chain_via_b:
          // focusing jump  (must be the first action)
        next_code.push_back(f18a::State::focusing_jump(p_port));
          // set a
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_b));
        next_code.push_back(f18a::Port::to_addr(a_port));
        // data pump
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_rstack));
        next_code.push_back(curr_code.size()-1);
        next_code.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_mem_loop_b));
        next_code.insert(next_code.end(), curr_code.begin(), curr_code.end());
        it = module_map.find(curr_node);
        if (it != module_map.end()) {
          it->second->append_init_node(next_code, false);
        }
        break;

      default:;
      }
      // finish iteration
      curr_code.swap(next_code);
      next_code.clear();
      next_node = curr_node;
      curr_node = prev_node;
      --pos;
    }
  }
  f18a::cell_list_type pre, post;
  // finally we must build the boot node
  frame.push_back(final_jump_adr); // cold ist the jump address
  frame.push_back(f18a::Port::to_addr(p_port)); // destination to first node
  frame.push_back(curr_code.size() + additional_data.size()); // destination to first node
  frame.insert(frame.end(), curr_code.begin(), curr_code.end());
  if (!additional_data.empty()) {
    frame.insert(frame.end(), additional_data.begin(), additional_data.end());
  }
#endif
}


void Board::create_build_path_stream(boot::Stream& str)
{
  f18a::Port::Enum port;
  //f18a::cell adr;
  f18a::cell_list_type focus;
  focus.push_back(evb::Manager::remote_op(evb::Manager::RemoteOp::write_a));
  focus.push_back(0);
  focus.push_back(0);
  switch (str.path()) {
  case boot::Path::chip_via_300: {
  }
  break;

  case boot::Path::chip_via_705: {
  }
  break;

  case boot::Path::chip_via_708: {
    boot::PathHelper pb(708);
    while (pb.current_node() != 700) {
      port = pb.step(ga144::Neighbour::west);
      focus[1] = f18a::Port::to_addr(port);
    }
  }
  break;

  default:;
  }

}

void Board::create_boot_stream(boot::Stream& str, Package_ptr package)
{

  typedef std::map<size_t, f18a::Module_ptr> node_module_map_type;
  node_module_map_type module_map;
  if (package.is_not_null()) {
    // read all modules
    size_t package_size = package->size();
    for (size_t i=0; i<package_size; ++i) {
      Module_ptr module = package->module(i);
      if (module.is_not_null()) {
        size_t module_size = module->size();
        for (size_t j=0; j<module_size; ++j) {
          f18a::Module_ptr mod = module->module(j);
          if (mod.is_not_null()) {
            module_map[module->node(j)] = mod;
          }
        }
      }
    }
  }

  f18a::cell_list_type frame1, frame2, additional_data;

  f18a::cell frame1_jump_adr = 0xaa;
  f18a::cell frame2_jump_adr = 0xa9;
  node_module_map_type::iterator it;

  switch (str.path()) {
  case boot::Path::chip_via_300: {
  }
  break;

  case boot::Path::chip_via_705: {
  }
  break;

  case boot::Path::chip_via_708: {
    it = module_map.find(708);
    if (it != module_map.end()) {
      frame2_jump_adr = it->second->reg().named_.P_;
    }
    create_boot_frame(BootFramePath::via_708, BootCodeType::chain_via_a, frame1_jump_adr, 0, package, additional_data, frame1);
    create_boot_frame(BootFramePath::for_708, BootCodeType::chain_via_a, frame2_jump_adr, 0, package, additional_data, frame2);
  }
  break;

  default:;
  }
  str.append(frame1);
  str.append(frame2);
}

void Board::append_default_init_code(ga144::Chip_ptr chip, size_t node, f18a::cell_list_type& code, bool is_boot_frame)
{
  f18a::cell adr = chip->processor(chip->index(node))->warm();
  if (is_boot_frame) {
    code.push_back(adr);
    code.push_back(0);
    code.push_back(0);
  }
  else {
    code.push_back(f18a::State::addr_to_jump(adr));
  }
}

//void Board::append_default_boot_stream(ga144::Chip_ptr chip, size_t node, f18a::cell_list_type& code)
//{
//  f18a::cell adr = chip->processor(chip->index(node))->warm();
//  code.push_back(adr);
//  code.push_back(0);
//  code.push_back(0);
//}


// ======================================================================================


bool PinType::to_string(Enum val, std::string& txt)
{
  switch (val) {
  case io_1: txt = "I/O 1"; return true;
  case io_3: txt = "I/O 3"; return true;
  case io_5: txt = "I/O 5"; return true;
  case io_17: txt = "I/O 17"; return true;
  case analog_in: txt = "analog in"; return true;
  case analog_out: txt = "analog out"; return true;
  case data: txt = "data"; return true;
  case address: txt = "address"; return true;
  default:;
  }
  return false;
}

bool PinType::to_description(Enum val, std::string& txt)
{
  switch (val) {
  case io_1: txt = "general purpose digital I/O"; return true;
  case io_3: txt = "general purpose digital I/O"; return true;
  case io_5: txt = "general purpose digital I/O"; return true;
  case io_17: txt = "general purpose digital I/O"; return true;
  case analog_in: txt = "analog input"; return true;
  case analog_out: txt = "analog output"; return true;
  case data: txt = "16 bit data I/O"; return true;
  case address: txt = "16 bit address"; return true;
  default:;
  }
  return false;
}

// ======================================================================================

bool PinStatus::to_string(Enum val, std::string& txt)
{
  switch (val) {
  case undefined: txt = "?"; return true;
  case hi_z: txt = "din"; return true;
  case weak_lo: txt = "w-lo"; return true;
  case lo: txt = "lo"; return true;
  case hi: txt = "hi"; return true;
  case analog_input: txt = "a-in"; return true;
  case vdd_calibration: txt = "a-vdd-cal"; return true;
  case analog_disabled: txt = "a-disabled"; return true;
  case vss_calibration: txt = "a-vss-cal"; return true;
  default:;
  }
  return false;
}

bool PinStatus::to_description(Enum val, std::string& txt)
{
  switch (val) {
  case undefined: txt = "status is undefined"; return true;
  case hi_z: txt = "high impedance (tristate)"; return true;
  case weak_lo: txt = "weak pulldown (47 kOhm)"; return true;
  case lo: txt = "Lo: sink <= 40 mA to Vss"; return true;
  case hi: txt = "Hi: source <= 40 mA from Vdd"; return true;
  case analog_input: txt = "high impedance input"; return true;
  case vdd_calibration: txt = "Vdd calibration"; return true;
  case analog_disabled: txt = "counter disabled"; return true;
  case vss_calibration: txt = "Vss calibration"; return true;
  default:;
  }
  return false;
}

// ======================================================================================

bool Pin::to_string(Enum val, std::string& txt)
{
  switch (val) {
  case h_001_1: txt = "001.1"; return true;      // SERDES data
  case h_001_17: txt = "001.17"; return true;     // SERDES clock
  case h_100_17: txt = "100.17"; return true;     // 1-pin GPIO node
  case h_117_ai: txt = "117.ai"; return true;     // analog input
  case h_117_ao: txt = "117.ao"; return true;     // analog output
  case h_200_17: txt = "200.17"; return true;     // 1-pin GPIO node
  case h_217_17: txt = "217.17"; return true;     // 1-pin GPIO node
  case h_300_1: txt = "300.1"; return true;      // sync data
  case h_300_17: txt = "300.17"; return true;     // sync clock
  case h_317_17: txt = "317.17"; return true;     // 1-pin GPIO node
  case h_417_17: txt = "417.17"; return true;     // 1-pin GPIO node
  case h_500_17: txt = "500.17"; return true;     // 1-pin GPIO node
  case h_517_17: txt = "517.17"; return true;     // 1-pin GPIO node
  case h_517_ai: txt = "517.ai"; return true;     // analog input
  case h_517_ao: txt = "517.a0"; return true;     // analog output
  case h_600_17: txt = "600.17"; return true;     // 1-pin GPIO node
  case h_617_ai: txt = "617.ai"; return true;     // analog input
  case h_617_ao: txt = "617.ao"; return true;     // analog output
  case h_701_1: txt = "701.1"; return true;      // SERDES data
  case h_701_17: txt = "701.17"; return true;     // SERDES clock
  case h_705_1: txt = "705.1"; return true;      // SPI clock
  case h_705_3: txt = "705.3"; return true;      // SPI chip enable
  case h_705_5: txt = "705.5"; return true;      // SPI data out
  case h_705_17: txt = "705.17"; return true;     // SPI data in
  case h_708_1: txt = "708.1"; return true;      // 
  case h_708_17: txt = "708.17"; return true;     // 
  case h_709_ai: txt = "709.ai"; return true;     // analog input
  case h_709_ao: txt = "709.ao"; return true;     // analog output
  case h_713_ai: txt = "713.ai"; return true;     // analog input
  case h_713_ao: txt = "713.ao"; return true;     // analog output
  case h_715_17: txt = "715.17"; return true;     // 1-pin GPIO node
  case h_717_ai: txt = "717.ai"; return true;     // analog input
  case h_717_ao: txt = "717.ao"; return true;     // analog output
  case t_001_1: txt = "001.1"; return true;      // SERDES data
  case t_001_17: txt = "001.17"; return true;     // SERDES clock
  case t_100_17: txt = "100.17"; return true;     // 1-pin GPIO node
  case t_117_ai: txt = "117.ai"; return true;     // analog input
  case t_117_ao: txt = "117.ao"; return true;     // analog output
  case t_200_17: txt = "200.17"; return true;     // 1-pin GPIO node
  case t_217_17: txt = "217.17"; return true;     // 1-pin GPIO node
  case t_300_1: txt = "300.1"; return true;      // sync data
  case t_300_17: txt = "300.17"; return true;     // sync clock
  case t_317_17: txt = "317.17"; return true;     // 1-pin GPIO node
  case t_417_17: txt = "417.17"; return true;     // 1-pin GPIO node
  case t_500_17: txt = "500.17"; return true;     // 1-pin GPIO node
  case t_517_17: txt = "517.17"; return true;     // 1-pin GPIO node
  case t_517_ai: txt = "517.ai"; return true;     // analog input
  case t_517_ao: txt = "517.ao"; return true;     // analog output
  case t_600_17: txt = "600.17"; return true;     // 1-pin GPIO node
  case t_617_ai: txt = "617.ai"; return true;     // analog input
  case t_617_ao: txt = "617.ao"; return true;     // analog output
  case t_701_1: txt = "701.1"; return true;      // SERDES data
  case t_701_17: txt = "701.17"; return true;     // SERDES clock
  case t_705_1: txt = "705.1"; return true;      // SPI clock
  case t_705_3: txt = "705.3"; return true;      // SPI chip enable
  case t_705_5: txt = "705.5"; return true;      // SPI data out
  case t_705_17: txt = "705.17"; return true;     // SPI data in
  case t_708_1: txt = "708.1"; return true;      // 
  case t_708_17: txt = "708.17"; return true;     // 
  case t_709_ai: txt = "709.ai"; return true;     // analog input
  case t_709_ao: txt = "709.ao"; return true;     // analog output
  case t_713_ai: txt = "713.ai"; return true;     // analog input
  case t_713_ao: txt = "713.ao"; return true;     // analog output
  case t_715_17: txt = "715.17"; return true;     // 1-pin GPIO node
  case t_717_ai: txt = "717.ai"; return true;     // analog input
  case t_717_ao: txt = "717.ao"; return true;     // analog output
  case t_007_0: // data 0
  case t_007_1: // data 1
  case t_007_2: // data 2
  case t_007_3: // data 3
  case t_007_4: // data 4
  case t_007_5: // data 5
  case t_007_6: // data 6
  case t_007_7: // data 7
  case t_007_8: // data 8
  case t_007_9: // data 9
  case t_007_10: // data 10
  case t_007_11: // data 11
  case t_007_12: // data 12
  case t_007_13: // data 13
  case t_007_14: // data 14
  case t_007_15: // data 15
  case t_007_16: // data 16
  case t_007_17: txt = "007.data"; return true;     // data 17
  case t_008_1: txt = "008.1"; return true;      // GPIO 1
  case t_008_3: txt = "008.3"; return true;      // GPIO 3
  case t_008_5: txt = "008.5"; return true;      // GPIO 5
  case t_008_17: txt = "008.17"; return true;     // GPIO 17
  case t_009_0:  // address 0
  case t_009_1:  // address 1
  case t_009_2:  // address 2
  case t_009_3:  // address 3
  case t_009_4:  // address 4
  case t_009_5:  // address 5
  case t_009_6:  // address 6
  case t_009_7:  // address 7
  case t_009_8:  // address 8
  case t_009_9:  // address 9
  case t_009_10: // address 10
  case t_009_11: // address 11
  case t_009_12: // address 12
  case t_009_13: // address 13
  case t_009_14: // address 14
  case t_009_15: // address 15
  case t_009_16: // address 16
  case t_009_17: txt = "009.addr"; return true;     // address 17
  default:;
  }
  return false;
}

bool Pin::to_description(Enum val, std::string& txt)
{
  switch (val) {
  case h_001_1: txt = "SERDES data node 001"; return true;      // SERDES data
  case h_001_17: txt = "SERDES clock node 001"; return true;     // SERDES clock
  case h_100_17: txt = "1-pin GPIO node 100"; return true;     // 1-pin GPIO node
  case h_117_ai: txt = "analog input node 117"; return true;     // analog input
  case h_117_ao: txt = "analog output node 117"; return true;     // analog output
  case h_200_17: txt = "1-pin GPIO node 200"; return true;     // 1-pin GPIO node
  case h_217_17: txt = "1-pin GPIO node 217"; return true;     // 1-pin GPIO node
  case h_300_1: txt = "sync data node 300"; return true;      // sync data
  case h_300_17: txt = "sync clock node 300"; return true;     // sync clock
  case h_317_17: txt = "1-pin GPIO node 317"; return true;     // 1-pin GPIO node
  case h_417_17: txt = "1-pin GPIO node 417"; return true;     // 1-pin GPIO node
  case h_500_17: txt = "1-pin GPIO node 500"; return true;     // 1-pin GPIO node
  case h_517_17: txt = "1-pin GPIO node 517"; return true;     // 1-pin GPIO node
  case h_517_ai: txt = "analog input node 517"; return true;     // analog input
  case h_517_ao: txt = "analog output node 517"; return true;     // analog output
  case h_600_17: txt = "1-pin GPIO node 600"; return true;     // 1-pin GPIO node
  case h_617_ai: txt = "analog input node 617"; return true;     // analog input
  case h_617_ao: txt = "analog output node 617"; return true;     // analog output
  case h_701_1: txt = "SERDES data node 701"; return true;      // SERDES data
  case h_701_17: txt = "SERDES clock node 701"; return true;     // SERDES clock
  case h_705_1: txt = "SPI clock node 705"; return true;      // SPI clock
  case h_705_3: txt = "SPI chip enable node 705"; return true;      // SPI chip enable
  case h_705_5: txt = "SPI data out node 705"; return true;      // SPI data out
  case h_705_17: txt = "SPI data in node 705"; return true;     // SPI data in
  case h_708_1: txt = "async TX out"; return true;      // 
  case h_708_17: txt = "async RX in"; return true;     // 
  case h_709_ai: txt = "analog input node 709"; return true;     // analog input
  case h_709_ao: txt = "analog output node 709"; return true;     // analog output
  case h_713_ai: txt = "analog input node 713"; return true;     // analog input
  case h_713_ao: txt = "analog output node 713"; return true;     // analog output
  case h_715_17: txt = "1-pin GPIO node 715"; return true;     // 1-pin GPIO node
  case h_717_ai: txt = "analog input node 717"; return true;     // analog input
  case h_717_ao: txt = "analog output node 717"; return true;     // analog output
  case t_001_1: txt = "target SERDES data node 10001"; return true;      // SERDES data
  case t_001_17: txt = "target SERDES clock node 10001"; return true;     // SERDES clock
  case t_100_17: txt = "target 1-pin GPIO node 10100"; return true;     // 1-pin GPIO node
  case t_117_ai: txt = "target analog input node 10117"; return true;     // analog input
  case t_117_ao: txt = "target analog output node 10117"; return true;     // analog output
  case t_200_17: txt = "target 1-pin GPIO node 10200"; return true;     // 1-pin GPIO node
  case t_217_17: txt = "target 1-pin GPIO node 10217"; return true;     // 1-pin GPIO node
  case t_300_1: txt = "target sync data node 10300"; return true;      // sync data
  case t_300_17: txt = "target sync clock node 10300"; return true;     // sync clock
  case t_317_17: txt = "target 1-pin GPIO node 10317"; return true;     // 1-pin GPIO node
  case t_417_17: txt = "target 1-pin GPIO node 10417"; return true;     // 1-pin GPIO node
  case t_500_17: txt = "target 1-pin GPIO node 10500"; return true;     // 1-pin GPIO node
  case t_517_17: txt = "target 1-pin GPIO node 10517"; return true;     // 1-pin GPIO node
  case t_517_ai: txt = "target analog input node 10517"; return true;     // analog input
  case t_517_ao: txt = "target analog output node 10517"; return true;     // analog output
  case t_600_17: txt = "target 1-pin GPIO node 10600"; return true;     // 1-pin GPIO node
  case t_617_ai: txt = "target analog input node 10617"; return true;     // analog input
  case t_617_ao: txt = "target analog output node 10617"; return true;     // analog output
  case t_701_1: txt = "target SERDES data node 10701"; return true;      // SERDES data
  case t_701_17: txt = "target SERDES clock node 10701"; return true;     // SERDES clock
  case t_705_1: txt = "target SPI clock node 10705"; return true;      // SPI clock
  case t_705_3: txt = "target chip enable node 10705"; return true;      // SPI chip enable
  case t_705_5: txt = "target SPI data out node 10705"; return true;      // SPI data out
  case t_705_17: txt = "target SPI data in node 10705"; return true;     // SPI data in
  case t_708_1: txt = "target async TX out"; return true;      // 
  case t_708_17: txt = "target async RX in"; return true;     // 
  case t_709_ai: txt = "target analog input node 10709"; return true;     // analog input
  case t_709_ao: txt = "target analog output node 10709"; return true;     // analog output
  case t_713_ai: txt = "target analog input node 10713"; return true;     // analog input
  case t_713_ao: txt = "target analog output node 10713"; return true;     // analog output
  case t_715_17: txt = "target 1-pin GPIO node 10715"; return true;     // 1-pin GPIO node
  case t_717_ai: txt = "target analog input node 10717"; return true;     // analog input
  case t_717_ao: txt = "target analog output node 10717"; return true;     // analog output
  case t_007_0:  // data 0
  case t_007_1:  // data 1
  case t_007_2:  // data 2
  case t_007_3:  // data 3
  case t_007_4:  // data 4
  case t_007_5:  // data 5
  case t_007_6:  // data 6
  case t_007_7:  // data 7
  case t_007_8:  // data 8
  case t_007_9:  // data 9
  case t_007_10: // data 10
  case t_007_11: // data 11
  case t_007_12: // data 12
  case t_007_13: // data 13
  case t_007_14: // data 14
  case t_007_15: // data 15
  case t_007_16: // data 16
  case t_007_17: txt = "target data bus node 10007"; return true;     // data 17
  case t_008_1: txt = "target control 1 node 10008"; return true;      // GPIO 1
  case t_008_3: txt = "target control 3 node 10008"; return true;      // GPIO 3
  case t_008_5: txt = "target control 5 node 10008"; return true;      // GPIO 5
  case t_008_17: txt = "target control 17 node 10008"; return true;     // GPIO 17
  case t_009_0:  // address 0
  case t_009_1:  // address 1
  case t_009_2:  // address 2
  case t_009_3:  // address 3
  case t_009_4:  // address 4
  case t_009_5:  // address 5
  case t_009_6:  // address 6
  case t_009_7:  // address 7
  case t_009_8:  // address 8
  case t_009_9:  // address 9
  case t_009_10: // address 10
  case t_009_11: // address 11
  case t_009_12: // address 12
  case t_009_13: // address 13
  case t_009_14: // address 14
  case t_009_15: // address 15
  case t_009_16: // address 16
  case t_009_17: txt = "target address bus node 10009"; return true;     // address 17
  default:;
  }
  return false;
}

bool Pin::host(Enum val)
{
  return (val >= h_001_1) && (val <= h_717_ao);
}
  
bool Pin::target(Enum val)
{
  return (val >= t_001_1) && (val <= t_009_17);
}

PinType::Enum Pin::type(Enum val)
{
  switch (val) {
  case h_001_1: return PinType::io_1;      // SERDES data
  case h_001_17: return PinType::io_17;     // SERDES clock
  case h_100_17: return PinType::io_17;     // 1-pin GPIO node
  case h_117_ai: return PinType::analog_in;     // analog input
  case h_117_ao: return PinType::analog_out;     // analog output
  case h_200_17: return PinType::io_17;     // 1-pin GPIO node
  case h_217_17: return PinType::io_17;     // 1-pin GPIO node
  case h_300_1: return PinType::io_1;      // sync data
  case h_300_17: return PinType::io_17;     // sync clock
  case h_317_17: return PinType::io_17;     // 1-pin GPIO node
  case h_417_17: return PinType::io_17;     // 1-pin GPIO node
  case h_500_17: return PinType::io_17;     // 1-pin GPIO node
  case h_517_17: return PinType::io_17;     // 1-pin GPIO node
  case h_517_ai: return PinType::analog_in;     // analog input
  case h_517_ao: return PinType::analog_out;     // analog output
  case h_600_17: return PinType::io_17;     // 1-pin GPIO node
  case h_617_ai: return PinType::analog_in;     // analog input
  case h_617_ao: return PinType::analog_out;     // analog output
  case h_701_1: return PinType::io_1;      // SERDES data
  case h_701_17: return PinType::io_17;     // SERDES clock
  case h_705_1: return PinType::io_1;      // SPI clock
  case h_705_3: return PinType::io_3;      // SPI chip enable
  case h_705_5: return PinType::io_5;      // SPI data out
  case h_705_17: return PinType::io_17;     // SPI data in
  case h_708_1: return PinType::io_1;      // 
  case h_708_17: return PinType::io_17;     // 
  case h_709_ai: return PinType::analog_in;     // analog input
  case h_709_ao: return PinType::analog_out;     // analog output
  case h_713_ai: return PinType::analog_in;     // analog input
  case h_713_ao: return PinType::analog_out;     // analog output
  case h_715_17: return PinType::io_17;     // 1-pin GPIO node
  case h_717_ai: return PinType::analog_in;     // analog input
  case h_717_ao: return PinType::analog_out;     // analog output
  case t_001_1: return PinType::io_1;      // SERDES data
  case t_001_17: return PinType::io_17;     // SERDES clock
  case t_100_17: return PinType::io_17;     // 1-pin GPIO node
  case t_117_ai: return PinType::analog_in;     // analog input
  case t_117_ao: return PinType::analog_out;     // analog output
  case t_200_17: return PinType::io_17;     // 1-pin GPIO node
  case t_217_17: return PinType::io_17;     // 1-pin GPIO node
  case t_300_1: return PinType::io_1;      // sync data
  case t_300_17: return PinType::io_17;     // sync clock
  case t_317_17: return PinType::io_17;     // 1-pin GPIO node
  case t_417_17: return PinType::io_17;     // 1-pin GPIO node
  case t_500_17: return PinType::io_17;     // 1-pin GPIO node
  case t_517_17: return PinType::io_17;     // 1-pin GPIO node
  case t_517_ai: return PinType::analog_in;     // analog input
  case t_517_ao: return PinType::analog_out;     // analog output
  case t_600_17: return PinType::io_17;     // 1-pin GPIO node
  case t_617_ai: return PinType::analog_in;     // analog input
  case t_617_ao: return PinType::analog_out;     // analog output
  case t_701_1: return PinType::io_1;      // SERDES data
  case t_701_17: return PinType::io_17;     // SERDES clock
  case t_705_1: return PinType::io_1;      // SPI clock
  case t_705_3: return PinType::io_3;      // SPI chip enable
  case t_705_5: return PinType::io_5;      // SPI data out
  case t_705_17: return PinType::io_17;     // SPI data in
  case t_708_1: return PinType::io_1;      // 
  case t_708_17: return PinType::io_17;     // 
  case t_709_ai: return PinType::analog_in;     // analog input
  case t_709_ao: return PinType::analog_out;     // analog output
  case t_713_ai: return PinType::analog_in;     // analog input
  case t_713_ao: return PinType::analog_out;     // analog output
  case t_715_17: return PinType::io_17;     // 1-pin GPIO node
  case t_717_ai: return PinType::analog_in;     // analog input
  case t_717_ao: return PinType::analog_out;     // analog output
  case t_007_0: // data 0
  case t_007_1: // data 1
  case t_007_2: // data 2
  case t_007_3: // data 3
  case t_007_4: // data 4
  case t_007_5: // data 5
  case t_007_6: // data 6
  case t_007_7: // data 7
  case t_007_8: // data 8
  case t_007_9: // data 9
  case t_007_10: // data 10
  case t_007_11: // data 11
  case t_007_12: // data 12
  case t_007_13: // data 13
  case t_007_14: // data 14
  case t_007_15: // data 15
  case t_007_16: // data 16
  case t_007_17: return PinType::data;     // data 17
  case t_008_1: return PinType::io_1;      // GPIO 1
  case t_008_3: return PinType::io_3;      // GPIO 3
  case t_008_5: return PinType::io_5;      // GPIO 5
  case t_008_17: return PinType::io_17;     // GPIO 17
  case t_009_0: // address 0
  case t_009_1: // address 1
  case t_009_2: // address 2
  case t_009_3: // address 3
  case t_009_4: // address 4
  case t_009_5: // address 5
  case t_009_6: // address 6
  case t_009_7: // address 7
  case t_009_8: // address 8
  case t_009_9: // address 9
  case t_009_10: // address 10
  case t_009_11: // address 11
  case t_009_12: // address 12
  case t_009_13: // address 13
  case t_009_14: // address 14
  case t_009_15: // address 15
  case t_009_16: // address 16
  case t_009_17: return PinType::address;     // address 17
  default:;
  }
  return PinType::undefined;
}


size_t Pin::node(Enum val)
{
  switch (val) {
  case h_001_1: 
  case h_001_17: return 1;
  case h_100_17: return 100;
  case h_117_ai: 
  case h_117_ao: return 117;
  case h_200_17: return 200;
  case h_217_17: return 217;
  case h_300_1: ;
  case h_300_17: return 300;
  case h_317_17: return 317;
  case h_417_17: return 417;
  case h_500_17: return 500;
  case h_517_17: 
  case h_517_ai: 
  case h_517_ao: return 517;
  case h_600_17: return 600;
  case h_617_ai: 
  case h_617_ao: return 617;
  case h_701_1: return 701;
  case h_701_17: return 701;
  case h_705_1: 
  case h_705_3: 
  case h_705_5: 
  case h_705_17: return 705;
  case h_708_1:  
  case h_708_17: return 708; 
  case h_709_ai: 
  case h_709_ao: return 709;
  case h_713_ai: 
  case h_713_ao: return 713;
  case h_715_17: return 715;
  case h_717_ai: 
  case h_717_ao: return 717;
  case t_001_1: 
  case t_001_17: return 10001;
  case t_100_17: return 10100;
  case t_117_ai:
  case t_117_ao: return 10117;
  case t_200_17: return 10200;
  case t_217_17: return 10217;
  case t_300_1: 
  case t_300_17: return 10300;
  case t_317_17: return 10317;
  case t_417_17: return 10417;
  case t_500_17: return 10500;
  case t_517_17:  
  case t_517_ai:  
  case t_517_ao: return 10517;
  case t_600_17: return 10600;
  case t_617_ai: 
  case t_617_ao: return 10617;
  case t_701_1: 
  case t_701_17: return 10701;
  case t_705_1: 
  case t_705_3: 
  case t_705_5: 
  case t_705_17: return 10705;
  case t_708_1:  
  case t_708_17: return 10708; 
  case t_709_ai: 
  case t_709_ao: return 10709;
  case t_713_ai: 
  case t_713_ao: return 10713;
  case t_715_17: return 10715;
  case t_717_ai: 
  case t_717_ao: return 10717;
  case t_008_1: 
  case t_008_3: 
  case t_008_5: 
  case t_008_17: return 10008;
  case t_007_0: // data 0
  case t_007_1: // data 1
  case t_007_2: // data 2
  case t_007_3: // data 3
  case t_007_4: // data 4
  case t_007_5: // data 5
  case t_007_6: // data 6
  case t_007_7: // data 7
  case t_007_8: // data 8
  case t_007_9: // data 9
  case t_007_10: // data 10
  case t_007_11: // data 11
  case t_007_12: // data 12
  case t_007_13: // data 13
  case t_007_14: // data 14
  case t_007_15: // data 15
  case t_007_16: // data 16
  case t_007_17: return 10007;
  case t_009_0: // address 0
  case t_009_1: // address 1
  case t_009_2: // address 2
  case t_009_3: // address 3
  case t_009_4: // address 4
  case t_009_5: // address 5
  case t_009_6: // address 6
  case t_009_7: // address 7
  case t_009_8: // address 8
  case t_009_9: // address 9
  case t_009_10: // address 10
  case t_009_11: // address 11
  case t_009_12: // address 12
  case t_009_13: // address 13
  case t_009_14: // address 14
  case t_009_15: // address 15
  case t_009_16: // address 16
  case t_009_17: return 10009;
  default:;
  }
  return Enum::limit;
}

// ======================================================================================

bool Connector::has_pin1(Enum val)
{
  switch (val) {
  case J2:
  case J3:
  case J9:
  case J18:
  case J41:
  case J49:
  case J51:
  case J52:
  case J55:
  case J58:
  case J62:
  case J63:
  case J64:
  case J69:
  case J70:
  case J71:
  case J74:
  case J75:
  case JP1:
    return false;

  default:;
  }
  return true;
}

bool Connector::to_string(Enum val, std::string& txt)
{
  switch (val) {
  case J1: txt = "J1"; return true;   // external power
  case J2: txt = "J2"; return true;   // power connector
  case J3: txt = "J3"; return true;   // USB port A
  case J4: txt = "J4"; return true;   // user supply J1.9
  case J5: txt = "J5"; return true;   // user supply J1.7
  case J6: txt = "J6"; return true;   // user supply J1.5
  case J7: txt = "J7"; return true;   // port A Access
  case J8: txt = "J8"; return true;   // port A Access
  case J9: txt = "J9"; return true;   // USB port B
  case J10: txt = "J10"; return true;   // host power select
  case J11: txt = "J11"; return true;   // host power select
  case J12: txt = "J12"; return true;   // port B Access
  case J13: txt = "J13"; return true;   // port B Access
  case J14: txt = "J14"; return true;   // target power select
  case J15: txt = "J15"; return true;   // target power select
  case J16: txt = "J16"; return true;   // target power select
  case J17: txt = "J17"; return true;   // port C Access
  case J18: txt = "J18"; return true;   // USB port C
  case J19: txt = "J19"; return true;   // port C Access
  case J20: txt = "J20"; return true;   // reset and boot
  case J21: txt = "J21"; return true;   // uncommitted host pins
  case J22: txt = "J22"; return true;   // reset and host communication
  case J23: txt = "J23"; return true;   // port data connections to host and target
  case J24: txt = "J24"; return true;   // port B Access
  case J25: txt = "J25"; return true;   // reset and boot
  case J26: txt = "J26"; return true;   // reset and boot
  case J27: txt = "J27"; return true;   // uncommitted host pins
  case J28: txt = "J28"; return true;   // uncommitted target pins
  case J29: txt = "J29"; return true;   // ground pins
  case J30: txt = "J30"; return true;   // uncommitted target pins
  case J31: txt = "J31"; return true;   // uncommitted target pins
  case J32: txt = "J32"; return true;   // uncommitted target pins
  case J33: txt = "J33"; return true;   // SD socket signals
  case J34: txt = "J34"; return true;   // reset and host communication
  case J35: txt = "J35"; return true;   // reset and host communication
  case J36: txt = "J36"; return true;   // uncommitted target pins
  case J37: txt = "J37"; return true;   // SPI bus expansion
  case J38: txt = "J38"; return true;   // SD/MMC socket signals
  case J39: txt = "J39"; return true;   // SPI bus expansion
  case J40: txt = "J40"; return true;   // SD/MMC socket signals
  case J41: txt = "J41"; return true;   // 
  //case J42: txt = "J42"; return true;   // 
  //case J43: txt = "J43"; return true;   // 
  //case J44: txt = "J44"; return true;   // 
  //case J45: txt = "J45"; return true;   // 
  //case J46: txt = "J46"; return true;   // 
  //case J47: txt = "J47"; return true;   // 
  case J48: txt = "J48"; return true;   // 
  case J49: txt = "J49"; return true;   // 
  case J50: txt = "J50"; return true;   // 
  case J51: txt = "J51"; return true;   // 
  case J52: txt = "J52"; return true;   // 
  case J53: txt = "J53"; return true;   // 
  case J54: txt = "J54"; return true;   // 
  case J55: txt = "J55"; return true;   // 
  case J56: txt = "J56"; return true;   // 
  case J57: txt = "J57"; return true;   // 
  case J58: txt = "J58"; return true;   // 
  case J59: txt = "J59"; return true;   // 
  case J60: txt = "J60"; return true;   // 
  case J61: txt = "J61"; return true;   // 
  case J62: txt = "J62"; return true;   // 
  case J63: txt = "J63"; return true;   // 
  case J64: txt = "J64"; return true;   // 
  case J65: txt = "J65"; return true;   // 
  case J66: txt = "J66"; return true;   // 
  case J67: txt = "J67"; return true;   // 
  case J68: txt = "J68"; return true;   // 
  case J69: txt = "J69"; return true;   // 
  case J70: txt = "J70"; return true;   // 
  case J71: txt = "J71"; return true;   // 
  //case J72: txt = "J72"; return true;   // 
  //case J73: txt = "J73"; return true;   // 
  case J74: txt = "J74"; return true;   // 
  case J75: txt = "J75"; return true;   // 
  case J76: txt = "J76"; return true;   // 
  case JP1: txt = "JP1"; return true;   // 
  default:;
  }
  return false;
}


bool Connector::to_description(Enum val, std::string& txt)
{
  switch (val) {
  case J1: txt = "external power"; return true;   // external power
  case J2: txt = "power connector"; return true;   // power connector
  case J3: txt = "USB A connector"; return true;   // USB port A
  case J4: txt = "user supply J1.9"; return true;   // user supply J1.9
  case J5: txt = "user supply J1.7"; return true;   // user supply J1.7
  case J6: txt = "user supply J1.5"; return true;   // user supply J1.5
  case J7: txt = "port A Access"; return true;   // port A Access
  case J8: txt = "port A Access"; return true;   // port A Access
  case J9: txt = "USB B connector"; return true;   // USB port B
  case J10: txt = "host power select"; return true;   // host power select
  case J11: txt = "host power select"; return true;   // host power select
  case J12: txt = " port B Access"; return true;   // port B Access
  case J13: txt = " port B Access"; return true;   // port B Access
  case J14: txt = "target power select"; return true;   // target power select
  case J15: txt = "target power select"; return true;   // target power select
  case J16: txt = "target power select"; return true;   // target power select
  case J17: txt = "port C Access"; return true;   // port C Access
  case J18: txt = "USB C connector"; return true;   // USB port C
  case J19: txt = "port C Access"; return true;   // port C Access
  case J20: txt = "reset and boot"; return true;   // reset and boot
  case J21: txt = "uncommitted host pins"; return true;   // uncommitted host pins
  case J22: txt = "reset and host communication"; return true;   // reset and host communication
  case J23: txt = "port data connections to host and target"; return true;   // port data connections to host and target
  case J24: txt = "port B Access"; return true;   // port B Access
  case J25: txt = "reset and boot"; return true;   // reset and boot
  case J26: txt = "reset and boot"; return true;   // reset and boot
  case J27: txt = "uncommitted host pins"; return true;   // uncommitted host pins
  case J28: txt = "uncommitted target pins"; return true;   // uncommitted target pins
  case J29: txt = "ground pins"; return true;   // ground pins
  case J30: txt = "uncommitted target pins"; return true;   // uncommitted target pins
  case J31: txt = "uncommitted target pins"; return true;   // uncommitted target pins
  case J32: txt = "uncommitted target pins"; return true;   // uncommitted target pins
  case J33: txt = "SD socket signals"; return true;   // SD socket signals
  case J34: txt = "reset and host communication"; return true;   // reset and host communication
  case J35: txt = "reset and host communication"; return true;   // reset and host communication
  case J36: txt = "uncommitted target pins"; return true;   // uncommitted target pins
  case J37: txt = "SPI bus expansion"; return true;   // SPI bus expansion
  case J38: txt = "SD/MMC socket signals"; return true;   // SD/MMC socket signals
  case J39: txt = "SPI bus expansion"; return true;   // SPI bus expansion
  case J40: txt = "SD/MMC socket signals"; return true;   // SD/MMC socket signals
  case J41: txt = "SMA RF connector"; return true;   // 
  //case J42: txt = ""; return true;   // 
  //case J43: txt = ""; return true;   // 
  //case J44: txt = ""; return true;   // 
  //case J45: txt = ""; return true;   // 
  //case J46: txt = ""; return true;   // 
  //case J47: txt = ""; return true;   // 
  case J48: txt = "SMA RF pins"; return true;   // 
  case J49: txt = "SMA RF connector"; return true;   // 
  case J50: txt = "SMA RF pins"; return true;   // 
  case J51: txt = "SMA RF connector"; return true;   // 
  case J52: txt = "DB25 connector"; return true;   // 
  case J53: txt = "SMA RF pins"; return true;   // 
  case J54: txt = "RS-232 interface pins"; return true;   // 
  case J55: txt = "SMA RF pins"; return true;   // 
  case J56: txt = "SMA RF pins"; return true;   // 
  case J57: txt = "general purpose LEDs"; return true;   // 
  case J58: txt = "DB25 connector"; return true;   // 
  case J59: txt = "RS-232 interface pins"; return true;   // 
  case J60: txt = "audio pins"; return true;   // 
  case J61: txt = "audio pins"; return true;   // 
  case J62: txt = "audio connector"; return true;   // 
  case J63: txt = "audio connector"; return true;   // 
  case J64: txt = "audio connector"; return true;   // 
  case J65: txt = "audio pins"; return true;   // 
  case J66: txt = "RJ48 pins"; return true;   // 
  case J67: txt = "VGA pins"; return true;   // 
  case J68: txt = "USB pins"; return true;   // 
  case J69: txt = "RJ48 connector"; return true;   // 
  case J70: txt = "VGA connector"; return true;   // 
  case J71: txt = "RJ11 connector"; return true;   // 
  //case J72: txt = ""; return true;   // 
  //case J73: txt = ""; return true;   // 
  case J74: txt = "SMA connector 001.1 SERDES data"; return true;   // 
  case J75: txt = "SMA connector 001.17 SERDES clock"; return true;   // 
  case JP1: txt = "SD connector"; return true;   // 
  default:;
  }
  return false;
}

// ======================================================================================

bool Jumper::to_string(Enum val, std::string& txt)
{
  switch (val) {
  case J10_1_2: txt = "J10 1-2"; return true;      // host VddC on J1.3
  case J10_2_3: txt = "J10 2-3"; return true;      // host VddC on main 1.8V bus
  case J11_1_2: txt = "J11 1-2"; return true;      // host VddI/A on J1.3
  case J11_2_3: txt = "J11 2-3"; return true;       // host VddI/A on main 1.8V bus
  case J14_1_2: txt = "J14 1-2"; return true;      // target VddC on J1.1
  case J14_2_3: txt = "J14 2-3"; return true;      // target VddC on main 1.8V bus
  case J15_1_2: txt = "J15 1-2"; return true;      // target VddI on J1.1
  case J15_2_3: txt = "J15 2-3"; return true;      // target VddI on main 1.8V bus
  case J16_1_2: txt = "J16 1-2"; return true;      // target VddA on J1.1
  case J16_2_3: txt = "J16 2-3"; return true;      // target VddA on main 1.8V bus
  case J20_1_3: txt = "J20 1-3"; return true;      // USB port A RTS signal connected to host RESET
  case J20_2_4: txt = "J20 2-4"; return true;      // host chip reset circuit / button
  case J22_1_2: txt = "J22 1-2"; return true;      // host 500.17 is connected to the target RESET
  case J22_3_4: txt = "J22 3-4"; return true;      // USB port C RTS signal connected to target RESET
  case J22_5_6: txt = "J22 5-6"; return true;      // power on circuit and pushbiutton is connected to the target RESET
  case J23_1_2: txt = "J23 1-2"; return true;      // USB port A Rx to host 708.17
  case J23_3_4: txt = "J23 3-4"; return true;      // USB port A Tx from host 708.1
  case J23_5_6: txt = "J23 5-6"; return true;      // USB port B Rx to host 200.17
  case J23_7_8: txt = "J23 7-8"; return true;      // USB port B Tx from host 100.1
  case J23_9_10: txt = "J23 9-10"; return true;     // USB port C Rx to host 708.17
  case J23_11_12: txt = "J23 11-12"; return true;    // USB port C Tx from host 708.1
  case J25_1_2: txt = "J25 1-2"; return true;      // flash chip is held in reset so that host 705's pins are available
  case J25_2_3: txt = "J25 2-3"; return true;      // flash chip is reset by host chip's reset and (if both J20 jumpers are inserted) by USB port A RTS
  case J26_1_2: txt = "J26 1-2"; return true;      // pulls host 705 pins high. Host 705 does not attempt to boot nor does it drive any of its pins
  case J34_1_2: txt = "J34 1-2"; return true;      // Host 300.1 <-> Target 300.1
  case J35_1_2: txt = "J35 1-2"; return true;      // Host 300.17 <-> Target 300.17
  case J37_1_2: txt = "J37 1-2"; return true;      // 
  case J37_3_4: txt = "J37 3-4"; return true;      // 
  case J38_1_J40_1: txt = "J38 1 J40-1"; return true;  // CLK/SCLK <-> SPI CLK MMC
  case J38_2_J40_2: txt = "J38 2 J40-2"; return true;  // DAT3/CS- <-> SPI CS- MMC
  case J38_3_J40_3: txt = "J38 3 J40-3"; return true;  // CMD/SI <-> SPI DO
  case J38_4_J40_4: txt = "J38 4 J40-4"; return true;  // DAT0/SO <-> SPI DI
  case J38_5_J40_5: txt = "J38 5 J40-5"; return true;  // VDD <-> 1.8V
  case J39_1_2: txt = "J39 1-2"; return true;      // host 600 selects SPI flash when its pin is low (reset condition), or MMC card when the pin is high.
  case J39_2_3: txt = "J39 2-3"; return true;      // SPI flash is always selected. Host 600 unused.
  default:;
  }
  return false;
}

bool Jumper::to_description(Enum val, std::string& txt)
{
  switch (val) {
  case J10_1_2: txt = "host VddC on J1.3"; return true;      // host VddC on J1.3
  case J10_2_3: txt = "host VddC on main 1.8V bus"; return true;      // host VddC on main 1.8V bus
  case J11_1_2: txt = "host VddI/A on J1.3"; return true;      // host VddI/A on J1.3
  case J11_2_3: txt = "host VddI/A on main 1.8V bus"; return true;       // host VddI/A on main 1.8V bus
  case J14_1_2: txt = "target VddC on J1.1"; return true;      // target VddC on J1.1
  case J14_2_3: txt = "target VddC on main 1.8V bus"; return true;      // target VddC on main 1.8V bus
  case J15_1_2: txt = "target VddI on J1.1"; return true;      // target VddI on J1.1
  case J15_2_3: txt = "target VddI on main 1.8V bus"; return true;      // target VddI on main 1.8V bus
  case J16_1_2: txt = "target VddA on J1.1"; return true;      // target VddA on J1.1
  case J16_2_3: txt = "target VddA on main 1.8V bus"; return true;      // target VddA on main 1.8V bus
  case J20_1_3: txt = "USB port A RTS signal connected to host RESET"; return true;      // USB port A RTS signal connected to host RESET
  case J20_2_4: txt = "host chip reset circuit / button"; return true;      // host chip reset circuit / button
  case J22_1_2: txt = "host 500.17 is connected to the target RESET"; return true;      // host 500.17 is connected to the target RESET
  case J22_3_4: txt = "USB port C RTS signal connected to target RESET"; return true;      // USB port C RTS signal connected to target RESET
  case J22_5_6: txt = "power on circuit and pushbiutton is connected to the target RESET"; return true;      // power on circuit and pushbiutton is connected to the target RESET
  case J23_1_2: txt = "USB port A Rx to host 708.17"; return true;      // USB port A Rx to host 708.17
  case J23_3_4: txt = "USB port A Tx from host 708.1"; return true;      // USB port A Tx from host 708.1
  case J23_5_6: txt = "USB port B Rx to host 200.17"; return true;      // USB port B Rx to host 200.17
  case J23_7_8: txt = "USB port B Tx from host 100.1"; return true;      // USB port B Tx from host 100.1
  case J23_9_10: txt = "USB port C Rx to host 708.17"; return true;     // USB port C Rx to host 708.17
  case J23_11_12: txt = "USB port C Tx from host 708.1"; return true;    // USB port C Tx from host 708.1
  case J25_1_2: txt = "flash chip is held in reset so that host 705's pins are available"; return true;      // flash chip is held in reset so that host 705's pins are available
  case J25_2_3: txt = "flash chip is reset by host chip's reset and (if both J20 jumpers are inserted) by USB port A RTS"; return true;      // flash chip is reset by host chip's reset and (if both J20 jumpers are inserted) by USB port A RTS
  case J26_1_2: txt = "pulls host 705 pins high. Host 705 does not attempt to boot nor does it drive any of its pins"; return true;      // pulls host 705 pins high. Host 705 does not attempt to boot nor does it drive any of its pins
  case J34_1_2: txt = "host 300.1 <-> Target 300.1"; return true;      // host 300.1 <-> Target 300.1
  case J35_1_2: txt = "host 300.17 <-> Target 300.17"; return true;      // host 300.17 <-> Target 300.17
  case J37_1_2: txt = ""; return true;      // 
  case J37_3_4: txt = ""; return true;      // 
  case J38_1_J40_1: txt = "CLK/SCLK <-> SPI CLK MMC"; return true;  // CLK/SCLK <-> SPI CLK MMC
  case J38_2_J40_2: txt = "DAT3/CS- <-> SPI CS- MMC"; return true;  // DAT3/CS- <-> SPI CS- MMC
  case J38_3_J40_3: txt = "CMD/SI <-> SPI DO"; return true;  // CMD/SI <-> SPI DO
  case J38_4_J40_4: txt = "DAT0/SO <-> SPI DI"; return true;  // DAT0/SO <-> SPI DI
  case J38_5_J40_5: txt = "VDD <-> 1.8V"; return true;  // VDD <-> 1.8V
  case J39_1_2: txt = "host 600 selects SPI flash when its pin is low (reset condition), or MMC card when the pin is high."; return true;      // host 600 selects SPI flash when its pin is low (reset condition), or MMC card when the pin is high.
  case J39_2_3: txt = "SPI flash is always selected. Host 600 unused."; return true;      // SPI flash is always selected. Host 600 unused.
  default:;
  }
  return false;
}

}
