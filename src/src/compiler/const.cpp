#include "cvmc.hpp"

namespace cvmc {

Constant::Constant() : literal_(0) {}
Constant::Constant(const context_type::position_type& w) : Expression(w), literal_(0) {}

ExprType Constant::expr_type() const {return ET_CONST;}
bool Constant::is_const() const {return true;}

void Constant::build_type()
{
}
//
//bool Constant::is_negative() const
//{
//	return false;
//}

Type* Constant::lvalue_type()
{
	return this->type();
}


//*************************************************************************************

bool IntConst::get_int(int& val) const {val = value_; return true;}
bool IntConst::get_long(long& val) const {val = value_; return true;}
bool IntConst::get_longlong(long long& val) const {val = value_; return true;}

bool IntConst::is_negative() const
{
	return value_ < 0;
}

int IntConst::compile(CompilationUnit& cu, bool push_result)
{
  if (push_result) {
  	return cu.code().compile_push_literal(value_);
  }
	return cu.code().compile_literal(value_);
}

void IntConst::compile_data(CompilationUnit& cu)
{
	cu.data().append_data(value_);
}

void IntConst::build_type()
{
	set_type(new Type(T_SINT));
}

//*************************************************************************************

bool UIntConst::get_int(int& val) const {val = value_; return true;}
bool UIntConst::get_long(long& val) const {val = value_; return true;}
bool UIntConst::get_longlong(long long& val) const {val = value_; return true;}

int UIntConst::compile(CompilationUnit& cu, bool push_result)
{
  if (push_result) {
  	return cu.code().compile_push_literal(static_cast<short>(value_));
  }
	return cu.code().compile_literal(static_cast<short>(value_));
}

void UIntConst::compile_data(CompilationUnit& cu)
{
	cu.data().append_data(value_);
}

void UIntConst::build_type()
{
	set_type(new Type(T_UINT));
}

//*************************************************************************************
	
bool LongConst::get_int(int& val) const {if ((value_	>= MIN_INT) && (value_ <= MAX_INT)) {val =	static_cast<int>(value_); return true;} return false;}
bool LongConst::get_long(long& val) const {val = value_; return true;}
bool LongConst::get_longlong(long long& val) const {val = value_; return true;}

bool LongConst::is_negative() const
{
	return value_ < 0;
}

int LongConst::compile(CompilationUnit& cu, bool push_result)
{
  if (push_result) {
  	return cu.code().compile_push_long(value_);
  }
	return cu.code().compile_literal_long(value_);
}

void LongConst::compile_data(CompilationUnit& cu)
{
	cu.data().append_data(value_ >> 16);
	cu.data().append_data(value_ & 0xffff);
}

void LongConst::build_type()
{
	set_type(new Type(T_SLONG));
}

//*************************************************************************************

bool ULongConst::get_int(int& val) const {if (value_ <= MAX_INT) {val =	static_cast<int>(value_); return true;} return false;}
bool ULongConst::get_long(long& val) const {val = value_; return true;}
bool ULongConst::get_longlong(long long& val) const {val = value_; return true;}

int ULongConst::compile(CompilationUnit& cu, bool push_result)
{
  if (push_result) {
  	return cu.code().compile_push_long(static_cast<long>(value_));
  }
	return cu.code().compile_literal_long(static_cast<long>(value_));
}

void ULongConst::compile_data(CompilationUnit& cu)
{
	cu.data().append_data(value_ >> 16);
	cu.data().append_data(value_ & 0xffff);
}

void ULongConst::build_type()
{
	set_type(new Type(T_ULONG));
}

//*************************************************************************************

bool LongLongConst::get_int(int& val) const {if ((value_	>= MIN_INT) && (value_ <= MAX_INT)) {val = static_cast<int>(value_); return true;} return false;}
bool LongLongConst::get_long(long& val) const {if ((value_	>= MIN_LONG) && (value_ <= MAX_LONG)) {val = static_cast<long>(value_); return true;} return true;}
bool LongLongConst::get_longlong(long long& val) const {val = value_; return true;}

bool LongLongConst::is_negative() const
{
	return value_ < 0;
}

	
int LongLongConst::compile(CompilationUnit& cu, bool push_result)
{
  int res = 0;
  res += cu.code().compile_push_long(static_cast<long>(value_));
  res += cu.code().compile_push_long(static_cast<long>(value_ >> 32));
  return res;
}

void LongLongConst::compile_data(CompilationUnit& cu)
{
	cu.data().append_data(value_ >> 48);
	cu.data().append_data((value_ >> 32) & 0xffff);
	cu.data().append_data((value_ >> 16) & 0xffff);
	cu.data().append_data(value_ & 0xffff);
}

void LongLongConst::build_type()
{
	set_type(new Type(T_SLONGLONG));
}

//*************************************************************************************


bool ULongLongConst::get_int(int& val) const {if (value_ <= MAX_INT) {val =	static_cast<int>(value_); return true;} return false;}
bool ULongLongConst::get_long(long& val) const {if (value_ <= MAX_LONG) {val =	static_cast<long>(value_); return true;} return true;}
bool ULongLongConst::get_longlong(long long& val) const {val = value_; return true;}

int ULongLongConst::compile(CompilationUnit& cu, bool push_result)
{
  int res = 0;
  res += cu.code().compile_push_long(static_cast<long>(value_));
  res += cu.code().compile_push_long(static_cast<long>(value_ >> 32));
  return res;
}

void ULongLongConst::compile_data(CompilationUnit& cu)
{
	cu.data().append_data(value_ >> 48);
	cu.data().append_data((value_ >> 32) & 0xffff);
	cu.data().append_data((value_ >> 16) & 0xffff);
	cu.data().append_data(value_ & 0xffff);
}

void ULongLongConst::build_type()
{
	set_type(new Type(T_ULONGLONG));
}



//*************************************************************************************

bool BoolConst::get_int(int& val) const {val = value_ ? 1 : 0; return true;}
bool BoolConst::get_long(long& val) const {val = value_ ? 1 : 0; return true;}
bool BoolConst::get_longlong(long long& val) const {val = value_ ? 1 : 0; return true;}
	
int BoolConst::compile(CompilationUnit& cu, bool push_result)
{
  if (push_result) {
  	return cu.code().compile_push_literal(value_ ? 1 : 0);
  }
	return cu.code().compile_literal(value_ ? 1 : 0);
}

void BoolConst::compile_data(CompilationUnit& cu)
{
	cu.data().append_data(value_);
}

void BoolConst::build_type()
{
	set_type(new Type(T_BOOL));
}

//*************************************************************************************

bool StringConst::hex_to_int(char ch, int& val)
{
  if ((ch >= '0') && (ch <= '9')) {
    val += ch - '0';
    return true;
  }
  if ((ch >= 'a') && (ch <= 'f')) {
    val += ch - 'a' + 10;
    return true;
  }
  if ((ch >= 'A') && (ch <= 'F')) {
    val += ch - 'A' + 10;
    return true;
  }
  return false;
}

bool StringConst::oct_to_int(char ch, int& val)
{
  if ((ch >= '0') && (ch <= '7')) {
    val += ch - '0';
    return true;
  }
  return false;
}

StringConst::StringConst()
{
}

StringConst::StringConst(const context_type::position_type& w, const std::string& val)
: Constant(w)
, value_(val)
{
  int ch;
  size_t pos = value_.find('\\');
  while (pos != std::string::npos) {
    value_.erase(pos, 1);
    if (pos >= value_.size()) {
    	throw parser_exception("StringConst::StringConst invalid escape");
    }
    switch (value_[pos]) {
    case 'n': value_[pos] = '\n'; break;
    case 'r': value_[pos] = '\r'; break;
    case '\'': value_[pos] = '\''; break;
    case '"': value_[pos] = '"'; break;
    case '?': value_[pos] = '?'; break;
    case 'a': value_[pos] = '\a'; break;
    case 'b': value_[pos] = '\b'; break;
    case 'f': value_[pos] = '\f'; break;
    case 't': value_[pos] = '\t'; break;
    case 'v': value_[pos] = '\v'; break;
    case 'x':
      if ((pos + 2) >= value_.size()) {
    	  throw parser_exception("StringConst::StringConst invalid escape");
      }
      ch = 0;
      if (!hex_to_int(value_[pos+1], ch)) {
    	  throw parser_exception("StringConst::StringConst invalid escape");
      }
      ch <<= 4;
      if (!hex_to_int(value_[pos+2], ch)) {
    	  throw parser_exception("StringConst::StringConst invalid escape");
      }
      value_.erase(pos, 2);
      value_[pos] = static_cast<char>(ch);
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      if ((pos + 3) >= value_.size()) {
    	  throw parser_exception("StringConst::StringConst invalid escape");
      }
      ch = 0;
      if (!oct_to_int(value_[pos+1], ch)) {
    	  throw parser_exception("StringConst::StringConst invalid escape");
      }
      ch <<= 3;
      if (!oct_to_int(value_[pos+2], ch)) {
    	  throw parser_exception("StringConst::StringConst invalid escape");
      }
      ch <<= 3;
      if (!oct_to_int(value_[pos+3], ch)) {
    	  throw parser_exception("StringConst::StringConst invalid escape");
      }
      value_.erase(pos, 3);
      value_[pos] = static_cast<char>(ch);
      break;
    default:
    	throw parser_exception("StringConst::StringConst unknown escape");
    }
    pos = value_.find('\\');
  }
}


int StringConst::compile(CompilationUnit& cu, bool push_result)
{
	size_t pos = cu.code().size();
	cu.code().compile_literal_short(0);
	if (!value_.empty()) {
		cu.code().manager()->add_string_literal(value_, pos);
	}
  int res = 0;
  if (push_result) {
    res += cu.code().compile_push_opt();
  }
	return res;
}

void StringConst::build_type()
{
	if (type() == 0) {
		set_type(new ReferenceType(T_POINTER, new Type(T_CHAR)));
	}
}

void StringConst::link1(Linker& lnk)
{
	size_t pos = lnk.add_string_literal(value_);
}

void StringConst::link2(Linker& lnk)
{
}

void StringConst::write(xml::Writer& out)
{
	throw not_implemented_exception("StringConst::write");
	//out << "<String>" << value_ << "</String>";
}

void StringConst::disassembly(std::ostream& out)
{
	out << value_;
}

void StringConst::read(xml::Iterator& it)
{
	throw not_implemented_exception("StringConst::read");
	//if (it.current() != xml::Element::tag) {throw exception("Function::read invalid file");}
	//std::string tag = it.tag();
	//it.next();
	//if (it.current() != xml::Element::content) {throw exception("Function::read invalid file");}
	//value_ = it.value();
	//it.next_tag();
	//if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw exception("Function::read invalid file");}
}

//*************************************************************************************

ConstData::ConstData()
{
}

ConstData::ConstData(const context_type::position_type& w)
: Constant(w)
{
}

void ConstData::add(Constant* c)
{
	data_list_.push_back(c);
}

//*************************************************************************************


int GlobalOffset::compile(CompilationUnit&, bool push_result)
{
	return 0;
}

void GlobalOffset::build_type()
{
}

//*************************************************************************************

DataOffset::DataOffset(unsigned int val, Type* t)
: value_(val)
{
	set_type(t);
}

int DataOffset::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	size_t offset = cu.code().size()+1;
	cu.code().compile_literal_long(value_);
	Function* f = cu.function();
	if (f == 0) {
		throw analyzer_exception("DataOffset::compile no function");
		f->add_data_fixup(offset);
	}
  if (push_result) {
    res += cu.code().compile_push_opt();
  }
	return res;
}

void DataOffset::build_type()
{
}


}

