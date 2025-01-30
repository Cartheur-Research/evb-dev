#include "cvmc.hpp"

namespace cvmc {

Context::Context()
: RefObj(OT_CONTEXT)
{
}

void Context::add_enumerator(const std::string& name, Constant* value)
{
	temp_objects_[name] = value;
}

Constant* Context::lookup_enumerator(const std::string& name) const
{
	ObjPtr_map_type::const_iterator it;
  it = objects_.find(name);
	if (it != objects_.end()) {
		return const_cast<Constant*>(dynamic_cast<const Constant*>(it->second.get()));
	}
  it = temp_objects_.find(name);
	if (it != temp_objects_.end()) {
		return const_cast<Constant*>(dynamic_cast<const Constant*>(it->second.get()));
	}
	return 0;
}

void Context::add_typedef(const std::string& name, Type* value)
{
	temp_typedefs_[name] = value;
}

Type* Context::lookup_typedef(const std::string& name) const
{
	ObjPtr_map_type::const_iterator it;
  it = typedefs_.find(name);
	if (it != typedefs_.end()) {
		return const_cast<Type*>(static_cast<const Type*>(it->second.get()));
	}
  it = temp_typedefs_.find(name);
	if (it != temp_typedefs_.end()) {
		return const_cast<Type*>(static_cast<const Type*>(it->second.get()));
	}
	return 0;
}

void Context::add_tag(const std::string& name, Type* value)
{
	temp_tags_[name] = value;
}

Type* Context::lookup_tag(const std::string& name) const
{
	ObjPtr_map_type::const_iterator it;
  it = tags_.find(name);
	if (it != tags_.end()) {
		return const_cast<Type*>(static_cast<const Type*>(it->second.get()));
	}
  it = temp_tags_.find(name);
	if (it != temp_tags_.end()) {
		return const_cast<Type*>(static_cast<const Type*>(it->second.get()));
	}
	return 0;
}

void Context::add(Function* value)
{
	temp_objects_[value->name()] = value;
}

void Context::add(Static* value)
{
	temp_objects_[value->name()] = value;
}

ObjPtr Context::lookup(const std::string& name, bool check_external) const
{
	ObjPtr_map_type::const_iterator it;
	it = objects_.find(name);
	if (it != objects_.end()) {
		return it->second;
	}
	it = temp_objects_.find(name);
	if (it != temp_objects_.end()) {
		return it->second;
	}
	if (check_external) {
    std::string ext_name = "_" + name;
		it = objects_.find(ext_name);
		if (it != objects_.end()) {
			return it->second;
		}
		it = temp_objects_.find(ext_name);
		if (it != temp_objects_.end()) {
			return it->second;
		}
	}
	return 0;
}

void Context::forget_temp()
{
  temp_tags_.clear();
  temp_typedefs_.clear();
  temp_objects_.clear();
}

void Context::save_temp()
{
  ObjPtr_map_type::iterator it;
  for (it=temp_tags_.begin(); it != temp_tags_.end(); ++it) {
    tags_[it->first] = it->second;
  }
  for (it=temp_typedefs_.begin(); it != temp_typedefs_.end(); ++it) {
    typedefs_[it->first] = it->second;
  }
  for (it=temp_objects_.begin(); it != temp_objects_.end(); ++it) {
    objects_[it->first] = it->second;
  }
}

//Function* Context::get_init()
//{
//	if (init_.is_null()) {
//		Function* f = new Function(this);
//		f->return_type(new Type(T_VOID));
//		init_ = f;
//	}
//	return static_cast<Function*>(init_.get());
//}


void Context::write(xml::Writer& out)
{
  ObjPtr_map_type::iterator it;
	out.newline();
	out.out() << "<Context>";
	out.indent(1);
	//if (!tags_.empty()) {
 //   for (it=tags_.begin(); it != tags_.end(); ++it) {
 //   }
 // }
	if (!objects_.empty()) {
	  Static* s;
	  Function* f;
		for (it=objects_.begin(); it !=objects_.end(); ++it) {
			f = dynamic_cast<Function*>(it->second.get());
			if (f) {
				if (!f->is_defined()) {
					continue;
				}
			}
			else {
				s = dynamic_cast<Static*>(it->second.get());
				if (s) {
					if (!s->is_defined()) {
						continue;
					}
				}
				else {
					continue;
				}
			}
			it->second.get()->write(out);
		}
	}
	out.newline(-1);
	out.out() << "</Context>";
}

void Context::read(xml::Iterator& it)
{
	ObjPtr obj;
	if (it.current() != xml::Element::tag) {throw exception("Context::read invalid file");}
	std::string tag = it.tag();
	it.next_tag();
	while (it.current() == xml::Element::tag) {
		if (it.tag() == "Function") {
			Function* f = new Function(this);
			obj = f;
			f->read(it);
			add(f);
			it.next_tag();
		}
		else if (it.tag() == "Static") {
			Static* s = new Static(this);
			obj = s;
			s->read(it);
			add(s);
			it.next_tag();
		}
		//else if (it.tag() == "Init") {
		//	Function* f = new Function(this);
		//	init_ = f;
		//	f->read(it);
		//}
		else {
			throw exception("Context::read invalid file");
		}
	}
	//it.next_tag();
	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw exception("Context::read invalid file");}
}

}
