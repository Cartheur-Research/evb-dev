#ifndef IDE_HPP
#define IDE_HPP

#ifdef _WIN32
#pragma warning (disable: 4251) // http://msdn2.microsoft.com/en-gb/library/esew7y1w.aspx
#pragma warning (disable: 4996)
#pragma warning (disable: 4265)
#pragma warning (disable: 4308)
#pragma warning (disable: 4242) // disable boost warning
#pragma warning (disable: 4275)
#pragma warning (disable: 4946)
#endif 


#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <exception>

#include <node_type.hpp>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

//namespace studio {
//class NodeType;
//typedef utils::RefCntPtr<NodeType> NodeType_ptr;
//}

#include <xml.hpp>
#include <utils.hpp>
#include <evb.hpp>

namespace connection {
	class Serial;
}

namespace evb {
  class Board;
  class Manager;
  typedef utils::RefCntPtr<Manager> Manager_ptr;
}

namespace ga144 {
  class Chip;
}

namespace f18a {
  class Processor;
}

namespace studio {

class EvbView;
class EvbWindow;
class Ga144View;
class Ga144Window;
class F18aView;
class F18aWindow;
class MainWindow;

class exception : public std::runtime_error {
public:
	explicit exception(const char* msg) : std::runtime_error(msg) {}
	explicit exception(const std::string& msg) : std::runtime_error(msg.c_str()) {}
};

class xml_exception : public exception {
public:
	explicit xml_exception(const char* msg) : exception(msg) {}
	explicit xml_exception(const std::string& msg) : exception(msg.c_str()) {}
};

#define ECHO_TEST_SIZE    10


class EchoTestWindow : public Gtk::Window {
private:
  evb::Board_ptr          board_;
  evb::Manager_ptr        manager_;
  Gtk::Grid               table_;

  boost::mutex            mutex_;
  boost::condition_variable cond_;
  Glib::Dispatcher        dispatcher_;

  Gtk::Label              ok_count_label_;
  Gtk::Label              nok_count_label_;
  Gtk::Label              total_count_label_;
  Gtk::Label              sample_label_;
  Gtk::Label              out_label_;
  Gtk::Label              in_label_;
  
  Gtk::Label              ok_count_;
  Gtk::Label              nok_count_;
  Gtk::Label              total_count_;

  Gtk::Label              sample_[ECHO_TEST_SIZE];
  Gtk::Label              out_[ECHO_TEST_SIZE];
  Gtk::Label              in_[ECHO_TEST_SIZE];

  Gtk::ButtonBox          button_box_;
  Gtk::Button             start_button_;
  Gtk::Button             stop_button_;
  Gtk::Button             reset_button_;

  volatile long           display_sample_data_[ECHO_TEST_SIZE];
  volatile long           display_out_data_[ECHO_TEST_SIZE];
  volatile long           display_in_data_[ECHO_TEST_SIZE];

  volatile long           sample_data_[ECHO_TEST_SIZE];
  volatile long           out_data_[ECHO_TEST_SIZE];
  volatile long           in_data_[ECHO_TEST_SIZE];
  volatile size_t         data_index_;

  volatile long           display_cnt_;  // test run
  volatile long           display_ok_;
  volatile long           display_nok_;
  volatile long           cnt_;  // test run
  volatile long           ok_;
  volatile long           nok_;
  volatile long           curr_;  // current test value
  volatile bool           active_;
  volatile bool           running_;
  volatile bool           finished_;

  void add_sample(long out, long in);
  void copy_sample_data();
  void notify();
  void display_samples();

public:
  EchoTestWindow();
  ~EchoTestWindow();

  void setup(EvbView&);
  bool reset();
  void on_reset();
  void start();
  void stop();

  void run();

};

class InstructionWindow : public Gtk::Window {
private:
  F18aView*               view_;
  size_t                  address_;
  long                    value_;

  Gtk::Grid               table_;

  Gtk::Label              addr_label_;
  Gtk::Entry              addr_;


  Gtk::Label              ext_arith_label_;
  Gtk::CheckButton        ext_arith_;

  Gtk::Label              label_label_;
  Gtk::Entry              label_;

  Gtk::RadioButton::Group instr_group;
  Gtk::RadioButton        instr4_;
  Gtk::RadioButton        instr3_;
  Gtk::RadioButton        instr2_;
  Gtk::RadioButton        instr1_;
  Gtk::RadioButton        instr0_;

  Gtk::ComboBoxText       i4_slot0_;
  Gtk::ComboBoxText       i4_slot1_;
  Gtk::ComboBoxText       i4_slot2_;
  Gtk::ComboBoxText       i4_slot3_;
  Gtk::Label              hex_data_label_;
  Gtk::Label              dec_data_label_;
  Gtk::Label              lbl_data_label_;

  Gtk::ComboBoxText       i3_slot0_;
  Gtk::ComboBoxText       i3_slot1_;
  Gtk::ComboBoxText       i3_slot2_;
  Gtk::Label              i3_data_label_;
  Gtk::Entry              i3_hex_data_;
  Gtk::Entry              i3_dec_data_;
  Gtk::Entry              i3_lbl_data_;

  Gtk::ComboBoxText       i2_slot0_;
  Gtk::ComboBoxText       i2_slot1_;
  Gtk::Label              i2_data_label_;
  Gtk::Entry              i2_hex_data_;
  Gtk::Entry              i2_dec_data_;
  Gtk::Entry              i2_lbl_data_;

  Gtk::ComboBoxText       i1_slot0_;
  Gtk::Label              i1_data_label_;
  Gtk::Entry              i1_hex_data_;
  Gtk::Entry              i1_dec_data_;
  Gtk::Entry              i1_lbl_data_;

  Gtk::Label              i0_data_label_;
  Gtk::Entry              i0_hex_data_;
  Gtk::Entry              i0_dec_data_;
  Gtk::Entry              i0_lbl_data_;

  Gtk::ButtonBox          button_box_;
  Gtk::Button             ok_button_;
  Gtk::Button             revert_button_;
  Gtk::Button             cancel_button_;

  static bool add_instruction(Gtk::ComboBoxText&, long&);

public:
  InstructionWindow();
  ~InstructionWindow();

  void setup(F18aView&, size_t addr);

  void set_default();
  void set_undefined();

  void open();
  void on_button_ok();
  void on_button_revert();
  void on_button_cancel();

  void on_modify_i3_hex_data();
  void on_modify_i3_dec_data();
  void on_modify_i3_lbl_data();

  void on_modify_i2_hex_data();
  void on_modify_i2_dec_data();
  void on_modify_i2_lbl_data();
  
  void on_modify_i1_hex_data();
  void on_modify_i1_dec_data();
  void on_modify_i1_lbl_data();

  void on_modify_i0_hex_data();
  void on_modify_i0_dec_data();
  void on_modify_i0_lbl_data();

  static bool read_hex(const std::string&, long& value);
  static bool read_dec(const std::string&, long& value);

};

class IOWindow : public Gtk::Window {
private:
  evb::Manager_ptr             manager_;

  // digital pins
  std::vector<size_t>             pin_list_;
  std::vector<Gtk::Label*>        name_list_;
  std::vector<Gtk::Label*>        descr_list_;
  std::vector<Gtk::Label*>        value_list_;
  std::vector<Gtk::RadioButton::Group*>  group_list_;
  std::vector<Gtk::RadioButton*>  hidden_list_;
  std::vector<Gtk::RadioButton*>  high_list_;
  std::vector<Gtk::RadioButton*>  low_list_;
  std::vector<Gtk::RadioButton*>  weak_low_list_;
  std::vector<Gtk::RadioButton*>  input_list_;
  std::vector<Gtk::Button*>       action_list_;

  // analog input pins
  std::vector<size_t>             ain_pin_list_;
  std::vector<Gtk::Label*>        ain_name_list_;
  std::vector<Gtk::Label*>        ain_descr_list_;
  std::vector<Gtk::Label*>        ain_value_list_;
  std::vector<Gtk::RadioButton::Group*>  ain_group_list_;
  std::vector<Gtk::RadioButton*>  ain_hidden_list_;
  std::vector<Gtk::RadioButton*>  ain_input_list_;
  std::vector<Gtk::RadioButton*>  ain_vdd_calib_list_;
  std::vector<Gtk::RadioButton*>  ain_vss_calib_list_;
  std::vector<Gtk::RadioButton*>  ain_disable_list_;
  std::vector<Gtk::Button*>       ain_action_list_;

  // analog output pins
  std::vector<size_t>             aout_pin_list_;
  std::vector<Gtk::Label*>        aout_name_list_;
  std::vector<Gtk::Label*>        aout_descr_list_;
  std::vector<Gtk::Label*>        aout_value_list_;
  std::vector<Gtk::Entry*>        aout_write_list_;
  std::vector<Gtk::Button*>       aout_action_list_;

  Gtk::ScrolledWindow             scroll_;
  Gtk::Grid                       table_;

public:
  IOWindow(evb::Manager_ptr, size_t from, size_t to);
  ~IOWindow();

  void on_high(size_t index);
  void on_low(size_t index);
  void on_weak_low(size_t index);
  void on_input(size_t index);
  void on_write(size_t index);

  void on_vdd_calib(size_t index);
  void on_vss_calib(size_t index);
  void on_disable(size_t index);
  void on_ainput(size_t index);
  void on_aread(size_t index);

  void on_awrite(size_t index);

};

class SourceWindow : public Gtk::Window {
private:
  evb::Board&          board_;
  F18aView*               view_;
  //Gtk::Grid               table_;
  Gtk::VBox               vbox_;

  Gtk::HBox               name_box_;
  Gtk::Label              name_label_;
  //Gtk::Entry              name_;
  Gtk::ComboBoxText       name_list_;

  Gtk::Label              source_label_;
  Gtk::Label              comment_label_;

  Gtk::ScrolledWindow     source_scroll_;
  Gtk::ScrolledWindow     comment_scroll_;

  Gtk::Frame              source_frame_;
  Gtk::Frame              comment_frame_;

  Gtk::HBox               text_box_;
  Gtk::VBox               source_box_;
  Gtk::VBox               comment_box_;

  Glib::RefPtr<Gtk::TextBuffer>         buffer_;
  Glib::RefPtr<Gtk::TextBuffer>         comment_buffer_;
  Gtk::TextView           text_;
  Gtk::VSeparator         sep_;
  Gtk::TextView           comment_text_;

  Gtk::ButtonBox          button_box_;
  Gtk::Button             assemble_button_;
  Gtk::Button             ok_button_;
  Gtk::Button             cancel_button_;

  std::string             old_module_name_;
  std::string             module_name_;
  size_t                  pos_;
  size_t                  selected_module_;
  bool                    is_rom_;

public:
  SourceWindow(evb::Board&, bool is_rom);
  ~SourceWindow();

  void setup(F18aView&, size_t pos);
  void text(const std::string&);
  void open();
  void on_button_assemble();
  void on_button_ok();
  void on_button_cancel();
  void on_module_changed();

};

class ConnectionWindow : public Gtk::Window {
private:
  EvbView*                view_;
  size_t                  port_;

  Gtk::Grid               table_;

  Gtk::Label              device_label_;
  Gtk::Entry              device_;

  Gtk::Label              delay_label_;
  Gtk::Entry              delay_;

  Gtk::Label              baud_label_;
  Gtk::ComboBoxText       baud_;

  Gtk::Label              bits_label_;
  Gtk::ComboBoxText       bits_;

  Gtk::Label              flow_label_;
  Gtk::ComboBoxText       flow_;

  Gtk::Label              parity_label_;
  Gtk::ComboBoxText       parity_;

  Gtk::Label              stop_label_;
  Gtk::ComboBoxText       stop_;

  Gtk::ButtonBox          button_box_;
  Gtk::Button             ok_button_;
  Gtk::Button             cancel_button_;

public:
  ConnectionWindow();
  ~ConnectionWindow();

  void setup(EvbView&, size_t port);

  void open();
  void on_button_ok();
  void on_button_cancel();

  void set_default();
  void set_undefined();

  void read(connection::Serial_ptr);
  void write(const connection::Serial_ptr);

  //void read(xml::Iterator&);
  //void write(xml::Writer&);

};

class ModuleWindow : public Gtk::Window {
private:
  EvbWindow&             owner_;
  Gtk::Grid                 table_;

  std::string               name_;
  std::vector<size_t>       node_list_;

  Gtk::HBox                 name_box_;
  Gtk::Label                name_label_;
  Gtk::Entry                name_entry_;
  Gtk::Button               rename_button_;
  Gtk::Entry                new_name_entry_;

  Gtk::ButtonBox            button_box_;
  Gtk::Button               compile_;
  Gtk::Button               write_;
  Gtk::Button               start_;
  Gtk::Button               run_;

  std::map<size_t, Gtk::Button*>   node_map_;
  size_t                    lines_;

  bool compile();
  bool write();
  bool start();

public:
  ModuleWindow(EvbWindow&, const std::string& name);
  ~ModuleWindow();

  void clear();
  void setup();

  void rename(const std::string& new_name);
  void on_node(size_t);
  void on_rename();
  void on_compile();
  void on_write();
  void on_start();
  void on_run();
};


class PackageWindow : public Gtk::Window {
private:
  EvbWindow&             owner_;
  Gtk::VBox                 vbox_;
  Gtk::VBox                 button_vbox_;
  Gtk::ListViewText         unassigned_list_;
  Gtk::ListViewText         assigned_list_;

  Gtk::ScrolledWindow       unassigned_scroll_;
  Gtk::ScrolledWindow       assigned_scroll_;

  Gtk::Frame                unassigned_box_;
  Gtk::Frame                assigned_box_;
  Gtk::HBox                 list_hbox_;

  Gtk::HBox                 name_box_;
  Gtk::Label                name_label_;
  Gtk::Entry                name_entry_;
  Gtk::Button               rename_button_;
  Gtk::Entry                new_name_entry_;

  Gtk::HBox                 exclude_box_;
  Gtk::Label                exclude_label_;
  Gtk::Entry                exclude_entry_;

  Gtk::VBox                 content_;
  Gtk::MenuBar              menu_;

  Gtk::Button               add_button_;
  Gtk::Button               remove_button_;
  Gtk::Button               up_button_;
  Gtk::Button               down_button_;

  Gtk::ButtonBox            button_box_;
  Gtk::Button               ok_button_;
  Gtk::Button               revert_button_;
  Gtk::Button               cancel_button_;

  Gtk::HBox                 boot_box_;
  Gtk::Label                boot_method_label_;
  Gtk::ComboBoxText         boot_method_;
  Gtk::Button               boot_button_;

  evb::Package_ptr       package_;
  std::string               name_;
  std::vector<evb::Module_ptr>   module_list_;
  std::map<std::string, size_t> assigned_map_;
public:
  PackageWindow(EvbWindow&, const std::string& name);
  virtual ~PackageWindow();

  EvbWindow& owner() {return owner_;}

  void setup();
  void setup(evb::Package_ptr);

  void on_button_add();
  void on_button_remove();
  void on_button_up();
  void on_button_down();

  void on_button_ok();
  void on_button_revert();
  void on_button_cancel();

  void on_button_boot();
};


class F18aView : public Gtk::DrawingArea {
  F18aWindow&                     window_;
  f18a::Processor_ptr             model_;
  Glib::Dispatcher*               change_dispatcher_;
  //std::string                     source_;
  int                             font_width_;
  int                             font_height_;
  InstructionWindow*              instr_window_;

  double draw_memory(
    const long* data,
    size_t size,
    size_t addr,
    std::map<long, std::string>& addr_map,
    const Cairo::RefPtr<Cairo::Context>&,
    const Glib::RefPtr<Pango::Layout>&,
    Pango::FontDescription&,
    double x,
    double y
  );

  double draw_register(
    size_t reg,
    long value,
    const Cairo::RefPtr<Cairo::Context>& cairo,
    const Glib::RefPtr<Pango::Layout>& pango,
    Pango::FontDescription& fdesc,
    double x,
    double y
  );

  double draw_register(
    const std::string&,
    long value,
    const Cairo::RefPtr<Cairo::Context>& cairo,
    const Glib::RefPtr<Pango::Layout>& pango,
    Pango::FontDescription& fdesc,
    double x,
    double y
  );

public:
  F18aView(F18aWindow&, f18a::Processor_ptr);
  virtual ~F18aView();

  F18aWindow& window() {return window_;}

  static size_t width();
  static size_t height();

  void read(xml::Iterator&);
  void write(xml::Writer&);

  void modify();

  void on_modify();
  void on_changed();

  bool on_button_press_event(GdkEventButton*);
  bool on_scroll_event(GdkEventScroll *ev);

  //void on_area_size_changed(const Gdk::Rectangle&);
  bool on_draw (const ::Cairo::RefPtr< ::Cairo::Context >& cr);
  void draw(const Cairo::RefPtr<Cairo::Context>&, const Glib::RefPtr<Pango::Layout>&, double x, double y);

  void draw_node(
    f18a::Processor_ptr,
    const Cairo::RefPtr<Cairo::Context>&,
    const Glib::RefPtr<Pango::Layout>&,
    Pango::FontDescription&,
    double x,
    double y
  );

  f18a::Processor_ptr model() {return model_;}
  const f18a::Processor_ptr model() const {return model_;}

  bool find_ram_rect(double mx, double my, Gdk::Rectangle& rect, double x, double y, size_t& ram_offset);
  bool rename_module(const std::string& old_name, const std::string& new_name);

  static void draw_hex(
    long val,
    size_t digits,
    const Cairo::RefPtr<Cairo::Context>& cairo,
    Pango::FontDescription& fdesc,
    double x,
    double y
  );

  //void source(const std::string&);
  //void rom_source(const std::string&);
  //void comment(const std::string&);
  //void rom_comment(const std::string&);
};

class F18aWindow : public Gtk::Window {
  Ga144View&                      owner_;
  std::vector<SourceWindow*>      source_window_list_;
  //std::vector<SourceWindow*>      rom_source_window_list_;
  SourceWindow*                   rom_source_window_;

  F18aView                        view_;
  Gtk::ScrolledWindow             scroll_;
  Gtk::EventBox                   view_event_;

  Gtk::VBox                       content_;
  Gtk::MenuBar                    menu_;
  Gtk::MenuItem                   proc_;
  Gtk::MenuItem                   action_;
  Gtk::MenuItem                   simulation_;
  Gtk::MenuItem                   status_;
  Gtk::MenuItem                   add_source_;
  //Gtk::MenuItem                   add_rom_source_;

  Gtk::Menu                       proc_menu_;
  Gtk::SeparatorMenuItem          proc_sep1_;
  Gtk::MenuItem                   close_;
  std::vector<Gtk::MenuItem*>     source_list_;
  //std::vector<Gtk::MenuItem*>     rom_source_list_;
  Gtk::MenuItem                   rom_source_;

  Gtk::Menu                       source_menu_;
  //Gtk::Menu                       rom_source_menu_;
  Gtk::MenuItem                   source_;
  //Gtk::MenuItem                   rom_source_;

  Gtk::MenuItem                   action_compile_rom_;
  Gtk::SeparatorMenuItem          action_sep1_;
  Gtk::MenuItem                   action_read_all_;
  Gtk::MenuItem                   action_read_ram_;
  Gtk::MenuItem                   action_read_rom_;
  Gtk::MenuItem                   action_read_and_compare_rom_;
  Gtk::MenuItem                   action_read_ram_and_rom_;
  Gtk::MenuItem                   action_read_stack_;
  Gtk::MenuItem                   action_read_return_stack_;
  Gtk::MenuItem                   action_read_a_;
  Gtk::MenuItem                   action_read_io_;
  Gtk::MenuItem                   action_write_all_;
  Gtk::MenuItem                   action_write_ram_;
  Gtk::MenuItem                   action_write_stack_;
  Gtk::MenuItem                   action_write_return_stack_;
  Gtk::MenuItem                   action_write_a_;
  Gtk::MenuItem                   action_write_b_;
  Gtk::MenuItem                   action_write_io_;
  Gtk::MenuItem                   action_start_;
  Gtk::MenuItem                   action_run_;
  Gtk::MenuItem                   action_test_;

  Gtk::Menu                       action_menu_;

  Gtk::Menu                       simulation_menu_;
  Gtk::MenuItem                   sim_step_;

  Gtk::Menu                       status_menu_;

  void update_source_menu();

public:
  F18aWindow(Ga144View&, const std::string& title, f18a::Processor_ptr);
  virtual ~F18aWindow();

  Ga144View& owner() {return owner_;}
  const Ga144View& owner() const {return owner_;}

  size_t full_node() const;

  void modify();

  void on_close();
  void on_add_source();
  //void on_add_rom_source();
  void on_source(size_t);
  //void on_rom_source(size_t);
  void on_rom_source();

  void read(xml::Iterator&);
  void write(xml::Writer&);

  void name(size_t pos, const std::string& val);

  void on_read_all();
  void on_write_all();
  void on_read_ram();
  void on_read_rom();
  void on_read_and_compare_rom();
  void on_read_ram_and_rom();
  void on_write_ram();
  void on_read_stack();
  void on_write_stack();
  void on_read_return_stack();
  void on_write_return_stack();
  void on_read_a();
  void on_write_a();
  void on_write_b();
  void on_read_io();
  void on_write_io();
  void on_start();
  void on_run();
  void on_test();
  void on_compiler_rom();

  void on_sim_step();

  SourceWindow* source(const std::string&);
  bool rename_module(const std::string& old_name, const std::string& new_name) {return view_.rename_module(old_name, new_name);}

};

class Ga144View : public Gtk::DrawingArea {
  Ga144Window&                    window_;
  ga144::Chip_ptr                 model_;
  std::string                     name_;
  Glib::Dispatcher                modify_dispatcher_;
  Glib::Dispatcher                change_dispatcher_;
  Gtk::Menu*                      node_menu_;
  size_t                          node_index_;
  size_t                          selected_node_;

  typedef F18aWindow* F18aWindowPtr;
  std::vector<F18aWindowPtr>      node_window_list_;

public:
  Ga144View(Ga144Window&, ga144::Chip_ptr);
  virtual ~Ga144View();

  F18aWindow& assert_window(size_t ind);

  ga144::Chip_ptr model() {return model_;}
  const ga144::Chip_ptr model() const {return model_;}

  Ga144Window& window() {return window_;}
  const Ga144Window& window() const {return window_;}

  const std::string& name() const {return name_;}
  void name(const std::string& val) {name_ = val;}

  static size_t width();
  static size_t height();
  bool node_rect(size_t col, size_t row, Gdk::Rectangle& rect);
  size_t node_to_pos(size_t col, size_t row);
  size_t node(size_t col, size_t row, bool with_chip);
  void node_to_name(size_t col, size_t row, std::string&);

  void read(xml::Iterator&);
  void write(xml::Writer&);

  bool on_button_press_event(GdkEventButton*);
  bool on_scroll_event(GdkEventScroll *ev);
  void on_modify();
  void modify();

  void on_changed();
  void changed();

  //void on_area_size_changed(const Gdk::Rectangle&);
  bool on_draw (const ::Cairo::RefPtr< ::Cairo::Context >& cr);
  void draw(const Cairo::RefPtr<Cairo::Context>&, const Glib::RefPtr<Pango::Layout>&, double x, double y);
  void draw_node(
    f18a::Processor_ptr,
    long info,
    NodeType_ptr type,
    const Cairo::RefPtr<Cairo::Context>&,
    const Glib::RefPtr<Pango::Layout>&,
    Pango::FontDescription&,
    double x,
    double y,
    double w,
    double h
  );

  void on_read_node();
  void on_write_node();
  void on_read_ram();
  void on_read_rom();
  void on_read_ram_and_rom();
  void on_write_ram();
  void on_read_stack();
  void on_write_stack();
  void on_read_return_stack();
  void on_write_return_stack();
  void on_read_a();
  void on_write_a();
  void on_write_b();
  void on_read_io();
  void on_write_io();
  void on_start();
  void on_run();

  size_t node_to_index(size_t node);
  F18aWindow* node_window(size_t node, bool assert);
  bool rename_module(const std::string& old_name, const std::string& new_name);
};

class Ga144Window : public Gtk::Window {
  EvbView&                        owner_;

  Ga144View                       view_;
  //Gtk::Viewport                   viewport_;
  Gtk::ScrolledWindow             scroll_;
  Gtk::EventBox                   view_event_;

  Gtk::VBox                       content_;
  Gtk::MenuBar                    menu_;
  Gtk::MenuItem                   chip_;
  Gtk::MenuItem                   action_;
  Gtk::MenuItem                   status_;

  Gtk::Menu                       chip_menu_;
  Gtk::SeparatorMenuItem          chip_sep1_;
  Gtk::MenuItem                   close_;

  Gtk::Menu                       action_menu_;
  Gtk::MenuItem                   compile_rom_;

  Gtk::Menu                       status_menu_;

public:
  Ga144Window(EvbView&, const std::string& title, ga144::Chip_ptr);
  virtual ~Ga144Window();

  EvbView& owner() {return owner_;}
  const EvbView& owner() const {return owner_;}

  void modify();

  void on_close();

  void read(xml::Iterator&);
  void write(xml::Writer&);

  void name(const std::string& val);

  F18aWindow* node_window(size_t node, bool assert) {return view_.node_window(node, assert);}
  bool rename_module(const std::string& old_name, const std::string& new_name) {return view_.rename_module(old_name, new_name);}

};

class EvbView : public Gtk::DrawingArea {
  //typedef ConnectionWindow* ConnectionWindowPtr;
  //std::vector<ConnectionWindowPtr>  connection_window_list_;

  EvbWindow&                      window_;
  evb::Board_ptr                  model_;
  Gtk::Image*                     image_;
  Ga144Window*                    host_window_;
  Ga144Window*                    target_window_;
  ConnectionWindow*               connection_;
  PackageWindow*                  package_;
  Glib::Dispatcher                connection_status_change_dispatcher_;
  Glib::Dispatcher                modify_dispatcher_;

  bool                            connector_highlight_[evb::Connector::limit];
  bool                            jumper_highlight_[evb::Jumper::limit];
  bool                            pin_highlight_[evb::Pin::limit];

  double                          zoom_;
  double                          trans_x_;
  double                          trans_y_;
  double                          image_w_;
  double                          image_h_;
  bool                            show_connector_;
  bool                            show_jumper_;
  bool                            show_host_pin_;
  bool                            show_target_pin_;

public:
  EvbView(EvbWindow&, evb::Board_ptr);
  virtual ~EvbView();

  evb::Board_ptr model() {return model_;}
  const evb::Board_ptr model() const {return model_;}

  EvbWindow& window() {return window_;}
  const EvbWindow& window() const {return window_;}

  Ga144Window& assert_host_window();
  Ga144Window& assert_target_window();
  
  void name(const std::string&);
  const std::string& name() const;

  void remove_image();
  void set_image(Gtk::Image* image);

  double width() const;
  double height() const;

  void read(xml::Iterator&);
  void write(xml::Writer&);
  
  //void attach(evb::Board&);
  //void detach();
  void reset();

  bool show_connector() const {return show_connector_;}
  void show_connector(bool);

  bool show_jumper() const {return show_jumper_;}
  void show_jumper(bool);

  bool show_host_pin() const {return show_host_pin_;}
  void show_host_pin(bool);

  bool show_target_pin() const {return show_target_pin_;}
  void show_target_pin(bool);

  bool connector_highlighted(size_t pos) const {return connector_highlight_[pos];}
  bool jumper_highlighted(size_t pos) const {return jumper_highlight_[pos];}
  bool pin_highlighted(size_t pos) const {return pin_highlight_[pos];}

  void highlight_connector(size_t, bool);
  void highlight_jumper(size_t, bool);
  void highlight_pin(size_t, bool);

  static bool pin_rect(size_t pin, size_t pos, Gdk::Rectangle& rect, Gdk::Point& move_tag, size_t& size);
  static bool connector_rect(size_t connector, Gdk::Rectangle& rect, Gdk::Point& move_tag, double& y1);
  static bool jumper_rect(size_t jumper, Gdk::Rectangle& rect);
  static bool usb_rect(size_t usb, Gdk::Rectangle& rect);
  static bool chip_rect(size_t chip, Gdk::Rectangle& rect);
  static bool inside_rect(int x, int y, const Gdk::Rectangle& rect);

  void on_area_size_changed(const Gdk::Rectangle&);
  bool on_draw (const ::Cairo::RefPtr< ::Cairo::Context >& cr);
  void draw(const Cairo::RefPtr<Cairo::Context>&, const Glib::RefPtr<Pango::Layout>&, double x, double y);

  bool on_button_press_event(GdkEventButton*);
  bool on_scroll_event(GdkEventScroll *ev);

  void update(ConnectionWindow&, size_t port);
  void on_connection_change();
  void on_modify();
  void modify();

  bool on_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip);

  F18aWindow* node_window(size_t node, bool assert);
  bool rename_module(const std::string& old_name, const std::string& new_name);

};

class EvbWindow : public Gtk::Window {
  MainWindow&                     owner_;
  EvbView                         view_;
  Gtk::EventBox                   view_event_;
  Gtk::VBox                       content_;
  Gtk::MenuBar                    menu_;
  Gtk::MenuItem                   board_;
  Gtk::MenuItem                   mview_;
  Gtk::MenuItem                   action_;
  Gtk::MenuItem                   highlight_;
  Gtk::MenuItem                   modules_;
  Gtk::MenuItem                   packages_;

  Gtk::Menu                       board_menu_;
  Gtk::MenuItem                   reset_;
  Gtk::MenuItem                   connect_;
  Gtk::MenuItem                   boot_async_708_;
  Gtk::MenuItem                   boot_async_708_kraken_707_;
  Gtk::MenuItem                   boot_async_708_kraken_709_;
  Gtk::MenuItem                   boot_async_708_kraken_707_709_;
  Gtk::SeparatorMenuItem          file_sep1_;
  Gtk::MenuItem                   boot_async_708_host_;
  Gtk::MenuItem                   boot_async_708_host_target_;
  Gtk::MenuItem                   boot_fast_708_host_;
  Gtk::MenuItem                   boot_fast_708_host_target_;
  Gtk::SeparatorMenuItem          file_sep2_;
  Gtk::MenuItem                   remove_all_tentacles_;
  Gtk::MenuItem                   span_host_;
  Gtk::MenuItem                   span_target_;
  Gtk::MenuItem                   span_host_target_;
  Gtk::SeparatorMenuItem          file_sep3_;
  Gtk::MenuItem                   close_;

  Gtk::Menu                       view_menu_;
  Gtk::CheckMenuItem              view_connector_;
  Gtk::CheckMenuItem              view_jumper_;
  Gtk::CheckMenuItem              view_host_pin_;
  Gtk::CheckMenuItem              view_target_pin_;

  Gtk::Menu                       action_menu_;
  Gtk::MenuItem                   action_host_io_;
  Gtk::MenuItem                   action_target_io_;
  //Gtk::CheckMenuItem              studio_;
  //Gtk::CheckMenuItem              poly_;
  Gtk::MenuItem                   test_echo_;

  Gtk::Menu*                      modules_menu_;
  Gtk::MenuItem                   modules_new_;
  Gtk::MenuItem                   modules_update_;
  Gtk::SeparatorMenuItem          modules_sep1_;
  std::map<std::string, Gtk::MenuItem*>   module_map_;
  std::map<std::string, ModuleWindow*>    module_window_map_;
  ModuleWindow*                   new_module_window_;

  Gtk::Menu*                      packages_menu_;
  Gtk::MenuItem                   packages_new_;
  Gtk::SeparatorMenuItem          packages_sep1_;
  std::map<std::string, Gtk::MenuItem*>   package_map_;
  std::map<std::string, PackageWindow*>   package_window_map_;
  PackageWindow*                  new_package_window_;

  evb::Manager_ptr                manager_;

  EchoTestWindow*                 test_echo_window_;
  IOWindow*                       host_io_;
  IOWindow*                       target_io_;

  std::vector<ModuleWindow*>      module_window_list_;
  std::vector<PackageWindow*>     package_window_list_;

  Glib::Dispatcher                error_dispatcher_;
  Glib::Dispatcher                success_dispatcher_;

  Gtk::Menu                       highlight_menu_;
  Gtk::MenuItem                   highlight_connector_;
  Gtk::MenuItem                   highlight_jumper_;
  Gtk::MenuItem                   highlight_host_pin_;
  Gtk::MenuItem                   highlight_target_pin_;
  Gtk::Menu                       highlight_connector_menu_;
  Gtk::Menu                       highlight_jumper_menu_;
  Gtk::Menu                       highlight_host_pin_menu_;
  Gtk::Menu                       highlight_target_pin_menu_;
  Gtk::CheckMenuItem              connector_highlight_[evb::Connector::limit];
  Gtk::CheckMenuItem              jumper_highlight_[evb::Jumper::limit];
  Gtk::CheckMenuItem              pin_highlight_[evb::Pin::t_bus];

  void delete_all_module_windows();
  void delete_all_package_windows();

public:
  EvbWindow(MainWindow&, evb::Board_ptr);
  virtual ~EvbWindow();

  EvbView& view() {return view_;}
  const EvbView& view() const {return view_;}

  evb::Manager_ptr manager();
  void modify();

  F18aWindow* node_window(size_t node, bool assert) {return view_.node_window(node, assert);};
  bool rename_module(const std::string& old_name, const std::string& new_name) {return view_.rename_module(old_name, new_name);}

  void name(const std::string&);
  //void attach(evb::Board&);
  //void detach();

  void read(xml::Iterator&);
  void write(xml::Writer&);

  void reset();
  void on_close();
  void on_reset();
  void on_boot_async_708();
  void on_boot_async_708_kraken_707();
  void on_boot_async_708_kraken_709();
  void on_boot_async_708_kraken_707_709();
  void on_boot_async_708_host();
  void on_boot_async_708_host_target();
  void on_boot_fast_708_host();
  void on_boot_fast_708_host_target();
  void on_remove_all_tentacles();

  void on_span_host();
  void on_span_target();
  void on_span_host_target();

  void on_host_chip();
  void on_target_chip();
  void on_test_echo();

  void on_host_io();
  void on_target_io();

  void on_error();
  void on_success();

  void on_view_connector();
  void on_view_jumper();
  void on_view_host_pin();
  void on_view_target_pin();

  void on_highlight_connector(size_t);
  void on_highlight_jumper(size_t);
  void on_highlight_pin(size_t);

  void on_update_modules();
  void on_new_module();
  void on_module_select(const std::string&);
  void accept_new_module(ModuleWindow&);

  void on_update_packages();
  void on_new_package();
  void on_package_select(const std::string&);
  void accept_new_package(PackageWindow&);

};

//class MainView : public Gtk::DrawingArea {
//private:
//  MainWindow&         window_;
//
//  MainView(const MainView&);
//  void operator=(const MainView&);
//
//
//public:
//  MainView(MainWindow&);
//
//  virtual ~MainView();
//
//  MainWindow& window() {return window_;}
//
//};

class MainWindow : public Gtk::Window {
  //MainView                        view_main_;
  Gtk::EventBox                   view_event_;
  Gtk::VBox                       content_;
  Gtk::Toolbar                    env_;
  Glib::RefPtr<Gtk::AccelGroup>   accel_;

  Gtk::MenuBar                    menu_;
  Gtk::MenuItem                   file_;
  Gtk::MenuItem                   edit_;
  Gtk::MenuItem                   view_;

  Gtk::Menu                       file_menu_;
  Gtk::MenuItem                   new_;

  Gtk::Menu                       new_menu_;
  Gtk::MenuItem                   new_evb001_;
  Gtk::MenuItem                   new_evb002_;

  Gtk::SeparatorMenuItem          file_sep1_;
  Gtk::MenuItem                   save_;
  Gtk::MenuItem                   save_as_;
  Gtk::MenuItem                   save_template_;
  Gtk::SeparatorMenuItem          file_sep2_;
  Gtk::MenuItem                   open_;
  Gtk::MenuItem                   open_template_;
  Gtk::SeparatorMenuItem          file_sep3_;
  Gtk::MenuItem                   quit_;
  Gtk::AccelKey*                  quit_accel_;

  Gtk::Menu                       edit_menu_;
  Gtk::MenuItem                   connection_;

  Gtk::Menu                       view_menu_;
  Gtk::MenuItem                   window_;
  Gtk::SeparatorMenuItem          view_sep1_;
  Gtk::MenuItem                   zoom_;
  Gtk::SeparatorMenuItem          view_sep2_;
  Gtk::MenuItem                   inc_width_;
  Gtk::MenuItem                   dec_width_;
  Gtk::MenuItem                   inc_height_;
  Gtk::MenuItem                   dec_height_;
  Gtk::CheckMenuItem              grid_;
  Gtk::CheckMenuItem              snap_;

  //ConnectionWindow*               connection_window_;
  std::vector<EvbWindow*>         board_list_;
  Gtk::Toolbar                    board_bar_;

  std::string                     filename_;

  Glib::Dispatcher                modify_dispatcher_;

  std::vector<EvbWindow*>         evb_window_list_;

  bool                            is_modified_;

public:
  MainWindow();
  virtual ~MainWindow();

  bool is_modified() const {return is_modified_;}
  void invalidate();
  void reset();
  void modify();
  void unmodify();
  void save();
  void load(const std::string& file);
  void store(const std::string& file);

  void on_modify();
  void on_menu_file_quit();
  void on_save();
  void on_new_001();
  void on_new_002();
  void open_new(const std::string& name, bool show, evb::Board_ptr);
  void on_click_board(EvbWindow&);
  //void on_connection_window();

  void setup(const std::string& file);

  void read(xml::Iterator&);
  void write(xml::Writer&);

  void filename(const std::string& val) {filename_ = val;}

};

}

#endif
