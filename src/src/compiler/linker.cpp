#include "cvmc.hpp"

namespace cvmc {


Linker::Linker()
: entry_point_("_main")
, exit_point_("_poly_terminate")
, code_size_(0)
, data_size_(0)
, literal_size_(0)
, setup_size_(0)
, file_size_(0)
, literal_base_(0)
, setup_base_(0)
, code_base_(0)
, data_base_(0)
, big_endian_(true)
{
}

void Linker::add_library(Library* c)
{
	library_list_.push_back(c);
}

size_t Linker::resolve(Static* s)
{
	resolve_map_type::iterator it = static_map_.find(s);
	if (it == static_map_.end()) {
		throw exception("Linker::resolve static variable not found");
	}
	return it->second;
}

size_t Linker::resolve(Function* f)
{
	resolve_map_type::iterator it = function_map_.find(f);
	if (it == function_map_.end()) {
		throw exception("Linker::resolve static variable not found");
	}
	return it->second;
}

size_t Linker::resolve_data(Function* f)
{
	resolve_map_type::iterator it = function_data_map_.find(f);
	if (it == function_data_map_.end()) {
		throw exception("Linker::resolve_data data area not found");
	}
	return it->second;
}

size_t Linker::add_string_literal(const std::string& str)
{
	size_t pos = 0;
	string_map_type::iterator it = string_map_.find(str);
	if (it == string_map_.end()) {
		pos = code_.size();
		string_xref_map_[pos] = str;
		size_t size = str.size();
		if (!str.empty()) {
			for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
				code_.append_data(*it);
			}
		}
		code_.append_data(0);
		literal_size_ += size+1;
		string_map_[str] = pos;
	}
	else {
		pos = it->second;
	}
	return pos;
}

void Linker::add_static(Static* s)
{
	ObjPtr_map_type::iterator nit = object_map_.find(s->name());
	if (nit != object_map_.end()) {
		throw exception("Linker::add_static object "+s->name()+" already exists");
	}
	object_map_[s->name()] = s;

	resolve_map_type::iterator it = static_map_.find(s);
	if (it != static_map_.end()) {
		throw exception("Linker::add_static static variable "+s->name()+" already exists");
	}
	// reserve global space
	static_map_[s] = data_size_;
	static_xref_map_[data_size_] = s->name();
	data_size_ += s->type_size();
	// reserve setup space
	if (s->init()) {
		setup_size_ += s->init()->code().size();
	}
}

void Linker::add_function(Function* f)
{
	//if (f->compilation_unit() == 0) {
 //   return;
 // }
	ObjPtr_map_type::iterator nit = object_map_.find(f->name());
	if (nit != object_map_.end()) {
		throw exception("Linker::add_function object "+f->name()+" already exists");
	}
	object_map_[f->name()] = f;

	resolve_map_type::iterator it = function_map_.find(f);
	if (it != function_map_.end()) {
		throw exception("Linker::add_function function "+f->name()+" already exists");
	}
	// reserve code space (is not the final address yet)
	function_map_[f] = code_size_;
	if (f->compilation_unit() == 0) {
		throw exception("Linker::add_function function "+f->name()+" not defined");
	}
	size_t function_data_size = f->compilation_unit()->data().size();
	if (function_data_size > 0) {
		function_data_map_[f] = code_size_;
		code_size_ += function_data_size;
	}
	code_size_ += f->compilation_unit()->code().size();
}

RefObj* Linker::find(const std::string& name)
{
	ObjPtr_map_type::iterator it = object_map_.find(name);
	if (it != object_map_.end()) {
		return it->second.get();
	}
	return 0;
}


ObjPtr Linker::lookup(const std::string& name) const
{
	ObjPtr res;
	size_t pos, limit = library();
	for (pos=0; pos<limit; ++pos) {
		res = library(pos)->lookup(name);
		if (res.is_not_null()) {
			return res;
		}
	}
	return Library::lookup(name);
}

void Linker::add_pending(RefObj* obj)
{
	resolve_map_type::iterator it = pending_.find(obj);
	if (it == pending_.end()) {
		pending_[obj] = pending_list_.size();
		pending_list_.push_back(obj);
	}
}

void Linker::disassembly(std::ostream& out)
{
	size_t pos = 0;
	size_t limit = code_.size();

	code_.disassemble_data(out, pos, 1);
	out << "\tfile size\n";
	out << "startup\n";
	while (pos < literal_base_) {
		code_.disassemble_code(out, pos, 1, this);
		out << "\n";
	}
	out << "literal\n";
	while (pos < setup_base_) {
		code_.disassemble_string_literal(out, pos, 1, this);
		out << "\n";
	}
	out << "setup\n";
	while (pos < code_base_) {
		code_.disassemble_code(out, pos, 1, this);
		out << "\n";
	}
	out << "code\n";
	while (pos < data_base_) {
		code_.disassemble_code(out, pos, 1, this);
		out << "\n";
	}
	out << "global\n";
	while (pos < file_size_) {
		code_.disassemble_global(out, pos, 1, this);
		out << "\n";
	}
}

void Linker::import_library(const std::string& filename)
{
	std::ifstream in(filename.c_str(), ::std::ios::binary);
	xml::Iterator it(in);
  it.begin();
	read(it);
	in.close();
}

ExtRefType Linker::get_adr_info(size_t adr, std::string& name, size_t& offset)
{
	ExtRefType res = EXR_UNDEFINED;
	if (adr >= literal_base_) {
		xref_map_type::iterator it;
		if (adr < code_base_) {
			it = string_xref_map_.lower_bound(adr);
			if (it == string_xref_map_.end()) {
				it = string_xref_map_.find(string_xref_map_.rbegin()->first);
			}
			res = EXR_STRING;
			offset = adr - it->first;
		}
		else if (adr < data_base_) {
			it = function_xref_map_.lower_bound(adr);
			if (it == function_xref_map_.end()) {
				it = function_xref_map_.find(function_xref_map_.rbegin()->first);
			}
			res = EXR_FUNCTION;
			offset = adr - it->first;
		}
		else {
			it = static_xref_map_.upper_bound(adr - data_base_);
			--it;
			if (it == static_xref_map_.end()) {
				it = static_xref_map_.find(static_xref_map_.rbegin()->first);
			}
			res = EXR_STATIC;
			offset = adr - it->first - data_base_;
		}
		name = it->second;
	}
	return res;
}

ObjPtr Linker::resolve(ObjPtr ptr)
{
  Function* f = dynamic_cast<Function*>(ptr.get());
  if (f) {
    if (f->compilation_unit() == 0) {
      const std::string& name = f->name();
      size_t limit3 = context();
      for (size_t i3=0; i3<limit3; ++i3) {
        Context* cntxt = context(i3);
        Function* fun = dynamic_cast<Function*>(cntxt->lookup(name, name[0] != '_').get());
        if ((fun != 0) && (fun->compilation_unit() != 0)) {
          return fun;
        }
      }

      size_t limit1 = library();
      for (size_t i1=0; i1<limit1; ++i1) {
        Library* lib = library(i1);
        size_t limit2 = lib->context();
        for (size_t i2=0; i2<limit2; ++i2) {
          Context* cntxt = lib->context(i2);
          Function* fun = dynamic_cast<Function*>(cntxt->lookup(name, name[0] != '_').get());
          if ((fun != 0) && (fun->compilation_unit() != 0)) {
            return fun;
          }
        }
      }
    }
  }
  return ptr;
}

void Linker::link()
{
	pending_list_.clear();
	pending_.clear();
	static_map_.clear();
	function_map_.clear();
	function_data_map_.clear();
	string_map_.clear();
	string_xref_map_.clear();
	static_xref_map_.clear();
	function_xref_map_.clear();
	resolved_pos_ = 0;
	code_size_ = 0;
	data_size_ = 0;
	literal_size_ = 0;
	setup_base_ = 0;
	setup_size_ = 0;
	code_base_ = 0;
	data_base_ = 0;
	literal_base_ = 0;

	// startup sequence
	code_.append_data(0); // placeholder for file length
	// setup stack pointer
	code_.compile_literal(-1);
	code_.compile_xs();
	// setup frame pointer
	code_.compile_literal(-1);
	code_.compile_xf();
	// setup global pointer
	size_t global_init_pos = code_.size()+1;
	code_.compile_literal(0, false);
	code_.compile_xg();
	// skip text
	size_t skip_pos = code_.size();
	code_.compile_branch();
	//
	//text_offset_ = text_.size();

  // filler	
 	code_.append_data(0); // current new pointer

  // new pointer
	size_t new_init_pos = code_.size();
 	code_.append_data(0); // current new pointer
 	code_.append_data(0); // initial new pointer

	// reserve space for communication with polyforth
	code_.append_data(0); // action code
	code_.append_data(0); // memory base
	code_.append_data(0); // memory size
	code_.append_data(0); // result code

	literal_base_ = code_.size();
	// start with entry
	ObjPtr obj = lookup(entry_point_);
	if (obj.is_null()) {
		throw exception("Linker::write cannot find entry point "+entry_point_);
	}
	pending_list_.push_back(obj);
	obj = lookup(exit_point_);
	if (obj.is_null()) {
		throw exception("Linker::write cannot find exit point "+exit_point_);
	}
	pending_list_.push_back(obj);
	// pass 1 resolves all references and add literals
	while (resolved_pos_ < pending_list_.size()) {
		obj = pending_list_.at(resolved_pos_);
    obj = resolve(obj);
    pending_list_[resolved_pos_] = obj;
		obj->link1(*this);
    ++resolved_pos_;
	}
	setup_base_ = code_.size();
	// skip text to his position
	code_.fixup_branch(skip_pos, code_.size());
  // code for new pointer initialization
	code_.compile_literal(new_init_pos+1);
  code_.compile_load_push();
	code_.compile_literal(new_init_pos);
  code_.compile_store();

	// now we can add the startup code for all static variables (pass2 for static variables)
	resolved_pos_ = 0;
	while (resolved_pos_ < pending_list_.size()) {
		obj = pending_list_.at(resolved_pos_++);
		Static* s = dynamic_cast<Static*>(obj.get());
		if (s) {
			s->link2(*this);
		}
	}

	// finalize initalisation routine
	size_t entry_call_pos = code_.size();
	code_.compile_call(0);
  code_.compile_push();
	size_t exit_call_pos = code_.size();
	code_.compile_call(0);
  // call exit


	//size_t restart_pos = code_.size();
	//code_.compile_branch();
	//code_.fixup_branch(restart_pos, 1);

	// now we can adjust all function address
	code_base_ = code_.size();	// this is the value that must be added to all function addresses to get the final address
	size_t local_adr, final_adr;
	resolved_pos_ = 0;
	while (resolved_pos_ < pending_list_.size()) {
		obj = pending_list_.at(resolved_pos_++);
		Function* f = dynamic_cast<Function*>(obj.get());
		if (f) {
			local_adr = function_map_[f];
			final_adr = local_adr + code_base_;
			function_map_[f] = final_adr;
			function_xref_map_[final_adr] = f->name();
		}
	}


	// finally we can process pass 2 for all functions
	resolved_pos_ = 0;
	while (resolved_pos_ < pending_list_.size()) {
		obj = pending_list_.at(resolved_pos_++);
		Function* f = dynamic_cast<Function*>(obj.get());
		if (f) {
			f->link2(*this);
		}
	}
	data_base_ = code_.size();
	// reserve room for data
	for (size_t i=0; i<data_size_; ++i) {
		code_.append_data(0);
	}
	file_size_ = code_.size();

  // fixup entry
	Function* f = dynamic_cast<Function*>(pending_list_.at(0).get());
	if (f) {
		code_.fixup_call(entry_call_pos, function_map_[f]);
	}
	else {
		throw exception("Linker::write cannot link entry point");
	}

  // fixup exit
	f = dynamic_cast<Function*>(pending_list_.at(1).get());
	if (f) {
		code_.fixup_call(exit_call_pos, function_map_[f]);
	}
	else {
		throw exception("Linker::write cannot link exit point");
	}

	// final fixup
	code_.data(global_init_pos, data_base_);
	code_.data(new_init_pos, file_size_);
	code_.data(new_init_pos+1, file_size_);
	code_.data(0, file_size_*2);


	// file layout
	//	+---------------------------+
	//	|		total size			|
	//	+---------------------------+
	//	|		init s				|
	//	|		init f				|
	//	|		init g				|
	//	|		init n				|
	//	|		br setup			|
	//	+---------------------------+
	//	|		literal				|
	//	~							~
	//	~							~
	//	|		literal				|
	//	+---------------------------+
	//	|		setup				|
	//	~							~
	//	~							~
	//	|		setup				|
	//	|		call entry			|
	//	|		br @1				|
	//	+---------------------------+
	//	|		code				|
	//	~							~
	//	~							~
	//	|		code				|
	//	+---------------------------+
	//	|		data				|
	//	~							~
	//	~							~
	//	|		data				|
	//	+---------------------------+

}

void Linker::write(std::ostream& out)
{
	code_.write(out, big_endian_);
}

void Linker::read(xml::Iterator& it)
{
	//library_list_.clear();
	//if (!it.start(true)) {throw exception("Library::read invalid file");}
	//it.next_tag();
	//if (it.current() == XML_PROCESSING_INSTRUCTION) {
	//	it.next_tag();
	//	while (it.current() == xml::Element::attribute) {
	//		it.next_tag();
	//	}
	//	if (it.current() != xml::Element::end) {throw exception("Context::read invalid file");}
	//	it.next_tag();
	//}
	if (it.current() != xml::Element::tag) {throw exception("Context::read invalid file");}
	std::string tag = it.tag();
	Library* lib = new Library();
	library_list_.push_back(lib);
	lib->read(it);
}

}
