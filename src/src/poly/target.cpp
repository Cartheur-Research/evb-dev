#include "pf.hpp"

namespace target {

Word::Word(const std::string& n, pF::VirtualMachine::Primitive::Enum)
: name_(n)
, type_(Type::primitive)
, opcode_(0)
, rom_nfa_(0)
, rom_cfa_(0)
{
}

Word::Word(const std::string& n, pF::VirtualMachine& vm)
: name_(n)
, type_(Type::secondary)
, opcode_(0)
, rom_nfa_(0)
, rom_cfa_(0)
{
  
}

Word::Word(const std::string& n, cell)
: name_(n)
, type_(Type::literal_)
, opcode_(0)
, rom_nfa_(0)
, rom_cfa_(0)
{
}


}
