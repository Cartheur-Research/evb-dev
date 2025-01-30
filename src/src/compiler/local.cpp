#include "cvmc.hpp"

namespace cvmc {

Local::Local(const context_type::position_type& w, Type* type) : Variable(w, type) {}

int Local::compile_adr(CompilationUnit& cu, size_t off)
{
	return cu.code().compile_adr_local(offset()+off);
}

int Local::compile_push(CompilationUnit& cu)
{
	int res = 0;
	throw not_implemented_exception("Local::compile_push");
	return res;
}

int Local::compile_pop(CompilationUnit& cu)
{
	int res = 0;
	throw not_implemented_exception("Local::compile_pop");
	return res;
}

int Local::compile_push(CompilationUnit& cu, size_t off)
{
	int res = 0;
	throw not_implemented_exception("Local::compile_push");
	return res;
}

int Local::compile_pop(CompilationUnit& cu, size_t off)
{
	int res = 0;
	throw not_implemented_exception("Local::compile_pop");
	return res;
}

int Local::compile_load(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	size_t size = type_size();
  if (type()->is_array()) { // arrays compile like a pointer
    size = 1;
  }
	if (size == 1) {
		res += cu.code().compile_load_local(offset());
    if (push_result) {res += cu.code().compile_push();}
	}
	else if (size == 2) {
    if (push_result) {
		  res += cu.code().compile_load_local(offset()-1);
      res += cu.code().compile_push();
		  res += cu.code().compile_load_local(offset());
      res += cu.code().compile_push();
    }
    else {
		  res += cu.code().compile_load_local(offset());
		  res += cu.code().compile_xd();
		  res += cu.code().compile_load_local(offset()-1);
    }
	}
	else if (size == 4) {
    res += cu.code().compile_reserve(4);
		res += cu.code().compile_adr_local(offset());
    res += cu.code().compile_push();
	  cu.code().manager()->add_call("^load_longlong", cu.code().size());
	  res += cu.code().compile_call(0);
	}
	else if (size > 0) {
		size_t pos = size-1;
		while (pos < size) {
			res += cu.code().compile_load_local(offset()-pos);
			res += cu.code().compile_push();
			--pos;
		}
	}
	else {
		throw exception("Local::compile_load size is 0");
	}
	return res;
}

int Local::compile_store(CompilationUnit& cu)
{
	int res = 0;
	size_t size = type_size();
	if (size == 1) {
		res += cu.code().compile_store_local(offset());
	}
	else if (size == 2) {
		res += cu.code().compile_store_local(offset()-1);
		res += cu.code().compile_xd();
		res += cu.code().compile_store_local(offset());
	}
	else if (size > 2) {
		size_t pos = 0;
		while (pos < size) {
			res += cu.code().compile_pop();
			res += cu.code().compile_store_local(offset()-pos);
			++pos;
		}
	}
	else {
		throw exception("Local::compile_store size is 0");
	}
	return res;
}

int Local::compile_load_word(CompilationUnit& cu, size_t off, bool push_result)
{
	int res = 0;
	res += cu.code().compile_load_local(offset()-off);
  if (push_result) {res += cu.code().compile_push_opt();}
	return res;
}

int Local::compile_store_word(CompilationUnit& cu, size_t off)
{
	int res = 0;
	res += cu.code().compile_store_local(offset()-off);
	return res;
}

void Local::print(std::ostream& out)
{
	out << "local(" << offset() << ")";
}

}
