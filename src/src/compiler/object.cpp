#include "cvmc.hpp"

namespace cvmc {

RefObj::RefObj()
: ref_cnt_(0)
, objtype_(OT_UNDEFINED)
, scope_type_(ST_UNDEFINED)
{
}

RefObj::RefObj(ObjType t)
: ref_cnt_(0)
, objtype_(t)
, scope_type_(ST_UNDEFINED)
{
}

RefObj::~RefObj()
{
}

void RefObj::dec_ref()
{
	if (--ref_cnt_ <= 0) {
		delete this;
	}
}

void RefObj::print(std::ostream&)
{
	throw not_implemented_exception("RefObj::print");
}

void RefObj::write(xml::Writer&)
{
	throw not_implemented_exception("RefObj::write");
}

void RefObj::read(xml::Iterator&)
{
	throw not_implemented_exception("RefObj::read");
}

void RefObj::link1(Linker&)
{
	throw not_implemented_exception("RefObj::link1");
}

void RefObj::link2(Linker&)
{
	throw not_implemented_exception("RefObj::link2");
}

XRefManager* RefObj::manager()
{
	throw not_implemented_exception("RefObj::manager");
	return 0;
}

//*************************************************************************************

ObjPtr::ObjPtr(RefObj* o)
: ptr_(o)
{
	if (o) {o->inc_ref();}
}

ObjPtr::~ObjPtr()
{
	if (ptr_) {ptr_->dec_ref();}
}

ObjPtr::ObjPtr(const ObjPtr& op)
: ptr_(op.ptr_)
{
	if (ptr_) {ptr_->inc_ref();}
}

ObjPtr& ObjPtr::operator=(const ObjPtr& op)
{
	RefObj* o = op.ptr_;
	if (o) {
		o->inc_ref();
	};
	clear();
	ptr_ = o;
	return *this;
}

ObjPtr& ObjPtr::operator=(RefObj* o)
{
	if (o) {o->inc_ref();}
	clear();
	ptr_ = o;
	return *this;
}

void ObjPtr::clear()
{
	if (ptr_) {
		ptr_->dec_ref();
		ptr_=0;
	}
}

Type* ObjPtr::as_type()
{
	if (ptr_) {
		//Type* t = dynamic_cast<Type*>(ptr_);
		//if (t == 0) {
		//	throw exception("ObjPtr::as_type object is ot a type");
		//}
		return dynamic_cast<Type*>(ptr_);
	}
	return 0;
}

Constant* ObjPtr::as_const()
{
	if (ptr_) {
		return dynamic_cast<Constant*>(ptr_);
	}
	return 0;
}

Function* ObjPtr::as_function()
{
	if (ptr_) {
		return dynamic_cast<Function*>(ptr_);
	}
	return 0;
}

Static* ObjPtr::as_static()
{
	if (ptr_) {
		return dynamic_cast<Static*>(ptr_);
	}
	return 0;
}

Variable* ObjPtr::as_variable()
{
	if (ptr_) {
		return dynamic_cast<Variable*>(ptr_);
	}
	return 0;
}

Expression* ObjPtr::as_expression()
{
	if (ptr_) {
		return dynamic_cast<Expression*>(ptr_);
	}
	return 0;
}

Statement* ObjPtr::as_statement()
{
	if (ptr_) {
		return dynamic_cast<Statement*>(ptr_);
	}
	return 0;
}

void ObjPtr::optimize(int mask)
{
	Statement* stmt = dynamic_cast<Statement*>(ptr_);
	if (stmt) {stmt->optimize(*this, mask);}
}

int ObjPtr::compile(CompilationUnit& cu, bool push_result)
{
	Statement* stmt = dynamic_cast<Statement*>(ptr_);
	if (stmt) {return stmt->compile(cu, push_result);}
	return 0;
}

//*************************************************************************************

CompilationUnit::CompilationUnit()
: func_(0)
{
	data_.is_data(true);
}

CompilationUnit::CompilationUnit(Function* f)
: func_(f)
{
	data_.is_data(true);
}

int CompilationUnit::compile_condition(Expression* expr, bool push_result)
{
	return expr->compile_condition(*this, push_result);
}

void CompilationUnit::write(xml::Writer& out)
{
	if (code_.size() > 0) {
		out.newline();
		code_.write(out);
	}
	if (data_.size() > 0) {
		out.newline();
		data_.write(out);
	}
}

void CompilationUnit::read(xml::Iterator& it)
{
	code_.clear();
	data_.clear();
	if (it.current() == xml::Element::end) {return;}
	if (it.current() != xml::Element::tag) {throw exception("CompilationUnit::read invalid file");}
	if (it.tag() == "code") {
		code_.read(it);
		it.next_tag();
	}
	if (it.current() == xml::Element::end) {return;}
	if (it.current() != xml::Element::tag) {throw exception("CompilationUnit::read invalid file");}
	if (it.tag() == "data") {
		data_.read(it);
		it.next_tag();
	}
}

//*************************************************************************************



}
