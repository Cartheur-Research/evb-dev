#include <connection.hpp>
#include <f18a.hpp>


#include <gtkmm.h>


#include <iostream>


namespace connection {

void sleep(long ms)
{
#ifdef _WIN32
  ::Sleep(ms);
#else
  ::usleep(ms * 1000);
#endif
}

Serial::Serial()
: service_(0)
, port_(0)
, read_thread_(0)
, write_thread_(0)
, baud_rate_(0)
, bits_(0)
, delay_ms_(0)
, read_active_(false)
, read_running_(false)
, write_active_(false)
, write_running_(false)
, failed_(false)
, write_log_(0)
, read_log_(0)
{
}

Serial::~Serial()
{
  close();
}

void Serial::modify()
{
  boost::mutex::scoped_lock lock(mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = dispatcher_list_.begin(); it != dispatcher_list_.end(); ++it) {
    (*it)->emit();
  }
}

void Serial::register_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(mutex_);
  dispatcher_list_.push_back(&d);
}

void Serial::deregister_dispatcher(Glib::Dispatcher& d)
{
  boost::mutex::scoped_lock lock(mutex_);
  for (std::vector<Glib::Dispatcher*>::iterator it = dispatcher_list_.begin(); it != dispatcher_list_.end(); ++it) {
    if (*it == &d) {
      dispatcher_list_.erase(it);
      return;
    }
  }
}

void Serial::reset()
{
  try {
    if (port_) {
      delete port_;
      port_ = 0;
    }
    if (service_) {
      delete service_;
      service_ = 0;
    }
    failed_ = false;
  }
  catch (...) {
  }
}

void Serial::close()
{
  if (port_) {
    try {
      read_active_ = false;
      write_active_ = false;
      port_->close();
      while (read_running_ && write_running_) {
        sleep(20);
      }
      sleep(20);
      delete read_thread_;
      read_thread_ = 0;
      delete write_thread_;
      write_thread_ = 0;
      read_data_.clear();
      write_data_.clear();

      if (write_log_) {
        write_log_->close();
        delete write_log_;
        write_log_ = 0;
      }
      if (read_log_) {
        read_log_->close();
        delete read_log_;
        read_log_ = 0;
      }
    }
    catch (...) {
    }
  }
  reset();
}

void Serial::run_read()
{
  read_running_ = true;
  while (read_active_) {
    try {
      boost::asio::read(*port_, boost::asio::buffer(read_buffer_, 3));
      long data = read_buffer_[2];
      data <<= 8;
      data |= read_buffer_[1];
      data <<= 8;
      data |= read_buffer_[0];
      data &= 0x3ffff;
      boost::mutex::scoped_lock lock(read_mutex_);
      read_data_.push_back(data);
    }
    catch (boost::system::system_error&) {
      failed();
    }
    catch (...) {
    }
  }
  read_running_ = false;
}

bool Serial::reset_evb()
{
  if (failed_) {return false;}
  //std::cout << std::endl << "reset EVB001" << std::endl;
  rts(false);
  connection::sleep(10);
  rts(true);
  //connection::sleep(20);
  return true;
}

void Serial::failed()
{
  read_active_ = false;
  write_active_ = false;
  failed_ = true;
  write_cond_.notify_one();
  modify();
}

void Serial::run_write()
{
  write_running_ = true;
  while (write_active_) {
    try {
      long data;
      bool has_data = false;
      {
        boost::mutex::scoped_lock lock(write_mutex_);
        if (write_data_.empty()) {
          boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
          time += boost::posix_time::milliseconds(100);
          write_cond_.timed_wait(lock, time);
        }
        if (!write_data_.empty()) {
          data = write_data_.front();
          //data = write_data_.front();
          write_data_.pop_front();
          has_data = true;
        }
      }
      if (has_data) {
        long orig_data = data;
        data ^= F18A_CELL_MASK;
        data <<= 6;
        data |= 0x12;
        write_buffer_[0] = static_cast<unsigned char>(data);
        write_buffer_[1] = static_cast<unsigned char>(data >> 8);
        write_buffer_[2] = static_cast<unsigned char>(data >> 16);
        //port_->write_some(boost::asio::buffer(write_buffer_, 3));
        boost::asio::write(*port_, boost::asio::buffer(write_buffer_, 3));
        //std::cout << "word : " << orig_data << " : " << std::hex << orig_data << std::dec << std::endl;
        //std::cout << "[0]  : " << static_cast<int>(write_buffer_[0]) << " : " << std::hex << static_cast<int>(write_buffer_[0]) << std::dec << std::endl;
        //std::cout << "[1]  : " << static_cast<int>(write_buffer_[1]) << " : " << std::hex << static_cast<int>(write_buffer_[1]) << std::dec << std::endl;
        //std::cout << "[2]  : " << static_cast<int>(write_buffer_[2]) << " : " << std::hex << static_cast<int>(write_buffer_[2]) << std::dec << std::endl;
        if (write_log_ != 0) {
          *write_log_ << "word : "
            << std::setfill(' ') << std::setw(6) << orig_data << " : "
            << std::hex << std::setfill('0') << std::setw(5) << orig_data
            << std::dec << std::endl;
        }
      }
    }
    catch (boost::system::system_error&) {
      failed();
    }
    catch (...) {
    }
  }
  write_running_ = false;
}

bool Serial::read(long& data)
{
  boost::mutex::scoped_lock lock(read_mutex_);
  if (failed_) {return false;}
  if (!read_data_.empty()) {
    data = read_data_.front();
    read_data_.pop_front();
    if (read_log_ != 0) {
      *read_log_ << "word : "
        << std::setfill(' ') << std::setw(6) << data << " : "
        << std::hex << std::setfill('0') << std::setw(5) << data
        << std::dec << std::endl;
    }
    return true;
  }
  return false;
}

bool Serial::read_wait(long& data, size_t timeout_in_ms)
{
  size_t cnt = 0;
  while (!read(data)) {
    if (++cnt < timeout_in_ms) {
      connection::sleep(1);
    }
    else {
      return false;
    }
  }
  return true;
}


bool Serial::write(long data)
{
  boost::mutex::scoped_lock lock(write_mutex_);
  if (failed_) {return false;}
  if (write_data_.empty()) {
    // release condition variable if data queue was empty
    write_cond_.notify_one();
  }
  if (write_active_) {
    // only write data if output is active
    write_data_.push_back(data);
    return true;
  }
  return false;
}

bool Serial::rts(bool val) // set RTS line to 0 (false) or 1 (true)
{
  if (failed_) {return false;}
  try {
    if (port_) {
#ifdef _WIN32
      ::EscapeCommFunction(port_->native_handle(), val ? SETRTS : CLRRTS);
#else
#endif
      return true;
    }
  }
  catch (...) {
  }
  return false;
}

bool Serial::dtr(bool val) // set CTS line to 0 (false) or 1 (true)
{
  if (failed_) {return false;}
  try {
    if (port_) {
#ifdef _WIN32
      ::EscapeCommFunction(port_->native_handle(), val ? SETDTR : CLRDTR);
#else
#endif
      return true;
    }
  }
  catch (...) {
  }
  return false;
}

bool Serial::connect()
{
  try {
    close();
//    std::string name;
//#ifdef _WIN32
//    name = "\\\\.\\COM";
//#else
//    name = "/dev/ttyS";
//#endif
//    name += boost::lexical_cast<std::string>(port);
    if (service_ == 0) {
      service_ = new boost::asio::io_service();
    }
    //port_ = new boost::asio::serial_port(*service_, name);
    if (port_ == 0) {
      port_ = new boost::asio::serial_port(*service_);
      port_->open(name_);
    }

    if (baud_rate_ == 0) {
      boost::asio::serial_port::baud_rate baud;
	    port_->get_option(baud);
      baud_rate_ = baud.value();
    }
    else {
      boost::asio::serial_port::baud_rate baud(baud_rate_); // 921600
  	  port_->set_option( baud );
    }

    if (bits_ == 0) {
      boost::asio::serial_port::character_size bits;
	    port_->get_option(bits);
      bits_ = bits.value();
    }
    else {
      boost::asio::serial_port::character_size bits(bits_); // 8
  	  port_->set_option( bits );
    }

    if (flow_.empty()) {
      boost::asio::serial_port::flow_control flow;
	    port_->get_option(flow);
      if (flow.value() == boost::asio::serial_port::flow_control::none) {flow_ = "none";}
      else if (flow.value() == boost::asio::serial_port::flow_control::software) {flow_ = "software";}
      else if (flow.value() == boost::asio::serial_port::flow_control::hardware) {flow_ = "hardware";}
    }
    else {
      boost::asio::serial_port::flow_control flow; // none
      if (flow_ == "none") {flow = boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none);}
      else if (flow_ == "software") {flow = boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::software);}
      else if (flow_ == "hardware") {flow = boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::hardware);}
  	  port_->set_option( flow );
    }

    if (parity_.empty()) {
      boost::asio::serial_port::parity parity;
	    port_->get_option(parity);
      if (parity.value() == boost::asio::serial_port::parity::none) {parity_ = "none";}
      else if (parity.value() == boost::asio::serial_port::parity::odd) {parity_ = "software";}
      else if (parity.value() == boost::asio::serial_port::parity::even) {parity_ = "hardware";}
    }
    else {
      boost::asio::serial_port::parity parity; // none
      if (parity_ == "none") {parity = boost::asio::serial_port::parity(boost::asio::serial_port::parity::none);}
      else if (parity_ == "odd") {parity = boost::asio::serial_port::parity(boost::asio::serial_port::parity::odd);}
      else if (parity_ == "even") {parity = boost::asio::serial_port::parity(boost::asio::serial_port::parity::even);}
  	  port_->set_option( parity );
    }

    if (stop_.empty()) {
      boost::asio::serial_port::stop_bits stop;
	    port_->get_option(stop);
      if (stop.value() == boost::asio::serial_port::stop_bits::one) {stop_ = "1";}
      else if (stop.value() == boost::asio::serial_port::stop_bits::onepointfive) {stop_ = "1.5";}
      else if (stop.value() == boost::asio::serial_port::stop_bits::two) {stop_ = "2";}
    }
    else {
      boost::asio::serial_port::stop_bits stop; // 1
      if (stop_ == "1") {stop = boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one);}
      else if (stop_ == "1.5") {stop = boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::onepointfive);}
      else if (stop_ == "2") {stop = boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::two);}
	    port_->set_option( stop );
    }
    // now start the threads
    if (!read_running_ && (read_thread_ != 0)) {
      sleep(10);
      delete read_thread_;
      read_thread_ = 0;
    }
    if (!write_running_ && (write_thread_ != 0)) {
      sleep(10);
      delete write_thread_;
      write_thread_ = 0;
    }
    failed_ = false;
    if (read_thread_ == 0) {
      read_active_ = true;
      read_thread_ = new boost::thread(sigc::mem_fun(*this, &Serial::run_read));
    }
    if (write_thread_ == 0) {
      write_active_ = true;
      write_thread_ = new boost::thread(sigc::mem_fun(*this, &Serial::run_write));
    }

    write_log_ = new std::ofstream("serout.log", std::ios_base::out|std::ios_base::binary);
    read_log_ = new std::ofstream("serin.log", std::ios_base::out|std::ios_base::binary);

    return true;
  }
  catch (...) {
    reset();
  }
  return false;
}

bool Serial::connected() const
{
  if (failed_) {return false;}
  return port_ != 0;
}

bool Serial::send_break()
{
  if (failed_) {return false;}
  if (port_ != 0) {
#ifdef _WIN32
    ::SetCommBreak(port_->native_handle());
    ::Sleep(125); // a break must be 125 ms
    ::ClearCommBreak(port_->native_handle());
#else
    port_->send_break();
#endif
    return true;
  }
  return false;
}

void Serial::read(xml::Iterator& it)
{
	if (it.tag() == "device") {
  	if (it.next() == xml::Element::content) {
      name_ = it.value();
    }
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "device")) {
      it.next_tag();
    }
  }
	if (it.tag() == "baudrate") {
  	if (it.next() == xml::Element::content) {
      try {
        baud_rate_ = boost::lexical_cast<long>(it.value());
      }
      catch (...) {
        baud_rate_ = 0;
      }
    }
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "baudrate")) {
      it.next_tag();
    }
  }
	if (it.tag() == "bits") {
  	if (it.next() == xml::Element::content) {
      try {
        bits_ = boost::lexical_cast<int>(it.value());
      }
      catch (...) {
        bits_ = 0;
      }
    }
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "bits")) {
      it.next_tag();
    }
  }
	if (it.tag() == "flow") {
  	if (it.next() == xml::Element::content) {
      flow_ = it.value();
    }
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "flow")) {
      it.next_tag();
    }
  }
	if (it.tag() == "parity") {
  	if (it.next() == xml::Element::content) {
      parity_ = it.value();
    }
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "parity")) {
      it.next_tag();
    }
  }
	if (it.tag() == "stop") {
  	if (it.next() == xml::Element::content) {
      stop_ = it.value();
    }
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "stop")) {
      it.next_tag();
    }
  }
	if (it.tag() == "delay") {
  	if (it.next() == xml::Element::content) {
      try {
        delay_ms_ = boost::lexical_cast<size_t>(it.value());
      }
      catch (...) {
        delay_ms_ = 0;
      }
    }
    if ((it.next_tag() == xml::Element::end) && (it.tag() == "delay")) {
      it.next_tag();
    }
  }
  else {
    reset_delay();
  }
}

void Serial::write(xml::Writer& w)
{
  w.out() << "<device>" << name_ << "</device>";
  w.newline();
  w.out() << "<baudrate>" << baud_rate_ << "</baudrate>";
  w.newline();
  w.out() << "<bits>" << bits_ << "</bits>";
  w.newline();
  w.out() << "<flow>" << flow_ << "</flow>";
  w.newline();
  w.out() << "<parity>" << parity_ << "</parity>";
  w.newline();
  w.out() << "<stop>" << stop_ << "</stop>";
  w.newline();
  if (delay_ms_ > 0) {
    w.out() << "<delay>" << delay_ms_ << "</delay>";
    w.newline();
  }
}



}
