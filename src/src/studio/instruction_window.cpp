#include <studio.hpp>
#include <connection.hpp>
#include <f18a.hpp>

namespace studio {


InstructionWindow::InstructionWindow()
: view_(0)
, address_(0)
, value_(0)
, instr4_(instr_group)
, instr3_(instr_group)
, instr2_(instr_group)
, instr1_(instr_group)
, instr0_(instr_group)
{
  size_t i;
  std::string opname, opnum;
  this->set_modal(true);
  this->set_deletable(false);
  set_title("Memory Word Dialog");

  set_default();

  addr_label_.set_text("address");
  addr_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  ext_arith_label_.set_text("ext. arith.");
  ext_arith_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  label_label_.set_text("label");
  label_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  i3_data_label_.set_text(boost::lexical_cast<std::string>(F18A_FINAL_SLOT_BITS) + " bits");
  i3_data_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  i2_data_label_.set_text(boost::lexical_cast<std::string>(F18A_FINAL_SLOT_BITS+F18A_SLOT_BITS) + " bits");
  i2_data_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  i1_data_label_.set_text(boost::lexical_cast<std::string>(F18A_FINAL_SLOT_BITS+F18A_SLOT_BITS+F18A_SLOT_BITS) + " bits");
  i1_data_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  i0_data_label_.set_text(boost::lexical_cast<std::string>(F18A_CELL_BITS) + " bits");
  i0_data_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  hex_data_label_.set_text("hex data");
  hex_data_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  dec_data_label_.set_text("dec data");
  dec_data_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  lbl_data_label_.set_text("label data");
  lbl_data_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  instr4_.set_label("no data");
  instr3_.set_label("slot 0-2 + data");
  instr2_.set_label("slot 0-1 + data");
  instr1_.set_label("slot 0 + data");
  instr0_.set_label("all data");

  i3_hex_data_.set_max_length(1);
  i2_hex_data_.set_max_length(2);
  i1_hex_data_.set_max_length(4);
  i0_hex_data_.set_max_length(5);

  i3_dec_data_.set_max_length(1);
  i2_dec_data_.set_max_length(3);
  i1_dec_data_.set_max_length(4);
  i0_dec_data_.set_max_length(6);

  addr_.set_max_length(10);
  label_.set_max_length(10);
  i3_lbl_data_.set_max_length(10);
  i2_lbl_data_.set_max_length(10);
  i1_lbl_data_.set_max_length(10);
  i0_lbl_data_.set_max_length(10);


  for (i=0; i<f18a::Opcode::limit; ++i) {
    f18a::Opcode::Enum op = static_cast<f18a::Opcode::Enum>(i);
    f18a::Opcode::to_string(op, opname);
    opnum = boost::lexical_cast<std::string>(i);
    if (f18a::Opcode::fit_slot3(op)) {
      i4_slot3_.append(opnum, opname);
    }
    if (f18a::Opcode::has_addr_field(op)) {
      i3_slot2_.append(opnum, opname);
      i2_slot1_.append(opnum, opname);
      i1_slot0_.append(opnum, opname);
    }
    else {
      i4_slot0_.append(opnum, opname);
      i4_slot1_.append(opnum, opname);
      i4_slot2_.append(opnum, opname);
      i3_slot0_.append(opnum, opname);
      i3_slot1_.append(opnum, opname);
      i2_slot0_.append(opnum, opname);
    }
  }

  i3_hex_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i3_hex_data) );
  i3_dec_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i3_dec_data) );
  i3_lbl_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i3_lbl_data) );

  i2_hex_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i2_hex_data) );
  i2_dec_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i2_dec_data) );
  i2_lbl_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i2_lbl_data) );

  i1_hex_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i1_hex_data) );
  i1_dec_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i1_dec_data) );
  i1_lbl_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i1_lbl_data) );

  i0_hex_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i0_hex_data) );
  i0_dec_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i0_dec_data) );
  i0_lbl_data_.signal_changed().connect(sigc::mem_fun(*this, &InstructionWindow::on_modify_i0_lbl_data) );

  //baud_label_.set_text("Baudrate");
  //baud_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  //bits_label_.set_text("Bits per byte");
  //bits_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  //flow_label_.set_text("Flow control");
  //flow_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  //parity_label_.set_text("Parity");
  //parity_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  //stop_label_.set_text("Stop bits");
  //stop_label_.set_alignment(Gtk::JUSTIFY_LEFT);

  // buttons
  ok_button_.set_label("OK");
  button_box_.pack_start(ok_button_, Gtk::PACK_SHRINK);
  button_box_.set_border_width(5);
  button_box_.set_layout(Gtk::BUTTONBOX_END);
  ok_button_.signal_clicked().connect(sigc::mem_fun(*this, &InstructionWindow::on_button_ok) );
    
  revert_button_.set_label("Revert");
  button_box_.pack_start(revert_button_, Gtk::PACK_SHRINK);
  revert_button_.signal_clicked().connect(sigc::mem_fun(*this, &InstructionWindow::on_button_revert) );
    
  cancel_button_.set_label("Cancel");
  button_box_.pack_start(cancel_button_, Gtk::PACK_SHRINK);
  cancel_button_.signal_clicked().connect(sigc::mem_fun(*this, &InstructionWindow::on_button_cancel) );

  table_.set_column_homogeneous(false);
  table_.set_border_width(10);
  table_.set_row_spacing(5);
  table_.set_column_spacing(5);


  size_t line = 0;

  table_.attach(addr_label_,          0, line, 1, 1);
  table_.attach(addr_,                1, line, 3, 1);
  ++line;

  table_.attach(label_label_,         0, line, 1, 1);
  table_.attach(label_,               1, line, 3, 1);
  table_.attach(ext_arith_,           4, line, 1, 1);
  table_.attach(ext_arith_label_,     5, line, 1, 1);
  ++line;

  table_.attach(instr4_,              0, line, 1, 1);
  table_.attach(i4_slot0_,            1, line, 1, 1);
  table_.attach(i4_slot1_,            2, line, 1, 1);
  table_.attach(i4_slot2_,            3, line, 1, 1);
  table_.attach(i4_slot3_,            4, line, 1, 1);
  table_.attach(hex_data_label_,      5, line, 1, 1);
  table_.attach(dec_data_label_,      6, line, 1, 1);
  table_.attach(lbl_data_label_,      7, line, 1, 1);
  ++line;

  table_.attach(instr3_,              0, line, 1, 1);
  table_.attach(i3_slot0_,            1, line, 1, 1);
  table_.attach(i3_slot1_,            2, line, 1, 1);
  table_.attach(i3_slot2_,            3, line, 1, 1);
  table_.attach(i3_data_label_,       4, line, 1, 1);
  table_.attach(i3_hex_data_,         5, line, 1, 1);
  table_.attach(i3_dec_data_,         6, line, 1, 1);
  table_.attach(i3_lbl_data_,         7, line, 1, 1);
  ++line;

  table_.attach(instr2_,              0, line, 1, 1);
  table_.attach(i2_slot0_,            1, line, 1, 1);
  table_.attach(i2_slot1_,            2, line, 1, 1);
  table_.attach(i2_data_label_,       4, line, 1, 1);
  table_.attach(i2_hex_data_,         5, line, 1, 1);
  table_.attach(i2_dec_data_,         6, line, 1, 1);
  table_.attach(i2_lbl_data_,         7, line, 1, 1);
  ++line;

  table_.attach(instr1_,              0, line, 1, 1);
  table_.attach(i1_slot0_,            1, line, 1, 1);
  table_.attach(i1_data_label_,       4, line, 1, 1);
  table_.attach(i1_hex_data_,         5, line, 1, 1);
  table_.attach(i1_dec_data_,         6, line, 1, 1);
  table_.attach(i1_lbl_data_,         7, line, 1, 1);
  ++line;

  table_.attach(instr0_,              0, line, 1, 1);
  table_.attach(i0_data_label_,       4, line, 1, 1);
  table_.attach(i0_hex_data_,         5, line, 1, 1);
  table_.attach(i0_dec_data_,         6, line, 1, 1);
  table_.attach(i0_lbl_data_,         7, line, 1, 1);
  ++line;

  //table_.attach(baud_,                1, 1, 1, 1);
  //table_.attach(bits_label_,          0, 2, 1, 1);
  //table_.attach(bits_,                1, 2, 1, 1);
  //table_.attach(flow_label_,          0, 3, 1, 1);
  //table_.attach(flow_,                1, 3, 1, 1);
  //table_.attach(parity_label_,        0, 4, 1, 1);
  //table_.attach(parity_,              1, 4, 1, 1);
  //table_.attach(stop_label_,          0, 5, 1, 1);
  //table_.attach(stop_,                1, 5, 1, 1);
  table_.attach(button_box_,          0, line, 6, 1);

  add(table_);
  show_all_children();

}

InstructionWindow::~InstructionWindow()
{
}

void InstructionWindow::setup(F18aView& view, size_t addr)
{
  std::string tmp;
  size_t dest;
  f18a::addr_map_type addr_map;
  view.model()->addr_map(addr_map);
  view_ = &view;
  address_ = addr;
  value_ = view.model()->module()->ram().data(addr);
  {
    std::stringstream ss;
    ss << std::setfill ('0') << std::setw(3)  << std::hex << addr;
    addr_.set_text(ss.str());
  }
  {
    f18a::addr_map_type::iterator it = addr_map.find(addr);
    if (it != addr_map.end()) {
      label_.set_text(it->second);
    }
  }
  f18a::Opcode::Enum op;
  op = static_cast<f18a::Opcode::Enum>(f18a::Processor::get_instruction(value_, 0));
  std::string opval = boost::lexical_cast<std::string>(op);
  i4_slot0_.set_active_id(opval);
  i3_slot0_.set_active_id(opval);
  i2_slot0_.set_active_id(opval);
  i1_slot0_.set_active_id(opval);
  if (f18a::Opcode::has_addr_field(op)) {
    instr1_.set_active();
    dest = f18a::Processor::get_data(value_, 1);
    {
      std::stringstream ss;
      ss << std::hex << dest;
      i0_hex_data_.set_text(ss.str());
      i1_hex_data_.set_text(ss.str());
      i2_hex_data_.set_text(ss.str());
      i3_hex_data_.set_text(ss.str());
    }
    tmp = boost::lexical_cast<std::string>(dest);
    i0_dec_data_.set_text(tmp);
    i1_dec_data_.set_text(tmp);
    i2_dec_data_.set_text(tmp);
    i3_dec_data_.set_text(tmp);
    f18a::addr_map_type::iterator it = addr_map.find(dest);
    if (it != addr_map.end()) {
      i0_lbl_data_.set_text(it->second);
      i1_lbl_data_.set_text(it->second);
      i2_lbl_data_.set_text(it->second);
      i3_lbl_data_.set_text(it->second);
    }
  }
  else {
    op = static_cast<f18a::Opcode::Enum>(f18a::Processor::get_instruction(value_, 1));
    opval = boost::lexical_cast<std::string>(op);
    i4_slot1_.set_active_id(opval);
    i3_slot1_.set_active_id(opval);
    i2_slot1_.set_active_id(opval);
    if (f18a::Opcode::has_addr_field(op)) {
      instr2_.set_active();
      dest = f18a::Processor::get_data(value_, 2);
      {
        std::stringstream ss;
        ss << std::hex << dest;
        i0_hex_data_.set_text(ss.str());
        i1_hex_data_.set_text(ss.str());
        i2_hex_data_.set_text(ss.str());
        i3_hex_data_.set_text(ss.str());
      }
      tmp = boost::lexical_cast<std::string>(dest);
      i0_dec_data_.set_text(tmp);
      i1_dec_data_.set_text(tmp);
      i2_dec_data_.set_text(tmp);
      i3_dec_data_.set_text(tmp);
      f18a::addr_map_type::iterator it = addr_map.find(dest);
      if (it != addr_map.end()) {
        i0_lbl_data_.set_text(it->second);
        i1_lbl_data_.set_text(it->second);
        i2_lbl_data_.set_text(it->second);
        i3_lbl_data_.set_text(it->second);
      }
    }
    else {
      op = static_cast<f18a::Opcode::Enum>(f18a::Processor::get_instruction(value_, 2));
      opval = boost::lexical_cast<std::string>(op);
      i4_slot2_.set_active_id(opval);
      i3_slot2_.set_active_id(opval);
      if (f18a::Opcode::has_addr_field(op)) {
        instr3_.set_active();
        dest = f18a::Processor::get_data(value_, 3);
        {
          std::stringstream ss;
          ss << std::hex << dest;
          i0_hex_data_.set_text(ss.str());
          i1_hex_data_.set_text(ss.str());
          i2_hex_data_.set_text(ss.str());
          i3_hex_data_.set_text(ss.str());
        }
        tmp = boost::lexical_cast<std::string>(dest);
        i0_dec_data_.set_text(tmp);
        i1_dec_data_.set_text(tmp);
        i2_dec_data_.set_text(tmp);
        i3_dec_data_.set_text(tmp);
        f18a::addr_map_type::iterator it = addr_map.find(dest);
        if (it != addr_map.end()) {
          i0_lbl_data_.set_text(it->second);
          i1_lbl_data_.set_text(it->second);
          i2_lbl_data_.set_text(it->second);
          i3_lbl_data_.set_text(it->second);
        }
      }
      else {
        instr4_.set_active();
        op = static_cast<f18a::Opcode::Enum>(f18a::Processor::get_instruction(value_, 3));
        opval = boost::lexical_cast<std::string>(op);
        i4_slot3_.set_active_id(opval);
      }
    }
  }
}

void InstructionWindow::set_default()
{
}

void InstructionWindow::set_undefined()
{
}

void InstructionWindow::open()
{
  show();
}

bool InstructionWindow::add_instruction(Gtk::ComboBoxText& txt, long& val)
{
  int sel = txt.get_active_row_number();
  if (sel >= 0) {
    try {
      std::string opname = txt.get_active_id();
      size_t op = boost::lexical_cast<size_t>(opname);
      val <<= F18A_SLOT_BITS;
      val += op;
      return true;
    }
    catch (...) {
    }
  }
  return false;
}

bool InstructionWindow::read_hex(const std::string& txt, long& value)
{
  try {
    std::stringstream ss;
    ss << std::hex << txt;
    ss >> value;
    return true;
  }
  catch (...) {}
  return false;
}

bool InstructionWindow::read_dec(const std::string& txt, long& value)
{
  try {
    std::stringstream ss;
    ss << txt;
    ss >> value;
    return true;
  }
  catch (...) {}
  return false;
}

void InstructionWindow::on_modify_i3_hex_data()
{
  i3_dec_data_.set_text("");
  i3_lbl_data_.set_text("");
}

void InstructionWindow::on_modify_i3_dec_data()
{
  i3_hex_data_.set_text("");
  i3_lbl_data_.set_text("");
}

void InstructionWindow::on_modify_i3_lbl_data()
{
  i3_hex_data_.set_text("");
  i3_dec_data_.set_text("");
}

void InstructionWindow::on_modify_i2_hex_data()
{
  i2_dec_data_.set_text("");
  i2_lbl_data_.set_text("");
}

void InstructionWindow::on_modify_i2_dec_data()
{
  i2_hex_data_.set_text("");
  i2_lbl_data_.set_text("");
}

void InstructionWindow::on_modify_i2_lbl_data()
{
  i2_hex_data_.set_text("");
  i2_dec_data_.set_text("");
}

void InstructionWindow::on_modify_i1_hex_data()
{
  i1_dec_data_.set_text("");
  i1_lbl_data_.set_text("");
}

void InstructionWindow::on_modify_i1_dec_data()
{
  i1_hex_data_.set_text("");
  i1_lbl_data_.set_text("");
}

void InstructionWindow::on_modify_i1_lbl_data()
{
  i1_hex_data_.set_text("");
  i1_dec_data_.set_text("");
}

void InstructionWindow::on_modify_i0_hex_data()
{
  i0_dec_data_.set_text("");
  i0_lbl_data_.set_text("");
}

void InstructionWindow::on_modify_i0_dec_data()
{
  i0_hex_data_.set_text("");
  i0_lbl_data_.set_text("");
}

void InstructionWindow::on_modify_i0_lbl_data()
{
  i0_hex_data_.set_text("");
  i0_dec_data_.set_text("");
}

void InstructionWindow::on_button_ok()
{
  long value;
  hide();
  if (view_) {
    if (read_hex(addr_.get_text(), value)) {
      if (value >= F18A_RAM_SIZE) {
        Gtk::MessageDialog dialog(*this, "Reading address failed");
        dialog.set_secondary_text("The address is too big");
        dialog.run();
        return;
      }
      address_ = value;
    }
    else {
      Gtk::MessageDialog dialog(*this, "Reading address failed");
      dialog.set_secondary_text("The address is not a hex number");
      dialog.run();
      return;
    }
    //try {
    //  std::stringstream ss;
    //  ss << std::hex << addr_.get_text();
    //  ss >> address_;
    //  if (address_ >= F18A_RAM_SIZE) {
    //    Gtk::MessageDialog dialog(*this, "Reading address failed");
    //    dialog.set_secondary_text("The address is too big");
    //    dialog.run();
    //    return;
    //  }
    //}
    //catch (...) {
    //  Gtk::MessageDialog dialog(*this, "Reading address failed");
    //  dialog.set_secondary_text("The address is not a hex number");
    //  dialog.run();
    //  return;
    //}
    long data, mask, value = 0;
    std::string label;
    // address is ok
    if (instr4_.get_active()) {
      // 4 instruction slots
      if (
        !add_instruction(i4_slot0_, value)
        || !add_instruction(i4_slot1_, value)
        || !add_instruction(i4_slot2_, value)
        || !add_instruction(i4_slot3_, value)
      ) {
        Gtk::MessageDialog dialog(*this, "Generating word failed");
        dialog.set_secondary_text("Some instruction is invalid (4)");
        dialog.run();
        return;
      }
      value >>= F18A_SKIP_SLOT_BITS;
      value ^= 0x15555;
    }
    else if (instr3_.get_active()) {
      // 3 instruction slots
      if (
        !add_instruction(i3_slot0_, value)
        || !add_instruction(i3_slot1_, value)
        || !add_instruction(i3_slot2_, value)
      ) {
        Gtk::MessageDialog dialog(*this, "Generating word failed");
        dialog.set_secondary_text("Some instruction is invalid (3)");
        dialog.run();
        return;
      }
      value <<= F18A_FINAL_SLOT_BITS;
      mask = F18A_FINAL_SLOT_MASK;
      value ^= 0x15555 & ~mask;
      data = 0;
      if (read_hex(i3_hex_data_.get_text(), data)) {
      }
      else if (read_dec(i3_dec_data_.get_text(), data)) {
      }
      else {
        label = i3_lbl_data_.get_text();
        if (!label.empty()) {
        }
      }
      value |= data & mask;
    }
    else if (instr2_.get_active()) {
      // 2 instruction slots
    }
    else if (instr1_.get_active()) {
      // 1 instruction slots
    }
    else {
      // data only
    }
    value_ = value;
    view_->model()->module()->ram().data(address_, value_);
    view_->queue_draw();
  }
}

void InstructionWindow::on_button_cancel()
{
  hide();
}

void InstructionWindow::on_button_revert()
{
}


}
