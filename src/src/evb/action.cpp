#include <evb.hpp>
#include <connection.hpp>


#include <gtkmm.h>

/**

pf 2281 contains code for async writing back data.

*/

namespace evb {



// ======================================================================================



// ======================================================================================

Action::Action()
: op_(nop)
{

}

Action::Action(Enum op)
: op_(op)
{

}

Action::Action(Enum op, long par1)
: op_(op)
{
  parameter_.push_back(par1);
}

Action::Action(Enum op, long par1, long par2)
: op_(op)
{
  parameter_.push_back(par1);
  parameter_.push_back(par2);
}

Action::Action(Enum op, Module_ptr m)
: op_(op)
, module_(m)
{
}

Action::Action(Enum op, const f18a::cell_list_type& d)
: op_(op)
, data_(d)
{
}

void Action::signal_error(Manager& man)
{
  man.error();
}

void Action::signal_success(Manager& man)
{
  man.success();
}


void Action::action(Manager& man)
{
  switch (op_) {
  case nop: break;
  case delimiter: break;
  case reset: if (!man.reset()) {signal_error(man);} break;
  case boot: if (!man.boot(static_cast<evb::BootMethod::Enum>(parameter_[0]))) {signal_error(man);} break;
  case activate_kraken: if (!man.activate_kraken(parameter_[0], true)) {signal_error(man);} break;
  case remove_all_tentacles: if (!man.remove_all_tentacles()) {signal_error(man);} break;
  case span_host: if (!man.span_host(true)) {signal_error(man);} break;
  case span_target: if (!man.span_target(true)) {signal_error(man);} break;
  case span_host_target: if (!man.span_host_target(true)) {signal_error(man);} break;
  case destroy_tentacle: if (!man.destroy_tentacle()) {signal_error(man);} break;
  //case add_tentacle_segment: if (parameter_.size() >= 1) {if (!man.add_tentacle_segment(static_cast<ga144::Neighbour::Enum>(parameter_[0]))) {signal_error(man);}} break;
  //case remove_last_tentacle_segment: if (!man.remove_last_tentacle_segment()) {signal_error(man);} break;
  case read_ram: if (!man.read_ram(parameter_[0])) {signal_error(man);} break; // node
  case read_rom: if (!man.read_rom(parameter_[0])) {signal_error(man);} break; // node
  case read_and_compare_rom: if (!man.read_and_compare_rom(parameter_[0])) {signal_error(man);} else {signal_success(man);} break; // node
  case read_ram_and_rom: if (!man.read_ram_and_rom(parameter_[0])) {signal_error(man);} break; // node
  case write_ram: if (!man.write_ram(parameter_[0])) {signal_error(man);} break; // node
  case read_stack: if (!man.read_stack(parameter_[0])) {signal_error(man);} break; // node
  case write_stack: if (!man.write_stack(parameter_[0])) {signal_error(man);} break; // node
  case read_return_stack: if (!man.read_return_stack(parameter_[0])) {signal_error(man);} break; // node
  case write_return_stack: if (!man.write_return_stack(parameter_[0])) {signal_error(man);} break; // node
  case read_a: if (!man.read_a(parameter_[0])) {signal_error(man);} break; // node
  case write_a: if (!man.write_a(parameter_[0])) {signal_error(man);} break; // node
  case write_b: if (!man.write_b(parameter_[0])) {signal_error(man);} break; // node
  case read_io: if (!man.read_io(parameter_[0])) {signal_error(man);} break; // node
  case write_io: if (!man.write_io(parameter_[0], F18_INVALID_VALUE)) {signal_error(man);} break; // node
  case read: if (!man.read(parameter_[0])) {signal_error(man);} break; // node
  case write: if (!man.write(parameter_[0])) {signal_error(man);} break; // node
  case expand_tentacle_to: if (!man.add_tentacle_to(parameter_[0], true)) {signal_error(man);} break; // node
  case reduce_tentacle_to: if (!man.reduce_tentacle_to(parameter_[0])) {signal_error(man);} break; // node
  case call: // node
  case jump: // node
  case run: if (!man.run(parameter_[0])) {signal_error(man);} else {signal_success(man);} break; // node
  case start: if (!man.start(parameter_[0])) {signal_error(man);} break; // node
  case test: if (!man.test(parameter_[0])) {signal_error(man);} else {signal_success(man);} break; // node
  case write_pin: if (!man.set_pin(static_cast<evb::Pin::Enum>(parameter_[0]), parameter_[1])) {signal_error(man);} break;
  case compile_module: if (!man.compile_module(module_)) {signal_error(man);} break; // node
  case write_module: if (!man.write_module(module_)) {signal_error(man);} break; // node
  case start_module: if (!man.start_module(module_)) {signal_error(man);} break; // node
  case boot_708: if (!man.boot(BootMethod::fast_708_host)) {signal_error(man);} break;
  case boot_via_708: if (!man.boot_via(BootMethod::fast_708_host, data_)) {signal_error(man);} break; // boot_stream
  case boot_708_both: if (!man.boot(BootMethod::fast_708_host_target)) {signal_error(man);} break;
  case boot_via_708_both: if (!man.boot_via(BootMethod::fast_708_host_target, data_)) {signal_error(man);} break; // boot_stream
  default:;
  }
}



}
