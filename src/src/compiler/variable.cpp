#include "cvmc.hpp"

/*

a variable can be
  - global (or static)
  - a parameter
  - a local

definition:
  - global: reserve sizeof() words in global memory
  - parameter:  if is array then reserve 1 word else reserve sizeof() words
  - local:  reserve sizeof() words


int a;
fun(a); // call by value

int b[5];
fun(b); // call by reference


*/

namespace cvmc {

Variable::Variable(const context_type::position_type& w)
: Expression(w)
, offset_(0)
, initialized_(false)
, defined_(false)
{
}

Variable::Variable(const context_type::position_type& w, Type* type)
: Expression(w, type)
, offset_(0)
, initialized_(false)
, defined_(false)
{
}

Variable::Variable(Type* type)
: Expression(type)
, offset_(0)
, initialized_(false)
, defined_(false)
{
}

Variable::Variable()
: offset_(0)
, initialized_(false)
, defined_(false)
{
}

void Variable::initialize()
{
	initialized_ = true;
}

void Variable::build_type()
{
}

Type* Variable::lvalue_type()
{
	return this->type();
}

bool Variable::is_lvalue() const
{
	return true;
}

int Variable::compile(CompilationUnit& cu, bool push_result)
{
  int res = 0;
	if (!initialized_) {
		IssueList::warning(location(), "Variable::compile variable "+name()+" not initialized");
	}
  if (type()->is_array()) { // arrays compile like a pointer
  	res += compile_adr(cu);
    if (push_result) {res += cu.code().compile_push_opt();}
  }
  else {
  	res += compile_load(cu, push_result);
  }
  return res;
}

}
