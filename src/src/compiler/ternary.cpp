#include "cvmc.hpp"

namespace cvmc {

TernaryExpr::TernaryExpr(const context_type::position_type& w, TerOp op) : Expression(w), op_(op) {}

ExprType TernaryExpr::expr_type() const {return ET_TERNARY;}

void TernaryExpr::build_type()
{
	set_type(arg1()->type());
}

bool TernaryExpr::has_side_effect() const
{
	return arg1()->has_side_effect() || arg2()->has_side_effect() || arg3()->has_side_effect();
}

bool TernaryExpr::is_const() const
{
	return arg1()->is_const() && arg2()->is_const() && arg3()->is_const();
}

void TernaryExpr::optimize(ObjPtr& ptr, int mask)
{
	arg1_.optimize(mask);
	arg2_.optimize(mask);
	arg3_.optimize(mask);
	if ((MASK(OPTM_CONST_FOLDING) & mask) != 0) {
    if (arg1()->is_const()) {
		  long long val1;
		  if (arg1()->get_longlong(val1)) {
        if (val1 != 0) {
          ptr = arg2_;
        }
        else {
          ptr = arg3_;
        }
        return;
      }
    }
  }
  if (arg2_.is_null() || arg3_.is_null()) {
		throw analyzer_exception("TernaryExpr::compile not both paths defined");
  }
  Type* t2 = arg2()->type();
  Type* t3 = arg3()->type();
  size_t s2 = t2->size();
  size_t s3 = t3->size();
  if (s2 > s3) {
    if (!arg2()->type()->is_assignment_compatible_with(*arg3()->type())) {
  		throw analyzer_exception("TernaryExpr::compile cannot match types");
    }
    set_type(t2);
	  arg3(new AutoCast(location(), t3, arg3()));
  }
  else if (s2 < s3) {
    if (!arg3()->type()->is_assignment_compatible_with(*arg2()->type())) {
  		throw analyzer_exception("TernaryExpr::compile cannot match types");
    }
    set_type(t3);
	  arg2(new AutoCast(location(), t2, arg2()));
  }
  else {
    if (!arg2()->type()->is_equivalent_with(*arg3()->type())) {
  		throw analyzer_exception("TernaryExpr::compile both types do not match");
    }
  }
}

int TernaryExpr::compile(CompilationUnit& cu, bool push_result)
{
  int res = 0;
	res += cu.compile_condition(arg1(), false);
	size_t fixup1 = cu.code().size();
	res += cu.code().compile_cond_branch();
	size_t fixup2 = 0;
	res += arg2()->compile(cu, false);
	fixup2 = cu.code().size();
	res += cu.code().compile_branch();
	cu.code().fixup_branch(fixup1, cu.code().size());
	res += arg3()->compile(cu, false);
	cu.code().fixup_branch(fixup2, cu.code().size());
  return res;
}

int TernaryExpr::compile_condition(CompilationUnit& cu, bool push_result)
{
	return compile(cu, push_result);
}

}
