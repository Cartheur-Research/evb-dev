#include "cvmc.hpp"

namespace cvmc {

BinaryExpr::BinaryExpr(const context_type::position_type& w, BinOp op)
: Expression(w)
, op_(op)
{
}

BinaryExpr::BinaryExpr(const context_type::position_type& w, BinOp op, Expression* arg1, Expression* arg2)
: Expression(w)
, op_(op)
, arg1_(arg1)
, arg2_(arg2)
{
	build_type();
}

BinaryExpr::BinaryExpr(BinOp op, Expression* arg1, Expression* arg2)
: op_(op)
, arg1_(arg1)
, arg2_(arg2)
{
	build_type();
}

bool BinaryExpr::is_shift() const
{
	switch (op_) {
  case BIN_LEFT:
  case BIN_RIGHT:
    return true;

  default:;
  }
  return false;
}

bool BinaryExpr::is_binary(BinOp op) const
{
	return op_ == op;
}

ExprType BinaryExpr::expr_type() const {return ET_BINARY;}

void BinaryExpr::build_type()
{
	switch (op_) {
		case BIN_AND:
		case BIN_OR:
		case BIN_EQ:
		case BIN_NE:
		case BIN_LT:
		case BIN_LE:
		case BIN_GT:
		case BIN_GE: set_type(new Type(T_BOOL)); break;
		default: set_type(arg1()->type());
	}
}

bool BinaryExpr::has_side_effect() const
{
	return arg1()->has_side_effect() || arg2()->has_side_effect();
}

bool BinaryExpr::is_const() const
{
	return arg1()->is_const() && arg2()->is_const();
}

void BinaryExpr::optimize(ObjPtr& ptr, int mask)
{
	arg1_.optimize(mask);
	arg2_.optimize(mask);

	if (((MASK(OPTM_CONST_FOLDING) & mask) != 0) && is_const()) {
		{ // try int
			int val1 , val2;
			if (arg1()->get_int(val1) && arg2()->get_int(val2)) {
				switch (op_) {
					case BIN_ADD: val1 += val2; break;
					case BIN_SUB: val1 -= val2; break;
					case BIN_MUL: val1 *= val2; break;
					case BIN_DIV: val1 /= val2; break;
					case BIN_MOD: val1 %= val2; break;
					case BIN_BITAND: val1 &= val2; break;
					case BIN_BITXOR: val1 ^= val2; break;
					case BIN_BITOR: val1 |= val2; break;
					case BIN_LEFT: val1 <<= val2; break;
					case BIN_RIGHT: val1 >>= val2; break;
					case BIN_AND: ptr = new BoolConst(location(), (val1 != 0) && (val2 != 0)); return;
					case BIN_OR: ptr = new BoolConst(location(), (val1 != 0) || (val2 != 0)); return;
					case BIN_EQ: ptr = new BoolConst(location(), val1 == val2); return;
					case BIN_NE: ptr = new BoolConst(location(), val1 != val2); return;
					case BIN_LT: ptr = new BoolConst(location(), val1 < val2); return;
					case BIN_LE: ptr = new BoolConst(location(), val1 <= val2); return;
					case BIN_GT: ptr = new BoolConst(location(), val1 > val2); return;
					case BIN_GE: ptr = new BoolConst(location(), val1 >= val2); return;
					default: return;
				}
				ptr = new IntConst(location(), val1);
				return;
			}
		}
		{ // try long
			long val1 , val2;
			if (arg1()->get_long(val1) && arg2()->get_long(val2)) {
				switch (op_) {
					case BIN_ADD: val1 += val2; break;
					case BIN_SUB: val1 -= val2; break;
					case BIN_MUL: val1 *= val2; break;
					case BIN_DIV: val1 /= val2; break;
					case BIN_MOD: val1 %= val2; break;
					case BIN_BITAND: val1 &= val2; break;
					case BIN_BITXOR: val1 ^= val2; break;
					case BIN_BITOR: val1 |= val2; break;
					case BIN_LEFT: val1 <<= val2; break;
					case BIN_RIGHT: val1 >>= val2; break;
					case BIN_AND: ptr = new BoolConst(location(), (val1 != 0) && (val2 != 0)); return;
					case BIN_OR: ptr = new BoolConst(location(), (val1 != 0) || (val2 != 0)); return;
					case BIN_EQ: ptr = new BoolConst(location(), val1 == val2); return;
					case BIN_NE: ptr = new BoolConst(location(), val1 != val2); return;
					case BIN_LT: ptr = new BoolConst(location(), val1 < val2); return;
					case BIN_LE: ptr = new BoolConst(location(), val1 <= val2); return;
					case BIN_GT: ptr = new BoolConst(location(), val1 > val2); return;
					case BIN_GE: ptr = new BoolConst(location(), val1 >= val2); return;
					default: return;
				}
				ptr = new LongConst(location(), val1);
				return;
			}
		}
		{ // try long long
			long long val1 , val2;
			if (arg1()->get_longlong(val1) && arg2()->get_longlong(val2)) {
				switch (op_) {
					case BIN_ADD: val1 += val2; break;
					case BIN_SUB: val1 -= val2; break;
					case BIN_MUL: val1 *= val2; break;
					case BIN_DIV: val1 /= val2; break;
					case BIN_MOD: val1 %= val2; break;
					case BIN_BITAND: val1 &= val2; break;
					case BIN_BITXOR: val1 ^= val2; break;
					case BIN_BITOR: val1 |= val2; break;
					case BIN_LEFT: val1 <<= val2; break;
					case BIN_RIGHT: val1 >>= val2; break;
					case BIN_AND: ptr = new BoolConst(location(), (val1 != 0) && (val2 != 0)); return;
					case BIN_OR: ptr = new BoolConst(location(), (val1 != 0) || (val2 != 0)); return;
					case BIN_EQ: ptr = new BoolConst(location(), val1 == val2); return;
					case BIN_NE: ptr = new BoolConst(location(), val1 != val2); return;
					case BIN_LT: ptr = new BoolConst(location(), val1 < val2); return;
					case BIN_LE: ptr = new BoolConst(location(), val1 <= val2); return;
					case BIN_GT: ptr = new BoolConst(location(), val1 > val2); return;
					case BIN_GE: ptr = new BoolConst(location(), val1 >= val2); return;
					default: return;
				}
				ptr = new LongLongConst(location(), val1);
				return;
			}
		}
	}
	if ((MASK(OPTM_PEEPHOLE) & mask) != 0) {
		if (arg2()->is_const()) {
			long long val2;
			if (arg2()->get_longlong(val2)) {
				switch (op_) {
					case BIN_ADD:
						if (val2 == 0) {ptr = arg1_; return;}
						if (val2 == 1) {ptr = new UnaryExpr(location(), UN_INC, arg1()); return;}
						if (val2 == -1) {ptr = new UnaryExpr(location(), UN_DEC, arg1()); return;}
						if ((val2 > 0) && (val2 <= 0x1ff)) {
              ptr = new UnaryExpr(location(), UN_INC, arg1(), static_cast<int>(val2)); return;
            }
						break;

					case BIN_SUB:
						if (val2 == 0) {ptr = arg1_; return;}
						if (val2 == -1) {ptr = new UnaryExpr(location(), UN_INC, arg1()); return;}
						if (val2 == 1) {ptr = new UnaryExpr(location(), UN_DEC, arg1()); return;}
						break;

					case BIN_MUL:
						if (val2 == 1) {ptr = arg1_; return;}
						if (val2 == 0) {ptr = new IntConst(location(), 0); return;}
						if (val2 == 2) {ptr = new UnaryExpr(location(), UN_MUL2, arg1()); return;}
						if (val2 == -1) {ptr = new UnaryExpr(location(), UN_NEGATE, arg1()); return;}
						if (((val2 & (val2-1)) == 0) && (val2 > 0)) {
							// multiplicator is a power of 2, therefore we can replace the multiplication with a shift operation
							int shift = 0;
							while (val2) {++shift; val2 >>= 1;}
							ptr = new BinaryExpr(location(), BIN_LEFT, arg1(), new IntConst(location(), shift));
							return;
						}
						break;


					case BIN_DIV:
						if (val2 == 1) {ptr = arg1_; return;}
						if (val2 == 2) {ptr = new UnaryExpr(location(), UN_DIV2, arg1()); return;}
						if (((val2 & (val2-1)) == 0) && (val2 > 0)) {
							// divisor is a power of 2, therefore we can replace the division with a shift operation
							int shift = 0;
							while (val2) {++shift; val2 >>= 1;}
							ptr = new BinaryExpr(location(), BIN_RIGHT, arg1(), new IntConst(location(), shift));
							return;
						}
						break;

					case BIN_MOD:
						if (val2 == 1) {ptr = new IntConst(location(), 0); return;}
						if (((val2 & (val2-1)) == 0) && (val2 > 0)) {
							// divisor is a power of 2, therefore we can replace the modulo with a bit and operation
							--val2;
							if (val2 <= 0x7fff) {
								ptr = new BinaryExpr(location(), BIN_BITAND, arg1(), new IntConst(location(), static_cast<int>(val2)));
								return;
							}
							if (val2 <= 0x7fffffff) {
								ptr = new BinaryExpr(location(), BIN_BITAND, arg1(), new LongConst(location(), static_cast<long>(val2)));
								return;
							}
							ptr = new BinaryExpr(location(), BIN_BITAND, arg1(), new LongLongConst(location(), val2));
							return;
						}
						break;

					case BIN_BITAND:
						if (val2 == 0) {ptr = new IntConst(location(), 0); return;}
						if (val2 == -1) {ptr = arg1_; return;}
						break;

					case BIN_BITXOR:
						if (val2 == 0) {ptr = arg1_; return;}
						break;

					case BIN_BITOR:
						if (val2 == -1) {ptr = new IntConst(location(), -1); return;}
						if (val2 == 0) {ptr = arg1_; return;}
						break;

					case BIN_LEFT:
						if (val2 == 0) {ptr = arg1_; return;}
						break;

					case BIN_RIGHT:
						if (val2 == 0) {ptr = arg1_; return;}
						break;

					case BIN_AND:
						//if (val2 == 0) {return new BoolConst(false);}
						if (val2 != 0) {ptr = arg1_; return;}
						break;

					case BIN_OR:
						if (val2 == 0) {ptr = arg1_; return;}
						break;


					case BIN_EQ:
						if (val2 == 0) {ptr = new UnaryExpr(location(), UN_EQ0, arg1()); return;}
						break;

					case BIN_NE:
						if (val2 == 0) {ptr = new UnaryExpr(location(), UN_NE0, arg1()); return;}
						break;

					case BIN_LT:
						if (val2 == 0) {ptr = new UnaryExpr(location(), UN_LT0, arg1()); return;}
						break;

					case BIN_LE:
						if (val2 == 0) {ptr = new UnaryExpr(location(), UN_LE0, arg1()); return;}
						break;

					case BIN_GT:
						if (val2 == 0) {ptr = new UnaryExpr(location(), UN_GT0, arg1()); return;}
						break;

					case BIN_GE:
						if (val2 == 0) {ptr = new UnaryExpr(location(), UN_GE0, arg1()); return;}
						break;

					default:;
				}
			}
		}
		else if ((op_ == BIN_ADD) && arg2()->is_unary(UN_NEGATE)) {op_ = BIN_SUB; arg2_ = static_cast<UnaryExpr*>(arg2())->arg1(); optimize(ptr, mask);}
		else if ((op_ == BIN_SUB) && arg2()->is_unary(UN_NEGATE)) {op_ = BIN_ADD; arg2_ = static_cast<UnaryExpr*>(arg2())->arg1(); optimize(ptr, mask);}
	}
	Type* t1 = arg1()->type();
	Type* t2 = arg2()->type();

	if (t1->size() < t2->size()) {
		switch (op_) {
		case BIN_AND:
		case BIN_OR:
		case BIN_EQ:
		case BIN_NE:
		case BIN_LT:
		case BIN_LE:
		case BIN_GT:
		case BIN_GE:
		  arg1(new AutoCast(location(), t2, arg1()));
		  t1 = arg1()->type();
      set_type(new Type(T_BOOL));
      break;
    case BIN_LEFT:
    case BIN_RIGHT:
      if (t2->size() != 1) {
        t2 = t2->is_unsigned() ? new Type(T_UINT) : new Type(T_INT);
		    arg2(new AutoCast(location(), t2 , arg2()));
      }
      break;
		default:
		  arg1(new AutoCast(location(), t2, arg1()));
		  t1 = arg1()->type();
      set_type(t2);
		}
	}
	else if (t1->size() > t2->size()) {
		switch (op_) {
		case BIN_AND:
		case BIN_OR:
		case BIN_EQ:
		case BIN_NE:
		case BIN_LT:
		case BIN_LE:
		case BIN_GT:
		case BIN_GE:
		  arg2(new AutoCast(location(), t1, arg2()));
		  t2 = arg2()->type();
      set_type(new Type(T_BOOL));
      break;
    case BIN_LEFT:
    case BIN_RIGHT:
      if (t2->size() != 1) {
        t2 = t2->is_unsigned() ? new Type(T_UINT) : new Type(T_INT);
		    arg2(new AutoCast(location(), t2 , arg2()));
      }
      break;

		default:
		  arg2(new AutoCast(location(), t1, arg2()));
		  t2 = arg2()->type();
      set_type(t1);
		}
	}
 	if (t1->is_integer() != t2->is_integer()) {
  };

	if (t1->is_unsigned()) {
		if (!t2->is_unsigned()) {
		}
	}
	else if (t2->is_unsigned()) {
	}
}

int BinaryExpr::compile_condition(CompilationUnit& cu, bool push_result)
{
	//int res = 0;
	//size_t size = type_size();
	//switch (size) {
	//case 1:
	//	switch (op_) {
	//	case BIN_NE:
	//		res += arg1()->compile(cu, true);
	//		res += arg2()->compile(cu, false);
	//		res += cu.code().compile_ne();
 //     if (push_result) {
 //       res += cu.code().compile_push_opt();
 //     }
	//		return res;

	//	case BIN_EQ:
	//		res += arg1()->compile(cu, true);
	//		res += arg2()->compile(cu, false);
	//		res += cu.code().compile_eq();
 //     if (push_result) {
 //       res += cu.code().compile_push_opt();
 //     }
	//		return res;

	//	case BIN_LE:
	//		res += arg1()->compile(cu, true);
	//		res += arg2()->compile(cu, false);
	//		res += cu.code().compile_le();
 //     if (push_result) {
 //       res += cu.code().compile_push_opt();
 //     }
	//		return res;

	//	default:;
	//	  throw not_implemented_exception("BinaryExpr::compile_condition");
	//	}
	//	break;

	//default:;
	//}
	return compile(cu, push_result);
}

Call* BinaryExpr::assert_call(const char* n)
{
  Call* c = static_cast<Call*>(call_.get());
	if (c == 0) {
		c = new Call(n);
		c->add(arg1());
		c->add(arg2());
    call_ = c;
	}
  return c;
}


int BinaryExpr::compile(CompilationUnit& cu, bool push_result)
{
  size_t fixup1;
	int res = 0;
  Type* t1 = arg1()->type();
  Type* t2 = arg2()->type();
	size_t size1 = arg1()->type_size();
	size_t size2 = arg2()->type_size();
	size_t size = size1 > size2 ? size1 : size2;
	switch (size) {
	case 1:
		switch (op_) {

		case BIN_MUL:
      if (t1->is_integer() && t2->is_integer()) {
			  res += assert_call("^mul_int")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile + 1");
      }
			break;

		case BIN_ADD:
      if (t1->is_integer() && t2->is_integer()) {
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
			  res += cu.code().compile_add();
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile + 1");
      }
			break;

		case BIN_SUB:
      if (t1->is_integer() && t2->is_integer()) {
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
			  res += cu.code().compile_sub();
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile - 1");
      }
			break;

		case BIN_BITAND:
      if (t1->is_integer() && t2->is_integer()) {
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
			  res += cu.code().compile_and();
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile & 1");
      }
			break;

		case BIN_BITOR:
      if (t1->is_integer() && t2->is_integer()) {
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
			  res += cu.code().compile_or();
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile | 1");
      }
			break;

		case BIN_BITXOR:
      if (t1->is_integer() && t2->is_integer()) {
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
			  res += cu.code().compile_xor();
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile ^ 1");
      }
			break;

		case BIN_GT:
      if (t1->is_integer() && t2->is_integer()) {
        bool is_unsigned = t1->is_unsigned();
        if (is_unsigned != t2->is_unsigned()) {
          unsigned int value;
          if (arg1()->is_const()) {
            if (!arg1()->get_uint(value)) {
      		    throw not_implemented_exception("BinaryExpr::compile > cannot compare negative constant with unsigned expression");
            }
          }
          else if (arg2()->is_const()) {
            if (!arg2()->get_uint(value)) {
      		    throw not_implemented_exception("BinaryExpr::compile > cannot compare negative constant with unsigned expression");
            }
          }
          else {
    		    throw not_implemented_exception("BinaryExpr::compile > signed/unsigned mismatch");
          }
        }
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
        if (is_unsigned) {
			    res += cu.code().compile_ugt();
        }
        else {
			    res += cu.code().compile_gt();
        }
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile > 1");
      }
			break;

		case BIN_GE:
      if (t1->is_integer() && t2->is_integer()) {
        bool is_unsigned = t1->is_unsigned();
        if (is_unsigned != t2->is_unsigned()) {
          unsigned int value;
          if (arg1()->is_const()) {
            if (!arg1()->get_uint(value)) {
      		    throw not_implemented_exception("BinaryExpr::compile >= cannot compare negative constant with unsigned expression");
            }
          }
          else if (arg2()->is_const()) {
            if (!arg2()->get_uint(value)) {
      		    throw not_implemented_exception("BinaryExpr::compile >= cannot compare negative constant with unsigned expression");
            }
          }
          else {
    		    throw not_implemented_exception("BinaryExpr::compile >= signed/unsigned mismatch");
          }
        }
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
        if (is_unsigned) {
			    res += cu.code().compile_uge();
        }
        else {
			    res += cu.code().compile_ge();
        }
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile >= 1");
      }
			break;

		case BIN_LT:
      if (t1->is_integer() && t2->is_integer()) {
        bool is_unsigned = t1->is_unsigned();
        if (is_unsigned != t2->is_unsigned()) {
          unsigned int value;
          if (arg1()->is_const()) {
            if (!arg1()->get_uint(value)) {
      		    throw not_implemented_exception("BinaryExpr::compile < cannot compare negative constant with unsigned expression");
            }
          }
          else if (arg2()->is_const()) {
            if (!arg2()->get_uint(value)) {
      		    throw not_implemented_exception("BinaryExpr::compile < cannot compare negative constant with unsigned expression");
            }
          }
          else {
    		    throw not_implemented_exception("BinaryExpr::compile < signed/unsigned mismatch");
          }
          is_unsigned = true;
        }
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
        if (is_unsigned) {
			    res += cu.code().compile_ult();
        }
        else {
			    res += cu.code().compile_lt();
        }
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile < 1");
      }
			break;

		case BIN_LE:
      if (t1->is_integer_or_pointer() && t2->is_integer_or_pointer()) {
        bool is_unsigned = t1->is_unsigned();
        if (is_unsigned != t2->is_unsigned()) {
          unsigned int value;
          if (arg1()->is_const()) {
            if (!arg1()->get_uint(value)) {
      		    throw not_implemented_exception("BinaryExpr::compile <= cannot compare negative constant with unsigned expression");
            }
          }
          else if (arg2()->is_const()) {
            if (!arg2()->get_uint(value)) {
      		    throw not_implemented_exception("BinaryExpr::compile <= cannot compare negative constant with unsigned expression");
            }
          }
          else {
    		    throw not_implemented_exception("BinaryExpr::compile <= signed/unsigned mismatch");
          }
          is_unsigned = true;
        }
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
        if (is_unsigned) {
			    res += cu.code().compile_ule();
        }
        else {
			    res += cu.code().compile_le();
        }
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile <= 1");
      }
			break;

		case BIN_EQ:
      if (t1->is_integer() && t2->is_integer()) {
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
			  res += cu.code().compile_eq();
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile == 1");
      }
			break;

		case BIN_NE:
      if (t1->is_integer() && t2->is_integer()) {
			  res += arg1()->compile(cu, true);
			  res += arg2()->compile(cu, false);
			  res += cu.code().compile_ne();
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile != 1");
      }
			break;

		case BIN_AND:
	    res += cu.compile_condition(arg1(), false);
	    fixup1 = cu.code().size();
	    res += cu.code().compile_cond_branch();
	    res += cu.compile_condition(arg2(), false);
    	cu.code().fixup_branch(fixup1, cu.code().size());
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case BIN_OR:
	    res += cu.compile_condition(arg1(), false);
	    res += cu.code().compile_eq0();
	    fixup1 = cu.code().size();
	    res += cu.code().compile_cond_branch();
	    res += cu.compile_condition(arg2(), false);
	    res += cu.code().compile_eq0();
    	cu.code().fixup_branch(fixup1, cu.code().size());
	    res += cu.code().compile_eq0();
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case BIN_RIGHT:
      if (!t1->is_integer()) {
        throw generator_exception("BinaryExpr::compile 1 >> left side is not an integer");
      }
      if (!t2->is_integer()) {
        throw generator_exception("BinaryExpr::compile 1 >> right side is not an integer");
      }
			res += arg1()->compile(cu, true);
			res += arg2()->compile(cu, false);
      if (t1->is_unsigned()) {
			  res += cu.code().compile_sr();
      }
      else {
			  res += cu.code().compile_usr();
      }
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case BIN_LEFT:
      if (!t1->is_integer()) {
        throw generator_exception("BinaryExpr::compile 1 >> left side is not an integer");
      }
      if (!t2->is_integer()) {
        throw generator_exception("BinaryExpr::compile 1 >> right side is not an integer");
      }
			res += arg1()->compile(cu, true);
			res += arg2()->compile(cu, false);
			res += cu.code().compile_sl();
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		default: throw not_implemented_exception("BinaryExpr::compile 1");
		}
		break;

	case 2:
		switch (op_) {
		case BIN_ADD:
      if (t1->is_integer() && t2->is_integer()) {
			  res += assert_call("^add_long")->compile(cu, push_result);
      }
      else if (t1->is_float() && t2->is_float()) {
			  res += assert_call("^add_float")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile + 2");
      }
			break;

		case BIN_SUB:
      if (t1->is_integer() && t2->is_integer()) {
			  res += assert_call("^sub_long")->compile(cu, push_result);
      }
      else if (t1->is_float() && t2->is_float()) {
			  res += assert_call("^sub_float")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile - 2");
      }
			break;

		case BIN_BITAND:
      if (t1->is_integer() && t2->is_integer()) {
			  res += assert_call("^bitand_long")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile & 2");
      }
			break;

		case BIN_BITOR:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^bitor_long")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile | 2");
      }
			break;

		case BIN_BITXOR:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^bitxor_long")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile ^ 2");
      }
			break;

		case BIN_EQ:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^eq_long")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile == 2");
      }
			break;

		case BIN_NE:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^ne_long")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile != 2");
      }
			break;

    case BIN_MUL:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
  		    throw not_implemented_exception("BinaryExpr::compile * 2 signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^mul_ulong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^mul_long")->compile(cu, push_result);
        }
      }
      else if (t1->is_float() && t2->is_float()) {
    	  res += assert_call("^mul_float")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile * 2");
      }
			break;

    case BIN_DIV:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
  		    throw not_implemented_exception("BinaryExpr::compile / 2 signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^div_ulong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^div_long")->compile(cu, push_result);
        }
      }
      else if (t1->is_float() && t2->is_float()) {
    	  res += assert_call("^div_float")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile / 2");
      }
			break;

    case BIN_MOD:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
  		    throw not_implemented_exception("BinaryExpr::compile % 2 signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^mod_ulong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^mod_long")->compile(cu, push_result);
        }
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile % 2");
      }
			break;

		case BIN_LT:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
  		    throw not_implemented_exception("BinaryExpr::compile < 2 signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^lt_ulong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^lt_long")->compile(cu, push_result);
        }
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile < 2");
      }
			break;

		case BIN_LE:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
  		    throw not_implemented_exception("BinaryExpr::compile <= 2 signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^le_ulong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^le_long")->compile(cu, push_result);
        }
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile <= 2");
      }
			break;

		case BIN_GT:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
  		    throw not_implemented_exception("BinaryExpr::compile > 2 signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^gt_ulong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^gt_long")->compile(cu, push_result);
        }
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile > 2");
      }
			break;

		case BIN_GE:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
  		    throw not_implemented_exception("BinaryExpr::compile >= 2 signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^ge_ulong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^ge_long")->compile(cu, push_result);
        }
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile >= 2");
      }
			break;

		case BIN_RIGHT:
      if (!t1->is_integer()) {
        throw generator_exception("BinaryExpr::compile 2 >> left side is not an integer");
      }
      if (!t2->is_integer()) {
        throw generator_exception("BinaryExpr::compile 2 >> right side is not an integer");
      }
      if (t1->is_unsigned()) {
  			res += assert_call("^shr_ulong")->compile(cu, push_result);
      }
      else {
  			res += assert_call("^shr_long")->compile(cu, push_result);
      }
			break;

		case BIN_LEFT:
      if (!t1->is_integer()) {
        throw generator_exception("BinaryExpr::compile 2 >> left side is not an integer");
      }
      if (!t2->is_integer()) {
        throw generator_exception("BinaryExpr::compile 2 >> right side is not an integer");
      }
 			res += assert_call("^shl_long")->compile(cu, push_result);
			break;

		default: throw not_implemented_exception("BinaryExpr::compile 2");
		}
		break;


	case 4:
		switch (op_) {
		case BIN_ADD:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^add_longlong")->compile(cu, push_result);
      }
      else if (t1->is_float() && t2->is_float()) {
			  res += assert_call("^add_double")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile + 4");
      }
			break;

		case BIN_SUB:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^sub_longlong")->compile(cu, push_result);
      }
      else if (t1->is_float() && t2->is_float()) {
			  res += assert_call("^sub_double")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile - 4");
      }
			break;

		case BIN_MUL:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
  		    throw not_implemented_exception("BinaryExpr::compile * 4 signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
    	    res += assert_call("^mul_ulonglong")->compile(cu, push_result);
        }
        else {
    	    res += assert_call("^mul_longlong")->compile(cu, push_result);
        }
      }
      else if (t1->is_float() && t2->is_float()) {
			  res += assert_call("^mul_double")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile * 4");
      }
			break;

		case BIN_DIV:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
  		    throw not_implemented_exception("BinaryExpr::compile / 4 signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
    	    res += assert_call("^div_ulonglong")->compile(cu, push_result);
        }
        else {
    	    res += assert_call("^div_longlong")->compile(cu, push_result);
        }
      }
      else if (t1->is_float() && t2->is_float()) {
			  res += assert_call("^div_double")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile / 4");
      }
			break;

		case BIN_MOD:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
  		    throw not_implemented_exception("BinaryExpr::compile % 4 signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
    	    res += assert_call("^mod_ulonglong")->compile(cu, push_result);
        }
        else {
    	    res += assert_call("^mod_longlong")->compile(cu, push_result);
        }
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile % 4");
      }
			break;

		case BIN_BITAND:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^bitand_longlong")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile & 4");
      }
			break;

		case BIN_BITOR:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^bitor_longlong")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile | 4");
      }
			break;

		case BIN_BITXOR:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^bitxor_longlong")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile ^ 4");
      }
			break;

		case BIN_RIGHT:
      if (!t1->is_integer()) {
        throw generator_exception("BinaryExpr::compile 4 >> left side is not an integer");
      }
      if (!t2->is_integer()) {
        throw generator_exception("BinaryExpr::compile 4 >> right side is not an integer");
      }
      if (t1->is_unsigned()) {
  			res += assert_call("^shr_ulonglong")->compile(cu, push_result);
      }
      else {
  			res += assert_call("^shr_longlong")->compile(cu, push_result);
      }
			break;

		case BIN_LEFT:
      if (!t1->is_integer()) {
        throw generator_exception("BinaryExpr::compile 4 >> left side is not an integer");
      }
      if (!t2->is_integer()) {
        throw generator_exception("BinaryExpr::compile 4 >> right side is not an integer");
      }
 			res += assert_call("^shl_longlong")->compile(cu, push_result);
			break;

		case BIN_EQ:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^eq_longlong")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile == 4");
      }
			break;

		case BIN_NE:
      if (t1->is_integer() && t2->is_integer()) {
    	  res += assert_call("^ne_longlong")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile != 4");
      }
			break;

		case BIN_LE:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
          throw generator_exception("BinaryExpr::compile 4 <= signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^le_ulonglong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^le_longlong")->compile(cu, push_result);
        }
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile <= 4");
      }
			break;

		case BIN_LT:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
          throw generator_exception("BinaryExpr::compile 4 < signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^lt_ulonglong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^lt_longlong")->compile(cu, push_result);
        }
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile < 4");
      }
			break;

		case BIN_GE:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
          throw generator_exception("BinaryExpr::compile 4 >= signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^ge_ulonglong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^ge_longlong")->compile(cu, push_result);
        }
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile >= 4");
      }
			break;

		case BIN_GT:
      if (t1->is_integer() && t2->is_integer()) {
        if (t1->is_unsigned() != t2->is_unsigned()) {
          throw generator_exception("BinaryExpr::compile 4 > signed/unsigned mismatch");
        }
        if (t1->is_unsigned()) {
      	  res += assert_call("^gt_ulonglong")->compile(cu, push_result);
        }
        else {
      	  res += assert_call("^gt_longlong")->compile(cu, push_result);
        }
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile > 4");
      }
			break;

		default: throw not_implemented_exception("BinaryExpr::compile 4");
		}
		break;

	case 8:
		switch (op_) {
		case BIN_ADD:
      if (t1->is_float() && t2->is_float()) {
			  res += assert_call("^add_longdouble")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile + 8");
      }
			break;

		case BIN_SUB:
      if (t1->is_float() && t2->is_float()) {
			  res += assert_call("^sub_longdouble")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile - 8");
      }
			break;

		case BIN_MUL:
      if (t1->is_float() && t2->is_float()) {
			  res += assert_call("^mul_longdouble")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile * 8");
      }
			break;

		case BIN_DIV:
      if (t1->is_float() && t2->is_float()) {
			  res += assert_call("^div_longdouble")->compile(cu, push_result);
      }
      else {
		    throw not_implemented_exception("BinaryExpr::compile / 8");
      }
			break;
  
		default: throw not_implemented_exception("BinaryExpr::compile 8");
		}
		break;

	default: throw not_implemented_exception("BinaryExpr::compile");
	}
	return res;
}

}
