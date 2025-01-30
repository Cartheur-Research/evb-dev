#include "cvmc.hpp"

namespace cvmc {

Static::Static(Context* c)
: context_(c)
{
}

Static::Static(Type* type, Context* c, const std::string& n)
: Variable(type)
, context_(c)
{
	name(n);
}

Static::Static(const context_type::position_type& w, Type* type, Context* c, const std::string& n)
: Variable(w, type)
, context_(c)
{
	name(n);
}

XRefManager* Static::manager()
{
	if (manager_.is_null()) {
		manager_ = new XRefManager();
	}
	return static_cast<XRefManager*>(manager_.get());
}

void Static::init_statement(Statement* val)
{
	init_statement_ = val;
	if (init_statement_.is_not_null() && init_cu_.is_null()) {
		CompilationUnit* cu = new CompilationUnit();
		init_cu_ = cu;
		//code->stat(this);
		cu->code().manager(manager());
		init_statement()->compile(*cu, false);
	}
}

void Static::add_code_fixup(Code& code, size_t offs)
{
	if (offs == 0) {
		code.manager()->add_static_ref(name_, code.size());
	}
	else {
		code.manager()->add_static_ref(name_, code.size(), offs);
	}
}

int Static::compile_adr(CompilationUnit& cu, size_t off)
{
	add_code_fixup(cu.code(), off);
	return cu.code().compile_adr_global(offset()+off);
}

int Static::compile_push(CompilationUnit& cu)
{
	int res = 0;
	throw not_implemented_exception("");
	return res;
}

int Static::compile_pop(CompilationUnit& cu)
{
	int res = 0;
	throw not_implemented_exception("");
	return res;
}

int Static::compile_push(CompilationUnit& cu, size_t off)
{
	int res = 0;
	throw not_implemented_exception("");
	return res;
}

int Static::compile_pop(CompilationUnit& cu, size_t off)
{
	int res = 0;
	throw not_implemented_exception("");
	return res;
}

int Static::compile_load(CompilationUnit& cu, bool push_result)
{
	int res = 0;
	size_t size = type_size();
	if (size == 1) {
		add_code_fixup(cu.code(), 0);
		res += cu.code().compile_adr_global(offset());
		res += cu.code().compile_load();
    if (push_result) {res += cu.code().compile_push_opt();}
	}
	else if (size == 2) {
    if (push_result) {
		  res += cu.code().compile_adr_global(offset()+1);
		  res += cu.code().compile_load();
      res += cu.code().compile_push();
		  res += cu.code().compile_adr_global(offset());
		  res += cu.code().compile_load();
      res += cu.code().compile_push();
    }
    else {
		  res += cu.code().compile_adr_global(offset());
		  res += cu.code().compile_load();
      res += cu.code().compile_xd();
		  res += cu.code().compile_adr_global(offset()+1);
		  res += cu.code().compile_load();
    }
  }
	else if (size == 2) {
    res += cu.code().compile_reserve(4);
		res += cu.code().compile_adr_global(offset());
    res += cu.code().compile_push();
	  cu.code().manager()->add_call("^load_longlong", cu.code().size());
	  res += cu.code().compile_call(0);
  }
	else {
		size_t pos = size-1;
		while (pos > 1) {
			add_code_fixup(cu.code(), pos);
			res += cu.code().compile_adr_global(offset()+pos);
			res += cu.code().compile_load();
			res += cu.code().compile_push_opt();
			--pos;
		}
		add_code_fixup(cu.code(), 0);
		res += cu.code().compile_adr_global(offset());
		res += cu.code().compile_load();
		res += cu.code().compile_xd();
		add_code_fixup(cu.code(), 1);
		res += cu.code().compile_adr_global(offset()+1);
		res += cu.code().compile_load();
		res += cu.code().compile_xd();
	}
	return res;
}

int Static::compile_store(CompilationUnit& cu)
{
	int res = 0;
	size_t size = type_size();
	if (size == 1) {
		res += cu.code().compile_push_opt();
		add_code_fixup(cu.code(), 0);
		res += cu.code().compile_adr_global(offset());
		res += cu.code().compile_store();
	}
	else {
		res += cu.code().compile_push_long_opt();
		add_code_fixup(cu.code(), 0);
		res += cu.code().compile_adr_global(offset());
		res += cu.code().compile_store();
		res += cu.code().compile_inc();
		res += cu.code().compile_store();
		size_t pos = 2;
		while (pos < size) {
			res += cu.code().compile_inc();
			res += cu.code().compile_store();
			++pos;
		}
	}

	//switch (size) {
	//	case 1:
	//		res += code.compile_push_opt();
	//		add_code_fixup(code);
	//		res += code.compile_adr_global(offset());
	//		res += code.compile_store();
	//		break;

	//	case 8:
	//		res += code.compile_push_long_opt();
	//		add_code_fixup(code);
	//		res += code.compile_adr_global(offset());
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		break;

	//	case 4:
	//		res += code.compile_push_long_opt();
	//		add_code_fixup(code);
	//		res += code.compile_adr_global(offset());
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		break;

	//	case 2:
	//		res += code.compile_push_long_opt();
	//		add_code_fixup(code);
	//		res += code.compile_adr_global(offset());
	//		res += code.compile_store();
	//		res += code.compile_inc();
	//		res += code.compile_store();
	//		break;

	//	default:;
	//}
	return res;
}

int Static::compile_load_word(CompilationUnit& cu, size_t off, bool push_result)
{
	int res = 0;
	add_code_fixup(cu.code(), off);
	res += cu.code().compile_adr_global(offset()+off);
	res += cu.code().compile_load();
  if (push_result) {res += cu.code().compile_push_opt();}
	return res;
}

int Static::compile_store_word(CompilationUnit& cu, size_t off)
{
	int res = 0;
	res += cu.code().compile_push_opt();
	add_code_fixup(cu.code(), off);
	res += cu.code().compile_adr_global(offset()+off);
	res += cu.code().compile_store();
	return res;
}

void Static::print(std::ostream& out)
{
	out << "static(" << name_ << ")";
}

void Static::write(xml::Writer& out)
{
	size_t sz = type_size();
	out.newline();
	out.out() << "<Static name='";
	out.write(name_);
	out.out() << "' size='" << sz << "'>";
	out.newline(1);
	out.out() << "<Type>";
	type()->write(out);
	out.out() << "</Type>";
	if (manager_.is_not_null()) {
		manager_.get()->write(out);
	}
	if (init_cu_.is_not_null()) {
  	out.newline();
		out.out() << "<Init>";
		init_cu_->write(out);
		out.out() << "</Init>";
	}
	if (init_statement_.is_not_null()) {
  	out.newline();
		out.out() << "<InitStmt>";
		init_statement_->write(out);
		out.out() << "</InitStmt>";
  }
	out.newline(-1);
	out.out() << "</Static>";
}

void Static::read(xml::Iterator& it)
{
	ObjPtr obj;
	if (it.current() != xml::Element::tag) {throw exception("Static::read invalid file");}
	std::string id, tag = it.tag();
	it.next();
	size_t size = 0;
	while (it.current() == xml::Element::attribute) {
		if (it.key() == "name") {name_ = it.value();}
		else if (it.key() == "size") {size = boost::lexical_cast<size_t>(it.value());}
		it.next();
	}
	if ((it.current() != xml::Element::content) || (it.tag() != tag)) {throw exception("Static::read invalid file");}
	it.next_tag();
	while (it.current() == xml::Element::tag) {
		if (it.tag() == "Type") {
			it.next_tag();
			Type::read(it, type_);
			if ((size > 0) && (size != type_size())) {throw exception("Static::read invalid size");}
			it.next_tag();
			if ((it.current() != xml::Element::end) || (it.tag() != "Type")) {throw exception("Static::read invalid file");}
			it.next_tag();
		}
		else if (it.tag() == "XRM") {
			manager()->read(it);
			it.next_tag();
		}
		else if (it.tag() == "Init") {
			init_cu_ = new CompilationUnit();
			it.next_tag();
			init_cu_->read(it);
			if ((it.current() != xml::Element::end) || (it.tag() != "Init")) {throw exception("Static::read invalid file");}
			it.next_tag();
		}
		else if (it.tag() == "InitStmt") {
			it.next_tag();
			init_statement_ = Statement::readFrom(it);
			if ((it.current() != xml::Element::end) || (it.tag() != "InitStmt")) {throw exception("Static::read invalid file");}
			it.next_tag();
    }
	}
	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw exception("Static::read invalid file");}
}

void Static::link1(Linker& lnk)
{
	lnk.add_static(this);
	if (manager_.is_not_null()) {
		manager()->link1(lnk, 0);
	}

}

void Static::link2(Linker& lnk)
{
	size_t link_offset = lnk.code().size();
	if (init()) {
		lnk.code().append(init()->code());
	}
	if (manager_.is_not_null()) {
		manager()->link2(lnk, link_offset);
	}
}


}
