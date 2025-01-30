#include "cvmc.hpp"

namespace cvmc {

#define	PRINT

Parser::Parser(ContextHelper& hlp)
: RefObj(OT_PARSER)
, helper_(hlp)
, context_(0)
, optimize_(true)
, const_folding_(true)
, is_typedef_(false)
, block_level_(0)
, storage_class_mask_(0)
, type_qualifier_mask_(0)
, function_specifier_mask_(0)
, alignment_specifier_mask_(0)
{
}

void Parser::next()
{
	helper_.next();
	helper_.filter();
}

void Parser::error(const std::string& txt)
{
//	errors_.push_back(Issue(helper_.current().get_position(), txt));
	IssueList::error(helper_.location(), txt);
}

void Parser::warning(const std::string& txt)
{
//	warnings_.push_back(Issue(helper_.current().get_position(), txt));
	IssueList::warning(helper_.location(), txt);
}

XRefManager* Parser::manager()
{
	return static_cast<XRefManager*>(manager_.get());
}

void Parser::end_manager()
{
	manager_.clear();
}

void Parser::end_name()
{
	name_.clear();
  //helper_.remove_identifier();
}

bool Parser::parse_number_constant(const char*& warning, const std::string& txt, ObjPtr& e, bool with_sign)
{
  warning = 0;
	char ch;
	bool valid = true;
  bool is_neg = false;
  bool is_unsigned = false;
  bool is_long = false;
  bool is_longlong = false;
	unsigned long long val = 0;
	if (txt.empty()) {return false;}
	size_t size = txt.size();
	size_t pos = 0;
  if (with_sign) {
    ch = txt[0];
    if (ch == '-') {
      is_neg = true;
      ++pos;
    }
    else if (ch == '+') {
      ++pos;
    }
  }
	if (txt[pos] == '0') {
    ++pos;
		if (size <= pos) {
			IntConst* ex = new IntConst(0);
			ex->set_type(new Type(T_INT));
			e = ex;
			return true;
		}
		if ((txt[pos] == 'x') || (txt[pos] == 'X')) {
			if (size <= (pos+1)) {return false;}
			// hex
			while (++pos < size) {
				ch = txt[pos];
				if (ch != '_') {
					if ((ch >= '0') && (ch <= '9')) {val *= 16; val += ch - '0';}
					else if ((ch >= 'a') && (ch <= 'f')) {val *= 16; val += ch + 10 - 'a';}
					else if ((ch >= 'A') && (ch <= 'F')) {val *= 16; val += ch + 10 - 'A';}
					else if ((ch == 'u') || (ch == 'U') || (ch == 'l') || (ch == 'L')) {
            break;
          }
					else {return false;}
				}
			}
		}
		else if ((txt[pos] == 'b') || (txt[pos] == 'B')) {
			if (size <= (pos+1)) {return false;}
			// binary
			while (++pos < size) {
				ch = txt[pos];
				if (ch != '_') {
					if ((ch >= '0') && (ch <= '1')) {val *= 2; val += ch - '0';}
				  else if ((ch == 'u') || (ch == 'U') || (ch == 'l') || (ch == 'L')) {break;}
					else {return false;}
				}
			}
		}
		else {
			// octal
			while (pos < size) {
				ch = txt[pos++];
				if (ch != '_') {
				  if ((ch >= '0') && (ch <= '7')) {val *= 8; val += ch - '0';}
				  else if ((ch == 'u') || (ch == 'U') || (ch == 'l') || (ch == 'L')) {--pos; break;}
          else {return false;}
				}
			}
		}
	}
	else {
		// decimal
		while (pos < size) {
			ch = txt[pos++];
			if (ch != '_') {
				if ((ch >= '0') && (ch <= '9')) {val *= 10; val += ch - '0';}
				else if ((ch == 'u') || (ch == 'U') || (ch == 'l') || (ch == 'L')) {--pos; break;}
        else {return false;}
			}
		}
	}
	if (valid) {
		if (pos < size) {
			ch = txt[pos];
			if ((ch == 'u') || (ch == 'U')) {
        is_unsigned = true;
        ++pos;
      }
    }
		if (pos < size) {
			ch = txt[pos];
			if ((ch == 'l') || (ch == 'L')) {
        is_long = true;
        ++pos;
      }
		  if (pos < size) {
			  ch = txt[pos];
			  if ((ch == 'l') || (ch == 'L')) {
          is_longlong = true;
          ++pos;
        }
      }
    }
	  if (pos < size) {return false;}
    if (is_unsigned && is_neg) {return false;}
    if (is_unsigned) {
      if (is_longlong) {
			  ULongLongConst* ex = new ULongLongConst(val);
			  ex->set_type(new Type(T_ULONGLONG));
			  e = ex;
      }
      else if (is_long) {
    		if (val > 0xffffffffUL) {
    			warning = "Parser::parse_number_constant constant too big for unsigned long";
    		}
			  ULongConst* ex = new ULongConst(static_cast<unsigned long>(val));
			  ex->set_type(new Type(T_ULONG));
			  e = ex;
      }
      else {
    		if (val > 0xffffU) {
    			warning = "Parser::parse_number_constant constant too big for unsigned int";
    		}
			  UIntConst* ex = new UIntConst(static_cast<unsigned int>(val));
			  ex->set_type(new Type(T_UINT));
			  e = ex;
      }
    }
    else if (is_neg) {
      if (is_longlong) {
    		if (val > 0x8000000000000000ULL) {
    			warning = "Parser::parse_number_constant constant too low for long long";
    		}
			  LongLongConst* ex = new LongLongConst(-static_cast<long long>(val));
			  ex->set_type(new Type(T_LONGLONG));
			  e = ex;
      }
      else if (is_long) {
    		if (val > 0x80000000UL) {
    			warning = "Parser::parse_number_constant constant too low for long";
    		}
			  LongConst* ex = new LongConst(-static_cast<long>(val));
			  ex->set_type(new Type(T_LONG));
			  e = ex;
      }
      else {
    		if (val > 0x8000U) {
    			warning = "Parser::parse_number_constant constant too low for int";
    		}
			  IntConst* ex = new IntConst(-static_cast<int>(val));
			  ex->set_type(new Type(T_INT));
			  e = ex;
      }
    }
    else {
      if (is_longlong) {
    		if (val >= 0x8000000000000000ULL) {
    			warning = "Parser::parse_number_constant constant too high for long long";
    		}
			  LongLongConst* ex = new LongLongConst(static_cast<long long>(val));
			  ex->set_type(new Type(T_LONGLONG));
			  e = ex;
      }
      else if (is_long) {
    		if (val >= 0x80000000UL) {
    			warning = "Parser::parse_number_constant constant too high for long";
    		}
			  LongConst* ex = new LongConst(static_cast<long>(val));
			  ex->set_type(new Type(T_LONG));
			  e = ex;
      }
      else {
    		if (val >= 0x8000U) {
    			warning = "Parser::parse_number_constant constant too high for int";
    		}
			  IntConst* ex = new IntConst(static_cast<int>(val));
			  ex->set_type(new Type(T_INT));
			  e = ex;
      }
    }
    return true;
		//if (!is_neg && (val <= 0x7fff)) {
		//	IntConst* ex = new IntConst(static_cast<int>(val));
		//	ex->set_type(new Type(T_INT));
		//	e = ex;
		//	return true;
		//}
		//if (is_neg && (val <= 0x8000)) {
		//	IntConst* ex = new IntConst(-static_cast<int>(val));
		//	ex->set_type(new Type(T_INT));
		//	e = ex;
		//	return true;
  //  }
		//if (!is_neg && (val <= 0xffffU)) {
		//	UIntConst* ex = new UIntConst(static_cast<unsigned int>(val));
		//	ex->set_type(new Type(T_UINT));
		//	e = ex;
		//	return true;
		//}
		//if (!is_neg && (val <= 0x7fffffffL)) {
		//	LongConst* ex = new LongConst(static_cast<long>(val));
		//	ex->set_type(new Type(T_LONG));
		//	e = ex;
		//	return true;
		//}
		//if (is_neg && (val <= 0x80000000L)) {
		//	LongConst* ex = new LongConst(-static_cast<long>(val));
		//	ex->set_type(new Type(T_LONG));
		//	e = ex;
		//	return true;
		//}
		//if (!is_neg && (val <= 0xffffffffUL)) {
		//	ULongConst* ex = new ULongConst(static_cast<unsigned long>(val));
		//	ex->set_type(new Type(T_ULONG));
		//	e = ex;
		//	return true;
		//}
		//if (!is_neg && (val <= 0x7fffffffffffffffULL)) {
		//	LongLongConst* ex = new LongLongConst(static_cast<long long>(val));
		//	ex->set_type(new Type(T_LONGLONG));
		//	e = ex;
		//	return true;
		//}
		//if (is_neg && (val <= 0x8000000000000000ULL)) {
		//	LongLongConst* ex = new LongLongConst(-static_cast<long long>(val));
		//	ex->set_type(new Type(T_LONGLONG));
		//	e = ex;
		//	return true;
		//}
		//if (!is_neg && (val <= 0xffffffffffffffffULL)) {
		//	ULongLongConst* ex = new ULongLongConst(static_cast<unsigned long long>(val));
		//	ex->set_type(new Type(T_ULONGLONG));
		//	e = ex;
		//	return true;
		//}
	}
	return false;
}

//primary_expression
//	: IDENTIFIER	 // may be an enumeration constant, a variable or a function name
//	| '__asm' '(' string ')'
//	| I_CONSTANT
//	| C_CONSTANT
//	| F_CONSTANT
//	| string { string }
//	| '(' expression ')'
//	| generic_selection
//	;
bool Parser::parse_primary_expression(Function* owner, ObjPtr& e, ObjPtr& funcall_type)
{
	std::string val;
	if (helper_.is_left_paren()) {
		size_t pos = helper_.mark();
		next();
		if (!parse_expression(owner, e)) {
			// maybe we have cast operation
			helper_.go_back_to(pos);
			//error("Parser::parse_primary_expression expression expected");
			return false;
		}
		if (!helper_.is_right_paren()) {
			error("Parser::parse_primary_expression ) expected");
			return false;
		}
		next();
		return true;
	}
	switch(helper_.current_token_id()) {
	case boost::wave::T_MSEXT_ASM: {
		  // assembler instruction
		  next();
		  if (!helper_.is_left_paren()) {
			  error("Parser::parse_primary_expression ( expected");
			  return false;
		  }
		  next();
		  Assembler* ass = new Assembler(helper_.location());
		  e = ass;
		  if (helper_.current_token_id() != boost::wave::T_STRINGLIT) {
			  error("Parser::parse_primary_expression assembler target expected");
			  return false;
		  }
		  val = helper_.current().get_value().c_str();
		  ass->target(val.substr(1, val.size()-2));
		  next();
		  if (helper_.current_token_id() != boost::wave::T_COMMA) {
			  error("Parser::parse_primary_expression , expected");
			  return false;
		  }
		  next();
		  if (helper_.current_token_id() != boost::wave::T_STRINGLIT) {
			  error("Parser::parse_primary_expression assembler code expected");
			  return false;
		  }
		  while (helper_.current_token_id() == boost::wave::T_STRINGLIT) {
			  val = helper_.current().get_value().c_str();
			  ass->source() += val.substr(1, val.size()-2);
			  next();
		  }
		  if (!helper_.is_right_paren()) {
			  error("Parser::parse_primary_expression ) expected");
			  return false;
		  }
		  next();
		  return true;
	  }
	  break;

	case boost::wave::T_SIZEOF: {
			throw not_implemented_exception("Parser::parse_primary_expression sizeof");
    }
	  break;

	case boost::wave::T_IDENTIFIER: {
      val = helper_.identifier();
		  if (val.find("__sys__") == 0) {
			  // we have a call to a system function
        val.replace(0, 7, "^");
			  //val = "^";
			  //val += helper_.identifier().substr(7);
			  Function* func = Function::system(val);
			  if (func == 0) {
				  error("Parser::parse_primary_expression cannot find system function "+val);
				  return false;
			  }
			  // identifier is a system function call
			  next();
			  funcall_type = func;
			  return true;
		  }
		  else {
			  if (owner) {
				  Local* loc = owner->lookup_local(val);
				  if (loc) {
					  e = loc;
					  next();
					  return true;
				  }
				  Parameter* par = owner->lookup_parameter(val);
				  if (par) {
					  e = par;
					  next();
					  return true;
				  }
				  Label* lbl = owner->lookup_label(val);
				  if (lbl) {
					  e = lbl;
					  next();
					  return true;
				  }
				  if (func_name() == val) {
					  // recursive function name
					  next();
					  funcall_type = owner;
					  return true;
				  }
			  }
			  ObjPtr obj = context_->lookup(val, true);
			  if (obj.is_not_null()) {
				  Static* var = dynamic_cast<Static*>(obj.get());
				  if (var) {
					  // identifier is a variable
					  e = var;
					  next();
					  return true;
				  }
				  Function* func = dynamic_cast<Function*>(obj.get());
				  if (func) {
					  // identifier is a function call
					  next();
					  funcall_type = func;
					  return true;
				  }
				  Constant* cons = dynamic_cast<Constant*>(obj.get());
				  if (cons) {
					  e = cons;
					  next();
					  return true;
				  }
				  return false;
			  }
		  }
	  };
	  break;

  case boost::wave::T_LONGINTLIT:
	case boost::wave::T_INTLIT: {
		  std::string val = helper_.current().get_value().c_str();
      const char* warn;
		  if (!parse_number_constant(warn, val, e)) {
			  return false;
		  }
      if (warn) {
        warning(warn);
      }
		  //IntConst* ex = new IntConst(helper_.location(), boost::lexical_cast<int>(val));
		  //ex->type(new Type(T_INT));
		  //e = ex;
		  next();
	  }
	  return true;

	case boost::wave::T_STRINGLIT: {
		  std::string val = helper_.current().get_value().c_str();
		  StringConst* ex = new StringConst(helper_.location(), val.substr(1, val.size()-2));
		  ex->set_type(new ReferenceType(T_POINTER, new Type(T_CHAR)));
		  e = ex;
		  next();
		  while (helper_.current_token_id() == boost::wave::T_STRINGLIT) {
			  ex->value(ex->value() + helper_.current().get_value().c_str());
			  next();
		  }
	  }
	  return true;

	case boost::wave::T_CHARLIT: {
		  std::string val = helper_.current().get_value().c_str();
		  if (val.size() == 3) {
			  IntConst* ex = new IntConst(helper_.location(), val[1]);
			  ex->set_type(new Type(T_INT));
			  e = ex;
			  next();
		  }
		  else {
			  throw not_implemented_exception("Parser::parse_primary_expression invalid character literal");
		  }
	  }
	  return true;

	case boost::wave::T_TRUE: {
		  BoolConst* ex = new BoolConst(helper_.location(), true);
		  ex->set_type(new Type(T_INT));
		  e = ex;
		  next();
	  }
	  return true;

	case boost::wave::T_FALSE: {
		  BoolConst* ex = new BoolConst(helper_.location(), false);
		  ex->set_type(new Type(T_INT));
		  e = ex;
		  next();
	  }
	  return true;

	default:;
	}
	return false;
}

//string
//	: STRING_LITERAL
//	| FUNC_NAME
//	;
bool Parser::parse_string(ObjPtr& e)
{
	return false;
}

//generic_selection
//	: GENERIC '(' assignment_expression ',' generic_assoc_list ')'
//	;
bool Parser::parse_generic_selection(Function* owner, ObjPtr& e)
{
	return false;
}

//generic_assoc_list
//	: generic_association
//	| generic_assoc_list ',' generic_association
//	;
bool Parser::parse_generic_assoc_list(Function* owner, ObjPtr& e)
{
	return false;
}

//generic_association
//	: type_name ':' assignment_expression
//	| DEFAULT ':' assignment_expression
//	;
bool Parser::parse_generic_association(Function* owner, ObjPtr& e)
{
	return false;
}

//postfix_expression
//	: primary_expression
//	| postfix_expression '[' expression ']'
//	| postfix_expression '(' ')'
//	| postfix_expression '(' argument_expression_list ')'
//	| postfix_expression '.' IDENTIFIER
//	| postfix_expression PTR_OP IDENTIFIER
//	| postfix_expression INC_OP
//	| postfix_expression DEC_OP
//	| '(' type_name ')' '{' initializer_list '}'
//	| '(' type_name ')' '{' initializer_list ',' '}'
//	;
bool Parser::parse_postfix_expression_body(Function* owner, ObjPtr& e, ObjPtr& funcall_type)
{
	for (;;) {
		if (helper_.is_left_bracket()) { // [ = array access
			next();
			ObjPtr ex;
			if (!parse_expression(owner, ex)) {
				next();
				error("Parser::parse_postfix_expression_body array index is not an expression");
				return false;
			}
			if (!helper_.is_right_bracket()) {
				error("Parser::parse_postfix_expression_body ] expected");
				return false;
			}
			next();
			ArrayAccess* acc = new ArrayAccess(helper_.location(), e.as_expression());
			//e.as_expression()->append(e, *acc);
			e = acc;
			acc->add_index(ex.as_expression());
		}
		else if (helper_.is_left_paren()) { // ( = function call
			Function* f = funcall_type.as_function();
			if (f == 0) {
				next();
				error("Parser::parse_postfix_expression_body not a function");
				return false;
			}
			next();
			Call* call = new Call(helper_.location(), f, f->name());
			e = call;
			if (helper_.is_right_paren()) {
				// no arguments
				next();
				return true;
			}
			if (!parse_argument_expression_list(owner, *call)) {
				error("Parser::parse_postfix_expression_body argument expression list expected");
				return false;
			}
			if (!helper_.is_right_paren()) {
				error(")");
				return false;
			}
			//func()->add_reference(name, call->func());
			next();
		}
		else if (helper_.current_token_id() == boost::wave::T_DOT) {
			next();
			if (helper_.current_token_id() != boost::wave::T_IDENTIFIER) {
				error("Parser::parse_postfix_expression_body . identifier expected");
				return false;
			}
			UnionStructType* base_type = dynamic_cast<UnionStructType*>(e.as_expression()->type());
			if (base_type == 0) {
				error("Parser::parse_postfix_expression_body . base type is not a union or struct");
				return false;
			}
			// now we must take the address of the struct or union
			//UnaryExpr* ex = new UnaryExpr(helper_.location(), UN_REF);
			//ex->arg1(e.as_expression());
			//e = ex;
			// finally we can access the member
			UnionStructMember* member = base_type->member(helper_.identifier());
			if (member == 0) {
				error("Parser::parse_postfix_expression_body . struct or union does not have member "+helper_.identifier());
				return false;
			}
			OffsetAccess* acc = new OffsetAccess(helper_.location(), e.as_expression(), member->type(), false);
			e = acc;
			acc->add_offset(member);
			next();
			//throw not_implemented_exception("Parser::parse_postfix_expression_body .");
		}
		else if (helper_.current_token_id() == boost::wave::T_ARROW) {
			next();
			if (helper_.current_token_id() != boost::wave::T_IDENTIFIER) {
				error("Parser::parse_postfix_expression_body -> identifier expected");
				return false;
			}
			ReferenceType* ref_type = dynamic_cast<ReferenceType*>(e.as_expression()->type());
			if (ref_type == 0) {
				error("Parser::parse_postfix_expression_body -> base type is not a union or struct");
				return false;
			}
			//ArrayAccess* arr_acc = new ArrayAccess(helper_.location(), e.as_expression());
			//arr_acc->add_index(new IntConst(0));
			UnionStructType* base_type = dynamic_cast<UnionStructType*>(ref_type->base());
			if (base_type == 0) {
				error("Parser::parse_postfix_expression_body -> base type is not a union or struct");
				return false;
			}
			UnionStructMember* member = base_type->member(helper_.identifier());
			if (member == 0) {
				error("Parser::parse_postfix_expression_body -> struct or union does not have member "+helper_.identifier());
				return false;
			}
			OffsetAccess* acc = new OffsetAccess(helper_.location(), e.as_expression(), member->type(), true);
			e = acc;
			acc->add_offset(member);
			next();
			//throw not_implemented_exception("Parser::parse_postfix_expression_body ->");
		}
		else if (helper_.current_token_id() == boost::wave::T_PLUSPLUS) {
			next();
			UnaryExpr* ex = new UnaryExpr(helper_.location(), UN_POSTINC);
			ex->arg1(e.as_expression());
			e = ex;
		}
		else if (helper_.current_token_id() == boost::wave::T_MINUSMINUS) {
			next();
			UnaryExpr* ex = new UnaryExpr(helper_.location(), UN_POSTDEC);
			ex->arg1(e.as_expression());
			e = ex;
		}
		else {
			return true;
		}
	}
}

bool Parser::parse_postfix_expression(Function* owner, ObjPtr& e)
{
	ObjPtr funcall_type;
	if (parse_primary_expression(owner, e, funcall_type)) {
		// a postfix expression can start with a primary expression
		return parse_postfix_expression_body(owner, e, funcall_type);
	}
	if (helper_.is_left_paren()) {
		// or it can start with a casted initializer list
		//throw not_implemented_exception("Parser::parse_postfix_expression casted initializer list");
	}
	return false;
}

//argument_expression_list
//	: assignment_expression
//	| argument_expression_list ',' assignment_expression
//	;
bool Parser::parse_argument_expression_list(Function* owner, Call& c)
{
	ObjPtr e;
	if (parse_assignment_expression(owner, e)) {
		Function* f = c.func();
		size_t pos = 0;
		Type* t = f->parameter(pos++)->type();
		c.add(new AutoCast(helper_.location(), t, e.as_expression()));
		e.clear();
		while (helper_.current_token_id() == boost::wave::T_COMMA) {
			next();
			if (!parse_assignment_expression(owner, e)) {
				error("Parser::parse_argument_expression_list assignment expression expected");
				return false;
			}
			t = f->parameter(pos++)->type();
			if (t->is_equivalent_with(*e.as_expression()->type())) {
				c.add(e.as_expression());
			}
			else {
				c.add(new AutoCast(helper_.location(), t, e.as_expression()));
			}
			//end_expr();
		}
		return true;
	}
	return false;
}

//unary_expression
//	: postfix_expression
//	| INC_OP unary_expression
//	| DEC_OP unary_expression
//	| unary_operator cast_expression
//	| SIZEOF unary_expression
//	| SIZEOF '(' type_name ')'
//	| ALIGNOF '(' type_name ')'
//	;
bool Parser::parse_unary_expression(Function* owner, ObjPtr& e)
{
	UnOp op = parse_unary_operator();
	if (op != UN_UNDEFINED) {
		next();
		if (!parse_cast_expression(owner, e)) {
      parse_cast_expression(owner, e);
			error("Parser::parse_unary_expression cast expression expected");
			return false;
		}
		UnaryExpr* ex = new UnaryExpr(helper_.location(), op);
		ex->arg1(e.as_expression());
		e = ex;
		return true;
	}
	switch(helper_.current_token_id()) {
		case boost::wave::T_PLUSPLUS: {
				next();
				if (!parse_unary_expression(owner, e)) {
					error("Parser::parse_unary_expression unary expression expected");
					return false;
				}
				UnaryExpr* ex = new UnaryExpr(helper_.location(), UN_PREINC);
				ex->arg1(e.as_expression());
				e = ex;
			}
			return true;

		case boost::wave::T_MINUSMINUS: {
				next();
				if (!parse_unary_expression(owner, e)) {
					error("Parser::parse_unary_expression unary expression expected");
					return false;
				}
				UnaryExpr* ex = new UnaryExpr(helper_.location(), UN_PREDEC);
				ex->arg1(e.as_expression());
				e = ex;
			}
			return true;

		case boost::wave::T_SIZEOF: {
  		  next();
    	  if (!helper_.is_left_paren()) {
					error("Parser::parse_unary_expression ( expected");
					return false;
				}
		    next();
    		size_t pos = helper_.mark();
		    ObjPtr t;
        size_t s;
		    if (parse_type_name(owner, t)) {
          s = t.as_type()->size();
        }
        else {
			    helper_.go_back_to(pos);
          t.clear();
			    if (!parse_postfix_expression(owner, t)) {
					  error("Parser::parse_unary_expression invalid sizeof parameter");
            return false;
          }
          s = t.as_expression()->type()->size();
		    }
    	  if (!helper_.is_right_paren()) {
					error("Parser::parse_unary_expression ) expected");
					return false;
				}
		    next();
				//UnaryExpr* ex = new UnaryExpr(helper_.location(), UN_SIZEOF);
				//ex->arg1(t.as_expression());
				//e = ex;
        e = new IntConst(helper_.location(), s);
      }
			return true;

		case boost::wave::T_ALIGNOF:
	    throw not_implemented_exception("Parser::parse_unary_expression alignof");
      op = UN_ALIGNOF;
			return false;

		default:;
	}
	if (parse_postfix_expression(owner, e)) {
		return true;
	}
	return false;
}

//unary_operator
//	: '&'
//	| '*'
//	| '+'
//	| '-'
//	| '~'
//	| '!'
//	;
UnOp Parser::parse_unary_operator()
{
	switch(helper_.current_token_id()) {
		case boost::wave::T_AND: return UN_REF;
		case boost::wave::T_STAR: return UN_DEREF;
		case boost::wave::T_PLUS: return UN_PLUS;
		case boost::wave::T_MINUS: return UN_NEGATE;
		case boost::wave::T_NOT: return UN_NOT;
		case boost::wave::T_COMPL: return UN_BITNOT;
		default:;
	}
	return UN_UNDEFINED;
}

//cast_expression
//	: unary_expression
//	| '(' type_name ')' cast_expression
//	;
bool Parser::parse_cast_expression(Function* owner, ObjPtr& e)
{
	if (helper_.is_left_paren()) {
		size_t pos = helper_.mark();
		next();
		ObjPtr t;
		if (!parse_type_name(owner, t)) {
			helper_.go_back_to(pos);
			return parse_unary_expression(owner, e);
		}
		Cast* cast = new Cast(helper_.location());
		ObjPtr obj(cast);
		cast->to(t.as_type());
		//end_type();
		if (!helper_.is_right_paren()) {
			error("Parser::parse_cast_expression ) expected");
			return false;
		}
		next();
		if (!parse_cast_expression(owner, e)) {
			helper_.go_back_to(pos);
			return false;
		}
		cast->from(e.as_expression());
		e = cast;
		return true;
	}
	return parse_unary_expression(owner, e);
}

//multiplicative_expression
//	: cast_expression
//	| multiplicative_expression '*' cast_expression
//	| multiplicative_expression '/' cast_expression
//	| multiplicative_expression '%' cast_expression
//	;
bool Parser::parse_multiplicative_expression(Function* owner, ObjPtr& e)
{
	ObjPtr ex;
	BinOp op = BIN_UNDEFINED;
	while (parse_cast_expression(owner, e)) {
		if (ex.is_not_null()) {
			BinaryExpr* binop = new BinaryExpr(helper_.location(), op);
			binop->arg1(ex.as_expression());
			binop->arg2(e.as_expression());
			e = binop;
		}
		ex = e.as_expression();
		boost::wave::token_id id = helper_.current_token_id();
		if (id == boost::wave::T_STAR) {op = BIN_MUL;}
		else if (id == boost::wave::T_DIVIDE) {op = BIN_DIV;}
		else if (id == boost::wave::T_PERCENT) {op = BIN_MOD;}
		else {return true;}
		next();
	}
	if (ex.is_not_null()) {
		error("Parser::parse_multiplicative_expression cast expression expected");
	}
	return false;
}

//additive_expression
//	: multiplicative_expression
//	| additive_expression '+' multiplicative_expression
//	| additive_expression '-' multiplicative_expression
//	;
bool Parser::parse_additive_expression(Function* owner, ObjPtr& e)
{
	ObjPtr ex;
	BinOp op = BIN_UNDEFINED;
	while (parse_multiplicative_expression(owner, e)) {
		if (ex.is_not_null()) {
			// special handling must cover the case where something is added to a pointer or array
			bool array_index = false;
			ArrayIndex* acc = dynamic_cast<ArrayIndex*>(e.as_expression());
			if (acc == 0) {
				if (ex.as_expression()->type()->is_array_or_pointer()) {
					acc = new ArrayIndex(helper_.location(), ex.as_expression());
				}
			}
			if (acc != 0) {
				array_index = true;
				acc->add_index(
					op == BIN_ADD
					? e.as_expression()
					: new UnaryExpr(UN_NEGATE, e.as_expression())
				);
				e = acc;
			}
			if (!array_index) {
				BinaryExpr* binop = new BinaryExpr(helper_.location(), op);
				binop->arg1(ex.as_expression());
				binop->arg2(e.as_expression());
				e = binop;
			}
		}
		ex = e.as_expression();
		boost::wave::token_id id = helper_.current_token_id();
		if (id == boost::wave::T_PLUS) {op = BIN_ADD;}
		else if (id == boost::wave::T_MINUS) {op = BIN_SUB;}
		else {return true;}
		next();
	}
	if (ex.is_not_null()) {
		error("Parser::parse_additive_expression multiplicative expression expected");
	}
	return false;
}

//shift_expression
//	: additive_expression
//	| shift_expression LEFT_OP additive_expression
//	| shift_expression RIGHT_OP additive_expression
//	;
bool Parser::parse_shift_expression(Function* owner, ObjPtr& e)
{
	ObjPtr ex;
	BinOp op = BIN_UNDEFINED;
	while (parse_additive_expression(owner, e)) {
		if (ex.is_not_null()) {
			BinaryExpr* binop = new BinaryExpr(helper_.location(), op);
			binop->arg1(ex.as_expression());
			binop->arg2(e.as_expression());
			e = binop;
		}
		ex = e.as_expression();
		boost::wave::token_id id = helper_.current_token_id();
		if (id == boost::wave::T_SHIFTLEFT) {op = BIN_LEFT;}
		else if (id == boost::wave::T_SHIFTRIGHT) {op = BIN_RIGHT;}
		else {return true;}
		next();
	}
	if (ex.is_not_null()) {
		error("Parser::parse_shift_expression additive expression expected");
	}
	return false;
}

//relational_expression
//	: shift_expression
//	| relational_expression '<' shift_expression
//	| relational_expression '>' shift_expression
//	| relational_expression LE_OP shift_expression
//	| relational_expression GE_OP shift_expression
//	;
bool Parser::parse_relational_expression(Function* owner, ObjPtr& e)
{
  bool mismatch_error;
	ObjPtr ex;
	BinOp op = BIN_UNDEFINED;
	while (parse_shift_expression(owner, e)) {
		if (ex.is_not_null()) {
			BinaryExpr* binop = new BinaryExpr(helper_.location(), op);
			binop->arg1(ex.as_expression());
			binop->arg2(e.as_expression());
      mismatch_error = false;
      if (binop->arg1()->type()->is_unsigned() != binop->arg2()->type()->is_unsigned()) {
        mismatch_error = true;
        if (!binop->arg1()->type()->is_unsigned() && binop->arg1()->is_const() && !binop->arg1()->is_negative()) {mismatch_error = false;}
        if (!binop->arg2()->type()->is_unsigned() && binop->arg2()->is_const() && !binop->arg2()->is_negative()) {mismatch_error = false;}
      }
      if (mismatch_error) {
    		error("Parser::parse_relational_expression signed/unsigned mismatch");
      }
			e = binop;
		}
		ex = e.as_expression();
		boost::wave::token_id id = helper_.current_token_id();
		if (id == boost::wave::T_LESS) {op = BIN_LT;}
		else if (id == boost::wave::T_LESSEQUAL) {op = BIN_LE;}
		else if (id == boost::wave::T_GREATER) {op = BIN_GT;}
		else if (id == boost::wave::T_GREATEREQUAL) {op = BIN_GE;}
		else {return true;}
		next();
	}
	if (ex.is_not_null()) {
		error("Parser::parse_relational_expression shift expression expected");
	}
	return false;
}

//equality_expression
//	: relational_expression
//	| equality_expression EQ_OP relational_expression
//	| equality_expression NE_OP relational_expression
//	;
bool Parser::parse_equality_expression(Function* owner, ObjPtr& e)
{
	ObjPtr ex;
	BinOp op = BIN_UNDEFINED;
	while (parse_relational_expression(owner, e)) {
		if (ex.is_not_null()) {
			BinaryExpr* binop = new BinaryExpr(helper_.location(), op);
			binop->arg1(ex.as_expression());
			binop->arg2(e.as_expression());
			e = binop;
		}
		ex = e.as_expression();
		boost::wave::token_id id = helper_.current_token_id();
		if (id == boost::wave::T_EQUAL) {op = BIN_EQ;}
		else if ((id == boost::wave::T_NOTEQUAL) || (id == boost::wave::T_NOTEQUAL_ALT)) {
			op = BIN_NE;
		}
		else {return true;}
		next();
	}
	if (ex.is_not_null()) {
		error("Parser::parse_equality_expression relational expression expected");
	}
	return false;
}

//and_expression
//	: equality_expression
//	| and_expression '&' equality_expression
//	;
bool Parser::parse_and_expression(Function* owner, ObjPtr& e)
{
	ObjPtr ex;
	BinOp op = BIN_UNDEFINED;
	while (parse_equality_expression(owner, e)) {
		if (ex.is_not_null()) {
			BinaryExpr* binop = new BinaryExpr(helper_.location(), op);
			binop->arg1(ex.as_expression());
			binop->arg2(e.as_expression());
			e = binop;
		}
		ex = e.as_expression();
		boost::wave::token_id id = helper_.current_token_id();
		if ((id == boost::wave::T_AND) || (id == boost::wave::T_AND_ALT)) {op = BIN_BITAND;}
		else {return true;}
		next();
	}
	if (ex.is_not_null()) {
		error("Parser::parse_and_expression equality expression expected");
	}
	return false;
}

//exclusive_or_expression
//	: and_expression
//	| exclusive_or_expression '^' and_expression
//	;
bool Parser::parse_exclusive_or_expression(Function* owner, ObjPtr& e)
{
	ObjPtr ex;
	BinOp op = BIN_UNDEFINED;
	while (parse_and_expression(owner, e)) {
		if (ex.is_not_null()) {
			BinaryExpr* binop = new BinaryExpr(helper_.location(), op);
			binop->arg1(ex.as_expression());
			binop->arg2(e.as_expression());
			e = binop;
		}
		ex = e.as_expression();
		boost::wave::token_id id = helper_.current_token_id();
		if ((id == boost::wave::T_XOR) || (id == boost::wave::T_XOR_ALT) || (id == boost::wave::T_XOR_TRIGRAPH)) {op = BIN_BITXOR;}
		else {return true;}
		next();
	}
	if (ex.is_not_null()) {
		error("Parser::parse_exclusive_or_expression and expression expected");
	}
	return false;
}

//inclusive_or_expression
//	: exclusive_or_expression
//	| inclusive_or_expression '|' exclusive_or_expression
//	;
bool Parser::parse_inclusive_or_expression(Function* owner, ObjPtr& e)
{
	ObjPtr ex;
	BinOp op = BIN_UNDEFINED;
	while (parse_exclusive_or_expression(owner, e)) {
		if (ex.is_not_null()) {
			BinaryExpr* binop = new BinaryExpr(helper_.location(), op);
			binop->arg1(ex.as_expression());
			binop->arg2(e.as_expression());
			e = binop;
		}
		ex = e.as_expression();
		boost::wave::token_id id = helper_.current_token_id();
		if ((id == boost::wave::T_OR) || (id == boost::wave::T_OR_ALT) || (id == boost::wave::T_OR_TRIGRAPH)) {op = BIN_BITOR;}
		else {return true;}
		next();
	}
	if (ex.is_not_null()) {
		error("Parser::parse_inclusive_or_expression exclusive or expression expected");
	}
	return false;
}

//logical_and_expression
//	: inclusive_or_expression
//	| logical_and_expression AND_OP inclusive_or_expression
//	;
bool Parser::parse_logical_and_expression(Function* owner, ObjPtr& e)
{
	ObjPtr ex;
	BinOp op = BIN_UNDEFINED;
	while (parse_inclusive_or_expression(owner, e)) {
		if (ex.is_not_null()) {
			BinaryExpr* binop = new BinaryExpr(helper_.location(), op);
			binop->arg1(ex.as_expression());
			binop->arg2(e.as_expression());
			e = binop;
		}
		ex = e.as_expression();
		boost::wave::token_id id = helper_.current_token_id();
		if ((id == boost::wave::T_OROR) || (id == boost::wave::T_OROR_ALT) || (id == boost::wave::T_OROR_TRIGRAPH)) {op = BIN_OR;}
		else {return true;}
		next();
	}
	if (ex.is_not_null()) {
		error("Parser::parse_logical_and_expression inclusive or expression expected");
	}
	return false;
}

//logical_or_expression
//	: logical_and_expression
//	| logical_or_expression OR_OP logical_and_expression}
//	;
bool Parser::parse_logical_or_expression(Function* owner, ObjPtr& e)
{
	ObjPtr ex;
	BinOp op = BIN_UNDEFINED;
	while (parse_logical_and_expression(owner, e)) {
		if (ex.is_not_null()) {
			BinaryExpr* binop = new BinaryExpr(helper_.location(), op);
			binop->arg1(ex.as_expression());
			binop->arg2(e.as_expression());
			e = binop;
		}
		ex = e.as_expression();
		boost::wave::token_id id = helper_.current_token_id();
		if ((id == boost::wave::T_ANDAND) || (id == boost::wave::T_ANDAND_ALT)) {op = BIN_AND;}
		else {return true;}
		next();
	}
	if (ex.is_not_null()) {
		error("Parser::parse_logical_or_expression logical and expression expected");
	}
	return false;
}

//conditional_expression
//	: logical_or_expression
//	| logical_or_expression '?' expression ':' conditional_expression
//	;
bool Parser::parse_conditional_expression(Function* owner, ObjPtr cond_expr, ObjPtr& e)
{
	if (cond_expr.is_null()) {
		if (!parse_logical_or_expression(owner, cond_expr)) {
			return false;
		}
	}
	if (cond_expr.is_not_null()) {
		if (helper_.current_token_id() != boost::wave::T_QUESTION_MARK) {
			e = cond_expr;
			return true;
		}
		next();
		ObjPtr true_part;
		if (!parse_expression(owner, true_part)) {
			error("Parser::parse_conditional_expression expression expected");
			return false;
		}
		if (helper_.current_token_id() != boost::wave::T_COLON) {
			error("Parser::parse_conditional_expression : expected");
			return false;
		}
		next();
		ObjPtr false_part;
		if (!parse_conditional_expression(owner, 0, false_part)) {
			error("Parser::parse_conditional_expression conditional expression expected");
			return false;
		}
		TernaryExpr* tern = new TernaryExpr(helper_.location(), TER_QC);
		e = tern;
		tern->arg1(cond_expr.as_expression());
		tern->arg2(true_part.as_expression());
		tern->arg3(false_part.as_expression());
		return true;
	}
	return false;
}

//assignment_expression
//	: conditional_expression
//	| unary_expression assignment_operator assignment_expression
//	;
bool Parser::parse_assignment_expression(Function* owner, ObjPtr& e)
{
	size_t pos = helper_.mark();
	ObjPtr lhs;
	if (parse_unary_expression(owner, lhs)) {
		if (helper_.current_token_id() == boost::wave::T_QUESTION_MARK) {
			// we have a conditional expression
			if (!parse_conditional_expression(owner, lhs, e)) {
				error("Parser::parse_assignment_expression conditional expression expected");
				return false;
			}
			return true;
		}
		AssOp op = parse_assignment_operator();
		if (op == AS_UNDEFINED) {
			// no assignment
			helper_.go_back_to(pos);
			return parse_conditional_expression(owner, 0, e);
		}
		next();
		if (!parse_assignment_expression(owner, e)) {
			error("Parser::parse_assignment_expression assignment expression expected");
			return false;
		}
		Assignment* assign = new Assignment(helper_.location(), op);
		assign->lhs(lhs.as_expression());
		if (lhs.as_expression()->type()->is_equivalent_with(*e.as_expression()->type()) || op == AS_LEFT || op == AS_RIGHT) {
			assign->rhs(e.as_expression());
		}
		else {
			assign->rhs(
				new AutoCast(
					helper_.location(),
					lhs.as_expression()->lvalue_type(),
					e.as_expression(),
					(op == AS_ADD) || (op == AS_SUB) ? AutoCast::additive : AutoCast::normal
				)
			);
		}
		e = assign;
		return true;
	}
	helper_.go_back_to(pos);
	return parse_conditional_expression(owner, 0, e);
}

//assignment_operator
//	: '='
//	| MUL_ASSIGN
//	| DIV_ASSIGN
//	| MOD_ASSIGN
//	| ADD_ASSIGN
//	| SUB_ASSIGN
//	| LEFT_ASSIGN
//	| RIGHT_ASSIGN
//	| AND_ASSIGN
//	| XOR_ASSIGN
//	| OR_ASSIGN
//	;
AssOp Parser::parse_assignment_operator()
{
	switch (helper_.current_token_id()) {
		case boost::wave::T_ASSIGN: return AS_NORMAL;
		case boost::wave::T_STARASSIGN: return AS_MUL;
		case boost::wave::T_DIVIDEASSIGN: return AS_DIV;
		case boost::wave::T_PERCENTASSIGN: return AS_MOD;
		case boost::wave::T_PLUSASSIGN: return AS_ADD;
		case boost::wave::T_MINUSASSIGN: return AS_SUB;
		case boost::wave::T_SHIFTLEFTASSIGN: return AS_LEFT;
		case boost::wave::T_SHIFTRIGHTASSIGN: return AS_RIGHT;
		case boost::wave::T_ANDASSIGN: return AS_BITAND;
		case boost::wave::T_ANDASSIGN_ALT: return AS_BITAND;
		case boost::wave::T_ORASSIGN: return AS_BITOR;
		case boost::wave::T_ORASSIGN_ALT: return AS_BITOR;
		case boost::wave::T_ORASSIGN_TRIGRAPH: return AS_BITOR;
		case boost::wave::T_XORASSIGN: return AS_BITXOR;
		case boost::wave::T_XORASSIGN_ALT: return AS_BITXOR;
		case boost::wave::T_XORASSIGN_TRIGRAPH: return AS_BITXOR;
		default:;
	}
	return AS_UNDEFINED;
}

//expression
//	: assignment_expression
//	| expression ',' assignment_expression
//	;
bool Parser::parse_expression(Function* owner, ObjPtr& e)
{
	//end_expr();
	if (parse_assignment_expression(owner, e)) {
		while (helper_.current_token_id() == boost::wave::T_COMMA) {
			next();
			ObjPtr ex;
			if (!parse_assignment_expression(owner, ex)) {
				error("Parser::parse_expression assignment expression expected");
				return false;
			}
			BinaryExpr* bin = new BinaryExpr(helper_.location(), BIN_COMMA);
			bin->arg1(e.as_expression());
			bin->arg2(ex.as_expression());
			e = bin;
		}
		return true;
	}
	return false;
}

//constant_expression
//	: conditional_expression	// with constraints
//	;
bool Parser::parse_constant_expression(Function* owner, ObjPtr& e)
{
	if (parse_conditional_expression(owner, 0, e)) {
		e.optimize(-1);
		if (!e.as_expression()->is_const()) {
			error("Parser::parse_constant_expression not constant");
			return false;
		}
		return true;
	}
	return false;
}


//statement
//	: labeled_statement
//	| compound_statement
//	| expression_statement
//	| selection_statement
//	| iteration_statement
//	| jump_statement
//	;
bool Parser::parse_statement(Function* owner, ObjPtr& s)
{
	switch (helper_.current_token_id()) {
		case boost::wave::T_RETURN: return parse_return(owner, s);
		case boost::wave::T_GOTO: return parse_goto(owner, s);
		case boost::wave::T_CONTINUE: return parse_continue(owner, s);
		case boost::wave::T_BREAK: return parse_break(owner, s);
		case boost::wave::T_DEFAULT: return parse_default(owner, s);
		case boost::wave::T_CASE: return parse_case(owner, s);
		case boost::wave::T_DO: return parse_do_while(owner, s);
		case boost::wave::T_WHILE: return parse_while(owner, s);
		case boost::wave::T_FOR: return parse_for(owner, s);
		case boost::wave::T_IF: return parse_if(owner, s);
		case boost::wave::T_SWITCH: return parse_switch(owner, s);
		case boost::wave::T_IDENTIFIER: if (parse_label(owner, s)) {return true;}
		default:;
	}
	if (helper_.is_left_brace()) {
		return parse_compound_statement(owner, s);
	}
	//if (parse_labeled_statement()) {return true;}
	//if (parse_compound_statement()) {return true;}
	//if (parse_expression_statement()) {return true;}
	//if (parse_selection_statement()) {return true;}
	//if (parse_iteration_statement()) {return true;}
	//if (parse_jump_statement()) {return true;}
	return parse_expression_statement(owner, s);
}


bool Parser::parse_label(Function* owner, ObjPtr& s)
{
	size_t pos = helper_.mark();
	if (owner) {
		std::string name = helper_.identifier();
		next();
		if (helper_.current_token_id() != boost::wave::T_COLON) {
			helper_.go_back_to(pos);
			return false;
		}
		Label* lbl = owner->lookup_label(name);
		if (lbl == 0) {
			lbl = new Label(helper_.location());
			owner->add_label(helper_.identifier(), lbl);
		}
		else {
			lbl->location(helper_.location());
		}
		if (lbl->is_declared()) {
			error("Parser::parse_labeled_statement label already declared");
			return false;
		}
		next();
		lbl->declare();
		block()->add(lbl);
		if (!parse_statement(owner, s)) {
			error("statement expected");
			return false;
		}
		return true;
	}
	//(helper_.identifier());
	error("Parser::parse_labeled_statement no function");
	return false;
}


bool Parser::parse_case(Function* owner, ObjPtr& s)
{
	if (swtch()) {
		CaseStmt* dest = new CaseStmt(helper_.location(), swtch());
		ObjPtr obj(dest);
		next();
		ObjPtr e;
		if (!parse_constant_expression(owner, e)) {
			error("Parser::parse_case constant expression expected");
			return false;
		}
		Expression* ex = e.as_expression();
		if (ex == 0) {
			error("Parser::parse_case expression expected");
			return false;
		}
		dest->value(ex);
		if (helper_.current_token_id() != boost::wave::T_COLON) {
			error("Parser::parse_case : expected");
			return false;
		}
		next();
		block()->add(dest);
		swtch()->add(dest);
		if (!parse_statement(owner, s)) {
			error("Parser::parse_case statement expected");
			return false;
		}
		return true;
	}
	error("Parser::parse_case no switch");
	return false;
}


bool Parser::parse_default(Function* owner, ObjPtr& s)
{
	if (swtch()) {
		CaseStmt* dest = new CaseStmt(helper_.location(), swtch());
		ObjPtr obj(dest);
		next();
		if (helper_.current_token_id() != boost::wave::T_COLON) {
			error("Parser::parse_default : expected");
			return false;
		}
		next();
		block()->add(dest);
		swtch()->deflt(dest);
		if (!parse_statement(owner, s)) {
			error("Parser::parse_default statement expected");
			return false;
		}
		return true;
	}
	error("Parser::parse_case no switch");
	return false;
}

//labeled_statement
//	: IDENTIFIER ':' statement
//	| CASE constant_expression ':' statement
//	| DEFAULT ':' statement
//	;
bool Parser::parse_labeled_statement(Function* owner, ObjPtr& s)
{
	if (helper_.current_token_id() == boost::wave::T_IDENTIFIER) {
		return parse_label(owner, s);
	}	
	if (helper_.current_token_id() == boost::wave::T_CASE) {
		return parse_case(owner, s);
	}	
	if (helper_.current_token_id() == boost::wave::T_DEFAULT) {
		return parse_statement(owner, s);
	}	
	return false;
}

bool Parser::parse_switch(Function* owner, ObjPtr& s)
{
	SwitchStmt* dest = new SwitchStmt(helper_.location());
	Iteration* old_brk = brk();
	ObjPtr obj1(dest), obj2(old_brk);
	next();
	swtch(dest);
	brk(dest);
	if (!helper_.is_left_paren()) {
		error("Parser::parse_switch ( expected");
		return false;
	}
	next();
	ObjPtr e;
	if (!parse_expression(owner, e)) {
		error("Parser::parse_switch expression expected");
		return false;
	}
	dest->value(e.as_expression());
	if (!helper_.is_right_paren()) {
		error("Parser::parse_switch ) expected");
		return false;
	}
	next();
	block()->add(dest);
	if (!parse_statement(owner, s)) {
		error("Parser::parse_switch statement expected");
		return false;
	}
	dest->finish(owner);
	brk(old_brk);
	return true;
}

bool Parser::parse_if(Function* owner, ObjPtr& s)
{
	IfStmt* dest = new IfStmt(helper_.location());
	ObjPtr obj(dest);
	next();
	if (!helper_.is_left_paren()) {
		error("Parser::parse_if ( expected");
		return false;
	}
	next();
	ObjPtr e;
	if (!parse_expression(owner, e)) {
		error("Parser::parse_if expression expected");
		return false;
	}
	dest->cond(e.as_expression());
	if (!helper_.is_right_paren()) {
		error("Parser::parse_if ) expected");
		return false;
	}
	next();
	ObjPtr true_part;
	if (!parse_statement(owner, true_part)) {
		error("Parser::parse_if statement expected");
		return false;
	}
	dest->true_part(true_part.as_statement());
	if (helper_.current_token_id() == boost::wave::T_ELSE) {
		next();
		ObjPtr false_part;
		if (!parse_statement(owner, false_part)) {
			error("Parser::parse_if statement expected");
			return false;
		}
		dest->false_part(false_part.as_statement());
	}
	s = dest;
	return true;
}

bool Parser::parse_while(Function* owner, ObjPtr& s)
{
	WhileStmt* dest = new WhileStmt(helper_.location());
	Iteration* old_cont = cont();
	Iteration* old_brk = brk();
	ObjPtr obj1(dest), obj2(old_cont), obj3(old_brk);
	next();
	cont(dest);
	brk(dest);
	if (!helper_.is_left_paren()) {
		error("Parser::parse_while ( expected");
		return false;
	}
	next();
	ObjPtr e;
	if (!parse_expression(owner, e)) {
		error("Parser::parse_while expression expected");
		return false;
	}
	dest->cond(e.as_expression());
	if (!helper_.is_right_paren()) {
		error("Parser::parse_while ) expected");
		return false;
	}
	next();
	ObjPtr inner;
	if (!parse_statement(owner, inner)) {
		error("Parser::parse_while statement expected");
		return false;
	}
	dest->block(inner.as_statement());
	s = dest;
	cont(old_cont);
	brk(old_brk);
	return true;
}

bool Parser::parse_do_while(Function* owner, ObjPtr& s)
{
	DoWhileStmt* dest = new DoWhileStmt(helper_.location());
	ObjPtr obj(dest);
	next();
	ObjPtr inner;
	if (!parse_statement(owner, inner)) {
		error("Parser::parse_do_while statement expected");
		return false;
	}
	dest->block(inner.as_statement());
	if (helper_.current_token_id() != boost::wave::T_WHILE) {
		error("Parser::parse_do_while while expected");
		return false;
	}
	next();
	if (!helper_.is_left_paren()) {
		error("Parser::parse_do_while ( expected");
		return false;
	}
	next();
	ObjPtr e;
	if (!parse_expression(owner, e)) {
		error("Parser::parse_do_while expression expected");
		return false;
	}
	dest->cond(e.as_expression());
	if (!helper_.is_right_paren()) {
		error("Parser::parse_do_while ) expected");
		return false;
	}
	next();
	if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
		error("Parser::parse_do_while ; expected");
		return false;
	}
	s = dest;
	return true;
}

bool Parser::parse_for(Function* owner, ObjPtr& s)
{
	ForStmt* dest = new ForStmt(helper_.location());
	ObjPtr obj(dest);
	next();
	if (!helper_.is_left_paren()) {
		error("Parser::parse_for ( expected");
		return false;
	}
	next();
	ObjPtr init;
	if (helper_.current_token_id() == boost::wave::T_SEMICOLON) {}
	else if (parse_expression_statement(owner, init, false)) {
		dest->init(init.as_statement());
	}
	else if (parse_declaration(false)) {
	}
	else {
		error("Parser::parse_for declaration expected");
		return false;
	}
	if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
		error("Parser::parse_for ; expected");
		return false;
	}
	next();

	ObjPtr e;
	if (helper_.current_token_id() == boost::wave::T_SEMICOLON) {}
	else if (parse_expression(owner, e)) {
		dest->cond(e.as_expression());
	}
	else {
		error("expression expected");
		return false;
	}

	if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
		error("Parser::parse_for ; expected");
		return false;
	}
	next();

	if (!helper_.is_right_paren()) {
		e.clear();
		if (!parse_expression(owner, e)) {
			error("Parser::parse_for expression expected");
			return false;
		}
		dest->post(e.as_expression());
	}

	if (!helper_.is_right_paren()) {
		error("Parser::parse_for ) expected");
		return false;
	}
	next();
	if (!parse_statement(owner, s)) {
		error("Parser::parse_for statement expected");
		return false;
	}
	dest->block(s.as_statement());
	s = dest;
	return true;
}

//selection_statement
//	: IF '(' expression ')' statement ELSE statement
//	| IF '(' expression ')' statement
//	| SWITCH '(' expression ')' statement
//	;
bool Parser::parse_selection_statement(Function* owner, ObjPtr& s)
{
	if (helper_.current_token_id() == boost::wave::T_IF) {
		return parse_if(owner, s);
	}	
	if (helper_.current_token_id() == boost::wave::T_SWITCH) {
		return parse_switch(owner, s);
	}	
	return false;
}

//iteration_statement
//	: WHILE '(' expression ')' statement
//	| DO statement WHILE '(' expression ')' ';'
//	| FOR '(' expression_statement expression_statement ')' statement
//	| FOR '(' expression_statement expression_statement expression ')' statement
//	| FOR '(' declaration expression_statement ')' statement
//	| FOR '(' declaration expression_statement expression ')' statement
//	;
bool Parser::parse_iteration_statement(Function* owner, ObjPtr& s)
{
	if (helper_.current_token_id() == boost::wave::T_DO) {
		return parse_do_while(owner, s);
	}
	if (helper_.current_token_id() == boost::wave::T_WHILE) {
		return parse_while(owner, s);
	}
	if (helper_.current_token_id() == boost::wave::T_FOR) {
		return parse_for(owner, s);
	}
	return false;
}

bool Parser::parse_return(Function* owner, ObjPtr& s)
{
	Return* ret = new Return(helper_.location(), owner);
	ObjPtr obj(ret);
	next();
	ObjPtr e;
	if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
		if (!parse_expression(owner, e)) {
			error("Parser::parse_return expression expected");
			return false;
		}
		if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
			error("Parser::parse_return ; expected");
			return false;
		}
	}
	next();
	ret->return_type(owner->return_type());
	ret->expr(e.as_expression());
	helper_.flush();
	s = ret;
	return true;
}

bool Parser::parse_goto(Function* owner, ObjPtr& s)
{
	Jump* jmp = new Jump(helper_.location());
	ObjPtr obj(jmp);
	next();
	if (owner) {
		if (helper_.current_token_id() != boost::wave::T_IDENTIFIER) {
			error("Parser::parse_goto identiffier expected");
			return false;
		}
		Label* lbl = owner->lookup_label(helper_.identifier());
		if (lbl == 0) {
			lbl = new Label(helper_.location());
			owner->add_label(helper_.identifier(), lbl);
		}
		next();
		if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
			error("Parser::parse_goto ; expected");
			return false;
		}
		next();
		jmp->destination(lbl);
		s = jmp;
		return true;
	}
	error("Parser::parse_goto no function");
	return false;
}

bool Parser::parse_continue(Function* owner, ObjPtr& s)
{
	Iteration* iter = cont();
	Jump* jmp = new Jump(helper_.location());
	ObjPtr obj(jmp);
	next();
	if (iter) {
		if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
			error("Parser::parse_continue ; expected");
			return false;
		}
		s = jmp;
		next();
		jmp->destination(iter->cont());
		return true;
	}
	error("Parser::parse_continue no enclosing iteration");
	return false;
}

bool Parser::parse_break(Function* owner, ObjPtr& s)
{
	Iteration* iter = brk();
	Jump* jmp = new Jump(helper_.location());
	ObjPtr obj(jmp);
	next();
	if (iter) {
		if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
			error("Parser::parse_break ; expected");
			return false;
		}
		s = jmp;
		next();
		jmp->destination(iter->brk());
		return true;
	}
	error("Parser::parse_break no enclosing iteration");
	return false;
}

//jump_statement
//	: GOTO IDENTIFIER ';'
//	| CONTINUE ';'
//	| BREAK ';'
//	| RETURN ';'
//	| RETURN expression ';'
//	;
bool Parser::parse_jump_statement(Function* owner, ObjPtr& s)
{
	switch (helper_.current_token_id()) {
		case boost::wave::T_RETURN: return parse_return(owner, s);
		case boost::wave::T_GOTO: return parse_goto(owner, s);
		case boost::wave::T_CONTINUE: return parse_continue(owner, s);
		case boost::wave::T_BREAK: return parse_break(owner, s);

		default:;
	}
	return false;
}

//block_item_list
//	: block_item
//	| block_item_list block_item
//	;
bool Parser::parse_block_item_list(Function* owner, Block& blk)
{
	if (parse_block_item(owner, blk)) {
		while (parse_block_item(owner, blk)) {
		}
		return true;
	}
	return false;
}

//block_item
//	: declaration
//	| statement
//	;
bool Parser::parse_block_item(Function* owner, Block& blk)
{
	if (parse_declaration(owner)) {
		//helper_.flush();
		return true;
	}
	ObjPtr s;
	if (parse_statement(owner, s)) {
		s.optimize(-1);
		if (s.is_not_null()) {
			blk.add(s.as_statement());
		}
		//helper_.flush();
		return true;
	}
	return false;
}

//parameter_type_list
//	: parameter_list ',' ELLIPSIS
//	| parameter_list
//	;
//bool Parser::parse_parameter_type_list(bool is_new)
//{
//	if (parse_parameter_list(is_new)) {
//		if (helper_.current_token_id() == boost::wave::T_COMMA) {
//			next();
//			if (helper_.current_token_id() == boost::wave::T_ELLIPSIS) {
//				error("Parser::parse_parameter_type_list variadic functions are not supported");
//				next();
//				return false;
//				//next();
//				//return true;
//			}				
//		}
//		return true;
//	}
//	return false;
//}

//parameter_list
//	: parameter_declaration
//	| parameter_list ',' parameter_declaration
//	;
bool Parser::parse_parameter_list(Function* owner, Function* f, bool is_new)
{
	ObjPtr t;
	size_t pos = 0;
	if (f == 0) {
		error("Parser::parse_parameter_list no function");
		return false;
	}
  if (!is_new) {
    f->clear_parameter_names();
  }
again:
  end_name();
  //int par_pos = helper_.mark();
	if (parse_parameter_declaration(owner, t)) {
		if (t.as_type() == 0) {
			error("Parser::parse_parameter_list in function ("+f->name()+") no type");
			return false;
		}
		if (!t.as_type()->is_valid()) {
			error("Parser::parse_parameter_list in function ("+f->name()+") invalid type");
			return false;
		}
		Parameter* par = 0;
		if (f) {
			if (is_new) {
        //if (name().empty()) {
        //	ObjPtr tt;
        //  helper_.go_back_to(par_pos);
	       // parse_parameter_declaration(owner, tt);
        //}
				par = new Parameter(helper_.location(), t.as_type());
				f->add_parameter(name(), par);
			}
			else {
        if (pos >= f->parameter()) {
					error("Parser::parse_parameter_list more parameter in function ("+f->name()+") definition than in function declaration");
					return false;
        }
				par = f->parameter(pos);
				if (!par->type()->is_assignment_compatible_with(*t.as_type())) {
          par->type()->is_equivalent_with(*t.as_type());
          par->type()->is_assignment_compatible_with(*t.as_type());
					error("Parser::parse_parameter_list for funtion ("+f->name()+") parameter "+boost::lexical_cast<std::string>(pos)+" is not eqivalent to declaration");
					return false;
				}
				f->set_parameter(name(), par);
			}
		}
		if (helper_.current_token_id() == boost::wave::T_COMMA) {
			t.clear();
			next();
			if (helper_.current_token_id() == boost::wave::T_ELLIPSIS) {
				error("Parser::parse_parameter_list variadic functions ("+f->name()+") are not supported");
				next();
			}
			++pos;
			goto again;
		}
		if (!is_new) {
        if (++pos < f->parameter()) {
					error("Parser::parse_parameter_list less parameter in function ("+f->name()+") definition than in function declaration");
					return false;
        }
    }
		return true;
	}	

	//ObjPtr t;
	//if (parse_parameter_declaration(t)) {
	//	if (t.as_type() == 0) {
	//		error("Parser::parse_parameter_list no type");
	//		return false;
	//	}
	//	size_t pos = 0;
	//	if (!t.as_type()->is_valid()) {
	//		error("Parser::parse_parameter_list invalid type");
	//		return false;
	//	}
	//	Parameter* par = 0;
	//	if (f) {
	//		if (is_new) {
	//			par = new Parameter(helper_.location(), t.as_type());
	//			f->add_parameter(name(), par);
	//		}
	//		else {
	//			par = f->parameter(pos);
	//			if (!par->type()->is_equivalent_with(*type())) {
	//				error("Parser::parse_parameter_list parameter "+boost::lexical_cast<std::string>(pos)+" is not eqivalent to declaration");
	//				return false;
	//			}
	//		}
	//		//list_.push_back(par);
	//		end_decl();
	//		end_type();
	//		while (helper_.current_token_id() == boost::wave::T_COMMA) {
	//			next();
	//			if (helper_.current_token_id() == boost::wave::T_ELLIPSIS) {
	//				error("Parser::parse_parameter_list variadic functions are not supported");
	//				next();
	//			}
	//			else {
	//				ObjPtr t;
	//				if (!parse_parameter_declaration(t)) {
	//					error("Parser::parse_parameter_list invalid parameter declaration");
	//					return false;
	//				}
	//				if (!type()->is_valid()) {
	//					type()->is_valid();
	//					error("Parser::parse_parameter_list invalid type");
	//					return false;
	//				}
	//				if (is_new) {
	//					par = new Parameter(helper_.location(), type());
	//					f->add_parameter(name(), par);
	//				}
	//				else {
	//					par = f->parameter(pos);
	//					if (!par->type()->is_equivalent_with(*type())) {
	//						error("Parser::parse_parameter_list parameter "+boost::lexical_cast<std::string>(pos)+" is not eqivalent to declaration");
	//						return false;
	//					}
	//				}
	//			}
	//			end_decl();
	//			end_type();
	//			++pos;
	//		}
	//	}
	//	return true;
	//}
	return false;
}

//parameter_declaration
//	: declaration_specifiers declarator
//	| declaration_specifiers abstract_declarator
//	| declaration_specifiers
//	;
bool Parser::parse_parameter_declaration(Function* owner, ObjPtr& t)
{
	Declarator decl;
	if (parse_declaration_specifiers(owner, t, decl)) {
		size_t pos = helper_.mark();
    ObjPtr saved_t = t;
		if (parse_declarator(owner, t)) {
			return true;
		}
		helper_.go_back_to(pos);
    t = saved_t;
		if (parse_abstract_declarator(t)) {
			return true;
		}
		helper_.go_back_to(pos);
    t = saved_t;
		return true;
	}
	return false;
}

//identifier_list
//	: IDENTIFIER
//	| identifier_list ',' IDENTIFIER
//	;
bool Parser::parse_identifier_list()
{
	boost::wave::token_id id = helper_.current_token_id();
	if (id == boost::wave::T_IDENTIFIER) {
		next();
		id = helper_.current_token_id();
		while (id == boost::wave::T_COMMA) {
			next();
			id = helper_.current_token_id();
			if (id != boost::wave::T_IDENTIFIER) {
				error("Parser::parse_identifier_list identifier expected");
				return false;
			}
			id = helper_.current_token_id();
		}
		return true;
	}
	return false;
}

//direct_declarator
//	: IDENTIFIER
//	| '(' declarator ')'
//	| direct_declarator '[' ']'
//	| direct_declarator '[' '*' ']'
//	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
//	| direct_declarator '[' STATIC assignment_expression ']'
//	| direct_declarator '[' type_qualifier_list '*' ']'
//	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
//	| direct_declarator '[' type_qualifier_list assignment_expression ']'
//	| direct_declarator '[' type_qualifier_list ']'
//	| direct_declarator '[' assignment_expression ']'
//	| direct_declarator '(' parameter_type_list ')'
//	| direct_declarator '(' ')'
//	| direct_declarator '(' identifier_list ')'
//	;
bool Parser::parse_direct_declarator_body(Function* owner, ObjPtr& t)
{
	for (;;) {
		std::string fname;
		if (helper_.is_left_paren()) {
			// function declarator
			//size_t pos = helper_.mark();
      fname = name();
			ObjPtr fun = context_->lookup(name(), true);
			Function* f = fun.as_function();
			bool is_new = false;
			if (fun.is_not_null()) {
				if (f == 0) {
					error("Parser::parse_direct_declarator object "+name()+" already exist but is not a function");
					return false;
				}
				if (!t.as_type()->is_assignment_compatible_with(*fun.as_function()->return_type())) {
					error("Parser::parse_direct_declarator object "+name()+" inconsistent return type");
					return false;
				}
			}
			else {
				is_new = true;
				fun = f = new Function(context_);
				if (storage_class_mask_ & MASK(SC_SYSTEM)) {
					// system function declaration
					fname = "^";
				}
				else if (storage_class_mask_ & MASK(SC_EXTERN)) {
					// external function declaration
					fname = "_";
				}
        else if (this->name() == "main") {
					fname = "_";
          storage_class_mask_ |= MASK(SC_EXTERN);
				}
        else {
          fname.clear();
        }
				fname += this->name();
				f->name(fname);
				f->return_type(t.as_type());
				f->storage_class_mask(storage_class_mask_);
				f->qualifier_mask(type_qualifier_mask_);
			}
			next();
			if (!helper_.is_right_paren()) {
				if (!parse_parameter_list(owner, f, is_new)) {
					error("Parser::parse_direct_declarator parameter list expected");
					return false;
				}
				if (!helper_.is_right_paren()) {
					error("Parser::parse_direct_declarator ) expected");
					return false;
				}
			}
			next();
			t = fun;
		}
		else if (helper_.is_left_bracket()) {
			next();
			size_t pos = helper_.mark();
			ObjPtr e;
			int asize = 0;
			if (parse_assignment_expression(owner, e)) {
				Expression* ex = e.as_expression();
				if (ex == 0) {
					error("Parser::parse_direct_declarator expression expected");
					return false;
				}
				if (!ex->is_const()) {
					error("Parser::parse_direct_declarator non-const array size");
					return false;
				}
        ex->optimize(e, MASK(OPTM_CONST_FOLDING));
        ex = e.as_expression();
				if (!ex->get_int(asize)) {
          ex->get_int(asize);
					error("Parser::parse_direct_declarator invalid size constant");
					return false;
				}
				if (asize <= 0) {
					error("Parser::parse_direct_declarator invalid size");
					return false;
				}
			}
			if (!helper_.is_right_bracket()) {
				error("Parser::parse_direct_declarator ] expected");
				return false;
			}
			next();
			ArrayType* at = new ArrayType();
			at->limit(asize);
			at->storage_class_mask(storage_class_mask_);
			at->qualifier_mask(type_qualifier_mask_);
			if (!t.as_type()->append(*at)) {
				at->next(t.as_type());
				t = at;
			}
		}
		else {
			return true;
		}
	}
}

bool Parser::parse_direct_declarator(Function* owner, ObjPtr& t)
{
	if (helper_.is_left_paren()) {
		next();
		if (!parse_declarator(owner, t)) {
			error("Parser::parse_direct_declarator declarator expected");
			return false;
		}
		if (!helper_.is_right_paren()) {
			error("Parser::parse_direct_declarator ) expected");
			return false;
		}
		next();
		return parse_direct_declarator_body(owner, t);
	}
	if (helper_.current_token_id() == boost::wave::T_IDENTIFIER) {
		name(helper_.identifier());
		next();
////again:
//		if (helper_.is_left_paren()) {
//			//return true;
//		}
//		if (helper_.is_left_bracket()) {
//			next();
//			if (helper_.is_right_bracket()) {
//				next();
//				ArrayType* at = new ArrayType(t.as_type());
//				at->storage_class_mask(storage_class_mask_);
//				at->qualifier_mask(type_qualifier_mask_);
//				t = at;
//				//type(t);
//				res = true;
//				goto again;
//				//return true;
//			}
//			size_t pos = helper_.mark();
//			ObjPtr e;
//			if (parse_assignment_expression(owner, e)) {
//				Expression* ex = e.as_expression();
//				if (ex) {
//					if (!ex->is_const()) {
//						error("Parser::parse_direct_declarator non-const array size");
//						return false;
//					}
//					int asize;
//					if (!ex->get_int(asize)) {
//						error("Parser::parse_direct_declarator invalid size constant");
//						return false;
//					}
//					if (asize <= 0) {
//						error("Parser::parse_direct_declarator invalid size");
//						return false;
//					}
//					if (!helper_.is_right_bracket()) {
//						error("Parser::parse_direct_declarator ] expected");
//						return false;
//					}
//					next();
//					ArrayType* at = new ArrayType(t.as_type(), static_cast<size_t>(asize));
//					at->storage_class_mask(storage_class_mask_);
//					at->qualifier_mask(type_qualifier_mask_);
//					t = at;
//					res = true;
//					goto again;
//					//return true;
//				}
//			}
//			error("Parser::parse_direct_declarator invalid array specification");
//			return false;
//		}
//		//if (id == boost::wave::T_IDENTIFIER) {
		return parse_direct_declarator_body(owner, t);
	}
	//if (helper_.current_token_id() == boost::wave::T_SIZEOF) {
	//	name("sizeof");
	//	next();
	//	return parse_direct_declarator_body(owner, t);
	//	//throw not_implemented_exception("Parser::parse_direct_declarator sizeof");
	//}
	return false;
}

//alignment_specifier
//	: ALIGNAS '(' type_name ')'
//	| ALIGNAS '(' constant_expression ')'
//	;
bool Parser::parse_alignment_specifier()
{
	if (helper_.current_token_id() == boost::wave::T_ALIGNAS) {
		throw not_implemented_exception("Parser::parse_alignment_specifier");
	}
	return false;
}


//function_specifier
//	: INLINE
//	| NORETURN
//	;
bool Parser::parse_function_specifier()
{
	FunctionSpecifier fs = FS_UNDEFINED;
	switch (helper_.current_token_id()) {
	case boost::wave::T_INLINE: fs = FS_INLINE; break;
	//case boost::wave::T_NORETURN: fs = FS_NORETURN; break;
	}
	if (fs != FS_UNDEFINED) {
		next();
		function_specifier_mask_ |= MASK(fs);
		//func()->add_specifier(fs);
		return true;
	}
	return false;
}


//storage_class_specifier
//	: TYPEDEF					// identifiers must be flagged as TYPEDEF_NAME
//	| EXTERN
//	| STATIC
//	| THREAD_LOCAL
//	| AUTO
//	| REGISTER
//	;
bool Parser::parse_storage_class_specifier()
{
	//if (type() == 0) {type(new Type());}
	StorageClass sc = SC_UNDEFINED;
	switch (helper_.current_token_id()) {
		case boost::wave::T_TYPEDEF: sc = SC_TYPEDEF; is_typedef_ = true; next(); return true;
		case boost::wave::T_EXTERN: sc = SC_EXTERN; break;
		case boost::wave::T_STATIC: sc = SC_STATIC; break;
		//case boost::wave::T_THREAD_LOCAL: sc = SC_THREAD_LOCAL; break;
		case boost::wave::T_AUTO: sc = SC_AUTO; break;
		case boost::wave::T_REGISTER: sc = SC_REGISTER; break;

		case boost::wave::T_IDENTIFIER:
			if (helper_.identifier() == "__system") {sc = SC_SYSTEM; break;}
		default: return false;
	}
	if (sc != SC_UNDEFINED) {
		next();
		storage_class_mask_ |= MASK(sc);
		return true;
	}
	return false;
}


//initializer
//	: '{' initializer_list '}'
//	| '{' initializer_list ',' '}'
//	| assignment_expression
//	;
bool Parser::parse_initializer(Function* owner, Type* t, ObjPtr& e, Initializer* init)
{
	if (helper_.is_left_brace()) {
		next();
		for (;;) {
			Initializer* next_init = new Initializer(helper_.location());
			ObjPtr ie = next_init;
			if (init) {init->add(next_init);}
			if (e.is_null()) {e = next_init;}
			if (!parse_initializer_list(owner, t, ie, next_init)) {error("Parser::parse_initializer initializer list expected"); return false;}
			//boost::wave::token_id id = helper_.current_token_id();
			if (helper_.is_right_brace()) {
				next();
        if (next_init->size() == 1) { // one element initializer
          e = next_init->child(0);
        }
				return true;
			}
			if (helper_.current_token_id() == boost::wave::T_COMMA) {
				next();
				if (helper_.is_right_brace()) {
					next();
					return true;
				}
			}
			else {error("Parser::parse_initializer } expected"); return false;}
		}
	}
	if (init) {
		ObjPtr ie;
		if (!parse_assignment_expression(owner, ie)) {
			return false;
		}
		init->add(ie.as_expression());
		return true;
	}
	return parse_assignment_expression(owner, e);
}

//initializer_list
//	: designation initializer
//	| initializer
//	| initializer_list ',' designation initializer
//	| initializer_list ',' initializer
//	;
bool Parser::parse_initializer_list(Function* owner, Type* t, ObjPtr& e, Initializer* init)
{
	bool has_designatoon = parse_designation(owner, e);
	if (parse_initializer(owner, t, e, init)) {
		return true;
	}
	return false;
}


//direct_abstract_declarator
//	: '(' abstract_declarator ')'
//	| '[' ']'
//	| '[' '*' ']'
//	| '[' STATIC type_qualifier_list assignment_expression ']'
//	| '[' STATIC assignment_expression ']'
//	| '[' type_qualifier_list STATIC assignment_expression ']'
//	| '[' type_qualifier_list assignment_expression ']'
//	| '[' type_qualifier_list ']'
//	| '[' assignment_expression ']'
//	| direct_abstract_declarator '[' ']'
//	| direct_abstract_declarator '[' '*' ']'
//	| direct_abstract_declarator '[' STATIC type_qualifier_list assignment_expression ']'
//	| direct_abstract_declarator '[' STATIC assignment_expression ']'
//	| direct_abstract_declarator '[' type_qualifier_list assignment_expression ']'
//	| direct_abstract_declarator '[' type_qualifier_list STATIC assignment_expression ']'
//	| direct_abstract_declarator '[' type_qualifier_list ']'
//	| direct_abstract_declarator '[' assignment_expression ']'
//	| '(' ')'
//	| '(' parameter_type_list ')'
//	| direct_abstract_declarator '(' ')'
//	| direct_abstract_declarator '(' parameter_type_list ')'
//	;
bool Parser::parse_direct_abstract_declarator(ObjPtr& t)
{
	if (helper_.is_left_paren()) {
		throw not_implemented_exception("Parser::parse_direct_abstract_declarator");
	}
	if (helper_.is_left_bracket()) {
		throw not_implemented_exception("Parser::parse_direct_abstract_declarator");
	}
	return false;
}

//type_name
//	: specifier_qualifier_list abstract_declarator
//	| specifier_qualifier_list
//	;
bool Parser::parse_type_name(Function* owner, ObjPtr& t)
{
	if (parse_specifier_qualifier_list(owner, t)) {
		parse_abstract_declarator(t);
		return true;
	}
	return false;
}

//abstract_declarator
//	: pointer direct_abstract_declarator
//	| pointer
//	| direct_abstract_declarator
//	;
bool Parser::parse_abstract_declarator(ObjPtr& t)
{
	if (parse_pointer(t)) {
		parse_direct_abstract_declarator(t);
		return true;
	}
	return parse_direct_abstract_declarator(t);
}

//type_qualifier
//	: CONST
//	| RESTRICT
//	| VOLATILE
//	| ATOMIC
//	;
bool Parser::parse_type_qualifier()
{
	TypeQualifier tq = TQ_UNDEFINED;
	switch (helper_.current_token_id()) {
	case boost::wave::T_CONST: tq = TQ_CONST; break;
	case boost::wave::T_VOLATILE: tq = TQ_VOLATILE; break;
	default:;
	}
	if (tq  != TQ_UNDEFINED) {
		next();
		type_qualifier_mask_ |= MASK(tq);
		return true;
	}
	return false;
}

//specifier_qualifier_list
//	: type_specifier specifier_qualifier_list
//	| type_specifier
//	| type_qualifier specifier_qualifier_list
//	| type_qualifier
//	;
bool Parser::parse_specifier_qualifier_list(Function* owner, ObjPtr& t)
{
	Declarator decl;
	if (parse_type_specifier(owner, t, decl)) {
		parse_specifier_qualifier_list(owner, t);
		return true;
	}
	if (parse_type_qualifier()) {
		parse_specifier_qualifier_list(owner, t);
		return true;
	}
	return false;
}

bool Parser::parse_simple(ObjPtr& t, TypeType tt)
{
	if (t.is_null()) {
		t = new Type(tt);
		next();
		return true;
	} // start a new type definition
	//if (type()->type() == T_UNDEFINED) {type()->type(t); next(); return true;}
	if (t.as_type()->is_simple()) {
		switch(t.as_type()->type()) {
			case T_UNSIGNED:
				switch (tt) {
					case T_CHAR: t.as_type()->type(T_UCHAR); next(); return true;
					case T_SHORT: t.as_type()->type(T_USHORT); next(); return true;
					case T_INT: t.as_type()->type(T_UINT); next(); return true;
					case T_LONG: t.as_type()->type(T_ULONG); next(); return true;
//						case T_: type()->type(T_U); return true;
					default:;
				}
			break;

			case T_SIGNED:
				switch (tt) {
					case T_CHAR: t.as_type()->type(T_SCHAR); next(); return true;
					case T_SHORT: t.as_type()->type(T_SSHORT); next(); return true;
					case T_INT: t.as_type()->type(T_SINT); next(); return true;
					case T_LONG: t.as_type()->type(T_SLONG); next(); return true;
//						case T_: type()->type(T_U); return true;
					default:;
				}
			break;

			case T_LONG:
				switch (tt) {
					case T_LONG: t.as_type()->type(T_LONGLONG); next(); return true;
					case T_DOUBLE: t.as_type()->type(T_LONGDOUBLE); next(); return true;
					default:;
				}
			break;

			case T_ULONG:
				switch (tt) {
					case T_LONG: t.as_type()->type(T_ULONGLONG); next(); return true;
					default:;
				}
			break;

			case T_SLONG:
				switch (tt) {
					case T_LONG: t.as_type()->type(T_SLONGLONG); next(); return true;
					default:;
				}
			break;

			default:;
		}
		error("invalid type");
		return false;
	}
	return false;
}


//type_specifier
//	: VOID
//	| CHAR
//	| SHORT
//	| INT
//	| LONG
//	| FLOAT
//	| DOUBLE
//	| SIGNED
//	| UNSIGNED
//	| BOOL
//	| COMPLEX
//	| IMAGINARY			// non-mandated extension
//	| atomic_type_specifier
//	| struct_or_union_specifier
//	| enum_specifier
//	| TYPEDEF_NAME		// after it has been defined as such
//	;
bool Parser::parse_type_specifier(Function* owner, ObjPtr& t, Declarator& name)
{
	switch (helper_.current_token_id()) {
		case boost::wave::T_VOID: return parse_simple(t, T_VOID);
		case boost::wave::T_CHAR: return parse_simple(t, T_CHAR);
		case boost::wave::T_SHORT: return parse_simple(t, T_SHORT);
		case boost::wave::T_INT: return parse_simple(t, T_INT);
		case boost::wave::T_LONG: return parse_simple(t, T_LONG);
		case boost::wave::T_FLOAT: return parse_simple(t, T_FLOAT);
		case boost::wave::T_DOUBLE: return parse_simple(t, T_DOUBLE);
		case boost::wave::T_SIGNED: return parse_simple(t, T_SIGNED);
		case boost::wave::T_UNSIGNED: return parse_simple(t, T_UNSIGNED);
		case boost::wave::T_BOOL: return parse_simple(t, T_BOOL);

		case boost::wave::T_STRUCT:
		case boost::wave::T_UNION: return parse_struct_or_union_specifier(owner, t, name);

		case boost::wave::T_ENUM: return parse_enum_specifier(owner, t);

		case boost::wave::T_IDENTIFIER:
		{
			ObjPtr obj = context_->lookup_typedef(helper_.identifier());
			if (obj.is_null()) {
				return false;
			}
			Type* ot = t.as_type();
			if (ot == 0) {
				t = obj;
				ot = t.as_type();
				if (ot == 0) {
					return false;
				}
			}
			int scm = 0;
			int qm = 0;
			t = ot->copy();
			t.as_type()->storage_class_mask(storage_class_mask_);
			t.as_type()->qualifier_mask(type_qualifier_mask_);
			//type(t);
			next();
			return true;
		}
		default:;
	}
	return false;
}

//struct_or_union_specifier
//	: struct_or_union '{' struct_declaration_list '}'
//	| struct_or_union IDENTIFIER '{' struct_declaration_list '}'
//	| struct_or_union IDENTIFIER
//	;
bool Parser::parse_struct_or_union_specifier(Function* owner, ObjPtr& t, Declarator& decl)
{
	bool is_struct = helper_.current_token_id() == boost::wave::T_STRUCT;
	bool is_union = helper_.current_token_id() == boost::wave::T_UNION;
	if (is_struct || is_union) {
		std::string name;
		next();
		UnionStructType* us = 0;
		if (helper_.current_token_id() == boost::wave::T_IDENTIFIER) {
			name = helper_.identifier();
			Type* t = context_->lookup_tag(name);
			if (t) {
				us = dynamic_cast<UnionStructType*>(t);
				if (us == 0) {
					error("Parser::parse_struct_or_union_specifier tag "+name+" is not a struct or union");
					return false;
				}
				if (us->is_union() != is_union) {
					if (is_struct) {
						error("Parser::parse_struct_or_union_specifier union "+name+" cannot be declared as a struct");
					}
					else {
						error("Parser::parse_struct_or_union_specifier struct "+name+" cannot be declared as a union");
					}
					return false;
				}
			}
			else {
				decl.name(name);
        //context_->add_tag(name, t);
			}
			next();
		}
		//bool is_decl = false;
		if (helper_.is_left_brace()) {
			// definition
			next();
			if (us == 0) {
				//is_decl = true;
				us = new UnionStructType(name, is_union);
				decl.type(us);
				us->storage_class_mask(storage_class_mask_);
				us->qualifier_mask(type_qualifier_mask_);
        context_->add_tag(name, us);
				//if (type()) {
				//	//if (type()->type() != T_UNDEFINED) {
				//	//	error("Parser::parse_struct_or_union_specifier invalid struct declaration");
				//	//	return false;
				//	//}
				//	end_type();
				//}
			}
			if (!parse_struct_declaration_list(owner, *us)) {
				error("Parser::parse_struct_or_union_specifier struct declaration list expected");
				return false;
			}
			if (!helper_.is_right_brace()) {
				error("Parser::parse_struct_or_union_specifier } expected");
				return false;
			}
			//if (!name.empty() && is_decl) {
			//	context_->add_tag(name, us); /// todo
			//	//Do d;
			//	//d.add_tag(context_, name, us);
			//	//updater_.append(d);
			//}
			next();
		}
		else if (helper_.current_token_id() == boost::wave::T_SEMICOLON) {
			// declaration
			if (us == 0) {
				us = new UnionStructType(name, is_union);
				decl.type(us);
				us->storage_class_mask(storage_class_mask_);
				us->qualifier_mask(type_qualifier_mask_);
        context_->add_tag(name, us);
				//if (type()) {
				//	//if (type()->type() != T_UNDEFINED) {
				//	//	error("Parser::parse_struct_or_union_specifier invalid struct declaration");
				//	//	return false;
				//	//}
				//	us->storage_class_mask(storage_class_mask_);
				//	us->qualifier_mask(type_qualifier_mask_);
				//	end_type();
				//}
				//if (!name.empty()) {
				//	context_->add_tag(name, us);
				//}
			}
		}
		else {
			// struct must already be delared
			if (us == 0) {
				error("Parser::parse_struct_or_union_specifier tag "+name+" not found");
				return false;
			}
		}
		t = us;
		return true;
	}
	return false;
}

//struct_or_union
//	: STRUCT
//	| UNION
//	;
bool Parser::parse_struct_or_union()
{
	return false;
}

//struct_declaration_list
//	: struct_declaration
//	| struct_declaration_list struct_declaration
//	;
bool Parser::parse_struct_declaration_list(Function* owner, UnionStructType& us)
{
	if (!parse_struct_declaration(owner, us)) {
		error("Parser::parse_struct_declaration_list struct declaration list expected");
		return false;
	}
	while (helper_.current_token_id() == boost::wave::T_SEMICOLON) {
		next();
		if (helper_.is_right_brace()) {
			return true;
		}
		if (!parse_struct_declaration(owner, us)) {
			error("Parser::parse_struct_declaration_list struct declaration list expected");
			return false;
		}
	}
	return true;
}


//struct_declaration
//	: specifier_qualifier_list ';'	// for anonymous struct/union
//	| specifier_qualifier_list struct_declarator_list ';'
//	| static_assert_declaration
//	;
bool Parser::parse_struct_declaration(Function* owner, UnionStructType& us)
{
	if (parse_static_assert_declaration()) {
		return true;
	}
	ObjPtr t;
	if (!parse_specifier_qualifier_list(owner, t)) {
		error("Parser::parse_struct_declaration specifier qualifier list expected");
		return false;
	}
	if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
		if (!parse_struct_declarator_list(owner, us, t)) {
			error("Parser::parse_struct_declaration struct declarator list expected");
			return false;
		}
	}
	if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
		error("Parser::parse_struct_declaration ; expected");
		return false;
	}
	//end_type();
	return true;
}

//struct_declarator_list
//	: struct_declarator
//	| struct_declarator_list ',' struct_declarator
//	;
bool Parser::parse_struct_declarator_list(Function* owner, UnionStructType& us, ObjPtr& t)
{
	if (!parse_struct_declarator(owner, us, t)) {
		error("Parser::parse_struct_declarator_list struct declarator expected");
		return false;
	}
	while (helper_.current_token_id() == boost::wave::T_COMMA) {
		next();
		if (!parse_struct_declarator(owner, us, t)) {
			error("Parser::parse_struct_declarator_list struct declarator list expected");
			return false;
		}
	}
	return true;
}

//struct_declarator
//	: ':' constant_expression
//	| declarator ':' constant_expression
//	| declarator
//	;
bool Parser::parse_struct_declarator(Function* owner, UnionStructType& us, ObjPtr& t)
{
	if (parse_declarator(owner, t)) {
		if (t.as_type() == 0) {
			error("Parser::parse_struct_declarator no type");
			return false;
		}
		if (!t.as_type()->is_valid()) {
			error("Parser::parse_struct_declarator invalid type");
			return false;
		}
		UnionStructMember* mem = new UnionStructMember(helper_.location(), t.as_type());
		us.add(helper_.identifier(), mem);
		return true;
	}
	throw not_implemented_exception("Parser::parse_struct_declarator");
	return false;
}

//enum_specifier
//	: ENUM '{' enumerator_list '}'
//	| ENUM '{' enumerator_list ',' '}'
//	| ENUM IDENTIFIER '{' enumerator_list '}'
//	| ENUM IDENTIFIER '{' enumerator_list ',' '}'
//	| ENUM IDENTIFIER
//	;
bool Parser::parse_enum_specifier(Function* owner, ObjPtr& t)
{
	if (helper_.current_token_id() == boost::wave::T_ENUM) {
		next();
		std::string name;
		EnumType* ent = 0;
		if (helper_.current_token_id() == boost::wave::T_IDENTIFIER) {
			name = helper_.identifier();
			Type* tt = context_->lookup_tag(name);
			if (tt) {
				ent = dynamic_cast<EnumType*>(tt);
				if (ent == 0) {
					error("Parser::parse_enum_specifier tag "+name+" is not an enum");
					return false;
				}
				t = ent;
			}
			next();
		}
		if (helper_.is_left_brace()) {
			// definition
			next();
			if (ent == 0) {
				ent = new EnumType();
				t = ent;
				ent->storage_class_mask(storage_class_mask_);
				ent->qualifier_mask(type_qualifier_mask_);
				//if (type()) {
				//	//if (type()->type() != T_UNDEFINED) {
				//	//	error("Parser::parse_enum_specifier invalid struct declaration");
				//	//	return false;
				//	//}
				//	end_type();
				//}
				if (!name.empty()) {
					context_->add_tag(name, ent);
					//Do d;
					//d.add_tag(context_, name, ent);
					//updater_.append(d);
				}
			}
			if (!parse_enumerator_list(owner, *ent)) {
				error("Parser::parse_enum_specifier enumerator list expected");
				return false;
			}
			if (!helper_.is_right_brace()) {
				error("Parser::parse_enum_specifier } expected");
				return false;
			}
			next();
		}
		else if (helper_.current_token_id() == boost::wave::T_SEMICOLON) {
			// declaration
			if (ent == 0) {
				ent = new EnumType();
				t = ent;
				ent->storage_class_mask(storage_class_mask_);
				ent->qualifier_mask(type_qualifier_mask_);
				//if (type()) {
				//	//if (type()->type() != T_UNDEFINED) {
				//	//	error("Parser::parse_enum_specifier invalid struct declaration");
				//	//	return false;
				//	//}
				//	end_type();
				//}
				if (!name.empty()) {
					context_->add_tag(name, ent); /// todo
					//Do d;
					//d.add_tag(context_, name, ent);
					//updater_.append(d);
				}
			}
		}
		else {
			// struct must already be delared
			if (ent == 0) {
				error("Parser::parse_enum_specifier tag "+name+" not found");
				return false;
			}
		}
		return true;
	}
	return false;
}

//enumerator_list
//	: enumerator
//	| enumerator_list ',' enumerator
//	;
bool Parser::parse_enumerator_list(Function* owner, EnumType& en)
{
	if (!parse_enumerator(owner, en)) {
		error("Parser::parse_enumerator_list struct declarator expected");
		return false;
	}
	while (helper_.current_token_id() == boost::wave::T_COMMA) {
		next();
		if (helper_.is_right_brace()) {
			return true;
		}
		if (!parse_enumerator(owner, en)) {
			error("Parser::parse_enumerator_list struct declarator list expected");
			return false;
		}
	}
	return helper_.is_right_brace();
}

//enumerator	// identifiers must be flagged as ENUMERATION_CONSTANT
//	: enumeration_constant '=' constant_expression
//	| enumeration_constant
//	;
bool Parser::parse_enumerator(Function* owner, EnumType& ent)
{
	if (helper_.current_token_id() == boost::wave::T_IDENTIFIER) {
		std::string name = helper_.identifier();
    //helper_.remove_identifier();
		next();
		IntConst* value = 0;
		if (helper_.current_token_id() == boost::wave::T_ASSIGN) {
			next();
			ObjPtr e;
			if (!parse_constant_expression(owner, e)) {
				error("Parser::parse_enumerator constant expression expected");
				return false;
			}
			Expression* ex = e.as_expression();
			if (ex == 0) {
				error("Parser::parse_enumerator expression expected");
				return false;
			}
			IntConst* ic = dynamic_cast<IntConst*>(ex);
			if (ic) {
				ent.add(name, ic);
        context_->add_enumerator(name, ic);
				//Do d;
				//d.add_enumerator(context_, name, ic);
				//updater_.append(d);
			}
			else {
				int val;
				if (!ex->get_int(val)) {
					error("Parser::parse_enumerator invalid constant "+boost::lexical_cast<std::string>(val));
					return false;
				}
				value = new IntConst(helper_.location(), val);
				ent.add(name, value);
        context_->add_enumerator(name, value);
				//Do d;
				//d.add_enumerator(context_, name, value);
				//updater_.append(d);
			}
		}
		else {
			value = ent.add(name);
      context_->add_enumerator(name, value);
			//Do d;
			//d.add_enumerator(context_, name, value);
			//updater_.append(d);
		}
		return true;
	};
	return false;
}

//atomic_type_specifier
//	: ATOMIC '(' type_name ')'
//	;
bool Parser::parse_atomic_type_specifier()
{
	throw not_implemented_exception("Parser::parse_atomic_type_specifier");
	return false;
}


//designation
//	: designator_list '='
//	;
bool Parser::parse_designation(Function* owner, ObjPtr& e)
{
	size_t pos = helper_.mark();
	if (parse_designator_list(owner, e)) {
		if (helper_.current_token_id() != boost::wave::T_ASSIGN) {
			helper_.go_back_to(pos);
			return false;
		}
		next();
		return true;
	}
	helper_.go_back_to(pos);
	return false;
}

//designator_list
//	: designator
//	| designator_list designator
//	;
bool Parser::parse_designator_list(Function* owner, ObjPtr& e)
{
	if (helper_.is_left_bracket() || (helper_.current_token_id() == boost::wave::T_DOT)) {
		do {
			if (!parse_designator(owner, e)) {
				return false;
			}
		}
		while (helper_.is_left_bracket() || (helper_.current_token_id() == boost::wave::T_DOT));
		return true;
	}
	return false;
}

//designator
//	: '[' constant_expression ']'
//	| '.' IDENTIFIER
//	;
bool Parser::parse_designator(Function* owner, ObjPtr& e)
{
	if (helper_.is_left_bracket()) {
		next();
		ObjPtr ex;
		if (!parse_constant_expression(owner, ex)) {
			error("Parser::parse_designator constant expression expected");
			return false;
		}
		if (!helper_.is_right_bracket()) {
			error("Parser::parse_designator ] expected");
			return false;
		}
		next();
		ArrayAccess* acc = new ArrayAccess(helper_.location());
		acc->base(e.as_expression());
		acc->add_index(ex.as_expression());
		e = acc;
		return true;
	}
	throw not_implemented_exception("Parser::parse_designator");
	return false;
}


bool Parser::parse_type_qualifier_list()
{
	if (parse_type_qualifier()) {
		while (parse_type_qualifier()) {}
		return true;
	}
	return false;
}

//static_assert_declaration
//	: STATIC_ASSERT '(' constant_expression ',' STRING_LITERAL ')' ';'
//	;
bool Parser::parse_static_assert_declaration()
{
	if (helper_.current_token_id() == boost::wave::T_STATICASSERT) {
		throw not_implemented_exception("Parser::parse_static_assert_declaration");
	}
	return false;
}

//pointer
//	: '*' type_qualifier_list pointer
//	| '*' type_qualifier_list
//	| '*' pointer
//	| '*'
////	| '&' type_qualifier_list pointer
////	| '&' type_qualifier_list
////	| '&' pointer
////	| '&'
//	;
bool Parser::parse_pointer(ObjPtr& t)
{
	boost::wave::token_id id = helper_.current_token_id();
	if (id == boost::wave::T_STAR) {
		next();
		parse_type_qualifier_list();
		parse_pointer(t);
		ReferenceType* rt = new ReferenceType(T_POINTER);
		if (!t.as_type()->append(*rt)) {
			rt->next(t.as_type());
			t = rt;
		}
		//t->storage_class_mask(storage_class_mask_);
		//t->qualifier_mask(type_qualifier_mask_);
		//type(t.as_type());
		return true;
	}
	// reference not implemented in C
	//if (id == boost::wave::T_AND) {
	//	next();
	//	parse_type_qualifier_list();
	//	parse_pointer();
	//	ReferenceType* t = new ReferenceType(T_REFERENCE, type());
	//	//t->storage_class_mask(storage_class_mask_);
	//	//t->qualifier_mask(type_qualifier_mask_);
	//	type(t);
	//	return true;
	//}
	return false;
}

//init_declarator_list
//	: init_declarator
//	| init_declarator_list ',' init_declarator
//	;
bool Parser::parse_init_declarator_list(Function* owner, ObjPtr t)
{
	if (parse_init_declarator(owner, t)) {
		boost::wave::token_id id = helper_.current_token_id();
		while (id == boost::wave::T_COMMA) {
			next();
			if (!parse_init_declarator(owner, t)) {
				error("Parser::parse_init_declarator_list init declarator expected");
				return false;
			}
			id = helper_.current_token_id();
		}
		return true;
	}
	return false;
}

//expression_statement
//	: ';'
//	| expression ';'
//	;
bool Parser::parse_expression_statement(Function* owner, ObjPtr& s, bool eat_semicolon)
{
	if (helper_.current_token_id() == boost::wave::T_SEMICOLON) {
		if (eat_semicolon) {next();}
		return true;
	}
	ObjPtr e;
	if (parse_expression(owner, e)) {
		if (helper_.current_token_id() != boost::wave::T_SEMICOLON) {
			error("Parser::parse_expression_statement ; expected");
			return false;
		}
		s = e.as_expression();
		//if (optimize()) {stmt_.optimize(-1);}
		if (eat_semicolon) {next();}
		return true;
	}
	return false;
}

// compound_statement
//: '{' '}'
//| '{'	block_item_list '}'
//;
bool Parser::parse_compound_statement(Function* owner, ObjPtr& s)
{
	if (helper_.is_left_brace()) {
		Block* blk = new Block(helper_.location());
		ObjPtr obj = blk;
		ObjPtr old = block();
		block(blk);
		size_t locals = owner->local_size();
		next();
		if (helper_.is_right_brace()) {
			s = blk;
			next();
			helper_.flush();
			block(static_cast<Block*>(old.get()));
			return true;
		}
		enter_block();
		if (!parse_block_item_list(owner, *blk)) {
			exit_block();
			block(static_cast<Block*>(old.get()));
			error("Parser::parse_compound_statement invalid block");
			return false;
		}
		if (!helper_.is_right_brace()) {
			exit_block();
			block(static_cast<Block*>(old.get()));
			error("Parser::parse_compound_statement } expected");
			return false;
		}
		exit_block();
		next();
		//helper_.flush();
		owner->local_size(locals);
		s = blk;
		block(static_cast<Block*>(old.get()));
		return true;
	}
	return false;
}

//declaration_list
//	: declaration
//	| declaration_list declaration
//	;
bool Parser::parse_declaration_list(Function* owner)
{
	if (!parse_declaration(owner)) {return false;}
	while (parse_declaration(owner)) {}
	return false;
}

//declarator
//	: pointer direct_declarator
//	| direct_declarator
//	;
bool Parser::parse_declarator(Function* owner, ObjPtr& t)
{
	parse_pointer(t);
	return parse_direct_declarator(owner, t);
}

//declaration_specifiers
//	: storage_class_specifier declaration_specifiers
//	| storage_class_specifier
//	| type_specifier declaration_specifiers
//	| type_specifier
//	| type_qualifier declaration_specifiers
//	| type_qualifier
//	| function_specifier declaration_specifiers
//	| function_specifier
//	| alignment_specifier declaration_specifiers
//	| alignment_specifier
//	;
bool Parser::parse_declaration_specifiers(Function* owner, ObjPtr& t, Declarator& name)
{
	storage_class_mask_ = 0;
	type_qualifier_mask_ = 0;
	alignment_specifier_mask_ = 0;
	function_specifier_mask_ = 0;
	is_typedef_ = false;
	bool res = false;
again:
	if (parse_storage_class_specifier()) {
		res = true;
		goto again;
	}
	if (parse_type_specifier(owner, t, name)) {
		res = true;
		goto again;
	}
	if (parse_type_qualifier()) {
		res = true;
		goto again;
	}
	if (parse_function_specifier()) {
		res = true;
		goto again;
	}
	return res;
	//size_t pos = helper_.mark();
	//if (parse_storage_class_specifier()) {
	//	ObjPtr obj = type();
	//	parse_declaration_specifiers();
	//	if (type() == 0) {
	//		type(static_cast<Type*>(obj.get()));
	//	}
	//	return true;
	//}
	//helper_.go_back_to(pos);
	//end_type();
	//if (parse_type_specifier()) {
	//	ObjPtr obj = type();
	//	parse_declaration_specifiers();
	//	if (type() == 0) {
	//		type(static_cast<Type*>(obj.get()));
	//	}
	//	return true;
	//}
	//helper_.go_back_to(pos);
	//end_type();
	//if (parse_type_qualifier()) {
	//	ObjPtr obj = type();
	//	parse_declaration_specifiers();
	//	if (type() == 0) {
	//		type(static_cast<Type*>(obj.get()));
	//	}
	//	return true;
	//}
	//helper_.go_back_to(pos);
	//end_type();
	//if (parse_function_specifier()) {
	//	ObjPtr obj = type();
	//	parse_declaration_specifiers();
	//	if (type() == 0) {
	//		type(static_cast<Type*>(obj.get()));
	//	}
	//	return true;
	//}
	//helper_.go_back_to(pos);
	//end_type();
	//if (parse_alignment_specifier()) {
	//	ObjPtr obj = type();
	//	parse_declaration_specifiers();
	//	if (type() == 0) {
	//		type(static_cast<Type*>(obj.get()));
	//	}
	//	return true;
	//}
	//helper_.go_back_to(pos);
	//end_type();
	//return false;
}

//declaration
//	: declaration_specifiers ';'
//	| declaration_specifiers init_declarator_list ';'
//	| static_assert_declaration
//	;
bool Parser::parse_declaration(Function* owner, bool eat_semicolon)
{
	ObjPtr t;
	Declarator decl;
	if (parse_declaration_specifiers(owner, t, decl)) {
		boost::wave::token_id id =	helper_.current_token_id();
		if (id == boost::wave::T_SEMICOLON) {
			decl.context(*context_);
			decl.declare();
			next();
			helper_.flush();
			return true;
		}
		if (!parse_init_declarator_list(owner, t)) {
      error("Parser::parse_declaration init declaration list expected");
      return false;
    }
		id = helper_.current_token_id();
		if (id != boost::wave::T_SEMICOLON) {
			error("Parser::parse_declaration ; expected");
			return false;
		}
		if (eat_semicolon) {next();}
  	decl.context(*context_);
		decl.declare();
		end_name();
    helper_.flush();
		return true;
	}
	return parse_static_assert_declaration();
}


//init_declarator
//	: declarator '=' initializer
//	| declarator
//	;
bool Parser::parse_init_declarator(Function* owner, ObjPtr& t)
{
	// t is used to pass the basic type to the next declarator in a declarator list
	ObjPtr obj;
	if (parse_declarator(owner, t)) {
		bool is_initialized = false;
		boost::wave::token_id id = helper_.current_token_id();
		//if ((id == boost::wave::T_SEMICOLON) || (id == boost::wave::T_COLON)) {
		if (t.as_type() == 0) {
			return false;
		}
		if (!t.as_type()->is_valid()) {
			error("Parser::parse_init_declarator invalid type");
			return false;
		}
		if (is_typedef_) {
			// we have a new typedef type declaration
			context_->add_typedef(name(), t.as_type());
		}
		else {
			Variable* var = 0;
			Local* loc = 0;
			Static* stat = 0;
			bool is_static = (storage_class_mask_ & MASK(SC_STATIC)) != 0;
			std::string sname;
			ObjPtr e;
			if (id == boost::wave::T_ASSIGN) {
				//Type* t = type();
				//ObjPtr saved_type = t;
				//end_type();
				next();
				if ((owner == 0) || is_static) {
					if (storage_class_mask_ & MASK(SC_SYSTEM)) {
						sname = "^";
					}
					else if (storage_class_mask_ & MASK(SC_EXTERN)) {
						sname = "_";
            stat = dynamic_cast<Static*>(context_->lookup(name(), true).get());
					}
          else {
            stat = dynamic_cast<Static*>(context_->lookup(name(), true).get());
          }
          if (stat == 0) {
					  sname += name();
					  stat = new Static(helper_.location(), t.as_type(), context_, sname);
					  context_->add(stat);
					  obj = stat;
					  manager(stat->manager());
          }
				}
				if (!parse_initializer(owner, t.as_type(), e, 0)) {
					if (owner == 0) {
						end_manager();
					}
					error("initializer expected");
					return false;
				}
				is_initialized = true;
				//if (optimize()) {e.optimize(-1);}
				//type(t.as_type());
			}
			if (owner && !is_static) {
				// local variable definition
				loc = new Local(helper_.location(), t.as_type());
				loc->name(name());
				owner->add_local(name(), loc);
				var = loc;
			}
			else {
				// static variable definition
				if (stat == 0) {
					if (storage_class_mask_ & MASK(SC_SYSTEM)) {
						sname = "^";
					}
					else if (storage_class_mask_ & MASK(SC_EXTERN)) {
						sname = "_";
					}
					sname += name();
					stat = new Static(helper_.location(), t.as_type(), context_, sname);
					obj = stat;
					context_->add(stat);
				}
				stat->define();
				end_manager();
				var = stat;
				is_static = true;
			}
			if (is_initialized) {
				InitAssign* ass = new InitAssign(helper_.location());
				ObjPtr obj(ass);
				var->initialize();
				ass->lhs(var);
				if (var->type()->is_equivalent_with(*e.as_expression()->type())) {
					ass->rhs(e.as_expression());
				}
				else {
					ass->rhs(new AutoCast(helper_.location(), var->type(), e.as_expression()));
				}
				if (stat) {
					stat->init_statement(ass);
					//context_->add_statement(ass);
				}
				//if (loc && !is_static && (block() != 0)) {
				else {
					// add local init code to function statement list
					block()->add(ass);
				}
			}
		}
		// forget all indirections
		t = t.as_type()->base();
		//type(type()->base());
		return true;
	}
	return false;
}

//function_definition
//	: declaration_specifiers declarator declaration_list compound_statement
//	| declaration_specifiers declarator compound_statement
//	;
bool Parser::parse_function_definition(Function* owner)
{
	ObjPtr t_or_f;
	size_t pos = helper_.mark();
	Declarator decl;
	if (!parse_declaration_specifiers(owner, t_or_f, decl)) {
		helper_.go_back_to(pos);
		return false;
	}
	if (!parse_declarator(owner, t_or_f)) {
		helper_.go_back_to(pos);
		return false;
	}
	int scm = storage_class_mask_;
	decl.context(*context_);
	decl.declare();
	Function* f = t_or_f.as_function();
  if (f == 0) {
		helper_.go_back_to(pos);
		return false;
  }
	std::string vname, fname = f->name();
  //if (fname.empty()) {
		//helper_.go_back_to(pos);
  //  return false;
  //}
	bool is_main = fname == "main";
	if (is_main) {
		fname = "_main";
		storage_class_mask_ = scm |= MASK(SC_EXTERN);
		//func()->name(fname);
	}
	Type* t = t_or_f.as_type();
	boost::wave::token_id id =	helper_.current_token_id();
	if (id == boost::wave::T_SEMICOLON) {
		// declaration
		if (t_or_f.is_null()) {
			error("Parser::parse_function_definition no type or function declaration");
			return false;
		}
		if (f) {
			// function declaration
			if (block_level() > 0) {
				// we cannot declare a function inside a block
				error("Parser::parse_function_definition invalid function declaration inside a block");
				helper_.go_back_to(pos);
				return false;
			}
			// we have a function declaration
			if (is_main) {
				// declaration of "main" not allowed
				error("Parser::parse_function_definition illegal declaration of 'main'");
				return false;
			}
			// function declaration
			context_->add(f);
			//end_func();
		}
		else if (t) {
			// type declaration
			if (!t->is_valid()) {
				error("Parser::parse_function_definition invalid type");
				return false;
			}
			if (is_typedef_) {
				// we have a new typedef type declaration
				context_->add_typedef(this->name(), t);
			}
			else {
				vname = helper_.identifier();
				if (block_level() > 0) {
					// local variable declaration
					Local* var = new Local(helper_.location(), t);
					f->add_local(vname, var);
				}
				else {
					// static variable declaration
					ObjPtr obj = context_->lookup(vname, true);
					if (obj.is_null()) {
						if ((storage_class_mask_ & MASK(SC_SYSTEM)) != 0) {
							vname.insert(vname.begin(), '^');
						}
						else if ((storage_class_mask_ & MASK(SC_EXTERN)) != 0) {
							vname.insert(vname.begin(), '_');
						}
						context_->add(new Static(helper_.location(), t, context_, vname));
					}
					else {
						Static* stat = obj.as_static();
						if (stat == 0) {
							error("Parser::parse_function_definition variable name conflict with static object");
							next();
							return false;
						}
						if (stat->is_defined()) {
							error("Parser::parse_function_definition variable is already defined");
							next();
							return false;
						}
						stat->define();
					}
				}
			}
			//end_type();
		}
		next();
		end_name();
		helper_.flush();
		return true;
	}
	if (f == 0) {
		// not a function
		helper_.go_back_to(pos);
		return false;
	}
	end_name();
	// old style C functions not supported
	//parse_declaration_list();
	func_name(fname);
	manager(f->manager());
	ObjPtr s;
	if (!parse_compound_statement(f, s)) {
		helper_.go_back_to(pos);
		func_name("");
		end_manager();
		return false;
	}
	func_name("");
	f->content(s.as_statement());
	// function definition
	f->defined();
	if (optimize()) {f->optimize(-1);}
	f->compile();
	end_manager();
	context_->add(f);
#ifdef PRINT
	f->print(std::cout);
	f->compilation_unit()->code().print(std::cout);
#endif // PRINT
	helper_.flush();
	//end_func();
	return true;
}

//external_declaration
//	: function_definition
//	| declaration
//	;
bool Parser::parse_external_declaration()
{
	size_t pos = helper_.mark();
	if (parse_function_definition(0)) {
		//updater_.perform();
    context_->save_temp();
		return true;
	}
	//updater_.clear();
  context_->forget_temp();
	end_name();
	helper_.go_back_to(pos);
	if (parse_declaration(0)) {	// no function is owner of this declaration
		//updater_.perform();
    context_->save_temp();
		return true;
	}
	//updater_.clear();
  context_->forget_temp();
	return false;
}

//translation_unit
//	: external_declaration
//	| translation_unit external_declaration
//	;
void Parser::parse(Context& cntxt)
{
	context_ = &cntxt;
	helper_.filter();
	while (!helper_.finished()) {
		if (!parse_external_declaration()) {
			error("syntax error");
			next();
		}
	}
	context_ = 0;
}

}
