#include "cvmc.hpp"

namespace cvmc {

IfStmt::IfStmt(Expression* cond, Statement* true_part, Statement* false_part)
: cond_(cond)
, true_(true_part)
, false_(false_part)
{
}

void IfStmt::optimize(ObjPtr& ptr, int mask)
{
	cond_.optimize(mask);
	true_.optimize(mask);
	false_.optimize(mask);
	if ((MASK(OPTM_CONST_FOLDING) & mask) != 0) {
	  if (cond()->is_const()) {
		  long long value;
		  if (cond()->get_longlong(value)) {
			  if (value) {ptr = true_;}
			  else {ptr = false_;}
		  }
	  }
  }
	if ((MASK(OPTM_PEEPHOLE) & mask) != 0) {
	  if (true_.is_null()) {
		  if (false_.is_null()) {
			  if (cond()->has_side_effect()) {
				  ptr = cond();
			  }
			  else {
				  ptr.clear();
			  }
		  }
		  else {
			  cond(new UnaryExpr(location(), UN_NOT, cond()));
			  true_ = false_;
			  false_.clear();
		  }
	  }
  }
}

int IfStmt::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	bool has_false_part = false_.is_not_null();
	res += cu.compile_condition(cond(), false);
	size_t fixup1 = cu.code().size();
	res += cu.code().compile_cond_branch();
	size_t fixup2 = 0;
	res += true_part()->compile(cu, false);
	if (has_false_part) {
		fixup2 = cu.code().size();
		res += cu.code().compile_branch();
	}
	cu.code().fixup_branch(fixup1, cu.code().size());
	if (has_false_part) {
		res += false_part()->compile(cu, false);
		cu.code().fixup_branch(fixup2, cu.code().size());
	}
	return res;
}

//*************************************************************************************

LoopStmt::LoopStmt(const context_type::position_type& w)
: Iteration(w)
{
	init();
}

void LoopStmt::optimize(ObjPtr& ptr, int mask)
{
	block_.optimize(mask);
	if (block_.is_null()) {
		ptr.clear();
	}
}

int LoopStmt::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	size_t target1 = cu.code().size();
	continue_.compile(cu, false);
	res += block_.compile(cu, false);
	size_t fixup1 = cu.code().size();
	res += cu.code().compile_branch();
	break_.compile(cu, false);
	cu.code().fixup_branch(fixup1, target1);
	return res;
}

bool LoopStmt::is_continuable() const
{
	return true;
}

bool LoopStmt::is_breakable() const
{
	return true;
}

//*************************************************************************************

WhileStmt::WhileStmt(const context_type::position_type& w)
: Iteration(w)
{
	init();
}


void WhileStmt::optimize(ObjPtr& ptr, int mask)
{
	cond_.optimize(mask);
	block_.optimize(mask);
	if (block_.is_null()) {
		if (!cond()->has_side_effect()) {
			ptr.clear();
			return;
		}
	}
	if (cond()->is_const()) {
		long long value;
		if (cond()->get_longlong(value)) {
			if (value) {
				LoopStmt* stm = new LoopStmt(location());
				stm->block(block());
				ptr = stm;
				ptr.optimize(mask);
			}
			else {ptr.clear();}
		}
	}
}

int WhileStmt::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	size_t target1 = cu.code().size();
	continue_.compile(cu, false);
	res += cu.compile_condition(cond(), false);
	size_t fixup1 = cu.code().size();
	res += cu.code().compile_cond_branch();
	res += block_.compile(cu, false);
	size_t fixup2 = cu.code().size();
	res += cu.code().compile_branch();
	break_.compile(cu, false);
	cu.code().fixup_branch(fixup1, cu.code().size());
	cu.code().fixup_branch(fixup2, target1);
	return res;
}

bool WhileStmt::is_continuable() const
{
	return true;
}

bool WhileStmt::is_breakable() const
{
	return true;
}

//*************************************************************************************

DoWhileStmt::DoWhileStmt(const context_type::position_type& w)
: Iteration(w)
, inverted_(false)
{
	init();
}

void DoWhileStmt::optimize(ObjPtr& ptr, int mask)
{
	if (!inverted_) {
		cond_ = new UnaryExpr(location(), UN_EQ0, cond());
		inverted_ = true;
	}
	cond_.optimize(mask);
	block_.optimize(mask);
	if (block_.is_null()) {
		if (!cond()->has_side_effect()) {
			ptr.clear();
			return;
		}
	}
	if (cond()->is_const()) {
		long long value;
		if (cond()->get_longlong(value)) {
			if (value) {
				ptr = block_;
			}
			else {
				LoopStmt* stm = new LoopStmt(location());
				stm->block(block());
				ptr = stm;
				ptr.optimize(mask);
			}
		}
	}
}

int DoWhileStmt::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	size_t target1 = cu.code().size();
	res += block_.compile(cu, false);
	res += cu.compile_condition(cond(), false);
	size_t fixup1 = cu.code().size();
	continue_.compile(cu, false);
	res += cu.code().compile_cond_branch();
	break_.compile(cu, false);
	cu.code().fixup_branch(fixup1, target1);
	return res;
}

bool DoWhileStmt::is_continuable() const
{
	return true;
}

bool DoWhileStmt::is_breakable() const
{
	return true;
}

//*************************************************************************************

ForStmt::ForStmt(const context_type::position_type& w)
: Iteration(w)
, is_endless_(false)
{
	init();
}

void ForStmt::optimize(ObjPtr& ptr, int mask)
{
	init_.optimize(mask);
	cond_.optimize(mask);
	post_.optimize(mask);
	block_.optimize(mask);

	if (block_.is_null() && post_.is_null()) {
		if (!cond()->has_side_effect()) {
			ptr = init_;
			return;
		}
	}
	if (cond()->is_const()) {
		long long value;
		if (cond()->get_longlong(value)) {
			if (value) {
				is_endless_ = true;
			}
			else {ptr = block_;}
		}
	}

}

int ForStmt::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	res += init_.compile(cu, false);
	size_t target1 = cu.code().size();
	continue_.compile(cu, false);
	if (!is_endless_) {res += cu.compile_condition(cond(), false);}
	size_t fixup1 = cu.code().size();
	if (!is_endless_) {res += cu.code().compile_cond_branch();}
	res += block_.compile(cu, false);
	res += post_.compile(cu, false);
	size_t fixup2 = cu.code().size();
	res += cu.code().compile_branch();
	break_.compile(cu, false);
	if (!is_endless_) {cu.code().fixup_branch(fixup1, cu.code().size());}
	cu.code().fixup_branch(fixup2, target1);
	return 0;
}

bool ForStmt::is_continuable() const
{
	return true;
}

bool ForStmt::is_breakable() const
{
	return true;
}



}
