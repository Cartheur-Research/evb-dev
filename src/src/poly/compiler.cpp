#include "pf.hpp"

namespace pF {

Compiler::Compiler()
: vm_(0)
, h_(0)
, text_(0)
, text_len_(0)
, offset_(0)
, line_(0)
, pos_(0)
, compiling_(false)
{
}

Compiler::~Compiler()
{
}

void Compiler::error(const std::string& name, const std::string& reason)
{
  Error res;
  res.offset_ = offset_;
  res.line_ = line_;
  res.position_ = pos_;
  res.name_ = name;
  res.reason_ = reason;
  error_list_.push_back(res);
}

size_t Compiler::skip(bool match, char del)
{
  size_t cnt = 0;
  if (del == ' ') {
    if (match) {
      while (offset_ < text_len_) {
        if (text_[offset_] > del) {
          break;
        }
        ++offset_;
        ++cnt;
      }
    }
    else {
      while (offset_ < text_len_) {
        if (text_[offset_] <= del) {
          break;
        }
        ++offset_;
        ++cnt;
      }
    }
  }
  else {
    if (match) {
      while (pos_ < text_len_) {
        if (text_[offset_] != del) {
          break;
        }
        ++offset_;
        ++cnt;
      }
    }
    else {
      while (pos_ < text_len_) {
        if (text_[offset_] == del) {
          break;
        }
        ++offset_;
        ++cnt;
      }
      //error("", "end of file before delimiter reached");
    }
  }
  return cnt;
}

std::string Compiler::word(char del)
{
  size_t start;
  size_t cnt;
  if (del == ' ') {
    skip(true, del);
    start = offset_;
    cnt = skip(false, del);
    if (cnt > 0) {
      return std::string(text_ + start, cnt);
    }
  }
  else {
    start = offset_;
    cnt = skip(false, del);
    if (cnt > 0) {
      return std::string(text_ + start, cnt);
    }
  }
  return "";
}

void Compiler::compile_word(cell w)
{
  if (vm_) {
    vm_->store(h_++, w);
  }
}

void Compiler::compile_next_word()
{
  std::string w = word();
  if (compiling_) {
    // check for primitive
    VirtualMachine::Primitive::Enum pr = VirtualMachine::Primitive::from_name(w);
    if (pr != VirtualMachine::Primitive::undefined) {
      // compile primitive code
      compile_word(VirtualMachine::Primitive::to_code(pr));
      return;
    }
    // check for compiling word
    // check for target word
    dictionary_type::iterator it = target_dictionary_.find(w);
    if (it != target_dictionary_.end()) {
      // compile address
      compile_word(it->second);
      return;
    }
    // try number
  }
  else {
    // interpreting
  }
}

Compiler::error_list_type Compiler::compile(const char* txt, size_t len)
{
  text_ = txt;
  text_len_ = len;
  offset_ = 0;
  line_ = 0;
  pos_ = 0;
  stack_.clear();
  error_list_.clear();
  target_dictionary_.clear();
  while (offset_ < text_len_) {
    compile_next_word();
  }
  if (compiling_) {
    error("", "end of file reached and still compiling");
  }
  return error_list_;
}

}

//================================================================================

namespace target {


Compiler::Compiler(pF::VirtualMachine& vm)
: vm_(vm)
, line_(0)
, line_pos_(0)
, compiling_(false)
{
}

Compiler::~Compiler()
{
}

#define CR  13
#define LF  10
#define CR_MASK 1
#define LF_MASK 2

void Compiler::skip_whitespace(const char* txt, size_t len, size_t& pos)
{
  int endline_mask = 0;
  while ((pos < len) && (txt[pos] <= ' ')) {
    switch (txt[pos]) {
    case CR:
      if ((endline_mask & CR_MASK) != 0) {
        ++line_;
        line_pos_ = 0;
      }
      else if (endline_mask == 0) {
        endline_mask |= CR_MASK;
        ++line_;
        line_pos_ = 0;
      }
      else {
        endline_mask |= CR_MASK;
      }
      break;

    case LF:
      if ((endline_mask & LF_MASK) != 0) {
        ++line_;
        line_pos_ = 0;
      }
      else if (endline_mask == 0) {
        endline_mask |= LF_MASK;
        ++line_;
        line_pos_ = 0;
      }
      else {
        endline_mask |= LF_MASK;
      }
      break;

    default:
      endline_mask = 0;
      ++line_pos_;
    }
    ++pos;
  }
}

std::string Compiler::next(const char* txt, size_t len, char del, size_t& pos)
{
  if (del == ' ') {
    skip_whitespace(txt, len, pos);
    if (pos >= len) {return "";}
    size_t start = pos;
    while ((pos < len) && (txt[pos] > ' ')) {
      ++pos;
      ++line_pos_;
    }
    return std::string(txt+start, pos-start);
  }
  if (pos >= len) {return "";}
  size_t start = pos;
  int endline_mask = 0;
  while ((pos < len) && (txt[pos] != del)) {
    switch (txt[pos]) {
    case CR:
      if ((endline_mask & CR_MASK) != 0) {
        ++line_;
        line_pos_ = 0;
      }
      else if (endline_mask == 0) {
        endline_mask |= CR_MASK;
        ++line_;
        line_pos_ = 0;
      }
      else {
        endline_mask |= CR_MASK;
      }
      break;

    case LF:
      if ((endline_mask & LF_MASK) != 0) {
        ++line_;
        line_pos_ = 0;
      }
      else if (endline_mask == 0) {
        endline_mask |= LF_MASK;
        ++line_;
        line_pos_ = 0;
      }
      else {
        endline_mask |= LF_MASK;
      }
      break;

    default:
      endline_mask = 0;
      ++line_pos_;
    }
    ++pos;
  }
  if (pos == len) {
    // error
    return "";
  }
  return std::string(txt+start, pos-start);
}

void Compiler::compile(const char* txt, size_t len)
{
  size_t pos = 0;
  line_ = 0;
  line_pos_ = 0;
  while (pos < len) {
    std::string name = next(txt, len, ' ', pos);
    if (name.empty()) {break;}
  }
}


}

