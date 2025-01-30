#include "cvmc.hpp"

namespace cvmc {

Parameter::Parameter()
//: size_(0)
{
	initialize();
}

Parameter::Parameter(Type* t)
//: Variable(type)
//, size_(0)
{
	initialize();
  //if (type) {
  //  if (type->is_array()) {size_ = 1;}
  //  else {size_ = type->size();}
  //}
  set_type(t);
}

Parameter::Parameter(const context_type::position_type& w, Type* t)
: Variable(w)
//, size_(0)
{
	initialize();
  //if (type) {
  //  if (type->is_array()) {size_ = 1;}
  //  else {size_ = type->size();}
  //}
  set_type(t);
}

void Parameter::set_type(Type* val)
{
  ObjPtr ptr(val);
  if (val) {
    ArrayType* arr = dynamic_cast<ArrayType*>(val);
    if (arr) {
      Variable::set_type(new ReferenceType(T_POINTER, arr->base()));
      return;
    }
  }
  Variable::set_type(val);
}

//size_t Parameter::type_size()
//{
//  return size_;
//}

int Parameter::compile_adr(CompilationUnit& cu, size_t off)
{
	return cu.code().compile_adr_parameter(offset()+off);
}

int Parameter::compile_push(CompilationUnit& cu)
{
	int res = 0;
	throw not_implemented_exception("");
	return res;
}

int Parameter::compile_pop(CompilationUnit& cu)
{
	int res = 0;
	throw not_implemented_exception("");
	return res;
}

int Parameter::compile_push(CompilationUnit& cu, size_t off)
{
	int res = 0;
	throw not_implemented_exception("");
	return res;
}

int Parameter::compile_pop(CompilationUnit& cu, size_t off)
{
	int res = 0;
	throw not_implemented_exception("");
	return res;
}

int Parameter::compile_load(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	size_t size = type_size();
	if (size == 1) {
		res += cu.code().compile_load_parameter(offset());
    if (push_result) {res += cu.code().compile_push_opt();}
	}
	else if (size == 2) {
    if (push_result) {
		  res += cu.code().compile_load_parameter(offset()+1);
      res += cu.code().compile_push();
		  res += cu.code().compile_load_parameter(offset());
      res += cu.code().compile_push();
    }
    else {
		  res += cu.code().compile_load_parameter(offset());
		  res += cu.code().compile_xd();
		  res += cu.code().compile_load_parameter(offset()+1);
    }
	}
	else if (size == 4) {
    res += cu.code().compile_reserve(4);
		res += cu.code().compile_adr_parameter(offset());
    res += cu.code().compile_push();
	  cu.code().manager()->add_call("^load_longlong", cu.code().size());
	  res += cu.code().compile_call(0);
	}
	else if (size > 0) {
		size_t pos = size-1;
		while (pos < size) {
			res += cu.code().compile_load_parameter(offset()+pos);
			res += cu.code().compile_push_opt();
			--pos;
		}
	}
	else {
		throw exception("Parameter::compile_load size is 0");
	}
	return res;
}

int Parameter::compile_store(CompilationUnit& cu)
{
	int res = 0;
	size_t size = type_size();
	if (size == 1) {
		res += cu.code().compile_store_parameter(offset());
	}
	else if (size == 2) {
		res += cu.code().compile_store_parameter(offset()+1);
		res += cu.code().compile_xd();
		res += cu.code().compile_store_parameter(offset());
	}
	else if (size > 2) {
		size_t pos = 0;
		while (pos < size) {
			res += cu.code().compile_pop();
			res += cu.code().compile_store_parameter(offset()+pos);
			++pos;
		}
	}
	else {
		throw exception("Parameter::compile_store size is 0");
	}
	return res;
}

int Parameter::compile_load_word(CompilationUnit& cu, size_t off, bool push_result)
{
	int res = 0;
	res += cu.code().compile_load_parameter(offset()+off);
  if (push_result) {res += cu.code().compile_push_opt();}
	return res;
}

int Parameter::compile_store_word(CompilationUnit& cu, size_t off)
{
	int res = 0;
	res += cu.code().compile_store_parameter(offset()+off);
	return res;
}

void Parameter::print(std::ostream& out)
{
	int off = offset();
	off -= 2;
	out << "parameter(" << off << ")";
}

void Parameter::write(xml::Writer& out)
{
	size_t sz = type_size();
	out.newline();
	out.out() << "<Par name='";
	out.write(name_);
	out.out() << "'>";
	type()->write(out);
	out.out() << "</Par>";
}

void Parameter::read(xml::Iterator& it)
{
	ObjPtr obj;
	if (it.current() != xml::Element::tag) {throw exception("Parameter::read invalid file");}
	std::string tag = it.tag();
	it.next();
	while (it.current() == xml::Element::attribute) {
		if (it.key() == "name") {name_ = it.value();}
		it.next();
	}
	if ((it.current() != xml::Element::content) || (it.tag() != tag)) {throw exception("Parameter::read invalid file");}
	it.next_tag();
	Type::read(it, type_);
	it.next_tag();
	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw exception("Parameter::read invalid file");}
}

}
