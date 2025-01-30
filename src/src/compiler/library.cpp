#include "cvmc.hpp"

namespace cvmc {

Library::Library()
{
}

void Library::add(Context* c)
{
	context_list_.push_back(c);
}

ObjPtr Library::lookup(const std::string& name) const
{
	ObjPtr res;
  if (!name.empty()) {
	  size_t pos, limit = context();
	  for (pos=0; pos<limit; ++pos) {
		  res = context(pos)->lookup(name, name[0] == '_');
		  if (res.is_not_null()) {
			  return res;
		  }
	  }
  }
	return res;
}

void Library::write(xml::Writer& out)
{
	out.out() << "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>";
	out.newline();
	out.out() << "<?xml-stylesheet type='text/xsl' href='lib.xsl'?>";
	out.newline();
	out.out() << "<Library>";
	out.indent(1);
	if (!context_list_.empty()) {
		for (ObjPtr_list_type::iterator it=context_list_.begin(); it !=context_list_.end(); ++it) {
			it->get()->write(out);
		}
	}
	out.newline(-1);
	out.out() << "</Library>";
}

void Library::read(xml::Iterator& it)
{
	if (it.current() != xml::Element::tag) {throw exception("Context::read invalid file");}
	std::string tag = it.tag();
	it.next_tag();
	while (it.current() == xml::Element::tag) {
		if (it.tag() == "Context") {
			Context* cntxt = new Context();
			context_list_.push_back(cntxt);
			cntxt->read(it);
			it.next_tag();
		}
		else {
			throw exception("Context::read invalid file");
		}
	}
	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw exception("Context::read invalid file");}
}

}
