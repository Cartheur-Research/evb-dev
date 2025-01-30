#include "cvmc.hpp"

namespace cvmc {

//*************************************************************************************

UnionStructMember::UnionStructMember(const context_type::position_type& w)
: Constant(w)
, offset_(0)
{
}

UnionStructMember::UnionStructMember(const context_type::position_type& w, Type* t)
: Constant(w)
, offset_(0)
{
	set_type(t);
}

UnionStructMember::UnionStructMember(Type* t)
: offset_(0)
{
	set_type(t);
}

bool UnionStructMember::is_negative() const
{
	return false;
}

bool UnionStructMember::get_int(int& val) const
{
	val = offset_;
	return static_cast<size_t>(val) == offset_;
}

void UnionStructMember::write(xml::Writer& out)
{
  out.out() << "<Member name=\"" << name() << "\" offset=\"" << offset_ << "\">";
  type()->write(out);
	out.out() << "</Member";
}

void UnionStructMember::read(xml::Iterator& it)
{
}

bool UnionStructMember::is_recursive_equivalent_with(UnionStructMember& usm, std::multimap<void*, void*>& map)
{
  return type()->is_recursive_equivalent_with(*usm.type(), map);
}

//*************************************************************************************

UnionStructType::UnionStructType(const std::string& name, bool is_union)
: Type(T_STRUCT)
, name_(name)
, size_(0)
, is_union_(is_union)
{
}

Type* UnionStructType::copy()
{
	return new UnionStructType(*this);
}

void UnionStructType::add(const std::string& name, UnionStructMember* mem)
{
	if (member(name) != 0) {
		throw analyzer_exception("StructType::add duplicate name "+name);
	}
	members_[name] = mem;
	member_list_.push_back(mem);
	size_t ts = mem->type_size();
	if (is_union_) {
		mem->offset(0);
		if (ts > size_) {size_ = ts;}
	}
	else {
		mem->offset(size_);
		size_ += ts;
	}
}

size_t UnionStructType::size() const
{
	return size_;
}

UnionStructMember* UnionStructType::member(size_t pos)
{
	if (pos < member_list_.size()) {return static_cast<UnionStructMember*>(member_list_[pos].get());}
	return 0;
}

UnionStructMember* UnionStructType::member(const std::string& name)
{
	ObjPtr_map_type::iterator it = members_.find(name);
	if (it != members_.end()) {
		return static_cast<UnionStructMember*>(it->second.get());
	}
	return 0;
}

bool UnionStructType::is_equivalent_with(Type& t)
{
  std::multimap<void*, void*> map;
  return is_recursive_equivalent_with(t, map);
}

bool UnionStructType::is_recursive_equivalent_with(Type& t, std::multimap<void*, void*>& map)
{
  UnionStructType* ust = dynamic_cast<UnionStructType*>(&t);
  if (ust == 0) {return false;}
  size_t limit = no_of_member();
  if (limit != ust->no_of_member()) {
    return false;
  }
  for (size_t i=0; i<limit; ++i) {
    if (!member(i)->is_recursive_equivalent_with(*ust->member(i), map)) {
      return false;
    }
  }
  return true;
}

void UnionStructType::write(xml::Writer& out)
{
  if (is_union_) {
		out.out() << "<Union name=\"";
  }
  else {
		out.out() << "<Struct name=\"";
  }
  out.out() << name_;
  out.out() << "\"/>";
 // if (out.visited(name_)) {
	//	out.out() << "\"/>";
 //   return;
 // }
	//out.out() << "\">";
	//out.indent(1);
 // for (size_t i=0; i<size_; ++i) {
	//	out.newline();
 //   member(i)->write(out);
 // }
 // // first appearance of this name
	//out.newline(-1);
 // if (is_union_) {
	//	out.out() << "</Union>";
 // }
 // else {
	//	out.out() << "</Struct>";
 // }
}

void UnionStructType::read(xml::Iterator& it)
{
	if (it.current() != xml::Element::tag) {throw exception("UnionStructType::read invalid file");}
	std::string tag = it.tag();
	it.next();
	while (it.current() == xml::Element::attribute) {
		if (it.key() == "name") {name_ = it.value();}
		it.next();
	}
  if (name_.empty()) {
    throw exception("UnionStructType::read no name specified");
  }
	if (it.current() == xml::Element::empty) {
    return;
  }
	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {
    throw exception("UnionStructType::read invalid closing tag");
  }

}

}
