#include "cvmc.hpp"

namespace cvmc {

Assignment::Assignment(const context_type::position_type& w, AssOp op)
: Expression(w)
, op_(op)
{
  //if (op_ == AS_RIGHT) {
  //  int test = 0;
  //}
}

Assignment::Assignment(const context_type::position_type& w, AssOp op, Expression* lhs, Expression* rhs)
: Expression(w)
, op_(op)
, lhs_(lhs)
, rhs_(rhs)
{
  //if (op_ == AS_RIGHT) {
  //  int test = 0;
  //}
}

bool Assignment::has_side_effect() const
{
	return true;
}

ExprType Assignment::expr_type() const
{
	return ET_ASSIGN;
}

void Assignment::build_type()
{
	set_type(lhs()->type());
}

bool Assignment::is_const() const
{
	return op_ == AS_NORMAL && rhs()->is_const();
}

void Assignment::optimize(ObjPtr& ptr, int mask)
{
	lhs_.optimize(MASK(OPTM_CONST_FOLDING));
	rhs_.optimize(mask);
	Type& t1 = *lhs()->lvalue_type();
	Type& t2 = *rhs()->type();
	if (t1.size() != t2.size()) {
		switch (op_) {
		case AS_LEFT: // for shift operations both argumnts need not be the same size
		case AS_RIGHT:
      break;

    default:
		  rhs(
			  new AutoCast(
				  location(),
				  &t1,
				  rhs(),
				  (op_ == AS_ADD) || (op_ == AS_SUB) ? AutoCast::additive : AutoCast::normal
			  )
		  );
    }
	}
}

int Assignment::compile(CompilationUnit& cu, bool push_result)
{
  int val;
  Call*	c;
	ObjPtr obj;
	int res = 0;
	if (!lhs()->is_lvalue()) {
		throw analyzer_exception("Assignment::compile not an lvalue");
	}
	Type& t1 = *lhs()->lvalue_type();
	Type& t2 = *rhs()->type();
  size_t s1 = t1.size();
  size_t s2 = t2.size();
	if (!t1.is_assignment_compatible_with(t2)) {
		lhs()->lvalue_type();
		t1.is_assignment_compatible_with(t2);
		throw analyzer_exception("Assignment::compile not assignment compatible");
	}
	Variable* var = dynamic_cast<Variable*>(lhs());
	if (t1.is_reference() && (op_ != AS_NORMAL)) {
		if ((var != 0) && !var->is_initialized()) {
			throw analyzer_exception("Assignment::compile invalid reference initialization");
		}
	}
	if (s1 == 1) {
		if (s2 != 1) {
			throw not_implemented_exception("Assignment::compile size not equal");
		}
		switch (op_) {
		case AS_NORMAL:
      if (t1.is_integer() && t2.is_integer()) {
			  if (t1.is_reference()) {
				  res += rhs()->compile_adr(cu);
			  }
			  else {
				  res += rhs()->compile(cu, false);
			  }
			  res += lhs()->compile_store(cu);
      }
      else if (t1.is_float() && t2.is_float()) {
			  if (t1.is_reference()) {
				  res += rhs()->compile_adr(cu);
			  }
			  else {
				  res += rhs()->compile(cu, false);
			  }
			  res += lhs()->compile_store(cu);
      }
      else if (t1.is_array_or_pointer() && t2.is_array_or_pointer()) {
			  if (t1.is_reference()) {
				  res += rhs()->compile_adr(cu);
			  }
			  else {
				  res += rhs()->compile(cu, false);
			  }
			  res += lhs()->compile_store(cu);
      }
      else {
  			throw not_implemented_exception("Assignment::compile integer<->?");
      }
			break;

		case AS_ADD:
			//if (t1.is_reference() && t2.is_integer()) {
			//	res += lhs()->compile_load(cu, false);
			//	res += cu.code().compile_load();
			//	res += cu.code().compile_push_opt();
			//	res += rhs()->compile(cu, false);
			//	res += cu.code().compile_add();
			//	res += cu.code().compile_push_opt();
			//	res += lhs()->compile_load(cu, false);
			//	res += cu.code().compile_store();
			//}
      if (t1.is_integer() && t2.is_integer()) {
				res += lhs()->compile_load(cu, true);
				res += rhs()->compile(cu, false);
				res += cu.code().compile_add();
				res += lhs()->compile_store(cu);
			}
      else {
  			throw not_implemented_exception("Assignment::compile += incompatible types");
      }
			break;

		case AS_SUB:
			res += lhs()->compile_load(cu, true);
			res += rhs()->compile(cu, false);
			res += cu.code().compile_sub();
			res += lhs()->compile_store(cu);
			break;

		case AS_BITAND:
			res += lhs()->compile_load(cu, true);
			res += rhs()->compile(cu, false);
			res += cu.code().compile_and();
			res += lhs()->compile_store(cu);
			break;

		case AS_BITXOR:
			res += lhs()->compile_load(cu, true);
			res += rhs()->compile(cu, false);
			res += cu.code().compile_xor();
			res += lhs()->compile_store(cu);
			break;

		case AS_BITOR:
			res += lhs()->compile_load(cu, true);
			res += rhs()->compile(cu, false);
			res += cu.code().compile_or();
			res += lhs()->compile_store(cu);
			break;

		case AS_MUL:
			res += lhs()->compile_load(cu, true);
			res += rhs()->compile(cu, false);
			res += cu.code().compile_mul();
			res += lhs()->compile_store(cu);
			break;

		case AS_LEFT:
			//res += lhs()->compile_load(cu);
			//res += cu.code().compile_push_opt();
			//res += rhs()->compile(cu);
			//res += cu.code().compile_push_opt();
      if (rhs()->is_const() && rhs()->get_int(val) && (val >= 0) && (val < 5)) {
        if (val > 0) {
 			    res += lhs()->compile_load(cu, false);
          while (--val >= 0) {
      			res += cu.code().compile_mul2();
          }
  			  res += lhs()->compile_store(cu);
        }       
      }
      else {
			  c = new Call("^shl_int");
        obj = c;
			  c->add(lhs());
			  c->add(rhs());
        res += c->compile(cu, false);
			  res += lhs()->compile_store(cu);
      }
			break;

		case AS_RIGHT:
      if (rhs()->is_const() && rhs()->get_int(val) && (val >= 0) && (val < 5)) {
        if (val > 0) {
 			    res += lhs()->compile_load(cu, false);
          while (--val >= 0) {
            if (t1.is_unsigned()) {
        			res += cu.code().compile_udiv2();
            }
            else {
        			res += cu.code().compile_div2();
            }
          }
  			  res += lhs()->compile_store(cu);
        }       
      }
      else {
        if (t1.is_unsigned()) {
  			  c = new Call("^shr_uint");
        }
        else {
  			  c = new Call("^shr_int");
        }
        obj = c;
			  c->add(lhs());
			  c->add(rhs());
        res += c->compile(cu, false);
			  res += lhs()->compile_store(cu);
      }
			break;


		//case AS_DIV:
		//case AS_MOD:
		//case AS_LEFT:
		//case AS_RIGHT:
		default: throw not_implemented_exception("Assignment::compile");
		}
	}
	else if (s1 == 2) {
		switch (op_) {
		case AS_NORMAL:
		  if (s2 != 2) {
			  throw not_implemented_exception("Assignment::compile size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
			  res += rhs()->compile(cu, false);
			  res += lhs()->compile_store(cu);
      }
      else if (t1.is_float() && t2.is_float()) {
			  res += rhs()->compile(cu, false);
			  res += lhs()->compile_store(cu);
      }
      else {
  			throw not_implemented_exception("Assignment::compile integer<->double");
      }
			break;

    case AS_ADD:
		  if (s2 != 2) {
			  throw not_implemented_exception("Assignment::compile += size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
        if (t1.is_unsigned()) {
  			  c = new Call("^addassign_long");
        }
        else {
  			  c = new Call("^addassign_long");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else if (t1.is_float() && t2.is_float()) {
 			  c = new Call("^addassign_float");
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile += incompatible types");
      }
			break;

    case AS_SUB:
		  if (s2 != 2) {
			  throw not_implemented_exception("Assignment::compile -= size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
        if (t1.is_unsigned()) {
  			  c = new Call("^subassign_long");
        }
        else {
  			  c = new Call("^subassign_long");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else if (t1.is_float() && t2.is_float()) {
 			  c = new Call("^subassign_float");
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile -= incompatible types");
      }
			break;

    case AS_MUL:
		  if (s2 != 2) {
			  throw not_implemented_exception("Assignment::compile *= size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
        if (t1.is_unsigned()) {
  			  c = new Call("^mulassign_ulong");
        }
        else {
  			  c = new Call("^mulassign_long");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else if (t1.is_float() && t2.is_float()) {
 			  c = new Call("^mulassign_float");
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile *= incompatible types");
      }
			break;

    case AS_DIV:
		  if (s2 != 2) {
			  throw not_implemented_exception("Assignment::compile /= size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
        if (t1.is_unsigned()) {
  			  c = new Call("^divassign_ulong");
        }
        else {
  			  c = new Call("^divassign_long");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else if (t1.is_float() && t2.is_float()) {
 			  c = new Call("^divassign_float");
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile /= incompatible types");
      }
			break;

    case AS_MOD:
		  if (s2 != 2) {
			  throw not_implemented_exception("Assignment::compile %= size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
        if (t1.is_unsigned()) {
  			  c = new Call("^modassign_ulong");
        }
        else {
  			  c = new Call("^modassign_long");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile %= incompatible types");
      }
			break;

    case AS_BITAND:
		  if (s2 != 2) {
			  throw not_implemented_exception("Assignment::compile size not equal");
		  }
      if (t1.is_integer() != t2.is_integer()) {
  			throw not_implemented_exception("Assignment::compile integer<->float");
      }
      else {
        if (t1.is_unsigned()) {
  			  c = new Call("^bitandassign_long");
        }
        else {
  			  c = new Call("^bitandassign_long");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
			break;

    case AS_BITOR:
		  if (s2 != 2) {
			  throw not_implemented_exception("Assignment::compile size not equal");
		  }
      if (t1.is_integer() != t2.is_integer()) {
  			throw not_implemented_exception("Assignment::compile integer<->float");
      }
      else {
        if (t1.is_unsigned()) {
  			  c = new Call("^bitorassign_long");
        }
        else {
  			  c = new Call("^bitorassign_long");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
			break;

    case AS_BITXOR:
		  if (s2 != 2) {
			  throw not_implemented_exception("Assignment::compile size not equal");
		  }
      if (t1.is_integer() != t2.is_integer()) {
  			throw not_implemented_exception("Assignment::compile integer<->float");
      }
      else {
        if (t1.is_unsigned()) {
  			  c = new Call("^bitxorassign_long");
        }
        else {
  			  c = new Call("^bitxorassign_long");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
			break;

    case AS_RIGHT:
      if (!t1.is_integer()) {
  			throw not_implemented_exception("Assignment::compile >>= can only shift integer types");
      }
      if (!t2.is_integer()) {
  			throw not_implemented_exception("Assignment::compile >>= can only shift by integer types");
      }
		  if (s2 != 1) {
			  throw not_implemented_exception("Assignment::compile >>= invalid shift size");
		  }
      if (t1.is_unsigned()) {
  			c = new Call("^shrp_ulong");
      }
      else {
  			c = new Call("^shrp_long");
      }
      obj = c;
			c->add(rhs());
			c->add(new UnaryExpr(UN_REF, lhs()));
      res += c->compile(cu, false);
			break;

    case AS_LEFT:
      if (!t1.is_integer()) {
  			throw not_implemented_exception("Assignment::compile <<= can only shift integer types");
      }
      if (!t2.is_integer()) {
  			throw not_implemented_exception("Assignment::compile <<= can only shift by integer types");
      }
		  if (s2 != 1) {
			  throw not_implemented_exception("Assignment::compile <<= invalid shift size");
		  }
 			c = new Call("^shlp_long");
      obj = c;
			c->add(rhs());
			c->add(new UnaryExpr(UN_REF, lhs()));
      res += c->compile(cu, false);
			break;

		default:
			throw not_implemented_exception("Assignment::compile size = 2");
		}

	}
	else if (s1 == 4) {
		if (s2 != 4) {
			throw not_implemented_exception("Assignment::compile size not equal");
		}
		switch (op_) {
		case AS_NORMAL:
      if (t1.is_integer() && t2.is_integer()) {
			  res += rhs()->compile(cu, false);
			  res += lhs()->compile_store(cu);
      }
      else if (t1.is_float() && t2.is_float()) {
			  res += rhs()->compile(cu, false);
			  res += lhs()->compile_store(cu);
      }
      else {
  			throw not_implemented_exception("Assignment::compile integer<->double");
      }
			break;

    case AS_ADD:
		  if (s2 != 4) {
			  throw not_implemented_exception("Assignment::compile += size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
        if (t1.is_unsigned()) {
  			  c = new Call("^addassign_ulonglong");
        }
        else {
  			  c = new Call("^addassign_longlong");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else if (t1.is_float() && t2.is_float()) {
 			  c = new Call("^addassign_double");
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile += incompatible types");
      }
			break;

    case AS_SUB:
		  if (s2 != 4) {
			  throw not_implemented_exception("Assignment::compile -= size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
        if (t1.is_unsigned()) {
  			  c = new Call("^subassign_ulonglong");
        }
        else {
  			  c = new Call("^subassign_longlong");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else if (t1.is_float() && t2.is_float()) {
 			  c = new Call("^subassign_double");
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile -= incompatible types");
      }
			break;

    case AS_MUL:
		  if (s2 != 4) {
			  throw not_implemented_exception("Assignment::compile *= size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
        if (t1.is_unsigned()) {
  			  c = new Call("^mulassign_ulonglong");
        }
        else {
  			  c = new Call("^mulassign_longlong");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else if (t1.is_float() && t2.is_float()) {
 			  c = new Call("^mulassign_double");
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile *= incompatible types");
      }
			break;

    case AS_DIV:
		  if (s2 != 4) {
			  throw not_implemented_exception("Assignment::compile /= size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
        if (t1.is_unsigned()) {
  			  c = new Call("^divassign_ulonglong");
        }
        else {
  			  c = new Call("^divassign_longlong");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else if (t1.is_float() && t2.is_float()) {
 			  c = new Call("^divassign_double");
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile /= incompatible types");
      }
			break;

    case AS_MOD:
		  if (s2 != 4) {
			  throw not_implemented_exception("Assignment::compile %= size not equal");
		  }
      if (t1.is_integer() && t2.is_integer()) {
        if (t1.is_unsigned()) {
  			  c = new Call("^modassign_ulonglong");
        }
        else {
  			  c = new Call("^modassign_longlong");
        }
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile %= incompatible types");
      }
			break;

 		//case AS_LEFT:
   //   if (t1.is_integer() != t2.is_integer()) {
  	//		throw not_implemented_exception("Assignment::compile integer<->double");
   //   }
   //   else {
			//  if (t1.is_reference()) {
			//	  res += rhs()->compile_adr(cu);
			//  }
			//  else {
			//	  res += rhs()->compile(cu);
			//  }

			//  res += lhs()->compile_store(cu);
   //   }
   //   break;

		default:
			throw not_implemented_exception("Assignment::compile size = 4");
		}
	}
	else if (s1 == 8) {
		if (s2 != 8) {
			throw not_implemented_exception("Assignment::compile size not equal");
		}
		switch (op_) {
		case AS_NORMAL:
      if (t1.is_integer() && t2.is_integer()) {
			  res += rhs()->compile(cu, false);
			  res += lhs()->compile_store(cu);
      }
      else if (t1.is_float() && t2.is_float()) {
			  res += rhs()->compile(cu, false);
			  res += lhs()->compile_store(cu);
      }
      else {
  			throw not_implemented_exception("Assignment::compile integer<->double");
      }
			break;

    case AS_ADD:
		  if (s2 != 8) {
			  throw not_implemented_exception("Assignment::compile += size not equal");
		  }
      if (t1.is_float() && t2.is_float()) {
 			  c = new Call("^addassign_longdouble");
        obj = c;
			  c->add(rhs());
			  c->add(new UnaryExpr(UN_REF, lhs()));
        res += c->compile(cu, false);
      }
      else {
  			throw not_implemented_exception("Assignment::compile += incompatible types");
      }
			break;
    }
  }
	else {
		throw not_implemented_exception("Assignment::compile size");
	}
	//lhs()->initialize();
	if (var) {
		var->initialize();
	}
	return res;
}

//*************************************************************************************


InitAssign::InitAssign(const context_type::position_type& w)
: Assignment(w, AS_NORMAL)
{
}

int InitAssign::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	if (!lhs()->is_lvalue()) {
		throw analyzer_exception("InitAssign::compile not an lvalue");
	}
	Type& t1 = *lhs()->lvalue_type();
	Type& t2 = *rhs()->type();
	if (!t1.is_assignment_compatible_with(t2)) {
		lhs()->lvalue_type();
		t1.is_assignment_compatible_with(t2);
		throw analyzer_exception("InitAssign::compile not assignment compatible");
	}
  if (!rhs()->is_const()) {
    return Assignment::compile(cu, false);
  }
	Variable* var = dynamic_cast<Variable*>(lhs());
	size_t s1 = t1.size();
	size_t s2 = t2.size();
	if (s1 == 1) {
		if (s2 != 1) {
			throw not_implemented_exception("InitAssign::compile size not equal");
		}
		if (t1.is_reference()) {
			res += rhs()->compile_adr(cu);
		}
		else {
			res += rhs()->compile(cu, false);
		}
		res += lhs()->compile_store(cu);
	}
	else if (s1 == 2) {
		if (s2 != 2) {
			throw not_implemented_exception("InitAssign::compile size not equal");
		}
		if (t1.is_reference()) {
			res += rhs()->compile_adr(cu);
		}
		else {
			res += rhs()->compile(cu, false);
		}
		res += lhs()->compile_store(cu);
	}
	else {
		if (s1 != s2) {
			throw not_implemented_exception("InitAssign::compile size not equal");
		}
		size_t data_offset = cu.data().size();
		rhs()->compile_data(cu);
		if (call_.is_null()) {
			call_ = new Call("^copy_up");
			call()->add(new UnaryExpr(UN_REF, lhs()));
			call()->add(new DataOffset(data_offset, new ReferenceType(T_POINTER, rhs()->type())));
			call()->add(new UIntConst(this->location(), s1));
		}
		call()->compile(cu, false);
	}
	return res;
}


}
