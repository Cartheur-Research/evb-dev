#include "cvmc.hpp"

#include <sstream>


namespace cvmc {

ContextHelper::ContextHelper(context_type::iterator_type& it, context_type::iterator_type& last)
: iterator_(it)
, last_(last)
, backlog_position_(0)
{
	try {
		filter();
	}
	catch (boost::wave::preprocess_exception& ex) {
		handle_exception(ex);
	}
	catch (boost::wave::macro_handling_exception& ex) {
		handle_exception(ex);
	}
	update();
}

void ContextHelper::update()
{
	if (backlog_position_ >= backlog_.size()) {
		backlog_.push_back(*iterator_);
	}
	current_id_ = boost::wave::token_id(backlog_.at(backlog_position_));
	if (current_id_ == boost::wave::T_LESS) {
		int a = 0;
	}
	if (current_id_ == boost::wave::T_IDENTIFIER) {
		std::string id = current().get_value().c_str();
		return;
	}
}

const std::string& ContextHelper::identifier()
{
	if (current_id_ == boost::wave::T_IDENTIFIER) {
		identifier_ = current().get_value().c_str();
	}
	//else {
	//	identifier_.clear();
	//}
	return identifier_;
}

bool ContextHelper::is_left_paren()
{
	return current_id_ == boost::wave::T_LEFTPAREN;
}

bool ContextHelper::is_right_paren()
{
	return current_id_ == boost::wave::T_RIGHTPAREN;
}

bool ContextHelper::is_left_bracket()
{
	return current_id_ == boost::wave::T_LEFTBRACKET
		|| current_id_ == boost::wave::T_LEFTBRACKET_ALT
		|| current_id_ == boost::wave::T_LEFTBRACKET_TRIGRAPH
	;
}

bool ContextHelper::is_right_bracket()
{
	return current_id_ == boost::wave::T_RIGHTBRACKET
		|| current_id_ == boost::wave::T_RIGHTBRACKET_ALT
		|| current_id_ == boost::wave::T_RIGHTBRACKET_TRIGRAPH
	;
}

bool ContextHelper::is_left_brace()
{
	return current_id_ == boost::wave::T_LEFTBRACE
		|| current_id_ == boost::wave::T_LEFTBRACE_ALT
		|| current_id_ == boost::wave::T_LEFTBRACE_TRIGRAPH
	;
}

bool ContextHelper::is_right_brace()
{
	return current_id_ == boost::wave::T_RIGHTBRACE
		|| current_id_ == boost::wave::T_RIGHTBRACE_ALT
		|| current_id_ == boost::wave::T_RIGHTBRACE_TRIGRAPH
	;
}

bool ContextHelper::filter()
{
again:
	if (iterator_ != last_) {
		boost::wave::token_id id = boost::wave::token_id(*iterator_);
		if (id == boost::wave::T_SPACE) {++iterator_; goto again;}
		if (id == boost::wave::T_NEWLINE) {++iterator_; goto again;}
		if (id == boost::wave::T_PP_LINE)
		{
			while (++iterator_ != last_) {
				id = boost::wave::token_id(*iterator_);
				if (id == boost::wave::T_NEWLINE) {++iterator_; goto again;}
			}
			update();
			return false;
		}
		update();
		return true;
	}
	update();
	return false;
}

void ContextHelper::next()
{
	if (++backlog_position_ >= backlog_.size()) {
		if (iterator_ != last_) {
			try {
				++iterator_;
				if (filter()) {
				}
			}
			catch (boost::wave::preprocess_exception& ex) {
				handle_exception(ex);
			}
			catch (boost::wave::macro_handling_exception& ex) {
				handle_exception(ex);
			}
			return;
		}
	}
	update();
}

bool ContextHelper::finished() const
{
	if (current_id_ == boost::wave::T_EOF) {return true;}
	return false;
}

void ContextHelper::go_back_to(size_t pos)
{
	backlog_position_ = pos;
	update();
}

void ContextHelper::flush()
{
	backlog_position_ = 0;
	backlog_.clear();
	backlog_.push_back(*iterator_);
}

void ContextHelper::handle_exception(boost::wave::preprocess_exception& ex)
{
	std::ostringstream os;
	os << ex.file_name() << ":" << ex.line_no() << ":" << ex.column_no() << " " << ex.what() << " | " << ex.description();
	//switch (ex.get_errorcode()) {
	//}
	if (!ex.is_recoverable()) {
		IssueList::error(os.str());
		throw ex;
	}
	IssueList::warning(os.str());
}

void ContextHelper::handle_exception(boost::wave::macro_handling_exception& ex)
{
	std::ostringstream os;
	os << ex.file_name() << ":" << ex.line_no() << ":" << ex.column_no() << " " << ex.what() << " | " << ex.description();
	//switch (ex.get_errorcode()) {
	//}
	if (!ex.is_recoverable()) {
		IssueList::error(os.str());
		throw ex;
	}
	IssueList::warning(os.str());
}

//*************************************************************************************

Issue::Issue(const context_type::position_type& w1, const std::string& w2, bool is_error)
: RefObj(OT_ERROR)
, where_(w1)
, what_(w2)
, has_where_(true)
, is_error_(is_error)
{
}

Issue::Issue(const std::string& w2, bool is_error)
: RefObj(OT_ERROR)
, what_(w2)
, has_where_(false)
, is_error_(is_error)
{
}

void Issue::print(std::ostream& out)
{
	out << (is_error_ ? "error " : "warning ");
	if (has_where_) {
		out << where_ << " | ";
	}
	out << what_;
}

//*************************************************************************************

IssueList::IssueList()
{
}

IssueList& IssueList::error()
{
	static IssueList singleton;
	return singleton;
}

IssueList& IssueList::warning()
{
	static IssueList singleton;
	return singleton;
}

void IssueList::add(Issue* elem)
{
	data_.push_back(elem);
}

void IssueList::error(const context_type::position_type& w, const std::string& txt)
{
	error().add(new Issue(w, txt, true));
}

void IssueList::warning(const context_type::position_type& w, const std::string& txt)
{
	warning().add(new Issue(w, txt, false));
}

void IssueList::error(const std::string& txt)
{
	error().add(new Issue(txt, true));
}

void IssueList::warning(const std::string& txt)
{
	warning().add(new Issue(txt, false));
}

void IssueList::print(std::ostream& out)
{
	if (!data_.empty()) {
		for (size_t i=0; i<data_.size(); ++i) {
			issue(i)->print(out);
			out << std::endl;
		}
	}
}

//*************************************************************************************

//Fixup::Fixup()
//: type_(FIX_UNDEFINED)
//, offset_(0)
//, actual_offset_(0)
//, target_offset_(0)
//, literal_(0)
//{
//}
//
//Fixup::Fixup(FixupType t, const std::string& name, size_t offset)
//: type_(t)
//, name_(name)
//, offset_(offset)
//, actual_offset_(0)
//, target_offset_(0)
//, literal_(0)
//{
//}
//
//void Fixup::update(Code& code, Linker& lnk)
//{
//	if (type_ == FIX_STRING_LITERAL) {
//		code.fixup_string(lnk, *this);
//	}
//	else {
//		target_offset_ = lnk.fixup_position(target(), is_function());
//		code.fixup(*this, target_offset_);
//	}
//}
//	
//void Fixup::write(XMLWriter& out)
//{
//	out.out() << "<Fixup type='" << type_ << "'";
//	if (!name_.empty()) {
//		out.out() << " name='";
//		out.write(name_);
//		out.out() << "'";
//	}
//	if (literal_) {out.out() << " lit='" << literal_ << "'";}
//	if (offset_) {out.out() << " off='" << offset_ << "'";}
//	out.out() << "/>" ;
//}
//
//void Fixup::read(XMLIterator& it)
//{
//	ObjPtr obj;
//	if (it.current() != xml::Element::tag) {throw exception("Fixup::read invalid file");}
//	std::string tag = it.tag();
//	it.next();
//	while (it.current() == xml::Element::attribute) {
//		if (it.key() == "type") {type_ = static_cast<FixupType>(boost::lexical_cast<int>(it.value()));}
//		else if (it.key() == "name") {name_ = it.value();}
//		else if (it.key() == "lit") {literal_ = boost::lexical_cast<size_t>(it.value());}
//		else if (it.key() == "off") {offset_ = boost::lexical_cast<size_t>(it.value());}
//		it.next();
//	}
//	if ((it.current() != xml::Element::empty) || (it.tag() != tag)) {throw exception("Fixup::read invalid file");}
//}

//*************************************************************************************

ExtRef::ExtRef()
: type_(EXR_UNDEFINED)
, link_position_(0)
, offset_(0)
{
}

ExtRef::ExtRef(ExtRefType t)
: type_(t)
, link_position_(0)
, offset_(0)
{
}

ExtRef::ExtRef(ExtRefType t, const std::string& n)
: type_(t)
, link_position_(0)
, offset_(0)
, name_(n)
{
}

ExtRef::ExtRef(ExtRefType t, const std::string& n, size_t offs)
: type_(t)
, link_position_(0)
, offset_(offs)
, name_(n)
{
}

void ExtRef::write(xml::Writer& out)
{
	out.out() << "<XR type='" << type_ << "'";
	if (!name_.empty()) {out.out() << " name='" << name_ << "'";}
	if (offset_) {out.out() << " offset='" << offset_ << "'";}
	if (!fixup_positions_.empty()) {
		out.out() << " fixup='";
		//out.hex();
		size_t limit = fixup_positions_.size();
		out.out() << fixup_positions_[0];
		for (size_t i=1; i<limit; ++i) {
			out.out() << " " << fixup_positions_[i];
		}
		//out.dec();
		out.out() << "'";
	}
	out.out() << "/>" ;
}

void ExtRef::read(xml::Iterator& it)
{
	type_ = EXR_UNDEFINED;
	name_.clear();
	fixup_positions_.clear();
	if (it.current() != xml::Element::tag) {throw exception("Fixup::read invalid file");}
	std::string tag = it.tag();
	it.next();
	while (it.current() == xml::Element::attribute) {
		if (it.key() == "type") {type_ = static_cast<ExtRefType>(boost::lexical_cast<int>(it.value()));}
		else if (it.key() == "name") {name_ = it.value();}
		else if (it.key() == "offset") {type_ = static_cast<ExtRefType>(boost::lexical_cast<size_t>(it.value()));}
		else if (it.key() == "fixup") {
			std::stringstream ss(it.value());
			size_t pos;
			//ss >> std::hex;
			while (ss >> pos) {
				fixup_positions_.push_back(pos);
			}
		}
		it.next();
	}
	if ((it.current() != xml::Element::empty) || (it.tag() != tag)) {throw exception("Fixup::read invalid file");}
}

void ExtRef::link1(Linker& lnk, size_t offset)
{
	ObjPtr obj;
	switch (type_) {
	case EXR_STRING:
		link_position_ = lnk.add_string_literal(name_);
		break;

	case EXR_STATIC:
		obj = lnk.find(name_);
		if (obj.is_null()) {
			obj = lnk.lookup(name_);
			if (obj.is_null()) {
				throw exception("ExtRef::link1 cannot find object "+name_);
			}
			Static* s = dynamic_cast<Static*>(obj.get());
			if (s == 0) {
				throw exception("ExtRef::link1 object " + name_ + " is not a static variable");
			}
			lnk.add_pending(s);
		}
		break;

	case EXR_FUNCTION:
		obj = lnk.find(name_);
		if (obj.is_null()) {
			obj = lnk.lookup(name_);
			if (obj.is_null()) {
				throw exception("ExtRef::link1 cannot find object "+name_);
			}
			Function* f = dynamic_cast<Function*>(obj.get());
			if (f == 0) {
				throw exception("ExtRef::link1 object " + name_ + " is not a function");
			}
			lnk.add_pending(f);
		}
		break;

	default: throw exception("ExtRef::link1 invalid type");
	}
}

void ExtRef::link2(Linker& lnk, size_t code_offset)
{
	ObjPtr obj;
	Static* s = 0;
	Function* f = 0;
	size_t i, limit = fixup_positions_.size();
	switch (type_) {
	case EXR_STRING:
		for (i=0; i<limit; ++ i) {
			lnk.code().fixup_string_literal(fixup_positions_.at(i) + code_offset, link_position_ + offset_);
		}
		break;

	case EXR_STATIC:
		obj = lnk.find(name_);
		if (obj.is_null()) {
			throw exception("ExtRef::link2 cannot find static variable " + name_);
		}
		s = dynamic_cast<Static*>(obj.get());
		if (s == 0) {
			throw exception("ExtRef::link2 object " + name_ + " is not a static variable");
		}
		link_position_ = lnk.resolve(s);
		for (i=0; i<limit; ++ i) {
			lnk.code().fixup_global(fixup_positions_.at(i) + code_offset, link_position_+ offset_);
		}
		break;

	case EXR_FUNCTION:
		obj = lnk.find(name_);
		if (obj.is_null()) {
			throw exception("ExtRef::link2 cannot find function " + name_);
		}
		f = dynamic_cast<Function*>(obj.get());
		if (f == 0) {
			throw exception("ExtRef::link2 object " + name_ + " is not a function");
		}
		link_position_ = lnk.resolve(f);
		for (i=0; i<limit; ++ i) {
			lnk.code().fixup_call(fixup_positions_.at(i) + code_offset, link_position_+ offset_);
		}
		break;

	default: throw exception("ExtRef::link2 invalid type");
	}
}


//*************************************************************************************

XRefManager::XRefManager()
{
}

bool XRefManager::empty() const
{
	return string_literal_map_.empty() && xref_map_.empty();
}

void XRefManager::add(ExtRef* xr)
{
	if (xr->type() != EXR_STRING) {
		string_literal_map_[xr->name()] = xr;
	}
	else {
		xref_map_[xr->name()] = xr;
	}
	xref_list_.push_back(xr);
}

void XRefManager::write(xml::Writer& out)
{
	if (!empty()) {
		out.newline();
		out.out() << "<XRM>";
		out.indent(1);
		size_t limit = xrefs();
		for (size_t i=0; i<limit; ++i) {
			out.newline();
			xref(i)->write(out);
		}
		out.newline(-1);
		out.out() << "</XRM>";
	}
}

void XRefManager::read(xml::Iterator& it)
{
	ObjPtr obj;
	if (it.current() != xml::Element::tag) {throw exception("XRefManager::read invalid file");}
	std::string tag = it.tag();
	it.next_tag();
	while (it.current() == xml::Element::tag) {
		if (it.tag() == "XR") {
			ExtRef* xr = new ExtRef();
			obj = xr;
			xr->read(it);
			add(xr);
			it.next_tag();
		}
		else {
			throw exception("XRefManager::read invalid file");
		}
	}
	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw exception("Context::read invalid file");}
}

void XRefManager::add_string_literal(const std::string& txt, size_t pos)
{
  if (txt.empty()) {
  	throw exception("XRefManager::add_string_literal text is empty");
  }
	ExtRef* xref = 0;
	ObjPtr_map_type::iterator it = string_literal_map_.find(txt);
	if (it == string_literal_map_.end()) {
		xref = new ExtRef(EXR_STRING, txt);
		add(xref);
	}
	else {
		xref = static_cast<ExtRef*>(it->second.get());
		if (xref->type() != EXR_STRING) {
			throw exception("XRefManager::add_call xref is not a function");
		}
	}
	xref->add_fixup_position(pos);
}

void XRefManager::add_function(const std::string& name)
{
  if (name.empty()) {
  	throw exception("XRefManager::add_function name is empty");
  }
	ObjPtr_map_type::iterator it = xref_map_.find(name);
	if (it == xref_map_.end()) {
		ExtRef* xref = new ExtRef(EXR_FUNCTION, name);
		add(xref);
	}
}

void XRefManager::add_call(const std::string& name, size_t pos)
{
  if (name.empty()) {
  	throw exception("XRefManager::add_call name is empty");
  }
	ExtRef* xref = 0;
	ObjPtr_map_type::iterator it = xref_map_.find(name);
	if (it == xref_map_.end()) {
		xref = new ExtRef(EXR_FUNCTION, name);
		add(xref);
	}
	else {
		xref = static_cast<ExtRef*>(it->second.get());
		if (xref->type() != EXR_FUNCTION) {
			throw exception("XRefManager::add_call xref is not a function");
		}
	}
	xref->add_fixup_position(pos);
}

void XRefManager::add_static(const std::string& name)
{
  if (name.empty()) {
  	throw exception("XRefManager::add_static name is empty");
  }
	ObjPtr_map_type::iterator it = xref_map_.find(name);
	if (it == xref_map_.end()) {
		ExtRef* xref = new ExtRef(EXR_STATIC, name);
		add(xref);
	}
}

void XRefManager::add_static_ref(const std::string& name, size_t pos)
{
  if (name.empty()) {
  	throw exception("XRefManager::add_static_ref name is empty");
  }
	ExtRef* xref = 0;
	ObjPtr_map_type::iterator it = xref_map_.find(name);
	if (it == xref_map_.end()) {
		xref = new ExtRef(EXR_STATIC, name);
		add(xref);
	}
	else {
		xref = static_cast<ExtRef*>(it->second.get());
		if (xref->type() != EXR_STATIC) {
			throw exception("XRefManager::add_call xref is not a static variable");
		}
	}
	xref->add_fixup_position(pos);
}

void XRefManager::add_static_ref(const std::string& name, size_t pos, size_t offs)
{
  if (name.empty()) {
  	throw exception("XRefManager::add_static_ref name is empty");
  }
	ExtRef* xref = 0;
	ObjPtr_map_type::iterator it = xref_map_.find(name);
	if (it == xref_map_.end()) {
		xref = new ExtRef(EXR_STATIC, name);
		add(xref);
	}
	else {
		xref = static_cast<ExtRef*>(it->second.get());
		if (xref->type() != EXR_STATIC) {
			throw exception("XRefManager::add_call xref is not a static variable");
		}
	}
	xref->add_fixup_position(pos);
}

void XRefManager::link1(Linker& lnk, size_t offset)
{
	size_t limit = xrefs();
	for (size_t i=0; i<limit; ++i) {
		xref(i)->link1(lnk, offset);
	}
}

void XRefManager::link2(Linker& lnk, size_t offset)
{
	size_t limit = xrefs();
	for (size_t i=0; i<limit; ++i) {
		xref(i)->link2(lnk, offset);
	}
}

//*************************************************************************************

Declarator::Declarator()
{
}

void Declarator::declare()
{
	if (type_.is_not_null() && !name_.empty()) {
		context()->add_tag(name_, type());
	}
}

}
