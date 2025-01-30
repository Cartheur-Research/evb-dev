#include "cvmc.hpp"

namespace cvmc {

//*************************************************************************************

EnumType::EnumType()
: Type(T_ENUM)
, size_(1)
, next_value_(0)
, min_(0)
, max_(0)
{
}

Type* EnumType::copy()
{
	EnumType* res = new EnumType();
	*res = *this;
	return res;
}

void EnumType::add(const std::string& name, IntConst* cval)
{
	ObjPtr obj(cval);
	int val  = 0;
	if (!cval->get_int(val)) {
		throw analyzer_exception("EnumType::add invalid value "+boost::lexical_cast<std::string>(val));
	}
	IntConst* old = member(name);
	if (old != 0) {
		//int val1, val2;
		//if (old->get_int(val1) && cval->get_int(val2) && (val1 == val2)) {
		//	next_value_ = val+1;
		//	return;
		//}
		throw analyzer_exception("EnumType::add duplicate name "+name);
	}
	if (value(val) != 0) {
		throw analyzer_exception("EnumType::add duplicate value "+boost::lexical_cast<std::string>(val));
	}
	if (no_of_member() == 0) {
		min_ = max_ = val;
	}
	else {
		if (val < min_) {min_ = val;}
		if (val > max_) {max_ = val;}
	}
	members_[name] = cval;
	values_[val] = cval;
	member_list_.push_back(cval);
	next_value_ = val+1;
}

IntConst* EnumType::add(const std::string& name)
{
	context_type::position_type pos;
	IntConst* val = new IntConst(pos, next_value_);
	add(name, val);
	return val;
}

size_t EnumType::size() const
{
	return size_;
}

IntConst* EnumType::member(size_t pos)
{
	if (pos < member_list_.size()) {return static_cast<IntConst*>(member_list_[pos].get());}
	return 0;
}

IntConst* EnumType::member(const std::string& name)
{
	ObjPtr_map_type::iterator it = members_.find(name);
	if (it != members_.end()) {
		return static_cast<IntConst*>(it->second.get());
	}
	return 0;
}

IntConst* EnumType::value(int v)
{
	ObjPtr_int_map_type::iterator it = values_.find(v);
	if (it != values_.end()) {
		return static_cast<IntConst*>(it->second.get());
	}
	return 0;
}

bool EnumType::is_integer() const
{
  return true;
}

bool EnumType::is_integer_or_pointer() const
{
  return true;
}


}
