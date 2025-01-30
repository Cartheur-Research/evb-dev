#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#ifdef _WIN32
#pragma warning (disable: 4251) // http://msdn2.microsoft.com/en-gb/library/esew7y1w.aspx
#pragma warning (disable: 4996)
#endif 

#include <stdlib.h>
#include <exception>
#include <deque>
#include <vector>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <xml.hpp>
#include <utils.hpp>

namespace Glib {
  class Dispatcher;
}

namespace connection {

extern void sleep(long ms);

class Serial : public utils::RefCntObj {
  boost::mutex                mutex_;
  boost::mutex                read_mutex_;
  boost::mutex                write_mutex_;
  boost::condition_variable   read_cond_;
  boost::condition_variable   write_cond_;
  boost::asio::io_service*    service_;
  boost::asio::serial_port*   port_;
  boost::thread*              read_thread_;
  boost::thread*              write_thread_;
  std::vector<Glib::Dispatcher*>  dispatcher_list_;
  std::deque<long>            read_data_;
  std::deque<long>            write_data_;
  std::string                 name_;
  std::string                 flow_;
  std::string                 parity_;
  std::string                 stop_;
  long                        baud_rate_;
  int                         bits_;
  size_t                      delay_ms_;
  unsigned char               read_buffer_[3];
  unsigned char               write_buffer_[3];
  volatile bool               read_active_;
  volatile bool               read_running_;
  volatile bool               write_active_;
  volatile bool               write_running_;
  volatile bool               failed_;

  std::ofstream*              write_log_;
  std::ofstream*              read_log_;

  Serial(const Serial&);
  Serial& operator=(const Serial&);

  void failed();
  bool read(long&);

public:
  Serial();
  ~Serial();

  void delay(size_t ms) {delay_ms_ = ms;}
  void reset_delay() {delay_ms_ = 0;}
  size_t delay_ms() const {return delay_ms_;}

  const boost::asio::serial_port* port() const {return port_;}

  void modify();

  void reset();
  void close();
  bool connect();
  bool reset_evb();

  const std::string& name() const {return name_;}
  const std::string& flow() const {return flow_;}
  const std::string& parity() const {return parity_;}
  const std::string& stop() const {return stop_;}
  long baud_rate() const {return baud_rate_;}
  int bits() const {return bits_;}

  void name(const std::string& val) {name_ = val;}
  void flow(const std::string& val) {flow_ = val;}
  void parity(const std::string& val) {parity_ = val;}
  void stop(const std::string& val) {stop_ = val;}
  void baud_rate(long val) {baud_rate_ = val;}
  void bits(int val) {bits_ = val;}

  bool connected() const;
  bool send_break();

  void read(xml::Iterator&);
  void write(xml::Writer&);

  void run_read();
  void run_write();

  bool write(long);
  bool read_wait(long&, size_t timeout_in_ms = 100);

  bool rts(bool); // set RTS line to 0 (false) or 1 (true)
  bool dtr(bool); // set CTS line to 0 (false) or 1 (true)

  void register_dispatcher(Glib::Dispatcher&);
  void deregister_dispatcher(Glib::Dispatcher&);

};
typedef utils::RefCntPtr<Serial> Serial_ptr;


}

#endif
