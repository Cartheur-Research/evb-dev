#include "cvmc.hpp"

namespace cvmc {

Cast::Cast(const context_type::position_type& w, Type* t, Expression* e)
: Expression(w)
{
	from(e);
	to(t);
}

bool Cast::is_const() const
{
  return from()->is_const();
}

bool Cast::is_lvalue() const
{
  return from()->is_lvalue();
}

bool Cast::has_side_effect() const
{
  return from()->has_side_effect();
}

void Cast::build_type()
{
	from()->build_type();
	set_type(to());
}

void Cast::optimize(ObjPtr& ptr, int mask)
{
	from_.optimize(mask);
	if (from()->type()->is_equivalent_with(*to())) {
		ptr = from();
	}
}

int Cast::compile_push(CompilationUnit& cu)
{
	return from()->compile_push(cu);
}

int Cast::compile_pop(CompilationUnit& cu)
{
	return from()->compile_pop(cu);
}

int Cast::compile_push(CompilationUnit& cu, size_t offset)
{
	return from()->compile_push(cu, offset);
}

int Cast::compile_pop(CompilationUnit& cu, size_t offset)
{
	return from()->compile_pop(cu, offset);
}

int Cast::compile_load(CompilationUnit& cu, bool push_result)
{
	return from()->compile_load(cu, push_result);
}

int Cast::compile_store(CompilationUnit& cu)
{
	return from()->compile_store(cu);
}

int Cast::compile_load_word(CompilationUnit& cu, size_t offset, bool push_result)
{
	return from()->compile_load_word(cu, offset, push_result);
}

int Cast::compile_store_word(CompilationUnit& cu, size_t offset)
{
	return from()->compile_store_word(cu, offset);
}

int Cast::compile_adr(CompilationUnit& cu, size_t offset)
{
	return from()->compile_adr(cu, offset);
}

int Cast::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	Type& t1 = *from()->type();
	Type& t2 = *to();
	size_t s1 = t1.size();
	size_t s2 = t2.size();
  from()->compile(cu, push_result);
	if (s1 == s2) {
    if (s1 == 2) {
      if (t1.type() == T_FLOAT) {
      }
      else if (t2.type() == T_FLOAT) {
      }
      else if (t1.is_unsigned() != t2.is_unsigned()) {
			  IssueList::warning(this->location(), "possible loss of data due to explicit type conversion signed <-> unsigned");
		  }
    }
	}
	else if ((s1 == 2) && (s2 == 1)) {
		IssueList::warning(this->location(), "possible loss of data due to explicit type conversion");
	}
	else if ((s1 == 1) && (s2 == 2)) {
		if (t1.is_unsigned()) {
			res += cu.code().compile_zext();
		}
		else {
			res += cu.code().compile_sext();
		}
	}
	else if ((s1 == 4) && (s2 == 1)) {
		IssueList::warning(this->location(), "possible loss of data due to explicit type conversion");
		res += cu.code().compile_pop();
		res += cu.code().compile_pop();
		res += cu.code().compile_pop();
		res += cu.code().compile_pop();
	}
	else if ((s1 == 1) && (s2 == 4)) {
		if (t1.is_unsigned()) {
			res += cu.code().compile_zext();
		}
		else {
			res += cu.code().compile_sext();
		}
		res += cu.code().compile_push();
		res += cu.code().compile_xd();
		res += cu.code().compile_push();
		res += cu.code().compile_push();
		res += cu.code().compile_push();
	}
	else {
		throw not_implemented_exception("Cast::compile cast from size "+boost::lexical_cast<std::string>(s1)+" to size "+boost::lexical_cast<std::string>(s2)+" is not supported yet");
	}
	return res;
}

//*************************************************************************************

AutoCast::AutoCast(CastType ct)
: cast_type_(ct)
{
}

AutoCast::AutoCast(const context_type::position_type& w, CastType ct)
: Cast(w)
, cast_type_(ct)
{
}

AutoCast::AutoCast(const context_type::position_type& w, Type* t, Expression* e, CastType ct)
: Cast(w, t, e)
, cast_type_(ct)
{
}

void AutoCast::optimize(ObjPtr& ptr, int mask)
{
	from_.optimize(mask);
	if (from()->type()->is_equivalent_with(*to())) {
		ptr = from();
		return;
	}
	switch (cast_type_) {
	case additive:
		if (!to()->is_additive_compatible_with(*from()->type())) {
			if (from()->is_const()) {
				if (to()->size() == from()->type_size()) {
					return;
				}
			}
			Type* f = from()->type();
			Type* t = to();
			to()->is_assignment_compatible_with(*from()->type());
			throw exception("AutoCast::optimize cannot cast automatically");
		}
		break;

	default:
		if (!to()->is_assignment_compatible_with(*from()->type())) {
			if (from()->is_const()) {
				if (to()->size() == from()->type_size()) {
					return;
				}
			}
			Type* f = from()->type();
			Type* t = to();
			t->is_assignment_compatible_with(*f);
			throw exception("AutoCast::optimize cannot cast automatically");
		}
	}
}

void AutoCast::compile_data(CompilationUnit& cu)
{
	from()->compile_data(cu);
}

int AutoCast::compile(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	Type& t1 = *from()->type();
	Type& t2 = *to();
	size_t s1 = t1.size();
	size_t s2 = t2.size();
	if (t1.is_array() && t2.is_pointer()) {
		ArrayType& at = static_cast<ArrayType&>(t1);
		ReferenceType& rt = static_cast<ReferenceType&>(t2);
		size_t as = at.next()->size();
		size_t rs = rt.next()->size();
		if ((as == rs) || (rs == 0)) {
			s1 = s2 = 1;
		}
	}
	if (s1 == s2) {
		if (t1.is_unsigned() != t2.is_unsigned()) {
			Constant* cons = dynamic_cast<Constant*>(from());
			if (cons) {
				if (t2.is_unsigned()) {
					if (cons->is_negative()) {
						IssueList::warning(this->location(), "assigning a negative constant to a unsigned value");
					}
				}
				else {
				}
			}
			else {
				IssueList::warning(this->location(), "possible loss of data due to automatic type conversion signed <-> unsigned");
			}
		}
		res += from()->compile(cu, push_result);
	}
	else { // types have different sizes
    if (s1 > s2) { // shrink size
			IssueList::warning(this->location(), "possible loss of data due to automatic type conversion");
      if (s2 == 1) {
      }
      else if (s2 == 2) {
      }
      else {
			  throw not_implemented_exception("AutoCast::compile cast from size "+boost::lexical_cast<std::string>(s1)+" to size "+boost::lexical_cast<std::string>(s2)+" is not supported yet");
      }
    }
    else { // expand size
    }
		//if ((s1 == 2) && (s2 == 1)) {
		//	IssueList::warning(this->location(), "possible loss of data due to automatic type conversion");
		//}
		if ((s1 == 1) && (s2 == 2)) {
      if (t1.is_integer_or_pointer() && t2.is_integer_or_pointer()) {
			  res += from()->compile(cu, false);
			  if (t1.is_unsigned()) {
				  res += cu.code().compile_zext();
			  }
			  else {
				  res += cu.code().compile_sext();
			  }
        if (push_result) {
				  res += cu.code().compile_push_long();
        }
      }
      else {
			  throw not_implemented_exception("AutoCast::compile cast from size "+boost::lexical_cast<std::string>(s1)+" to size "+boost::lexical_cast<std::string>(s2)+" is not supported yet");
      }
		}
		else if ((s1 == 1) && (s2 == 4)) {
      if (t1.is_integer_or_pointer() && t2.is_integer_or_pointer()) {
			  res += from()->compile(cu, false);
			  if (t1.is_unsigned()) {
				  res += cu.code().compile_zext();
			  }
			  else {
				  res += cu.code().compile_sext();
			  }
				res += cu.code().compile_push();
				res += cu.code().compile_xd();
				res += cu.code().compile_push();
				res += cu.code().compile_push();
				res += cu.code().compile_push();
      }
      else {
			  throw not_implemented_exception("AutoCast::compile cast from size "+boost::lexical_cast<std::string>(s1)+" to size "+boost::lexical_cast<std::string>(s2)+" is not supported yet");
      }
    }
		else if ((s1 == 2) && (s2 == 4)) {
      if (t1.is_integer_or_pointer() && t2.is_integer_or_pointer()) {
        res += cu.code().compile_push();
        res += cu.code().compile_push();
        res += cu.code().compile_push();
        res += cu.code().compile_push();
			  res += from()->compile(cu, push_result);
        //res += cu.code().compile_push_long();
 			  if (t1.is_unsigned()) {
			  }
			  else {
			  }
      }
      else {
			  throw not_implemented_exception("AutoCast::compile cast from size "+boost::lexical_cast<std::string>(s1)+" to size "+boost::lexical_cast<std::string>(s2)+" is not supported yet");
      }
		}
		else {
			throw not_implemented_exception("AutoCast::compile cast from size "+boost::lexical_cast<std::string>(s1)+" to size "+boost::lexical_cast<std::string>(s2)+" is not supported yet");
		}
	}
	return res;
}


}
