#include "cvmc.hpp"

namespace cvmc {

Statement::Statement()
: RefObj(OT_STATEMENT)
{
}

Statement::Statement(const context_type::position_type& w)
: RefObj(OT_STATEMENT)
, where_(w)
{
}

void Statement::optimize(ObjPtr& ptr, int mask)
{
}

int Statement::compile(CompilationUnit& cu, bool push_result)
{
	throw not_implemented_exception("Statement::compile");
	return 0;
}

int Statement::compile_condition(CompilationUnit& cu, bool push_result)
{
	return compile(cu, push_result);
}

bool Statement::last_is_return() const
{
	return false;
}

Statement* Statement::readFrom(xml::Iterator& it)
{
  return 0;
}


//*************************************************************************************

Iteration::Iteration(const context_type::position_type& w) : Statement(w)
{
}

void Iteration::init()
{
	if (is_continuable()) {
		continue_ = new Label(location());
	}
	if (is_breakable()) {
		break_ = new Label(location());
	}
}

bool Iteration::is_continuable() const
{
	return false;
}

bool Iteration::is_breakable() const
{
	return false;
}

//*************************************************************************************

void Jump::fixup(Code& code, size_t destination)
{
	code.fixup_branch(value_, destination);
}

int Jump::compile(CompilationUnit& cu, bool push_result)
{
	value_ = cu.code().size();
	cu.code().compile_branch();
	Label* lbl = static_cast<Label*>(destination_.get());
	if (lbl) {
		lbl->add(cu, this);
	}
	return 0;
}

//*************************************************************************************

CaseJump::CaseJump(const context_type::position_type& w)
: Jump(w)
, is_added_(false)
{
}

void CaseJump::fixup(Code& code, size_t destination)
{
	if (!locations_.empty()) {
		for (std::vector<size_t>::iterator it=locations_.begin(); it!=locations_.end(); ++it) {
			size_t pos = *it;
			code.data()[pos] = destination - pos;
		}
	}
}

int CaseJump::compile(CompilationUnit& cu, bool push_result)
{
	locations_.push_back(cu.code().size());
	cu.code().append(0);
	Label* lbl = destination();
	if (lbl && !is_added_) {
		lbl->add(cu, this);
		is_added_ = true;
	}
	return 0;
}


//*************************************************************************************

Block::Block()
{
}

Block::Block(const context_type::position_type& w)
: Statement(w)
{
}

void Block::optimize(ObjPtr& ptr, int mask)
{
	size_t limit = list_.size();
	size_t i = 0;
	while (i < limit) {
		list_[i++].optimize(mask);
	}
	i = 0;
	while (i < limit) {
		if (list_[i].is_null()) {
			list_.erase(list_.begin() + i);
			--limit;
		}
		else {++i;}
	}
	if (limit == 0) {
		ptr.clear();
		return;
	}
	if (limit == 1) {
		ObjPtr obj = list_[0];
		ptr = obj;
		return;
	}
}

int Block::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	size_t limit = list_.size();
	size_t i = 0;
	while (i < limit) {
		res += statement(i++)->compile(cu, false);
	}
	return res;
}

bool Block::last_is_return() const
{
	size_t limit = list_.size();
	if (limit > 0) {
		return statement(limit-1)->last_is_return();
	}
	return false;
}

//*************************************************************************************

Return::Return(const context_type::position_type& w, Function* f)
: Statement(w)
, func_(f)
{
}

Return::Return(Expression* ex, Function* f)
: expr_(ex)
, func_(f)
{
}

int Return::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	size_t i, expr_size = 0;
	size_t return_size = return_type()->size();
  size_t par_size = func_->parameter_size();
	if (expr()) {
		// return statement has an expression
		expr()->compile(cu, false);
		expr_size = expr()->type_size();
		if (expr_size > return_size) {
      throw not_implemented_exception("Return::compile conversion not supported");
		}
		else if (expr_size < return_size) {
      throw not_implemented_exception("Return::compile conversion not supported");
		}
		else {
			switch (expr_size) {
			case 1: // return in register r
			case 2: break;
      default:
        for (i=0; i<return_size; ++i) {
          cu.code().compile_pop();
          cu.code().compile_store_parameter(par_size+i);
        }
			}
		}
	}
	else if (return_size > 0) {
		// expression missing
    throw generator_exception("Return::compile expression missing");
	}
	cu.code().compile_exit(par_size);
	//code.compile_return();
	res = expr_size;
	res -= return_size;
	return res;
}

void Return::optimize(ObjPtr& ptr, int mask)
{
	expr_.optimize(mask);
}

bool Return::last_is_return() const
{
	return true;
}

//*************************************************************************************


}
