#include "cvmc.hpp"

namespace cvmc {


Expression::Expression()
{
}

Expression::Expression(const context_type::position_type& w)
: Statement(w)
{
}

Expression::Expression(const context_type::position_type& w, Type* t)
: Statement(w)
, type_(t)
{
}

Expression::Expression(Type* t)
: type_(t)
{
}

//void Expression::initialize()
//{
//	throw new not_implemented_exception("Expression::initialize");
//}

void Expression::build_type()
{
	throw new not_implemented_exception("Expression::build_type");
}

Type* Expression::lvalue_type()
{
	throw new parser_exception("Expression::lvalue_type not an lvalue");
	return 0;
}

void Expression::set_type(Type* val)
{
  type_ = val;
}

Type* Expression::type()
{
	if (type_.is_null()) {
		build_type();
		if (type_.is_null()) {
		  build_type();
			throw new analyzer_exception("Type::type failed");
		}
	}
	return static_cast<Type*>(type_.get());
}

const Type* Expression::type() const
{
	return const_cast<Expression*>(this)->type();
}


size_t Expression::initializer_depth() const
{
	return 0;
}

size_t Expression::type_size()
{
	return type()->size();
}

ExprType Expression::expr_type() const
{
	return ET_UNDEFINED;
}

bool Expression::is_const() const
{
	return false;
}

bool Expression::is_negative() const
{
	return false;
}

bool Expression::is_lvalue() const
{
	return false;
}

bool Expression::has_side_effect() const
{
	return false;
}

bool Expression::is_unary(UnOp) const
{
	return false;
}

bool Expression::is_binary(BinOp) const
{
	return false;
}

bool Expression::get_int(int&) const {return false;}
bool Expression::get_long(long&) const {return false;}
bool Expression::get_longlong(long long&) const {return false;}

bool Expression::get_uint(unsigned int& value) const
{
	long val;
	if (get_long(val)) {
		if (val < 0) {return false;}
		if (val <= MAX_UINT) {
			value = static_cast<unsigned int>(val);
			return true;
		}
	}
	return false;
}

bool Expression::get_ulong(unsigned long& value) const
{
	long long val;
	if (get_longlong(val)) {
		if (val < 0) {return false;}
		if (val <= MAX_ULONG) {
			value = static_cast<unsigned long>(val);
			return true;
		}
	}
	return false;
}

bool Expression::get_ulonglong(unsigned long long& value) const
{
	long long val;
	if (get_longlong(val)) {
		value = static_cast<unsigned long long>(val);
		return true;
	}
	return false;
}

void Expression::compile_data(CompilationUnit& cu)
{
	throw not_implemented_exception("Expression::compile_data");
}

int Expression::compile_push(CompilationUnit& cu)
{
	int res = compile(cu, true);
	//switch (type_size()) {
	//	case 0: break;
	//	case 1: res += cu.code().compile_push_opt(); break;
	//	default: res += cu.code().compile_push_long_opt(); break;
	//}
	return res;
}

int Expression::compile_pop(CompilationUnit& cu)
{
	throw analyzer_exception("Expression::compile_pop not a lvalue");
	return -1;
}

int Expression::compile_push(CompilationUnit& cu, size_t offset)
{
	throw not_implemented_exception("Expression::compile_push");
	return 1;
}

int Expression::compile_pop(CompilationUnit& cu, size_t offset)
{
	throw analyzer_exception("Expression::compile_pop not a lvalue");
	return -1;
}

int Expression::compile_load(CompilationUnit& cu, bool push_result)
{
	throw analyzer_exception("Expression::compile_load not a lvalue");
	return 1;
}

int Expression::compile_store(CompilationUnit& cu)
{
	throw analyzer_exception("Expression::compile_store not a lvalue");
	return 1;
}

int Expression::compile_load_word(CompilationUnit& cu, size_t offset, bool push_result)
{
	throw analyzer_exception("Expression::compile_load not a lvalue");
	return 1;
}

int Expression::compile_store_word(CompilationUnit& cu, size_t offset)
{
	throw analyzer_exception("Expression::compile_store not a lvalue");
	return 1;
}

int Expression::compile_adr(CompilationUnit& cu, size_t offset)
{
	throw analyzer_exception("Expression::compile_store not addressable");
	return 1;
}

//void Expression::append(ObjPtr& ptr, ArrayAccess& acc)
//{
//	acc.base(ptr.as_expression());
//	ptr = &acc;
//}

//Call* Expression::assert_call(const char* name)
//{
//	throw not_implemented_exception("Expression::compile_store not addressable");
//
// // Call* c = static_cast<Call*>(call_.get());
//	//if (c == 0) {
// //   std::string name(n);
// //   if (
// //     (name == "^load_long")
// //   )
// //   {
//	//	  c = new Call(name);
// //     c->set_type();
//	//	  call()->add(arg1());
//	//	  call()->add(arg2());
// //   }
// //   call_ = c;
//	//}
//  return 0;
//}

//*************************************************************************************

Label::Label(const context_type::position_type& w)
: Expression(w)
, value_(0)
, is_declared_(false)
, is_compiled_(false)
{
}

void Label::add(CompilationUnit& cu, Jump* jmp)
{
	if (jmp) {
		if (is_compiled_) {
			jmp->fixup(cu.code(), value_);
		}
		else {
			fixups_.push_back(jmp);
		}
	}
}

int Label::compile(CompilationUnit& cu, bool push_result)
{
	value_ = cu.code().size();
	is_compiled_ = true;
	if (!fixups_.empty()) {
		size_t limit = fixups_.size();
		for (size_t i=0; i<limit; ++i) {
			Jump* jmp = static_cast<Jump*>(fixups_[i].get());
			if (jmp) {
				jmp->fixup(cu.code(), value_);
			}
		}
		fixups_.clear();
	}
	return 0;
}

//*************************************************************************************


ArrayAccess::ArrayAccess(Expression* ex)
: is_optimized_(false)
, is_direct_(false)
{
//	type(base->type());
	this->base(ex);
}

ArrayAccess::ArrayAccess(const context_type::position_type& w, Expression* ex)
: Expression(w)
, is_optimized_(false)
, is_direct_(false)
{
//	type(base->type());
	this->base(ex);
}

void ArrayAccess::build_type()
{
	if (base_.is_not_null() && !is_optimized_) {
		// after optimization, the type must not change
		set_type(base()->type()->deref());
	}
}

bool ArrayAccess::is_const() const
{
	if (index_.is_null()) {
		return base()->is_const();
	}
	return false;
}

bool ArrayAccess::is_lvalue() const
{
	return true;
}

void ArrayAccess::base(Expression* val)
{
	if (val) {
		base_ = val;
		build_type();
		is_optimized_ = false;
		ArrayType* arr = dynamic_cast<ArrayType*>(val->type());
		if (arr && (arr->limit() > 0)) {
			is_direct_ = true;
		}
	}
}

void ArrayAccess::optimize(ObjPtr& obj, int mask)
{
	if (!is_optimized_) {
		is_optimized_ = true;
		base_.optimize(mask);
		index_.optimize(mask);
		if (index()->is_const()) {
			ArrayAccess* acc = dynamic_cast<ArrayAccess*>(base());
			if (acc) {
				if (acc->index()->is_const()) {
					int outer;
					int inner;
					if (index()->get_int(inner) && acc->index()->get_int(outer)) {
						int os = acc->type_size();
						int is = type_size();
						outer *= os / is;
						index_ = new IntConst(outer + inner);
						base_ = acc->base();
						//ArrayType* at = dynamic_cast<ArrayType*>(base()->type());
						//if (at) {
						//	type(at->next());
						//}
						//else {
						//	throw exception("ArrayAccess::optimize invalid inner type");
						//}
					}
					is_direct_ = acc->is_direct_;
				}
			}
		}
	}
}

void ArrayAccess::add_index(Expression* index)
{
	if (index_.is_null()) {
		index_ = index;
	}
	else {
		index_ = new BinaryExpr(BIN_ADD, this->index(), index);
	}
}

Type* ArrayAccess::lvalue_type()
{
	return type();
}

int ArrayAccess::compile_push(CompilationUnit& cu)
{
	throw not_implemented_exception("ArrayAccess::compile_push");
	return 0;
}

int ArrayAccess::compile_pop(CompilationUnit& cu)
{
	throw not_implemented_exception("ArrayAccess::compile_pop");
	return 0;
}

int ArrayAccess::compile_push(CompilationUnit& cu, size_t offset)
{
	throw not_implemented_exception("ArrayAccess::compile_push");
	return 0;
}

int ArrayAccess::compile_pop(CompilationUnit& cu, size_t offset)
{
	throw not_implemented_exception("ArrayAccess::compile_pop");
	return 0;
}

int ArrayAccess::compile_load(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	int value;
  size_t type_size = type()->size();
	switch (type_size) {
	case 1:
		if (index()->is_const()) {
			if (index()->get_int(value)) {
				value *= type()->size();
				if (value >= 0) {
					if (is_direct_) {
						res += base()->compile_load_word(cu, value, push_result);
					}
					else {
						res += base()->compile(cu, false);
						if (value <= OFFSET_LIMIT) {
							res += cu.code().compile_load_offset(value);
						}
						else {
							res += cu.code().compile_add_opt(value);
							res += cu.code().compile_load();
						}
            if (push_result) {res += cu.code().compile_push_opt();}
					}
				}
				else {
					throw analyzer_exception("ArrayAccess::compile_load negative index");
				}
			}
			else {
				throw analyzer_exception("ArrayAccess::compile_load integer constant expected");
			}
		}
		else {
			if (is_direct_) {
				res += base()->compile_adr(cu);
  			res += cu.code().compile_push();
			}
			else {
				res += base()->compile(cu, true);
			}
			res += index()->compile(cu, false);
			res += cu.code().compile_mul_opt(type()->size());
			res += cu.code().compile_add();
			res += cu.code().compile_load();
		}
    if (push_result) {res += cu.code().compile_push_opt();}
		break;

	case 2:
  	res += compile_adr(cu);
    if (push_result) {
			res += cu.code().compile_stt();
			res += cu.code().compile_load();
      res += cu.code().compile_push_opt();
			res += cu.code().compile_ldt();
			res += cu.code().compile_inc();
			res += cu.code().compile_load();
      res += cu.code().compile_push_opt();
    }
    else {
			res += cu.code().compile_stt();
			res += cu.code().compile_load();
			res += cu.code().compile_xd();
			res += cu.code().compile_ldt();
			res += cu.code().compile_inc();
			res += cu.code().compile_load();
    }
		break;

	case 4:
    res += cu.code().compile_reserve(4);
  	res += compile_adr(cu);
    res += cu.code().compile_push_opt();
	  cu.code().manager()->add_call("^load_longlong", cu.code().size());
	  res += cu.code().compile_call(0);
		break;

	default:
		throw not_implemented_exception("ArrayAccess::compile_load");
	}
	return res;
}

int ArrayAccess::compile_store(CompilationUnit& cu)
{
	int res = 0;
	int value;
	switch (type()->size()) {
	case 1:
		if (index()->is_const()) {
			if (index()->get_int(value)) {
				value *= type()->size();
				if (value >= 0) {
					if (is_direct_) {
						res += base()->compile_store_word(cu, value);
					}
					else {
						res += cu.code().compile_push();
						res += base()->compile(cu, false);
						if (value <= OFFSET_LIMIT) {
							res += cu.code().compile_store_offset(value);
						}
						else {
							res += cu.code().compile_add_opt(value);
							res += cu.code().compile_store();
						}
					}
				}
				else {
					res += cu.code().compile_push();
					res += base()->compile(cu, false);
					res += cu.code().compile_sub_opt(-value);
					res += cu.code().compile_store();
				}
			}
			else {
				throw analyzer_exception("ArrayAccess::compile_load integer constant expected");
			}
		}
		else {
			res += cu.code().compile_push();
			if (is_direct_) {
				res += base()->compile_adr(cu);
  			res += cu.code().compile_push();
			}
			else {
				res += base()->compile(cu, true);
			}
			res += index()->compile(cu, false);
			res += cu.code().compile_mul_opt(type()->size());
			res += cu.code().compile_add();
			res += cu.code().compile_store();
		}
		break;

	case 2:
		res += cu.code().compile_push();
		res += cu.code().compile_xd();
		res += cu.code().compile_push();
		res += compile_adr(cu);
		res += cu.code().compile_store();
		res += cu.code().compile_inc();
		res += cu.code().compile_store();
		break;

	case 4:
  	res += compile_adr(cu);
    res += cu.code().compile_push_opt();
	  cu.code().manager()->add_call("^store_longlong", cu.code().size());
	  res += cu.code().compile_call(0);
		break;

	default:
		throw not_implemented_exception("ArrayAccess::compile_load");
	}
	return res;


	//int res = 0;
	//int value;
	//if (index()->is_const()) {
	//	if (index()->get_int(value)) {
	//		if (value == 0) {
	//			if (is_direct_) {
	//				res += base()->compile_store(cu);
	//			}
	//			else {
	//				res += cu.code().compile_push();
	//				res += base()->compile(cu);
	//				res += cu.code().compile_store();
	//			}
	//		}
	//		//else if (value > 0) {
	//		//	//res += cu.code().compile_push();
	//		//	//res += base()->compile_adr(cu, static_cast<size_t>(value));
	//		//	//res += base()->compile_store(cu);
	//		//	throw not_implemented_exception("Local::compile_store");
	//		//}
	//		else {
	//			res += cu.code().compile_push();
	//			res += base()->compile_adr(cu);
	//			res += cu.code().compile_push();
	//			res += index()->compile(cu);
	//			res += cu.code().compile_mul_opt(type()->size());
	//			res += cu.code().compile_add();
	//			res += cu.code().compile_store();
	//		}
	//	}
	//	else {
	//		throw analyzer_exception("ArrayAccess::compile_store integer constant expected");
	//	}
	//}
	//else {
	//	res += cu.code().compile_push();
	//	res += base()->compile_adr(cu);
	//	res += cu.code().compile_push();
	//	res += index()->compile(cu);
	//	res += cu.code().compile_mul_opt(type()->size());
	//	res += cu.code().compile_add();
	//	res += cu.code().compile_store();
	//}
	//return res;
}

int ArrayAccess::compile_load_word(CompilationUnit& cu, size_t offset, bool push_result)
{
	throw not_implemented_exception("ArrayAccess::compile_load_word");
	return 0;
}

int ArrayAccess::compile_store_word(CompilationUnit& cu, size_t offset)
{
	throw not_implemented_exception("ArrayAccess::compile_store_word");
	return 0;
}

int ArrayAccess::compile_adr(CompilationUnit& cu, size_t offset)
{
	int res = 0;
	int value;
	if (index()->is_const()) {
		if (index()->get_int(value)) {
			value *= type()->size();
			if (is_direct_) {
				res += base()->compile_adr(cu, value);
			}
			else {
				res += base()->compile(cu, false);
				res += cu.code().compile_add_opt(value);
			}
		}
		else {
			throw analyzer_exception("ArrayAccess::compile_adr integer constant expected");
		}
	}
	else {
		if (is_direct_) {
			res += base()->compile_adr(cu);
  		res += cu.code().compile_push();
		}
		else {
			res += base()->compile(cu, true);
		}
		res += index()->compile(cu, false);
		res += cu.code().compile_mul_opt(type()->size());
		res += cu.code().compile_add();
	}
	return res;

}

int ArrayAccess::compile(CompilationUnit& cu, bool push_result)
{
  return compile_load(cu, push_result);
}


//*************************************************************************************


ArrayIndex::ArrayIndex(Expression* ex)
: is_optimized_(false)
, is_direct_(false)
{
//	type(base->type());
	this->base(ex);
	//build_type();
}

ArrayIndex::ArrayIndex(const context_type::position_type& w, Expression* ex)
: Expression(w)
, is_optimized_(false)
, is_direct_(false)
{
//	type(base->type());
	//build_type();
	this->base(ex);
}

void ArrayIndex::build_type()
{
	if (base_.is_not_null() && !is_optimized_) {
		// after optimization, the type mustr not change
		set_type(base()->type());
	}
}

bool ArrayIndex::is_const() const
{
	if (index_.is_null()) {
		return base()->is_const();
	}
	return false;
}

int ArrayIndex::calculate_offset() const
{
	int res = 0;
	if (!is_const()) {
		throw generator_exception("ArrayIndex::calculate_offset not a constant offset");
	}
	if (index()->get_int(res)) {
		res *= type()->size();
	}
	else {
		throw generator_exception("ArrayIndex::calculate_offset integer constant expected");
	}
	return res;
}

//bool ArrayIndex::is_lvalue() const
//{
//	return true;
//}

void ArrayIndex::base(Expression* val)
{
	if (val) {
		base_ = val;
		build_type();
		is_optimized_ = false;
		ArrayType* arr = dynamic_cast<ArrayType*>(val->type());
		if (arr && (arr->limit() > 0)) {
			is_direct_ = true;
		}
	}
}

void ArrayIndex::optimize(ObjPtr& obj, int mask)
{
	if (!is_optimized_) {
		is_optimized_ = true;
		base_.optimize(mask);
		index_.optimize(mask);
		if (index()->is_const()) {
			ArrayIndex* acc = dynamic_cast<ArrayIndex*>(base());
			if (acc) {
				if (acc->index()->is_const()) {
					int outer;
					int inner;
					if (index()->get_int(inner) && acc->index()->get_int(outer)) {
						int os = acc->type_size();
						int is = type_size();
						outer *= os / is;
						index_ = new IntConst(outer + inner);
						base_ = acc->base();
					}
					is_direct_ = acc->is_direct_;
				}
			}
		}
	}
}

void ArrayIndex::add_index(Expression* index)
{
	if (index_.is_null()) {
		index_ = index;
	}
	else {
		index_ = new BinaryExpr(BIN_ADD, this->index(), index);
	}
}

//Type* ArrayIndex::lvalue_type()
//{
//	return type();
//}

int ArrayIndex::compile_push(CompilationUnit& cu)
{
	throw not_implemented_exception("ArrayIndex::compile_push");
	return 0;
}

int ArrayIndex::compile_pop(CompilationUnit& cu)
{
	throw not_implemented_exception("ArrayIndex::compile_pop");
	return 0;
}

int ArrayIndex::compile_push(CompilationUnit& cu, size_t offset)
{
	throw not_implemented_exception("ArrayIndex::compile_push");
	return 0;
}

int ArrayIndex::compile_pop(CompilationUnit& cu, size_t offset)
{
	throw not_implemented_exception("ArrayIndex::compile_pop");
	return 0;
}

int ArrayIndex::compile_load(CompilationUnit& cu, bool push_result)
{
	throw not_implemented_exception("ArrayIndex::compile_load");
	return 0;
}

int ArrayIndex::compile_store(CompilationUnit& cu)
{
	throw not_implemented_exception("ArrayIndex::compile_store");
	return 0;
	//int res = 0;
	//int value;
	//if (index()->is_const()) {
	//	value = calculate_offset();
	//	if (value == 0) {
	//		res += base()->compile_store(cu);
	//	}
	//	else if (value > 0) {
	//		res += cu.code().compile_push();
	//		res += base()->compile_adr(cu, static_cast<size_t>(value));
	//	}
	//	else {
	//		res += cu.code().compile_push();
	//		res += base()->compile_adr(cu);
	//		res += cu.code().compile_push();
	//		res += index()->compile(cu);
	//		res += cu.code().compile_mul_opt(type()->size());
	//		res += cu.code().compile_add();
	//		res += cu.code().compile_store();
	//	}
	//}
	//else {
	//	res += cu.code().compile_push();
	//	res += base()->compile_adr(cu);
	//	res += cu.code().compile_push();
	//	res += index()->compile(cu);
	//	res += cu.code().compile_mul_opt(type()->size());
	//	res += cu.code().compile_add();
	//	res += cu.code().compile_store();
	//}
	//return res;
}

int ArrayIndex::compile_load_word(CompilationUnit& cu, size_t offset, bool push_result)
{
	throw not_implemented_exception("ArrayIndex::compile_load_word");
	return 0;
}

int ArrayIndex::compile_store_word(CompilationUnit& cu, size_t offset)
{
	throw not_implemented_exception("ArrayIndex::compile_store_word");
	return 0;
}

int ArrayIndex::compile_adr(CompilationUnit& cu, size_t offset)
{
	throw not_implemented_exception("ArrayIndex::compile_adr");
	return 0;
}

int ArrayIndex::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	int value;
	if (index()->is_const()) {
		if (index()->get_int(value)) {
			value *= type()->size();
			if (is_direct_) {
				res += base()->compile_adr(cu, value);
			}
			else {
				res += base()->compile(cu, false);
				res += cu.code().compile_add_opt(value);
			}
		}
		else {
			throw analyzer_exception("ArrayIndex::compile_adr integer constant expected");
		}
	}
	else {
		if (is_direct_) {
			res += base()->compile_adr(cu);
		}
		else {
			res += base()->compile(cu, false);
		}
		res += cu.code().compile_push();
		res += index()->compile(cu, false);
		res += cu.code().compile_mul_opt(type()->size());
		res += cu.code().compile_add();
	}
	return res;
}

//*************************************************************************************


OffsetAccess::OffsetAccess(Expression* base, Type* t, bool deref)
: base_(base)
, deref_(deref)
{
	set_type(t);
}

OffsetAccess::OffsetAccess(const context_type::position_type& w, Expression* base, Type* t, bool deref)
: Expression(w)
, base_(base)
, deref_(deref)
{
	set_type(t);
}

void OffsetAccess::build_type()
{
}

bool OffsetAccess::is_const() const
{
	if (offset_.is_not_null()) {
		return offset()->is_const();
	}
	return false;
}

int OffsetAccess::calculate_offset() const
{
	int res = 0;
	if (!is_const()) {
		throw generator_exception("ArrayIndex::calculate_offset not a constant offset");
	}
	if (!offset()->get_int(res)) {
		throw generator_exception("ArrayIndex::calculate_offset integer constant expected");
	}
	return res;
}

bool OffsetAccess::is_lvalue() const
{
	return true;
}

void OffsetAccess::base(Expression* val)
{
	base_ = val;
	if (val) {
		build_type();
	}
}

void OffsetAccess::optimize(ObjPtr& obj, int mask)
{
	base_.optimize(mask);
	offset_.optimize(mask);
}

void OffsetAccess::add_offset(Expression* off)
{
	if (offset_.is_null()) {
		offset_ = off;
	}
	else {
		offset_ = new BinaryExpr(BIN_ADD, this->offset(), off);
	}
}

Type* OffsetAccess::lvalue_type()
{
	return type();
}

int OffsetAccess::compile_push(CompilationUnit& cu)
{
	throw not_implemented_exception("OffsetAccess::compile_push");
	return 0;
}

int OffsetAccess::compile_pop(CompilationUnit& cu)
{
	throw not_implemented_exception("OffsetAccess::compile_pop");
	return 0;
}

int OffsetAccess::compile_push(CompilationUnit& cu, size_t offset)
{
	throw not_implemented_exception("OffsetAccess::compile_push");
	return 0;
}

int OffsetAccess::compile_pop(CompilationUnit& cu, size_t offset)
{
	throw not_implemented_exception("OffsetAccess::compile_pop");
	return 0;
}

int OffsetAccess::compile_load(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	int value = calculate_offset();
	switch (type()->size()) {
	case 1:
		if (value >= 0) {
//				res += base()->compile_load_word(cu, static_cast<size_t>(value));
			if (deref_) {
				res += base()->compile(cu, false);
				res += cu.code().compile_add_opt(value);
			}
			else {
				res += base()->compile_adr(cu, value);
			}
			res += cu.code().compile_load();
		}
		else {
			res += cu.code().compile_push_opt();
			if (deref_) {
				res += base()->compile(cu, false);
			}
			else {
				res += base()->compile_adr(cu, 0);
			}
			res += cu.code().compile_add_opt(value);
			res += cu.code().compile_load();
		}
    if (push_result) {res += cu.code().compile_push_opt();}
		break;

	case 2:
		if (deref_) {
  		res += base()->compile(cu, false);
  		if (value > 0) {
				res += cu.code().compile_add_opt(value);
      }
      else if (value < 0) {
    		throw not_implemented_exception("OffsetAccess::compile_load 2");
      }
      if (push_result) {
			  res += cu.code().compile_stt();
			  res += cu.code().compile_inc();
			  res += cu.code().compile_load();
			  res += cu.code().compile_push();
			  res += cu.code().compile_ldt();
			  res += cu.code().compile_load();
			  res += cu.code().compile_push();
      }
      else {
			  res += cu.code().compile_stt();
			  res += cu.code().compile_load();
			  res += cu.code().compile_xd();
			  res += cu.code().compile_ldt();
			  res += cu.code().compile_inc();
			  res += cu.code().compile_load();
      }
    }
    else {
      if (push_result) {
	    	res += base()->compile_load_word(cu, value+1, true);
				res += base()->compile_load_word(cu, value, true);
      }
      else {
	    	res += base()->compile_load_word(cu, value+1, false);
 			  res += cu.code().compile_xd();
				res += base()->compile_load_word(cu, value, false);
      }
    }
		break;

  case 4:
		if (deref_) {
  		res += base()->compile(cu, false);
  		if (value > 0) {
				res += cu.code().compile_add_opt(value+4);
      }
      else if (value < 0) {
    		throw not_implemented_exception("OffsetAccess::compile_load 2");
      }
 			res += cu.code().compile_push();
    }
    else {
			res += base()->compile_adr(cu, value);
 			res += cu.code().compile_push();
    }
	  cu.code().manager()->add_call("^store_longlong", cu.code().size());
	  res += cu.code().compile_call(0);
		break;

  default:
  	throw not_implemented_exception("OffsetAccess::compile_load");
	}
	return res;
	throw not_implemented_exception("OffsetAccess::compile_load");
	return 0;
}

int OffsetAccess::compile_store(CompilationUnit& cu)
{
	int res = 0;
	int value = calculate_offset();
	switch (type()->size()) {
	case 1:
    if (deref_) {
			res += cu.code().compile_push_opt();
			res += base()->compile(cu, false);
      if (value > 0) {
  			res += cu.code().compile_store_offset(static_cast<size_t>(value));
      }
      else {
  			res += cu.code().compile_store();
      }
    }
		else if (value >= 0) {
			res += base()->compile_store_word(cu, static_cast<size_t>(value));
		}
		else {
			res += cu.code().compile_push_opt();
			res += base()->compile_adr(cu, 0);
			res += cu.code().compile_add_opt(value);
			res += cu.code().compile_store();
		}
		break;

	case 2:
		res += cu.code().compile_push_long_opt(false);
    if (deref_) {
			res += base()->compile(cu, false);
      if (value != 0) {
				res += cu.code().compile_add_opt(value);
      }
    }
		else if (value > 0) {
			res += base()->compile_adr(cu, value);
		}
		else {
			res += base()->compile_adr(cu, 0);
			res += cu.code().compile_add_opt(value);
		}
		res += cu.code().compile_store();
		res += cu.code().compile_inc();
		res += cu.code().compile_store();
		break;

	case 4:
		if (deref_) {
  		res += base()->compile(cu, false);
  		if (value > 0) {
				res += cu.code().compile_add_opt(value+4);
      }
      else if (value < 0) {
    		throw not_implemented_exception("OffsetAccess::compile_load 2");
      }
			res += cu.code().compile_store();
			res += cu.code().compile_inc();
			res += cu.code().compile_store();
			res += cu.code().compile_inc();
			res += cu.code().compile_store();
			res += cu.code().compile_inc();
			res += cu.code().compile_store();
    }
    else {
			res += base()->compile_store_word(cu, value);
			res += base()->compile_store_word(cu, value+1);
			res += base()->compile_store_word(cu, value+2);
			res += base()->compile_store_word(cu, value+3);
    }
		break;

	default: {
			throw not_implemented_exception("OffsetAccess::compile_store");
		}
		break;
	}
	return res;
}

int OffsetAccess::compile_load_word(CompilationUnit& cu, size_t offset, bool push_result)
{
	throw not_implemented_exception("OffsetAccess::compile_load_word");
	return 0;
}

int OffsetAccess::compile_store_word(CompilationUnit& cu, size_t offset)
{
	int res = 0;
	res += base()->compile(cu, 0);
	res += cu.code().compile_store_offset(offset);
	return res;
}

int OffsetAccess::compile_adr(CompilationUnit& cu, size_t off)
{
	int res = 0;
	int value;
	if (offset()->is_const()) {
		if (offset()->get_int(value)) {
			value += off;
			if (value >= 0) {
				res += base()->compile_adr(cu, value);
			}
			else {
				base()->compile_adr(cu);
				res += cu.code().compile_add_opt(value);
			}
			return res;
		}
		else {
			throw analyzer_exception("ArrayIndex::compile_adr integer constant expected");
		}
	}
	res += base()->compile_adr(cu, off);
	res += cu.code().compile_push_opt();
	res += offset()->compile(cu, false);
	res += cu.code().compile_add();
	return res;
}

int OffsetAccess::compile(CompilationUnit& cu, bool push_result)
{
  return compile_load(cu, push_result);
}

//*************************************************************************************


Initializer::Initializer()
{
}

Initializer::Initializer(const context_type::position_type& w)
: Expression(w)
{
}

void Initializer::build_type()
{
	if (list_.empty()) {
		set_type(Type::basic(T_VOID));
	}
	else {
    Type* t = new ArrayType(T_INITIALIZER, child(0)->type(), list_.size());
    ObjPtr ptr(t);
		set_type(t);
	}
}

bool Initializer::is_const() const
{
	size_t limit = size();
	for (size_t i=0; i<limit; ++i) {
		if (!child(i)->is_const()) {return false;}
	}
  return true;
}

size_t Initializer::initializer_depth() const
{
	size_t res = 0;
	size_t limit = size();
	for (size_t i=0; i<limit; ++i) {
		size_t new_depth = child(i)->initializer_depth();
		if (new_depth > res) {res = new_depth;}
	}
	return res+1;
}

int Initializer::compile(CompilationUnit& cu, bool push_result)
{
	//size_t limit = list_.size();
	//for (size_t i=0; i<limit; ++i) {
	//	list_.at(i).compile(code);
	//}
	return 0;
}

void Initializer::add(Expression* ex)
{
	list_.push_back(ex);
}

void Initializer::compile_data(CompilationUnit& cu)
{
	size_t limit = list_.size();
	for (size_t i=0; i<limit; ++i) {
		child(i)->compile_data(cu);
	}
}


}
