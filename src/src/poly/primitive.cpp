#include "pf.hpp"
#include "evb.hpp"
#include <map>

namespace pF {

typedef std::map<VirtualMachine::Primitive::Enum, cell> to_code_map_type;
typedef std::map<cell, VirtualMachine::Primitive::Enum> from_code_map_type;
typedef std::map<std::string, VirtualMachine::Primitive::Enum> name_map_type;

static to_code_map_type to_code_map_g;
static from_code_map_type from_code_map_g;
static name_map_type name_map_g;

std::string VirtualMachine::Primitive::initialize(evb::Manager& man)
{
  evb::Board_ptr board = man.board();
  evb::Module_ptr pf_mod = board->find_module("pFVM");
  if (pf_mod.is_null()) {return "module 'pFVM' not found";}
  if (!man.compile_module(*pf_mod)) {return "could not compile module 'pFVM'";}

  cell opcode_group = 0xf000;
  f18a::Processor_ptr proc = board->processor(5);
  if (proc.is_null()) {return "could not find node 005";}
  f18a::Module_ptr mod = proc->config_by_name("pFVM");
  if (mod.is_null()) {return "could not find compiled module 'pFVM' for node 005";}
  f18a::cell addr;
  if (!mod->find_label("'rdup", addr)) {return "could not find label 'rdup in node 005";}
  define(rdup, opcode_group | static_cast<cell>(addr), "rdup");
  if (!mod->find_label("'rdrop", addr)) {return "could not find label 'rdrop in node 005";}
  define(rdrop, opcode_group | static_cast<cell>(addr), "rdrop");
  if (!mod->find_label("'next", addr)) {return "could not find label 'next in node 005";}
  define(next, opcode_group | static_cast<cell>(addr), "(next)");
  if (!mod->find_label("'usr", addr)) {return "could not find label 'usr in node 005";}
  define(usr, opcode_group | static_cast<cell>(addr), "usr;");

  opcode_group = 0xb000;
  proc = board->processor(6);
  if (proc.is_null()) {return "could not find node 006";}
  mod = proc->config_by_name("pFVM");
  if (mod.is_null()) {return "could not find compiled module 'pFVM' for node 006";}
  if (!mod->find_label("'ex@", addr)) {return "could not find label 'ex@ in node 006";}
  define(xm_fetch, opcode_group | static_cast<cell>(addr), "x@");
  if (!mod->find_label("'ex!", addr)) {return "could not find label 'ex! in node 006";}
  define(xm_store, opcode_group | static_cast<cell>(addr), "x!");
  if (!mod->find_label("'cx?", addr)) {return "could not find label 'cx? in node 006";}
  define(xcx, opcode_group | static_cast<cell>(addr), "cx?");
  if (!mod->find_label("'mk!", addr)) {return "could not find label 'mk! in node 006";}
  define(mk_store, opcode_group | static_cast<cell>(addr), "mk!");
  if (!mod->find_label("'s", addr)) {return "could not find label 's in node 006";}
  define(sp_fetch, opcode_group | static_cast<cell>(addr), "sp@");
  if (!mod->find_label("'sus", addr)) {return "could not find label 'sus in node 006";}
  define(suspend, opcode_group | static_cast<cell>(addr), "suspend");
  if (!mod->find_label("'under", addr)) {return "could not find label 'under in node 006";}
  define(under, opcode_group | static_cast<cell>(addr), "under");
  if (!mod->find_label("'?dup", addr)) {return "could not find label '?dup in node 006";}
  define(qdup, opcode_group | static_cast<cell>(addr), "?dup");
  if (!mod->find_label("'min", addr)) {return "could not find label 'min in node 006";}
  define(min, opcode_group | static_cast<cell>(addr), "min");
  if (!mod->find_label("'max", addr)) {return "could not find label 'max in node 006";}
  define(max, opcode_group | static_cast<cell>(addr), "max");
  if (!mod->find_label("'u", addr)) {return "could not find label 'u in node 006";}
  define(u, opcode_group | static_cast<cell>(addr), "u");
  if (!mod->find_label("'u@", addr)) {return "could not find label 'u@ in node 006";}
  define(u_fetch, opcode_group | static_cast<cell>(addr), "u@");
  if (!mod->find_label("'u!", addr)) {return "could not find label 'u! in node 006";}
  define(u_store, opcode_group | static_cast<cell>(addr), "u!");

  opcode_group = 0xc000;
  proc = board->processor(105);
  if (proc.is_null()) {return "could not find node 105";}
  mod = proc->config_by_name("pFVM");
  if (mod.is_null()) {return "could not find compiled module 'pFVM' for node 105";}
  if (!mod->find_label("'else", addr)) {return "could not find label 'else in node 105";}
  define(else_, opcode_group | static_cast<cell>(addr), "(else)");
  if (!mod->find_label("'rp@", addr)) {return "could not find label 'rp@ in node 105";}
  define(rp_fetch, opcode_group | static_cast<cell>(addr), "rp@");
  if (!mod->find_label("'lit", addr)) {return "could not find label 'lit in node 105";}
  define(lit, opcode_group | static_cast<cell>(addr), "(lit)");
  if (!mod->find_label("'con", addr)) {return "could not find label 'con in node 105";}
  define(con, opcode_group | static_cast<cell>(addr), "con;");
  if (!mod->find_label("'var", addr)) {return "could not find label 'var in node 105";}
  define(var, opcode_group | static_cast<cell>(addr), "var;");
  if (!mod->find_label("'exit", addr)) {return "could not find label 'exit in node 105";}
  define(exit, opcode_group | static_cast<cell>(addr), "exit");
  if (!mod->find_label("'tor", addr)) {return "could not find label 'tor in node 105";}
  define(tor, opcode_group | static_cast<cell>(addr), ">r");
  if (!mod->find_label("'r@", addr)) {return "could not find label 'r@ in node 105";}
  define(r_fetch, opcode_group | static_cast<cell>(addr), "i");
  if (!mod->find_label("'rfrom", addr)) {return "could not find label 'rfrom in node 105";}
  define(rfrom, opcode_group | static_cast<cell>(addr), "r>");
  if (!mod->find_label("'exe", addr)) {return "could not find label 'exe in node 105";}
  define(exec, opcode_group | static_cast<cell>(addr), "(exec)");
  if (!mod->find_label("'if", addr)) {return "could not find label 'if in node 105";}
  define(if_, opcode_group | static_cast<cell>(addr), "(if)");
  if (!mod->find_label("'rx?", addr)) {return "could not find label 'rx? in node 105";}
  define(rxq, opcode_group | static_cast<cell>(addr), "rx?");
  if (!mod->find_label("'tx?", addr)) {return "could not find label 'tx? in node 105";}
  define(txq, opcode_group | static_cast<cell>(addr), "tx?");
  if (!mod->find_label("'rp!", addr)) {return "could not find label 'rp! in node 105";}
  define(rp_store, opcode_group | static_cast<cell>(addr), "rp!");
  
  opcode_group = 0x8000;
  proc = board->processor(106);
  if (proc.is_null()) {return "could not find node 106";}
  mod = proc->config_by_name("pFVM");
  if (mod.is_null()) {return "could not find compiled module 'pFVM' for node 106";}
  if (!mod->find_label("'1+", addr)) {return "could not find label '1+ in node 106";}
  define(inc, opcode_group | static_cast<cell>(addr), "1+");
  if (!mod->find_label("'1-", addr)) {return "could not find label '1- in node 106";}
  define(dec, opcode_group | static_cast<cell>(addr), "1-");
  if (!mod->find_label("'2*", addr)) {return "could not find label '2* in node 106";}
  define(mul2, opcode_group | static_cast<cell>(addr), "2*");
  if (!mod->find_label("'2/", addr)) {return "could not find label '2/ in node 106";}
  define(div2, opcode_group | static_cast<cell>(addr), "2/");
  if (!mod->find_label("'sp!", addr)) {return "could not find label 'sp! in node 106";}
  define(sp_store, opcode_group | static_cast<cell>(addr), "sp!");
  if (!mod->find_label("'!", addr)) {return "could not find label '! in node 106";}
  define(store, opcode_group | static_cast<cell>(addr), "!");
  if (!mod->find_label("'@", addr)) {return "could not find label '@ in node 106";}
  define(fetch, opcode_group | static_cast<cell>(addr), "@");
  if (!mod->find_label("'dup", addr)) {return "could not find label 'dup in node 106";}
  define(dup, opcode_group | static_cast<cell>(addr), "dup");
  if (!mod->find_label("'drop", addr)) {return "could not find label 'drop in node 106";}
  define(drop, opcode_group | static_cast<cell>(addr), "drop");
  if (!mod->find_label("'dip", addr)) {return "could not find label 'dip in node 106";}
  define(dip, opcode_group | static_cast<cell>(addr), "dup");
  if (!mod->find_label("'nip", addr)) {return "could not find label 'nip in node 106";}
  define(nip, opcode_group | static_cast<cell>(addr), "nip");
  if (!mod->find_label("'swap", addr)) {return "could not find label 'swap in node 106";}
  define(swap, opcode_group | static_cast<cell>(addr), "swap");
  if (!mod->find_label("'over", addr)) {return "could not find label 'over in node 106";}
  define(over, opcode_group | static_cast<cell>(addr), "over");
  if (!mod->find_label("'nop", addr)) {return "could not find label 'nop in node 106";}
  define(nop, opcode_group | static_cast<cell>(addr), "nop");
  if (!mod->find_label("'or", addr)) {return "could not find label 'or in node 106";}
  define(or, opcode_group | static_cast<cell>(addr), "or");
  if (!mod->find_label("'xor", addr)) {return "could not find label 'xor in node 106";}
  define(xor, opcode_group | static_cast<cell>(addr), "xor");
  if (!mod->find_label("'and", addr)) {return "could not find label 'and in node 106";}
  define(and, opcode_group | static_cast<cell>(addr), "and");
  if (!mod->find_label("'neg", addr)) {return "could not find label 'neg in node 106";}
  define(neg, opcode_group | static_cast<cell>(addr), "negate");
  if (!mod->find_label("'inv", addr)) {return "could not find label 'inv in node 106";}
  define(inv, opcode_group | static_cast<cell>(addr), "invert");
  if (!mod->find_label("'zeq", addr)) {return "could not find label 'zeq in node 106";}
  define(zeq, opcode_group | static_cast<cell>(addr), "0=");
  if (!mod->find_label("'zlt", addr)) {return "could not find label 'zlt in node 106";}
  define(zlt, opcode_group | static_cast<cell>(addr), "0<");
  if (!mod->find_label("'um+", addr)) {return "could not find label 'um+ in node 106";}
  define(umadd, opcode_group | static_cast<cell>(addr), "um+");
  if (!mod->find_label("'+", addr)) {return "could not find label '+ in node 106";}
  define(add, opcode_group | static_cast<cell>(addr), "+");
  if (!mod->find_label("'-", addr)) {return "could not find label '- in node 106";}
  define(sub, opcode_group | static_cast<cell>(addr), "-");

  opcode_group = 0xe000;
  proc = board->processor(205);
  if (proc.is_null()) {return "could not find node 205";}
  mod = proc->config_by_name("pFVM");
  if (mod.is_null()) {return "could not find compiled module 'pFVM' for node 205";}
  if (!mod->find_label("'8<<", addr)) {return "could not find label '8<< in node 205";}
  define(lsh8, opcode_group | static_cast<cell>(addr), "8<<");
  if (!mod->find_label("'8>>", addr)) {return "could not find label '8>> in node 205";}
  define(rsh8, opcode_group | static_cast<cell>(addr), "8>>");

  return "";
}


void VirtualMachine::Primitive::define(Enum val, cell code, const std::string& name)
{
  to_code_map_g[val] = code;
  from_code_map_g[code] = val;
  name_map_g[name] = val;
}

cell VirtualMachine::Primitive::to_code(Enum val)
{
  to_code_map_type::iterator it = to_code_map_g.find(val);
  if (it == to_code_map_g.end()) {
    return 0;
  }
  return it->second;
}

VirtualMachine::Primitive::Enum VirtualMachine::Primitive::from_code(cell val)
{
  from_code_map_type::iterator it = from_code_map_g.find(val);
  if (it == from_code_map_g.end()) {
    return undefined;
  }
  return it->second;
}

VirtualMachine::Primitive::Enum VirtualMachine::Primitive::from_name(const std::string& val)
{
  name_map_type::iterator it = name_map_g.find(val);
  if (it == name_map_g.end()) {
    return undefined;
  }
  return it->second;
}

}
