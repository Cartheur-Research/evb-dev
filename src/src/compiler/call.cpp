#include "cvmc.hpp"

namespace cvmc {

//Call::Call(const std::string& name, Function* type)
//: name_(name)
//, argument_size_(0)
//{
//	if (type) {
//		// set type of call to return type of function
//		func_ = type;
//		this->type(type->return_type());
//	}
//}
//
//Call::Call(const context_type::position_type& w, const std::string& name, Function* type)
//: Expression(w)
//, name_(name)
//, argument_size_(0)
//{
//	if (type) {
//		// set type of call to return type of function
//		func_ = type;
//		this->type(type->return_type());
//	}
//}

Call::Call(const std::string& name)
: name_(name)
, argument_size_(0)
, is_system_(true)
, is_external_(false)
{
	func_ = Function::system(name);
	set_type(func()->return_type());
}

Call::Call(Function* f, const std::string& name)
: name_(name)
, argument_size_(0)
, is_system_(false)
, is_external_(false)
{
	if (f) {
		// set type of call to return type of function
		func_ = f;
		set_type(f->return_type());
		if (f->has_storage_class(SC_EXTERN)) {
			is_external_ = true;
		}
	}
}

Call::Call(const context_type::position_type& w, Function* f, const std::string& name)
: Expression(w)
, name_(name)
, argument_size_(0)
, is_system_(false)
, is_external_(false)
{
	if (f) {
		// set type of call to return type of function
		func_ = f;
		set_type(f->return_type());
		if (f->has_storage_class(SC_EXTERN)) {
			is_external_ = true;
		}
	}
}

void Call::build_type()
{
}

bool Call::has_side_effect() const
{
	return true;
}

void Call::add(Expression* expr)
{
  //if (expr->type()->is_array()) {
  //  ArrayType* arr = dynamic_cast<ArrayType*>(expr->type());
  //  if (arr) {
  //    expr->set_type(new ReferenceType(T_POINTER, arr->base()));
  //  }
  //}
	argument_list_.push_back(expr);
	argument_size_ += expr->type_size();
}

void Call::optimize(ObjPtr& ptr, int mask)
{
	size_t arg_limit = this->size();
	for (size_t i=0; i<arg_limit; ++i) {
		argument(i)->optimize(argument_list_[i], mask);
    //if (argument(i)->type()->is_array()) {
    //  ArrayType* arr = dynamic_cast<ArrayType*>(argument(i)->type());
    //  if (arr) {
    //    argument(i)->set_type(new ReferenceType(T_POINTER, arr->base()));
    //  }
    //}
	}
}

int Call::compile_argument(CompilationUnit& cu, Expression* expr)
{
	int res = 0;
	Local* loc = dynamic_cast<Local*>(expr);
	if (loc) {
		loc->compile_push(cu);
	}
	return res;
}

int Call::compile(CompilationUnit& cu, bool push_result)
{
	XRefManager* xrefm = cu.code().manager();
	if (xrefm == 0) {
		throw exception("Call::compile no xref manager");
	}
	int res = 0;
  size_t i;
	size_t ret_size = this->func()->return_type()->size();
	size_t arg_limit = this->size();
	size_t par_limit = this->func()->parameter();
	if (arg_limit < par_limit) {
		throw analyzer_exception("Call::compile too few arguments");
	}
	else if (arg_limit > par_limit) {
		throw analyzer_exception("Call::compile too many arguments");
	}
  if (ret_size > 2) {
    res += cu.code().compile_reserve(ret_size);
  }
	for (i=0; i<arg_limit; ++i) {
    Expression* arg = argument(i);
    Parameter* par = this->func()->parameter(i);
		size_t tsize = arg->type_size();
		size_t psize = par->type_size();
    if (arg->type()->is_array()) {
      tsize = 1;
  		res += arg->compile_adr(cu);
      res += cu.code().compile_push_opt();
    }
    else {
  		res += arg->compile(cu, true);
    }
    if (tsize != psize) {
  		throw not_implemented_exception("Call::compile parameter size != argument size");
    }
		//switch (tsize) {
    //case 0: throw analyzer_exception("Call::compile argument size not supported");
		//case 1: res += cu.code().compile_push_opt(); break;
		//case 2: res += cu.code().compile_push_long_opt(); break;
		//default:;
		//}
		
	}
	xrefm->add_call(name(), cu.code().size());
	res += cu.code().compile_call(0);
  if (push_result) {
    if (ret_size == 1) {
      res += cu.code().compile_push_opt();
    }
    else if (ret_size == 2) {
      res += cu.code().compile_push_long_opt();
    }
  }
	//res += code.compile_cleanup(argument_size_);
	return res;
}

}
