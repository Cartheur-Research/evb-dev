#include "cvmc.hpp"

namespace cvmc {

UnaryExpr::UnaryExpr(const context_type::position_type& w, UnOp op)
: Expression(w)
, op_(op)
, value_(0)
{
}

UnaryExpr::UnaryExpr(const context_type::position_type& w, UnOp op, Expression* arg1)
: Expression(w)
, op_(op)
, arg1_(arg1)
, value_(0)
{
	build_type();
}

UnaryExpr::UnaryExpr(const context_type::position_type& w, UnOp op, Expression* arg1, int value)
: Expression(w)
, op_(op)
, arg1_(arg1)
, value_(value)
{
	build_type();
}

UnaryExpr::UnaryExpr(UnOp op, Expression* arg1)
: op_(op)
, arg1_(arg1)
, value_(0)
{
	build_type();
}


//void UnaryExpr::initialize()
//{
//	if (op_ == UN_DEREF) {
//		Variable* var = dynamic_cast<Variable*>(arg1());
//		if (var) {
//			var->initialize();
//			return;
//		}
//	}
//	Expression::initialize();
//}

bool UnaryExpr::is_unary(UnOp op) const
{
	return op_ == op;
}

void UnaryExpr::assert_lvalue()
{
	if (!arg1()->is_lvalue()) {
		throw analyzer_exception("UnaryExpr::assert_lvalue not an lvalue");
	}
}

Type* UnaryExpr::lvalue_type()
{
	if (op_ == UN_DEREF) {
		ReferenceType* rt = dynamic_cast<ReferenceType*>(arg1()->type());
		if (rt == 0) {
			throw analyzer_exception("UnaryExpr::assert_lvalue cannot dereference a non-pointer");
		}
		if (rt->type() == T_REFERENCE) {
			throw analyzer_exception("UnaryExpr::assert_lvalue cannot dereference a reference");
		}
		return rt->next();
	}
	return Expression::lvalue_type();
}


ExprType UnaryExpr::expr_type() const
{
	return ET_UNARY;
}

bool UnaryExpr::has_side_effect() const
{
	switch (op_) {
		case UN_PREINC:
		case UN_PREDEC:
		case UN_POSTINC:
		case UN_POSTDEC:
			return true;

		default:;
	}
	return arg1()->has_side_effect();
}

bool UnaryExpr::is_const() const
{
	return arg1()->is_const();
}

size_t UnaryExpr::type_size()
{
	if (op_ == UN_DEREF) {
		ReferenceType* ref = dynamic_cast<ReferenceType*>(arg1()->type());
		if (ref == 0) {
			throw analyzer_exception("UnaryExpr::type_size cannot dereference a non-refrence type");
		}
		return ref->next()->size();
	}
	return type()->size();
}

bool UnaryExpr::is_lvalue() const
{
	return op_ == UN_DEREF;
}

int UnaryExpr::compile_store(CompilationUnit& cu)
{
	size_t size = type_size();
	int res = 0;
	switch (op_) {
		case UN_DEREF:
			if (size == 1) {
				res += cu.code().compile_push_opt();
				res += arg1()->compile(cu, false);
				res += cu.code().compile_store();
			}
			else if (size == 2) {
				res += cu.code().compile_push_long_opt();
				res += arg1()->compile(cu, false);
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
			}
			else if (size == 4) {
				res += arg1()->compile(cu, false);
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
			}
			else if (size == 8) {
				res += arg1()->compile(cu, false);
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
				res += cu.code().compile_inc();
				res += cu.code().compile_store();
			}
			else {
				throw not_implemented_exception("UnaryExpr::compile_store for size "+boost::lexical_cast<std::string>(size));
			}
			return res;

		default:;
	}
	throw analyzer_exception("UnaryExpr::compile_store not a lvalue");
	return res;
}

int UnaryExpr::compile_load(CompilationUnit& cu, bool push_result)
{
	size_t size = type_size();
	int res = 0;
	switch (op_) {
		case UN_DEREF:
			if (size == 1) {
				res += arg1()->compile(cu, false);
				res += cu.code().compile_load();
        if (push_result) {res += cu.code().compile_push_opt();}
			}
			//else if (size == 2) {
			//	res += cu.code().compile_push_long_opt();
			//	res += arg1()->compile(cu);
			//	res += cu.code().compile_load();
			//	res += cu.code().compile_inc();
			//	res += cu.code().compile_load();
			//}
			else {
				throw not_implemented_exception("UnaryExpr::compile_load for size "+boost::lexical_cast<std::string>(size));
			}
			return res;

		default:;
	}
	throw analyzer_exception("UnaryExpr::compile_load not a lvalue");
	return res;
}

int UnaryExpr::compile_adr(CompilationUnit& cu, size_t offset)
{
	int res = 0;
	switch (op_) {
		case UN_DEREF:
			res += cu.code().compile_push_opt();
			res += arg1()->compile_adr(cu, offset);
			return res;

		default:;
	}
	throw analyzer_exception("UnaryExpr::compile_store not addressable");
	return res;
}

void UnaryExpr::build_type()
{
	arg1()->build_type();
	switch (op_) {
		case UN_NOT:
		case UN_NE0:
		case UN_EQ0:
		case UN_LE0:
		case UN_LT0:
		case UN_GE0:
		case UN_GT0:
			set_type(new Type(T_BOOL));
			break;

		case UN_REF:
			set_type(new ReferenceType(T_POINTER, arg1()->type()));
			break;

		case UN_DEREF:
			{
				ReferenceType* ref = dynamic_cast<ReferenceType*>(arg1()->type());
				if (ref == 0) {
					throw analyzer_exception("UnaryExpr::build_type cannot dereference a non-refrence type");
				}
				set_type(ref->next());
			}
			break;


		default:
			set_type(arg1()->type());
	}
}

void UnaryExpr::optimize(ObjPtr& ptr, int mask)
{
	arg1_.optimize(mask);
	switch (op_) {
		case UN_PREINC:
		case UN_PREDEC:
		case UN_POSTINC:
		case UN_POSTDEC:
			assert_lvalue();
			break;

		default:;
	}
	if (((MASK(OPTM_CONST_FOLDING) & mask) != 0) && is_const()) {
    //if (op_ == UN_SIZEOF) {
    //  size_t s = arg1()->type()->size();
    //  ptr = new IntConst(location(), s);
    //  return;
    //}
		{ // try int
			int val1;
			if (arg1()->get_int(val1)) {
				switch (op_) {
					case UN_PLUS: break;
					case UN_NEGATE: val1 = -val1; break;
					case UN_BITNOT: val1 = ~val1; break;
					case UN_NOT: ptr = new BoolConst(location(), !val1); return;
					case UN_INC: val1 = ++val1; break;
					case UN_DEC: val1 = --val1; break;
					case UN_MUL2: val1 = val1 << 1; break;
					case UN_DIV2: val1 = val1 >> 1; break;
					case UN_PREINC: val1 = ++val1; break;
					case UN_PREDEC: val1 = --val1; break;
					case UN_POSTINC: val1 = ++val1; break;
					case UN_POSTDEC: val1 = --val1; break;
					case UN_OFFSET: val1 += value_; break;
					default: return;
				}
				ptr = new IntConst(location(), val1);
				return;
			}
		}
		{ // try long
			long val1;
			if (arg1()->get_long(val1)) {
				switch (op_) {
					case UN_PLUS: break;
					case UN_NEGATE: val1 = -val1; break;
					case UN_BITNOT: val1 = ~val1; break;
					case UN_NOT: ptr = new BoolConst(location(), !val1); return;
					case UN_INC: val1 = ++val1; break;
					case UN_DEC: val1 = --val1; break;
					case UN_MUL2: val1 = val1 << 1; break;
					case UN_DIV2: val1 = val1 >> 1; break;
					case UN_PREINC: val1 = ++val1; break;
					case UN_PREDEC: val1 = --val1; break;
					case UN_POSTINC: val1 = ++val1; break;
					case UN_POSTDEC: val1 = --val1; break;
					case UN_OFFSET: val1 += value_; break;
					default: return;
				}
				ptr = new LongConst(location(), val1);
				return;
			}
		}
		{ // try long long
			long long val1;
			if (arg1()->get_longlong(val1)) {
				switch (op_) {
					case UN_PLUS: break;
					case UN_NEGATE: val1 = -val1; break;
					case UN_BITNOT: val1 = ~val1; break;
					case UN_NOT: ptr = new BoolConst(location(), !val1); return;
					case UN_INC: val1 = ++val1; break;
					case UN_DEC: val1 = --val1; break;
					case UN_MUL2: val1 = val1 << 1; break;
					case UN_DIV2: val1 = val1 >> 1; break;
					case UN_PREINC: val1 = ++val1; break;
					case UN_PREDEC: val1 = --val1; break;
					case UN_POSTINC: val1 = ++val1; break;
					case UN_POSTDEC: val1 = --val1; break;
					case UN_OFFSET: val1 += value_; break;
					default: return;
				}
				ptr = new LongLongConst(location(), val1);
				return;
			}
		}
	}
	if ((MASK(OPTM_PEEPHOLE) & mask) != 0) {
		if (op_ == UN_PLUS) {ptr = arg1_;}
		else if ((op_ == UN_OFFSET) && (value_ == 0)) {ptr = arg1_;}
		else if ((op_ == UN_BITNOT) && arg1()->is_unary(UN_BITNOT)) {ptr = static_cast<UnaryExpr*>(arg1())->arg1(); optimize(ptr, mask);}
		else if ((op_ == UN_NEGATE) && arg1()->is_unary(UN_NEGATE)) {ptr = static_cast<UnaryExpr*>(arg1())->arg1(); optimize(ptr, mask);}
	}
}


//Expression* UnaryExpr::evaluated()
//{
//	Local* loc = dynamic_cast<Local*>(arg1());
//	if (loc) {return loc;}
//	Parameter* par = dynamic_cast<Parameter*>(arg1());
//	if (par) {return par;}
//	Static* var = dynamic_cast<Static*>(arg1());
//	if (var) {return var;}
//	return this;
//}

int UnaryExpr::compile(CompilationUnit& cu, bool push_result)
{
	ReferenceType* ref;
	//Local* loc;
	//Parameter* par;
	//Static* stat;
	Variable* var;
	//ObjPtr obj;
	//Call* c;
	int res = 0;
  if (op_ == UN_REF) {
		res += arg1()->compile_adr(cu);
    return res;
  }

	size_t size = type_size();
  Type* t = arg1()->type();
	size_t arg_size = arg1()->type_size();
	switch (arg_size) {
	case 1:
		switch (op_) {
		case UN_PLUS:
      res += arg1()->compile(cu, push_result);
      break;

		case UN_NEGATE:
      res += arg1()->compile(cu, false);
      res += cu.code().compile_negate();
      if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
      if (push_result) {res += cu.code().compile_push_opt();}
      break;

		case UN_BITNOT:
      res += arg1()->compile(cu, false);
      res += cu.code().compile_not();
      if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
      if (push_result) {res += cu.code().compile_push_opt();}
      break;

		case UN_NOT:
      res += arg1()->compile(cu, false);
      res += cu.code().compile_eq0();
      if (push_result) {res += cu.code().compile_push_opt();}
      break;

		case UN_INC:
      res += arg1()->compile(cu, false);
      if (value_ == 1) {
        res += cu.code().compile_inc();
      }
      else if (value_ == -1) {
        res += cu.code().compile_dec();
      }
      else {
        res += cu.code().compile_push_opt();
        res += cu.code().compile_add_opt(value_);
      }
      if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
      if (push_result) {res += cu.code().compile_push_opt();}
      break;

		case UN_DEC:
      res += arg1()->compile(cu, false);
      if (value_ == 1) {
        res += cu.code().compile_dec();
      }
      else if (value_ == -1) {
        res += cu.code().compile_inc();
      }
      else {
        res += cu.code().compile_push_opt();
        res += cu.code().compile_literal(value_);
        res += cu.code().compile_sub();
      }
      if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
      if (push_result) {res += cu.code().compile_push_opt();}
      break;

		case UN_MUL2:
      res += arg1()->compile(cu, false);
      res += cu.code().compile_mul2();
      if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
      if (push_result) {res += cu.code().compile_push_opt();}
      break;

		case UN_DIV2:
			res += arg1()->compile(cu, false);
			if (arg1()->type()->is_unsigned()) {
				res += cu.code().compile_udiv2();
			}
			else {
				res += cu.code().compile_div2();
			}
      if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case UN_PREINC:
			if (!arg1()->is_lvalue()) {
				arg1()->is_lvalue();
				throw not_implemented_exception("UnaryExpr::compile not an lvalue");
			}
			var = dynamic_cast<Variable*>(arg1());
			if (var) {
				res += var->compile_load(cu, false);
				res += cu.code().compile_inc();
        if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
				res += var->compile_store(cu);
        if (push_result) {res += cu.code().compile_push_opt();}
			}
			else {
				ref = dynamic_cast<ReferenceType*>(arg1()->type());
				if (ref) {
					if (cu.code().optimize_space()) {
			      if (call_.is_null()) {
              if (t->type() == T_BOOL) {call_ = new Call("^preinc_bool");}
              else {call_ = new Call("^preinc_int");}
				      call()->add(arg1());
			      }
			      res += call()->compile(cu, false);
					}
					else {
						res += arg1()->compile(cu, false);
						res += cu.code().compile_stt();
						res += cu.code().compile_load();
						res += cu.code().compile_inc();
            if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
						res += cu.code().compile_push();
						res += cu.code().compile_xt();
						res += cu.code().compile_store();
						res += cu.code().compile_ldt();
					}
          if (push_result) {res += cu.code().compile_push_opt();}
				}
				else {
					res += arg1()->compile_adr(cu);
					res += cu.code().compile_stt();
					res += cu.code().compile_load();
					res += cu.code().compile_inc();
          if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
					res += cu.code().compile_push();
					res += cu.code().compile_xt();
					res += cu.code().compile_store();
					res += cu.code().compile_ldt();
          if (push_result) {res += cu.code().compile_push_opt();}
        }
			}
			break;

		case UN_PREDEC:
			var = dynamic_cast<Variable*>(arg1());
			if (var) {
				res += var->compile_load(cu, false);
				res += cu.code().compile_dec();
        if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
				res += var->compile_store(cu);
        if (push_result) {res += cu.code().compile_push_opt();}
			}
			else {
				ref = dynamic_cast<ReferenceType*>(arg1()->type());
				if (ref) {
					if (cu.code().optimize_space()) {
			      if (call_.is_null()) {
              if (t->type() == T_BOOL) {call_ = new Call("^predec_bool");}
              else {call_ = new Call("^predec_int");}
				      call()->add(arg1());
			      }
			      res += call()->compile(cu, false);
					}
					else {
						res += arg1()->compile(cu, false);
						res += cu.code().compile_stt();
						res += cu.code().compile_load();
						res += cu.code().compile_dec();
            if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
						res += cu.code().compile_push();
						res += cu.code().compile_xt();
						res += cu.code().compile_store();
						res += cu.code().compile_ldt();
					}
          if (push_result) {res += cu.code().compile_push_opt();}
				}
				else {throw not_implemented_exception("UnaryExpr::compile --.");}
			}
			break;

		case UN_POSTINC:
			var = dynamic_cast<Variable*>(arg1());
			if (var) {
				res += var->compile_load(cu, false);
				res += cu.code().compile_stt();
				res += cu.code().compile_inc();
        if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
				res += var->compile_store(cu);
				res += cu.code().compile_ldt();
        if (push_result) {res += cu.code().compile_push_opt();}
			}
			else {
				ref = dynamic_cast<ReferenceType*>(arg1()->type());
				if (ref) {
					if (cu.code().optimize_space()) {
			      if (call_.is_null()) {
              if (t->type() == T_BOOL) {call_ = new Call("^postinc_bool");}
              else {call_ = new Call("^postinc_int");}
				      call()->add(arg1());
			      }
			      res += call()->compile(cu, false);
					}
					else {
						res += arg1()->compile(cu, false);
						res += cu.code().compile_stt();
						res += cu.code().compile_load();
						res += cu.code().compile_inc();
            if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
						res += cu.code().compile_push();
						res += cu.code().compile_xt();
						res += cu.code().compile_store();
						res += cu.code().compile_ldt();
					}
          if (push_result) {res += cu.code().compile_push_opt();}
				}
				else {throw not_implemented_exception("UnaryExpr::compile .++");}
			}
			break;

		case UN_POSTDEC:
			var = dynamic_cast<Variable*>(arg1());
			if (var) {
				res += var->compile_load(cu, false);
				res += cu.code().compile_stt();
				res += cu.code().compile_dec();
        if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
				res += var->compile_store(cu);
				res += cu.code().compile_ldt();
        if (push_result) {res += cu.code().compile_push_opt();}
			}
			else {
				ref = dynamic_cast<ReferenceType*>(arg1()->type());
				if (ref) {
					if (cu.code().optimize_space()) {
			      if (call_.is_null()) {
              if (t->type() == T_BOOL) {call_ = new Call("^postdec_bool");}
              else {call_ = new Call("^postdec_int");}
				      call()->add(arg1());
			      }
			      res += call()->compile(cu, false);
					}
					else {
						res += arg1()->compile(cu, false);
						res += cu.code().compile_stt();
						res += cu.code().compile_load();
						res += cu.code().compile_dec();
            if (t->type() == T_BOOL) {res += cu.code().compile_ne0();}
						res += cu.code().compile_push();
						res += cu.code().compile_xt();
						res += cu.code().compile_store();
						res += cu.code().compile_ldt();
					}
          if (push_result) {res += cu.code().compile_push_opt();}
        }
			  else {
				  throw not_implemented_exception("UnaryExpr::compile .--");
			  }
			}
			break;

		case UN_EQ0:
			res += arg1()->compile(cu, false);
			res += cu.code().compile_eq0();
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case UN_NE0:
			res += arg1()->compile(cu, false);
			res += cu.code().compile_ne0();
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case UN_GT0:
			res += arg1()->compile(cu, false);
      if (t->is_unsigned()) {
  			res += cu.code().compile_ne0();
      }
      else {
  			res += cu.code().compile_gt0();
      }
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case UN_GE0:
      if (t->is_unsigned()) {
        throw not_implemented_exception("UnaryExpr::compile 1 not optimized");
      }
			res += arg1()->compile(cu, false);
			res += cu.code().compile_ge0();
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case UN_LT0:
      if (t->is_unsigned()) {
        throw not_implemented_exception("UnaryExpr::compile 1 not optimized");
      }
			res += arg1()->compile(cu, false);
			res += cu.code().compile_lt0();
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case UN_LE0:
			res += arg1()->compile(cu, false);
      if (t->is_unsigned()) {
  			res += cu.code().compile_eq0();
      }
      else {
  			res += cu.code().compile_le0();
      }
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case UN_DEREF:
      //size_t dsize = 
			res += arg1()->compile(cu, false);
			res += cu.code().compile_load();
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case UN_OFFSET:
			res += arg1()->compile(cu, false);
			res += cu.code().compile_adr_offset(value_);
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		default: throw not_implemented_exception("UnaryExpr::compile 1");
		}
		break;

	case 2:
		switch (op_) {
		case UN_PLUS:
      res += arg1()->compile(cu, push_result);
      break;

		case UN_NEGATE:
			if (call_.is_null()) {
				call_ = new Call("^neg_long");
				call()->add(arg1());
			}
			res += call()->compile(cu, push_result);
			//c = new Call("^neg_long");
			//obj = c;
			//c->add(arg1());
			//res += c->compile(cu);
			break;

		case UN_BITNOT:
			if (call_.is_null()) {
				call_ = new Call("^bitnot_long");
				call()->add(arg1());
			}
			res += call()->compile(cu, push_result);
			//c = new Call("^bitnot_long");
			//obj = c;
			//c->add(arg1());
			//res += c->compile(cu);
			break;

		case UN_INC:
			if (call_.is_null()) {
				call_ = new Call("^inc_long");
				call()->add(arg1());
			}
			res += call()->compile(cu, push_result);
			break;

		case UN_DEC:
			if (call_.is_null()) {
				call_ = new Call("^dec_long");
				call()->add(arg1());
			}
			res += call()->compile(cu, push_result);
			break;

		case UN_PREINC:
			if (call_.is_null()) {
				call_ = new Call("^preinc_long");
				call()->add(arg1());
			}
			res += call()->compile(cu, push_result);
			break;

		case UN_PREDEC:
			if (call_.is_null()) {
				call_ = new Call("^predec_long");
				call()->add(arg1());
			}
			res += call()->compile(cu, push_result);
			break;

		case UN_POSTINC:
			if (call_.is_null()) {
				call_ = new Call("^postinc_long");
				call()->add(arg1());
			}
			res += call()->compile(cu, push_result);
			break;

		case UN_POSTDEC:
			if (call_.is_null()) {
				call_ = new Call("^postdec_long");
				call()->add(arg1());
			}
			res += call()->compile(cu, push_result);
			break;

		//case UN_DEREF:
		//	res += arg1()->compile(cu, false);
		//	res += cu.code().compile_load_push();
		//	res += cu.code().compile_load_offset(1);
		//	res += cu.code().compile_xd();
		//	res += cu.code().compile_pop();
		//	res += cu.code().compile_xd();
		//	break;


		case UN_NOT:
			res += arg1()->compile(cu, false);
			res += cu.code().compile_push_opt();
			res += cu.code().compile_xd();
			res += cu.code().compile_or();
			res += cu.code().compile_eq0();
      if (push_result) {res += cu.code().compile_push_opt();}
			break;

		case UN_LT0:
      if (t->is_integer()) {
        if (t->is_unsigned()) {
			    res += arg1()->compile(cu, false);
			    res += cu.code().compile_false();
          if (push_result) {res += cu.code().compile_push_opt();}
        }
        else {
			    if (call_.is_null()) {
				    call_ = new Call("^lt0_long");
				    call()->add(arg1());
			    }
			    res += call()->compile(cu, push_result);
        }
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 2 <0 not an integer");
      }
			break;

		case UN_LE0:
      if (t->is_integer()) {
        if (t->is_unsigned()) {
			    res += arg1()->compile(cu, false);
			    res += cu.code().compile_push_opt();
			    res += cu.code().compile_xd();
			    res += cu.code().compile_or();
			    res += cu.code().compile_eq0();
          if (push_result) {res += cu.code().compile_push_opt();}
        }
        else {
			    if (call_.is_null()) {
				    call_ = new Call("^le0_long");
				    call()->add(arg1());
			    }
			    res += call()->compile(cu, push_result);
        }
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 2 <=0 not an integer");
      }
			break;

		case UN_GT0:
      if (t->is_integer()) {
        if (t->is_unsigned()) {
			    res += arg1()->compile(cu, false);
			    res += cu.code().compile_push_opt();
			    res += cu.code().compile_xd();
			    res += cu.code().compile_or();
			    res += cu.code().compile_ne0();
          if (push_result) {res += cu.code().compile_push_opt();}
        }
        else {
			    if (call_.is_null()) {
				    call_ = new Call("^gt0_long");
				    call()->add(arg1());
			    }
			    res += call()->compile(cu, push_result);
        }
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 2 >0 not an integer");
      }
			break;

		case UN_GE0:
      if (t->is_integer()) {
        if (t->is_unsigned()) {
			    res += arg1()->compile(cu, false);
			    res += cu.code().compile_true();
          if (push_result) {res += cu.code().compile_push_opt();}
        }
        else {
			    if (call_.is_null()) {
				    call_ = new Call("^ge0_long");
				    call()->add(arg1());
			    }
			    res += call()->compile(cu, push_result);
        }
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 2 <0 not an integer");
      }
			break;

		case UN_EQ0:
      if (t->is_integer()) {
			  res += arg1()->compile(cu, false);
			  res += cu.code().compile_push_opt();
			  res += cu.code().compile_xd();
			  res += cu.code().compile_or();
			  res += cu.code().compile_eq0();
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 2 ==0 not an integer");
      }
			break;

		case UN_NE0:
      if (t->is_integer()) {
			  res += arg1()->compile(cu, false);
			  res += cu.code().compile_push_opt();
			  res += cu.code().compile_xd();
			  res += cu.code().compile_or();
			  res += cu.code().compile_ne0();
        if (push_result) {res += cu.code().compile_push_opt();}
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 2 !=0 not an integer");
      }
			break;

		default: throw not_implemented_exception("UnaryExpr::compile 2");
		}
		break;

	case 4:
		switch (op_) {

		case UN_NOT:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^bitnot_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 ! not an integer");
      }
			break;

		case UN_INC:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^inc_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 +1 not an integer");
      }
			break;

		case UN_DEC:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^dec_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 -1 not an integer");
      }
			break;

		case UN_PREINC:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^preinc_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 ++x not an integer");
      }
			break;

		case UN_PREDEC:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^predec_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 --x not an integer");
      }
			break;

		case UN_POSTINC:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^postinc_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 x++ not an integer");
      }
			break;

		case UN_POSTDEC:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^postdec_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 x-- not an integer");
      }
			break;

		case UN_DEREF:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^deref_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 * not an integer");
      }
			break;

		case UN_NEGATE:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^neg_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 - not an integer");
      }
			break;

		case UN_EQ0:
			res += arg1()->compile(cu, false);
			res += cu.code().compile_pop();
			res += cu.code().compile_or();
			res += cu.code().compile_or();
			res += cu.code().compile_or();
			res += cu.code().compile_eq0();
			break;

		case UN_NE0:
			res += arg1()->compile(cu, false);
			res += cu.code().compile_pop();
			res += cu.code().compile_or();
			res += cu.code().compile_or();
			res += cu.code().compile_or();
			res += cu.code().compile_ne0();
			break;

		case UN_LT0:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^lt0_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 <0 not an integer");
      }
			break;

		case UN_LE0:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^le0_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 <=0 not an integer");
      }
			break;

		case UN_GT0:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^gt0_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 >0 not an integer");
      }
			break;

		case UN_GE0:
      if (t->is_integer()) {
			  if (call_.is_null()) {
				  call_ = new Call("^ge0_longlong");
				  call()->add(arg1());
			  }
			  res += call()->compile(cu, push_result);
      }
      else {
        throw not_implemented_exception("UnaryExpr::compile 4 >=0 not an integer");
      }
			break;

		default: throw not_implemented_exception("UnaryExpr::compile 4");
		}
		break;

	default: throw not_implemented_exception("UnaryExpr::compile");
	}
	return res;
}

}
