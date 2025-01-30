#include "cvmc.hpp"
#include <sstream>

namespace cvmc {

#define	PRINT

Function::Function(Context* cntxt)
: context_(cntxt)
, local_size_(0)
, max_local_size_(0)
, parameter_size_(0)
, specifier_mask_(0)
, is_defined_(false)
{
}

Function::Function(Context* cntxt, const std::string& name)
: context_(cntxt)
, name_(name)
, local_size_(0)
, max_local_size_(0)
, parameter_size_(0)
, specifier_mask_(0)
, is_defined_(false)
{
}

bool Function::is_external() const
{
	if (!name_.empty()) {
		switch (name_[0]) {
		case '_':
		case '^':
			return true;
		default:;
		}
	}
	return false;
}

bool Function::is_system() const
{
	return !name_.empty() && (name_[0] == '^');
}

void Function::name(const std::string& t)
{
  name_ = t;
}

XRefManager* Function::manager()
{
	if (manager_.is_null()) {
		manager_ = new XRefManager();
	}
	return static_cast<XRefManager*>(manager_.get());
}

void Function::add_statement(Statement* value)
{
	if (content_.is_null()) {
		content_ = new Block();
	}
	Block* blk = dynamic_cast<Block*>(content());
	if (blk == 0) {throw exception("Function::add_statement no block");}
	blk->add(value);
}

void Function::add_local(const std::string& name, Local* loc)
{
#ifdef	PRINT
		std::cout << "Function::add_local " << name << std::endl;
#endif
	local_list_.push_back(loc);
	locals_[name] = loc; 
	local_size_ += loc->type_size();
	loc->offset(local_size_-1);
	if (local_size_ > max_local_size_) {max_local_size_ = local_size_;}
}

Local* Function::lookup_local(const std::string& name) const
{
	ObjPtr_map_type::const_iterator it = locals_.find(name);
	if (it != locals_.end()) {
		return const_cast<Local*>(static_cast<const Local*>(it->second.get()));
	}
	return 0;
}

void Function::add_parameter(const std::string& name, Parameter* value)
{
#ifdef	PRINT
		std::cout << "Function::add_parameter " << name << std::endl;
#endif
	size_t size = value->type_size();
	// push up all parameter by size
	size_t limit = parameter();
	for (size_t i=0; i<limit; ++i) {
		Parameter* par = parameter(i);
		par->offset(par->offset() + size);
	}
	// remember parameter name
	if (!name.empty()) {
		parameter_[name] = value;
		value->name(name);
	}
	value->offset(0);
	parameter_size_ += size;
	parameter_list_.push_back(value);
}

void Function::clear_parameter_names()
{
	size_t limit = parameter_.size();
	for (size_t i=0; i<limit; ++i) {
		Parameter* par = parameter(i);
		par->name("");
	}
  parameter_.clear();
}

void Function::set_parameter(const std::string& name, Parameter* value)
{
#ifdef	PRINT
		std::cout << "Function::set_parameter " << name << std::endl;
#endif
	// remember parameter name
	if (!name.empty()) {
		parameter_[name] = value;
		value->name(name);
	}
}
Parameter* Function::lookup_parameter(const std::string& name) const
{
	ObjPtr_map_type::const_iterator it = parameter_.find(name);
	if (it != parameter_.end()) {
		return const_cast<Parameter*>(static_cast<const Parameter*>(it->second.get()));
	}
	return 0;
}

void Function::add_label(const std::string& name, Label* lbl)
{
#ifdef	PRINT
		std::cout << "Function::add_label " << name << std::endl;
#endif
	labels_[name] = lbl;
	label_list_.push_back(lbl);
}

Label* Function::lookup_label(const std::string& name) const
{
	ObjPtr_map_type::const_iterator it = parameter_.find(name);
	if (it != parameter_.end()) {
		return const_cast<Label*>(static_cast<const Label*>(it->second.get()));
	}
	return 0;
}

//size_t Function::all_parameter() const
//{
//	int res = 0;
//	size_t limit = parameter_.size();
//	const Parameter* par;
//	for (size_t i=0; i<limit; ++i) {
//		par = parameter(i);
//		res += const_cast<Parameter*>(par)->type_size();
//	}
//	return res;
//}

void Function::optimize(ObjPtr& ptr, int mask)
{
	content_.optimize(mask);
}

void Function::optimize(int mask)
{
	content_.optimize(mask);
}

int Function::compile()
{
	CompilationUnit* cu = new CompilationUnit(this);
	compilation_unit(cu);
	cu->code().manager(manager());
	compile(*cu, false);
	return 0;
}

int Function::compile(CompilationUnit& cu, bool push_result)
{
	if (!is_defined()) {return 0;}
	int res = 0;
	if (content_.is_not_null()) {
		res += cu.code().compile_enter(max_local_size_);
		res += content()->compile(cu, false);
		if (!content()->last_is_return()) {
			res += cu.code().compile_exit(parameter_size_);
		}
	}
	else {
		res += cu.code().compile_return();
	}
	return res;
}

void Function::print(std::ostream& out)
{
	return_type()->print(out);
	out << " " << name_ << "(";
	if (is_defined()) {
		out << std::endl << "{" << std::endl;
		out << "}" << std::endl;
	}
	else {
		out << ";" << std::endl;
	}
}

UnionStructType* Function::get_ulmul_t()
{
static ObjPtr ulmul_t;
  if (ulmul_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_ulmul_t", false);
    ust->add("hi", new UnionStructMember(Type::basic(T_ULONG)));
    ust->add("lo", new UnionStructMember(Type::basic(T_ULONG)));
    ulmul_t = ust;
  }
  return static_cast<UnionStructType*>(ulmul_t.get());
}

UnionStructType* Function::get_ullmul_t()
{
static ObjPtr ullmul_t;
  if (ullmul_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_ullmul_t", false);
    ust->add("hi", new UnionStructMember(Type::basic(T_ULONGLONG)));
    ust->add("lo", new UnionStructMember(Type::basic(T_ULONGLONG)));
    ullmul_t = ust;
  }
  return static_cast<UnionStructType*>(ullmul_t.get());
}

UnionStructType* Function::get_div_t()
{
static ObjPtr div_t;
  if (div_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_div_t", false);
    ust->add("quot", new UnionStructMember(Type::basic(T_INT)));
    ust->add("rem", new UnionStructMember(Type::basic(T_INT)));
    div_t = ust;
  }
  return static_cast<UnionStructType*>(div_t.get());
}

UnionStructType* Function::get_ldiv_t()
{
static ObjPtr ldiv_t;
  if (ldiv_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_ldiv_t", false);
    ust->add("quot", new UnionStructMember(Type::basic(T_LONG)));
    ust->add("rem", new UnionStructMember(Type::basic(T_LONG)));
    ldiv_t = ust;
  }
  return static_cast<UnionStructType*>(ldiv_t.get());
}

UnionStructType* Function::get_lldiv_t()
{
static ObjPtr lldiv_t;
  if (lldiv_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_lldiv_t", false);
    ust->add("quot", new UnionStructMember(Type::basic(T_LONGLONG)));
    ust->add("rem", new UnionStructMember(Type::basic(T_LONGLONG)));
    lldiv_t = ust;
  }
  return static_cast<UnionStructType*>(lldiv_t.get());
}

UnionStructType* Function::get_udiv_t()
{
static ObjPtr udiv_t;
  if (udiv_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_udiv_t", false);
    ust->add("quot", new UnionStructMember(Type::basic(T_UINT)));
    ust->add("rem", new UnionStructMember(Type::basic(T_UINT)));
    udiv_t = ust;
  }
  return static_cast<UnionStructType*>(udiv_t.get());
}

UnionStructType* Function::get_uldiv_t()
{
static ObjPtr uldiv_t;
  if (uldiv_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_uldiv_t", false);
    ust->add("quot", new UnionStructMember(Type::basic(T_ULONG)));
    ust->add("rem", new UnionStructMember(Type::basic(T_ULONG)));
    uldiv_t = ust;
  }
  return static_cast<UnionStructType*>(uldiv_t.get());
}

UnionStructType* Function::get_ulldiv_t()
{
static ObjPtr ulldiv_t;
  if (ulldiv_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_ulldiv_t", false);
    ust->add("quot", new UnionStructMember(Type::basic(T_ULONGLONG)));
    ust->add("rem", new UnionStructMember(Type::basic(T_ULONGLONG)));
    ulldiv_t = ust;
  }
  return static_cast<UnionStructType*>(ulldiv_t.get());
}

UnionStructType* Function::get_float_t()
{
static ObjPtr float_t;
  if (float_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_float_t", false);
    ust->add("classify", new UnionStructMember(Type::basic(T_INT)));
    ust->add("sign", new UnionStructMember(Type::basic(T_BOOL)));
    ust->add("exp", new UnionStructMember(Type::basic(T_INT)));
    ust->add("mant", new UnionStructMember(Type::basic(T_ULONG)));
    float_t = ust;
  }
  return static_cast<UnionStructType*>(float_t.get());
}

UnionStructType* Function::get_double_t()
{
static ObjPtr double_t;
  if (double_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_double_t", false);
    ust->add("classify", new UnionStructMember(Type::basic(T_INT)));
    ust->add("sign", new UnionStructMember(Type::basic(T_BOOL)));
    ust->add("exp", new UnionStructMember(Type::basic(T_INT)));
    ust->add("mant", new UnionStructMember(Type::basic(T_ULONGLONG)));
    double_t = ust;
  }
  return static_cast<UnionStructType*>(double_t.get());
}

UnionStructType* Function::get_longdouble_t()
{
static ObjPtr longdouble_t;
  if (longdouble_t.is_null()) {
    UnionStructType* ust = new UnionStructType("_longdouble_t", false);
    ust->add("classify", new UnionStructMember(Type::basic(T_INT)));
    ust->add("sign", new UnionStructMember(Type::basic(T_BOOL)));
    ust->add("exp", new UnionStructMember(Type::basic(T_INT)));
    ust->add("mant_hi", new UnionStructMember(Type::basic(T_ULONGLONG)));
    ust->add("mant_lo", new UnionStructMember(Type::basic(T_ULONGLONG)));
    longdouble_t = ust;
  }
  return static_cast<UnionStructType*>(longdouble_t.get());
}

Function* Function::system_bool(const std::string& name)
{
static ObjPtr predec_bool;
static ObjPtr preinc_bool;
static ObjPtr postdec_bool;
static ObjPtr postinc_bool;
	Function* f = 0;
	if (name == "^predec_bool") {
		if (predec_bool.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_INT)))));
			predec_bool = f; 
		}
		f = static_cast<Function*>(predec_bool.get());
	}
	else if (name == "^preinc_bool") {
		if (preinc_bool.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_INT)))));
			preinc_bool = f; 
		}
		f = static_cast<Function*>(preinc_bool.get());
	}
	else if (name == "^postdec_bool") {
		if (postdec_bool.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_INT)))));
			postdec_bool = f; 
		}
		f = static_cast<Function*>(postdec_bool.get());
	}
	else if (name == "^postinc_bool") {
		if (postinc_bool.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_INT)))));
			postinc_bool = f; 
		}
		f = static_cast<Function*>(postinc_bool.get());
	}
  return f;
}

Function* Function::system_int(const std::string& name)
{
static ObjPtr predec_int;
static ObjPtr preinc_int;
static ObjPtr postdec_int;
static ObjPtr postinc_int;
static ObjPtr switch_int;
static ObjPtr shl_int;
static ObjPtr shr_int;
static ObjPtr shr_uint;
static ObjPtr mul_int;
static ObjPtr div_int;
static ObjPtr addassign_int;
static ObjPtr compressed_switch_int;
// int
	Function* f = 0;
	if (name == "^predec_int") {
		if (predec_int.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_INT));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_INT)))));
			predec_int = f; 
		}
		f = static_cast<Function*>(predec_int.get());
	}
	else if (name == "^preinc_int") {
		if (preinc_int.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_INT));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_INT)))));
			preinc_int = f; 
		}
		f = static_cast<Function*>(preinc_int.get());
	}
	else if (name == "^postdec_int") {
		if (postdec_int.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_INT));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_INT)))));
			postdec_int = f; 
		}
		f = static_cast<Function*>(postdec_int.get());
	}
	else if (name == "^postinc_int") {
		if (postinc_int.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_INT));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_INT)))));
			postinc_int = f; 
		}
		f = static_cast<Function*>(postinc_int.get());
	}
	else if (name == "^switch_int") {
		if (switch_int.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_INT));
			f->add_parameter("limit", new Parameter(Type::basic(T_INT)));
			f->add_parameter("value", new Parameter(Type::basic(T_INT)));
			switch_int = f; 
		}
		f = static_cast<Function*>(switch_int.get());
	}
	else if (name == "^shl_int") {
		if (shl_int.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_INT));
			f->add_parameter("value", new Parameter(Type::basic(T_INT)));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			shl_int = f; 
		}
		f = static_cast<Function*>(shl_int.get());
	}
	else if (name == "^shr_int") {
		if (shr_int.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_INT));
			f->add_parameter("value", new Parameter(Type::basic(T_INT)));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			shr_int = f; 
		}
		f = static_cast<Function*>(shr_int.get());
	}
	else if (name == "^shr_uint") {
		if (shr_uint.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_UINT));
			f->add_parameter("value", new Parameter(Type::basic(T_UINT)));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			shr_uint = f; 
		}
		f = static_cast<Function*>(shr_uint.get());
	}
	else if (name == "^mul_int") {
		if (mul_int.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_INT)));
			f->add_parameter("b", new Parameter(Type::basic(T_INT)));
			//f->add_parameter("res", new Parameter(new ReferenceType(T_POINTER, get_umul_t())));
			mul_int = f; 
		}
		f = static_cast<Function*>(mul_int.get());
	}
	else if (name == "^div_int") {
		if (div_int.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_INT)));
			f->add_parameter("b", new Parameter(Type::basic(T_INT)));
			f->add_parameter("res", new Parameter(new ReferenceType(T_POINTER, get_div_t())));
			div_int = f; 
		}
		f = static_cast<Function*>(div_int.get());
	}
	else if (name == "^compressed_switch_int") {
		if (compressed_switch_int.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("value", new Parameter(Type::basic(T_INT)));
			f->add_parameter("limit", new Parameter(Type::basic(T_INT)));
			compressed_switch_int = f; 
		}
		f = static_cast<Function*>(compressed_switch_int.get());
	}
  return f;
}

Function* Function::system_uint(const std::string& name)
{
static ObjPtr mul_uint;
static ObjPtr div_uint;
	Function* f = 0;
	if (name == "^mul_uint") {
		if (mul_uint.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_ULONG));
			f->add_parameter("a", new Parameter(Type::basic(T_UINT)));
			f->add_parameter("b", new Parameter(Type::basic(T_UINT)));
			//f->add_parameter("res", new Parameter(new ReferenceType(T_POINTER, get_umul_t())));
			mul_uint = f; 
		}
		f = static_cast<Function*>(mul_uint.get());
	}
	else if (name == "^div_uint") {
		if (div_uint.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_UINT)));
			f->add_parameter("b", new Parameter(Type::basic(T_UINT)));
			f->add_parameter("res", new Parameter(new ReferenceType(T_POINTER, get_udiv_t())));
			div_uint = f; 
		}
		f = static_cast<Function*>(div_uint.get());
	}
  return f;
}

Function* Function::system_long(const std::string& name)
{
static ObjPtr load_long;
static ObjPtr store_long;
static ObjPtr add_long;
static ObjPtr sub_long;
static ObjPtr bitand_long;
static ObjPtr bitnot_long;
static ObjPtr bitor_long;
static ObjPtr bitxor_long;
static ObjPtr addassign_long;
static ObjPtr subassign_long;
static ObjPtr bitandassign_long;
static ObjPtr bitorassign_long;
static ObjPtr bitxorassign_long;
static ObjPtr dec_long;
static ObjPtr inc_long;
static ObjPtr neg_long;
static ObjPtr postdec_long;
static ObjPtr postinc_long;
static ObjPtr predec_long;
static ObjPtr preinc_long;
static ObjPtr eq_long;
static ObjPtr ne_long;
static ObjPtr shl_long;
static ObjPtr shlp_long;
static ObjPtr shr_long;
static ObjPtr shrp_long;
static ObjPtr mul_long;
static ObjPtr div_long;
static ObjPtr mod_long;
static ObjPtr lt0_long;
static ObjPtr le0_long;
static ObjPtr gt0_long;
static ObjPtr ge0_long;
static ObjPtr eq0_long;
static ObjPtr ne0_long;
static ObjPtr lt_long;
static ObjPtr le_long;
static ObjPtr gt_long;
static ObjPtr ge_long;
	Function* f = 0;
	if (name == "^load_long") {
		if (load_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("src", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_VOID))));
			load_long = f; 
		}
		f = static_cast<Function*>(load_long.get());
	}
	else if (name == "^store_long") {
		if (store_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("src", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_VOID))));
			store_long = f; 
		}
		f = static_cast<Function*>(store_long.get());
	}
	else if (name == "^add_long") {
		if (add_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			add_long = f; 
		}
		f = static_cast<Function*>(add_long.get());
	}
	else if (name == "^sub_long") {
		if (sub_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			sub_long = f; 
		}
		f = static_cast<Function*>(sub_long.get());
	}
	else if (name == "^mul_long") {
		if (mul_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			mul_long = f; 
		}
		f = static_cast<Function*>(mul_long.get());
	}
	else if (name == "^div_long") {
		if (div_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			div_long = f; 
		}
		f = static_cast<Function*>(div_long.get());
	}
	else if (name == "^mod_long") {
		if (mod_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			mod_long = f; 
		}
		f = static_cast<Function*>(mod_long.get());
	}
	else if (name == "^bitand_long") {
		if (bitand_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			bitand_long = f; 
		}
		f = static_cast<Function*>(bitand_long.get());
	}
	else if (name == "^bitnot_long") {
		if (bitnot_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			bitnot_long = f; 
		}
		f = static_cast<Function*>(bitnot_long.get());
	}
	else if (name == "^bitor_long") {
		if (bitor_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			bitor_long = f; 
		}
		f = static_cast<Function*>(bitor_long.get());
	}
	else if (name == "^bitxor_long") {
		if (bitxor_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			bitxor_long = f; 
		}
		f = static_cast<Function*>(bitxor_long.get());
	}
	else if (name == "^addassign_long") {
		if (addassign_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONG))));
			addassign_long = f; 
		}
		f = static_cast<Function*>(addassign_long.get());
	}
	else if (name == "^subassign_long") {
		if (subassign_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONG))));
			subassign_long = f; 
		}
		f = static_cast<Function*>(subassign_long.get());
	}
	else if (name == "^bitandassign_long") {
		if (bitandassign_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONG))));
			bitandassign_long = f; 
		}
		f = static_cast<Function*>(bitandassign_long.get());
	}
	else if (name == "^bitorassign_long") {
		if (bitorassign_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONG))));
			bitorassign_long = f; 
		}
		f = static_cast<Function*>(bitorassign_long.get());
	}
	else if (name == "^bitxorassign_long") {
		if (bitxorassign_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONG))));
			bitxorassign_long = f; 
		}
		f = static_cast<Function*>(bitxorassign_long.get());
	}
	else if (name == "^dec_long") {
		if (dec_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			dec_long = f; 
		}
		f = static_cast<Function*>(dec_long.get());
	}
	else if (name == "^inc_long") {
		if (inc_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			inc_long = f; 
		}
		f = static_cast<Function*>(inc_long.get());
	}
	else if (name == "^neg_long") {
		if (neg_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			neg_long = f; 
		}
		f = static_cast<Function*>(neg_long.get());
	}
	else if (name == "^postdec_long") {
		if (postdec_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_LONG)))));
			postdec_long = f; 
		}
		f = static_cast<Function*>(postdec_long.get());
	}
	else if (name == "^postinc_long") {
		if (postinc_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_LONG)))));
			postinc_long = f; 
		}
		f = static_cast<Function*>(postinc_long.get());
	}
	else if (name == "^predec_long") {
		if (predec_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_LONG)))));
			predec_long = f; 
		}
		f = static_cast<Function*>(predec_long.get());
	}
	else if (name == "^preinc_long") {
		if (preinc_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_LONG)))));
			preinc_long = f; 
		}
		f = static_cast<Function*>(preinc_long.get());
	}
	else if (name == "^eq_long") {
		if (eq_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			eq_long = f; 
		}
		f = static_cast<Function*>(eq_long.get());
	}
	else if (name == "^ne_long") {
		if (ne_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			ne_long = f; 
		}
		f = static_cast<Function*>(ne_long.get());
	}
	else if (name == "^shl_long") {
		if (shl_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("value", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			shl_long = f; 
		}
		f = static_cast<Function*>(shl_long.get());
	}
	else if (name == "^shr_long") {
		if (shr_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("value", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			shr_long = f; 
		}
		f = static_cast<Function*>(shr_long.get());
	}
	else if (name == "^shlp_long") {
		if (shlp_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONG))));
			shlp_long = f; 
		}
		f = static_cast<Function*>(shlp_long.get());
	}
	else if (name == "^shrp_long") {
		if (shrp_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONG))));
			shrp_long = f; 
		}
		f = static_cast<Function*>(shrp_long.get());
	}
	else if (name == "^lt0_long") {
		if (lt0_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			lt0_long = f; 
		}
		f = static_cast<Function*>(lt0_long.get());
	}
	else if (name == "^le0_long") {
		if (le0_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			le0_long = f; 
		}
		f = static_cast<Function*>(le0_long.get());
	}
	else if (name == "^gt0_long") {
		if (gt0_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			gt0_long = f; 
		}
		f = static_cast<Function*>(gt0_long.get());
	}
	else if (name == "^ge0_long") {
		if (ge0_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			ge0_long = f; 
		}
		f = static_cast<Function*>(ge0_long.get());
	}
	else if (name == "^lt_long") {
		if (lt_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			lt_long = f; 
		}
		f = static_cast<Function*>(lt_long.get());
	}
	else if (name == "^le_long") {
		if (le_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			le_long = f; 
		}
		f = static_cast<Function*>(le_long.get());
	}
	else if (name == "^gt_long") {
		if (gt_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			gt_long = f; 
		}
		f = static_cast<Function*>(gt_long.get());
	}
	else if (name == "^ge_long") {
		if (ge_long.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONG)));
			ge_long = f; 
		}
		f = static_cast<Function*>(ge_long.get());
	}
  return f;
}

Function* Function::system_ulong(const std::string& name)
{
static ObjPtr lt_ulong;
static ObjPtr le_ulong;
static ObjPtr gt_ulong;
static ObjPtr ge_ulong;
static ObjPtr shr_ulong;
static ObjPtr shrp_ulong;
static ObjPtr mul_ulong;
static ObjPtr div_ulong;
static ObjPtr mod_ulong;
	Function* f = 0;
	if (name == "^mul_ulong") {
		if (mul_ulong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_ULONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONG)));
			//f->add_parameter("res", new Parameter(new ReferenceType(T_POINTER, get_umul_t())));
			mul_ulong = f; 
		}
		f = static_cast<Function*>(mul_ulong.get());
	}
	else if (name == "^div_ulong") {
		if (div_ulong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_ULONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONG)));
			//f->add_parameter("res", new Parameter(new ReferenceType(T_POINTER, get_umul_t())));
			div_ulong = f; 
		}
		f = static_cast<Function*>(div_ulong.get());
	}
	else if (name == "^mod_ulong") {
		if (mod_ulong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_ULONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONG)));
			//f->add_parameter("res", new Parameter(new ReferenceType(T_POINTER, get_umul_t())));
			mod_ulong = f; 
		}
		f = static_cast<Function*>(mod_ulong.get());
	}
	else if (name == "^shr_ulong") {
		if (shr_ulong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_ULONG));
			f->add_parameter("value", new Parameter(Type::basic(T_ULONG)));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			shr_ulong = f; 
		}
		f = static_cast<Function*>(shr_ulong.get());
	}
	else if (name == "^shrp_ulong") {
		if (shrp_ulong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONG));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONG))));
			shrp_ulong = f; 
		}
		f = static_cast<Function*>(shrp_ulong.get());
	}
	else if (name == "^lt_ulong") {
		if (lt_ulong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONG)));
			lt_ulong = f; 
		}
		f = static_cast<Function*>(lt_ulong.get());
	}
	else if (name == "^le_ulong") {
		if (le_ulong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONG)));
			le_ulong = f; 
		}
		f = static_cast<Function*>(le_ulong.get());
	}
	else if (name == "^gt_ulong") {
		if (gt_ulong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONG)));
			gt_ulong = f; 
		}
		f = static_cast<Function*>(gt_ulong.get());
	}
	else if (name == "^ge_ulong") {
		if (ge_ulong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONG)));
			ge_ulong = f; 
		}
		f = static_cast<Function*>(ge_ulong.get());
	}
  return f;
}

Function* Function::system_longlong(const std::string& name)
{
static ObjPtr load_longlong;
static ObjPtr store_longlong;
static ObjPtr add_longlong;
static ObjPtr sub_longlong;
static ObjPtr bitand_longlong;
static ObjPtr bitor_longlong;
static ObjPtr bitxor_longlong;
static ObjPtr bitnot_longlong;
static ObjPtr dec_longlong;
static ObjPtr inc_longlong;
static ObjPtr neg_longlong;
static ObjPtr postdec_longlong;
static ObjPtr postinc_longlong;
static ObjPtr predec_longlong;
static ObjPtr preinc_longlong;
static ObjPtr deref_longlong;
static ObjPtr shl_longlong;
static ObjPtr shr_longlong;
static ObjPtr mul_longlong;
static ObjPtr div_longlong;
//static ObjPtr eq0_longlong;
//static ObjPtr ne0_longlong;
static ObjPtr lt0_longlong;
static ObjPtr le0_longlong;
static ObjPtr gt0_longlong;
static ObjPtr ge0_longlong;
static ObjPtr eq_longlong;
static ObjPtr ne_longlong;
static ObjPtr lt_longlong;
static ObjPtr le_longlong;
static ObjPtr gt_longlong;
static ObjPtr ge_longlong;
	Function* f = 0;
	if (name == "^load_longlong") {
		if (load_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("src", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_VOID))));
			load_longlong = f; 
		}
		f = static_cast<Function*>(load_longlong.get());
	}
	else if (name == "^store_longlong") {
		if (store_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("src", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_VOID))));
			store_longlong = f; 
		}
		f = static_cast<Function*>(store_longlong.get());
	}
	else if (name == "^add_longlong") {
		if (add_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			add_longlong = f; 
		}
		f = static_cast<Function*>(add_longlong.get());
	}
	else if (name == "^sub_longlong") {
		if (sub_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			sub_longlong = f; 
		}
		f = static_cast<Function*>(sub_longlong.get());
	}
	else if (name == "^bitand_longlong") {
		if (bitand_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			bitand_longlong = f; 
		}
		f = static_cast<Function*>(bitand_longlong.get());
	}
	else if (name == "^bitor_longlong") {
		if (bitor_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			bitor_longlong = f; 
		}
		f = static_cast<Function*>(bitor_longlong.get());
	}
	else if (name == "^bitxor_longlong") {
		if (bitxor_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			bitxor_longlong = f; 
		}
		f = static_cast<Function*>(bitxor_longlong.get());
	}
	else if (name == "^bitnot_longlong") {
		if (bitnot_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			bitnot_longlong = f; 
		}
		f = static_cast<Function*>(bitnot_longlong.get());
	}
	else if (name == "^inc_longlong") {
		if (inc_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			inc_longlong = f; 
		}
		f = static_cast<Function*>(inc_longlong.get());
	}
	else if (name == "^dec_longlong") {
		if (dec_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			dec_longlong = f; 
		}
		f = static_cast<Function*>(dec_longlong.get());
	}
	else if (name == "^neg_longlong") {
		if (neg_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			neg_longlong = f; 
		}
		f = static_cast<Function*>(neg_longlong.get());
	}
	else if (name == "^postdec_longlong") {
		if (postdec_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_LONGLONG)))));
			postdec_longlong = f; 
		}
		f = static_cast<Function*>(postdec_longlong.get());
	}
	else if (name == "^postinc_longlong") {
		if (postinc_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_LONGLONG)))));
			postinc_longlong = f; 
		}
		f = static_cast<Function*>(postinc_longlong.get());
	}
	else if (name == "^predec_longlong") {
		if (predec_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_LONGLONG)))));
			predec_longlong = f; 
		}
		f = static_cast<Function*>(predec_longlong.get());
	}
	else if (name == "^preinc_longlong") {
		if (preinc_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, new ReferenceType(T_POINTER, Type::basic(T_LONGLONG)))));
			preinc_longlong = f; 
		}
		f = static_cast<Function*>(preinc_longlong.get());
	}
	else if (name == "^deref_longlong") {
		if (deref_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONGLONG))));
			deref_longlong = f; 
		}
		f = static_cast<Function*>(deref_longlong.get());
	}
	else if (name == "^shl_longlong") {
		if (shl_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("value", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			shl_longlong = f; 
		}
		f = static_cast<Function*>(shl_longlong.get());
	}
	else if (name == "^shr_longlong") {
		if (shr_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("value", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			shr_longlong = f; 
		}
		f = static_cast<Function*>(shr_longlong.get());
	}
	//else if (name == "^eq0_longlong") {
	//	if (eq0_longlong.is_null()) {
	//		f = new Function(0, name);
	//		f->return_type(Type::basic(T_BOOL));
	//		f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
	//		eq0_longlong = f; 
	//	}
	//	f = static_cast<Function*>(eq0_longlong.get());
	//}
	//else if (name == "^ne0_longlong") {
	//	if (ne0_longlong.is_null()) {
	//		f = new Function(0, name);
	//		f->return_type(Type::basic(T_BOOL));
	//		f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
	//		ne0_longlong = f; 
	//	}
	//	f = static_cast<Function*>(ne0_longlong.get());
	//}
	else if (name == "^lt0_longlong") {
		if (lt0_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			lt0_longlong = f; 
		}
		f = static_cast<Function*>(lt0_longlong.get());
	}
	else if (name == "^le0_longlong") {
		if (le0_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			le0_longlong = f; 
		}
		f = static_cast<Function*>(le0_longlong.get());
	}
	else if (name == "^gt0_longlong") {
		if (gt0_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			gt0_longlong = f; 
		}
		f = static_cast<Function*>(gt0_longlong.get());
	}
	else if (name == "^ge0_longlong") {
		if (ge0_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			ge0_longlong = f; 
		}
		f = static_cast<Function*>(ge0_longlong.get());
	}
	else if (name == "^eq_longlong") {
		if (eq_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			eq_longlong = f; 
		}
		f = static_cast<Function*>(eq_longlong.get());
	}
	else if (name == "^ne_longlong") {
		if (ne_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			ne_longlong = f; 
		}
		f = static_cast<Function*>(ne_longlong.get());
	}
	else if (name == "^lt_longlong") {
		if (lt_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			lt_longlong = f; 
		}
		f = static_cast<Function*>(lt_longlong.get());
	}
	else if (name == "^le_longlong") {
		if (le_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			le_longlong = f; 
		}
		f = static_cast<Function*>(le_longlong.get());
	}
	else if (name == "^gt_longlong") {
		if (gt_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			gt_longlong = f; 
		}
		f = static_cast<Function*>(gt_longlong.get());
	}
	else if (name == "^ge_longlong") {
		if (ge_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGLONG)));
			ge_longlong = f; 
		}
		f = static_cast<Function*>(ge_longlong.get());
	}
  return f;
}

Function* Function::system_ulonglong(const std::string& name)
{
static ObjPtr mul_ulonglong;
static ObjPtr mulext_ulonglong;
static ObjPtr div_ulonglong;
static ObjPtr divmod_ulonglong;
static ObjPtr shr_ulonglong;
static ObjPtr lt_ulonglong;
static ObjPtr le_ulonglong;
static ObjPtr gt_ulonglong;
static ObjPtr ge_ulonglong;
	Function* f = 0;
	if (name == "^mul_ulonglong") {
		if (mul_ulonglong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONGLONG)));
			mul_ulonglong = f; 
		}
		f = static_cast<Function*>(mul_ulonglong.get());
	}
	else if (name == "^mulext_ulonglong") {
		if (mulext_ulonglong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("res", new Parameter(new ReferenceType(T_POINTER, get_ullmul_t())));
			mulext_ulonglong = f; 
		}
		f = static_cast<Function*>(mulext_ulonglong.get());
	}
	else if (name == "^div_ulonglong") {
		if (div_ulonglong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONGLONG)));
			div_ulonglong = f; 
		}
		f = static_cast<Function*>(div_ulonglong.get());
	}
	else if (name == "^divmod_ulonglong") {
		if (divmod_ulonglong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("res", new Parameter(new ReferenceType(T_POINTER, get_ulldiv_t())));
			divmod_ulonglong = f; 
		}
		f = static_cast<Function*>(divmod_ulonglong.get());
	}
	else if (name == "^shr_ulonglong") {
		if (shr_ulonglong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_ULONGLONG));
			f->add_parameter("value", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("shift", new Parameter(Type::basic(T_INT)));
			shr_ulonglong = f; 
		}
		f = static_cast<Function*>(shr_ulonglong.get());
	}
	else if (name == "^lt_ulonglong") {
		if (lt_ulonglong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONGLONG)));
			lt_ulonglong = f; 
		}
		f = static_cast<Function*>(lt_ulonglong.get());
	}
	else if (name == "^le_ulonglong") {
		if (le_ulonglong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONGLONG)));
			le_ulonglong = f; 
		}
		f = static_cast<Function*>(le_ulonglong.get());
	}
	else if (name == "^gt_ulonglong") {
		if (gt_ulonglong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONGLONG)));
			gt_ulonglong = f; 
		}
		f = static_cast<Function*>(gt_ulonglong.get());
	}
	else if (name == "^ge_ulonglong") {
		if (ge_ulonglong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("a", new Parameter(Type::basic(T_ULONGLONG)));
			f->add_parameter("b", new Parameter(Type::basic(T_ULONGLONG)));
			ge_ulonglong = f; 
		}
		f = static_cast<Function*>(ge_ulonglong.get());
	}
  return f;
}

Function* Function::system_float(const std::string& name)
{
static ObjPtr reintptolong_float;
static ObjPtr reintpfromlong_float;
static ObjPtr compose_float;
static ObjPtr decompose_float;
static ObjPtr normalize_float;
static ObjPtr add_float;
static ObjPtr sub_float;
static ObjPtr mul_float;
static ObjPtr div_float;
static ObjPtr addassign_float;
static ObjPtr subassign_float;
static ObjPtr mulassign_float;
static ObjPtr divassign_float;
static ObjPtr align_float_exponents;
static ObjPtr nan_float;
static ObjPtr signaling_float;
	Function* f = 0;
	if (name == "^compose_float") {
		if (compose_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_FLOAT));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, get_float_t())));
			compose_float = f; 
		}
		f = static_cast<Function*>(compose_float.get());
	}
	else if (name == "^decompose_float") {
		if (decompose_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("val", new Parameter(Type::basic(T_FLOAT)));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, get_float_t())));
			decompose_float = f; 
		}
		f = static_cast<Function*>(decompose_float.get());
	}
	else if (name == "^normalize_float") {
		if (normalize_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, get_float_t())));
			normalize_float = f; 
		}
		f = static_cast<Function*>(normalize_float.get());
	}
	else if (name == "^signaling_float") {
		if (signaling_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_BOOL));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, get_float_t())));
			signaling_float = f; 
		}
		f = static_cast<Function*>(signaling_float.get());
	}
	else if (name == "^add_float") {
		if (add_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_FLOAT));
			f->add_parameter("a", new Parameter(Type::basic(T_FLOAT)));
			f->add_parameter("b", new Parameter(Type::basic(T_FLOAT)));
			add_float = f; 
		}
		f = static_cast<Function*>(add_float.get());
	}
	else if (name == "^sub_float") {
		if (sub_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_FLOAT));
			f->add_parameter("a", new Parameter(Type::basic(T_FLOAT)));
			f->add_parameter("b", new Parameter(Type::basic(T_FLOAT)));
			sub_float = f; 
		}
		f = static_cast<Function*>(sub_float.get());
	}
	else if (name == "^mul_float") {
		if (mul_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_FLOAT));
			f->add_parameter("a", new Parameter(Type::basic(T_FLOAT)));
			f->add_parameter("b", new Parameter(Type::basic(T_FLOAT)));
			mul_float = f; 
		}
		f = static_cast<Function*>(mul_float.get());
	}
	else if (name == "^div_float") {
		if (div_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_FLOAT));
			f->add_parameter("a", new Parameter(Type::basic(T_FLOAT)));
			f->add_parameter("b", new Parameter(Type::basic(T_FLOAT)));
			div_float = f; 
		}
		f = static_cast<Function*>(div_float.get());
	}
	else if (name == "^addassign_float") {
		if (addassign_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_FLOAT)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_FLOAT))));
			addassign_float = f; 
		}
		f = static_cast<Function*>(addassign_float.get());
	}
	else if (name == "^subassign_float") {
		if (subassign_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_FLOAT)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_FLOAT))));
			subassign_float = f; 
		}
		f = static_cast<Function*>(subassign_float.get());
	}
	else if (name == "^mulassign_float") {
		if (mulassign_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_FLOAT)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_FLOAT))));
			mulassign_float = f; 
		}
		f = static_cast<Function*>(mulassign_float.get());
	}
	else if (name == "^divassign_float") {
		if (divassign_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_FLOAT)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_FLOAT))));
			divassign_float = f; 
		}
		f = static_cast<Function*>(divassign_float.get());
	}
	else if (name == "^align_float_exponents") {
		if (align_float_exponents.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(new ReferenceType(T_POINTER, get_float_t())));
			f->add_parameter("b", new Parameter(new ReferenceType(T_POINTER, get_float_t())));
			align_float_exponents = f; 
		}
		f = static_cast<Function*>(align_float_exponents.get());
	}
	else if (name == "^nan_float") {
		if (nan_float.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_FLOAT));
			f->add_parameter("value", new Parameter(Type::basic(T_ULONG)));
			f->add_parameter("signaling", new Parameter(Type::basic(T_BOOL)));
			nan_float = f; 
		}
		f = static_cast<Function*>(nan_float.get());
	}
  return f;
}

Function* Function::system_double(const std::string& name)
{
static ObjPtr compose_double;
static ObjPtr decompose_double;
static ObjPtr add_double;
static ObjPtr sub_double;
static ObjPtr mul_double;
static ObjPtr div_double;
static ObjPtr addassign_double;
static ObjPtr subassign_double;
static ObjPtr mulassign_double;
static ObjPtr divassign_double;
	Function* f = 0;
	if (name == "^compose_double") {
		if (compose_double.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_DOUBLE));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, get_double_t())));
			compose_double = f; 
		}
		f = static_cast<Function*>(compose_double.get());
	}
	else if (name == "^decompose_double") {
		if (decompose_double.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("val", new Parameter(Type::basic(T_DOUBLE)));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, get_double_t())));
			decompose_double = f; 
		}
		f = static_cast<Function*>(decompose_double.get());
	}
	else if (name == "^add_double") {
		if (add_double.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_DOUBLE));
			f->add_parameter("a", new Parameter(Type::basic(T_DOUBLE)));
			f->add_parameter("b", new Parameter(Type::basic(T_DOUBLE)));
			add_double = f; 
		}
		f = static_cast<Function*>(add_double.get());
	}
	else if (name == "^sub_double") {
		if (sub_double.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_DOUBLE));
			f->add_parameter("a", new Parameter(Type::basic(T_DOUBLE)));
			f->add_parameter("b", new Parameter(Type::basic(T_DOUBLE)));
			sub_double = f; 
		}
		f = static_cast<Function*>(sub_double.get());
	}
	else if (name == "^mul_double") {
		if (mul_double.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_DOUBLE));
			f->add_parameter("a", new Parameter(Type::basic(T_DOUBLE)));
			f->add_parameter("b", new Parameter(Type::basic(T_DOUBLE)));
			mul_double = f; 
		}
		f = static_cast<Function*>(mul_double.get());
	}
	else if (name == "^div_double") {
		if (div_double.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_DOUBLE));
			f->add_parameter("a", new Parameter(Type::basic(T_DOUBLE)));
			f->add_parameter("b", new Parameter(Type::basic(T_DOUBLE)));
			div_double = f; 
		}
		f = static_cast<Function*>(div_double.get());
	}
	else if (name == "^addassign_double") {
		if (addassign_double.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_DOUBLE)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_DOUBLE))));
			addassign_double = f; 
		}
		f = static_cast<Function*>(addassign_double.get());
	}
	else if (name == "^subassign_double") {
		if (subassign_double.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_DOUBLE)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_DOUBLE))));
			subassign_double = f; 
		}
		f = static_cast<Function*>(subassign_double.get());
	}
	else if (name == "^mulassign_double") {
		if (mulassign_double.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_DOUBLE)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_DOUBLE))));
			mulassign_double = f; 
		}
		f = static_cast<Function*>(mulassign_double.get());
	}
	else if (name == "^divassign_double") {
		if (divassign_double.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_DOUBLE)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_DOUBLE))));
			divassign_double = f; 
		}
		f = static_cast<Function*>(divassign_double.get());
	}
  return f;
}

Function* Function::system_longdouble(const std::string& name)
{
static ObjPtr compose_longdouble;
static ObjPtr decompose_longdouble;
static ObjPtr add_longdouble;
static ObjPtr sub_longdouble;
static ObjPtr mul_longdouble;
static ObjPtr div_longdouble;
static ObjPtr addassign_longdouble;
static ObjPtr subassign_longdouble;
static ObjPtr mulassign_longdouble;
static ObjPtr divassign_longdouble;
	Function* f = 0;
	if (name == "^compose_longdouble") {
		if (compose_longdouble.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGDOUBLE));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, get_longdouble_t())));
			compose_longdouble = f; 
		}
		f = static_cast<Function*>(compose_longdouble.get());
	}
	else if (name == "^decompose_longdouble") {
		if (decompose_longdouble.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("val", new Parameter(Type::basic(T_LONGDOUBLE)));
			f->add_parameter("ptr", new Parameter(new ReferenceType(T_POINTER, get_longdouble_t())));
			decompose_longdouble = f; 
		}
		f = static_cast<Function*>(decompose_longdouble.get());
	}
	else if (name == "^add_longdouble") {
		if (add_longdouble.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGDOUBLE));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGDOUBLE)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGDOUBLE)));
			add_longdouble = f; 
		}
		f = static_cast<Function*>(add_longdouble.get());
	}
	else if (name == "^sub_longdouble") {
		if (sub_longdouble.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGDOUBLE));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGDOUBLE)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGDOUBLE)));
			sub_longdouble = f; 
		}
		f = static_cast<Function*>(sub_longdouble.get());
	}
	else if (name == "^mul_longdouble") {
		if (mul_longdouble.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGDOUBLE));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGDOUBLE)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGDOUBLE)));
			mul_longdouble = f; 
		}
		f = static_cast<Function*>(mul_longdouble.get());
	}
	else if (name == "^div_longdouble") {
		if (div_longdouble.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGDOUBLE));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGDOUBLE)));
			f->add_parameter("b", new Parameter(Type::basic(T_LONGDOUBLE)));
			div_longdouble = f; 
		}
		f = static_cast<Function*>(div_longdouble.get());
	}
	else if (name == "^addassign_longdouble") {
		if (addassign_longdouble.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGDOUBLE)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONGDOUBLE))));
			addassign_longdouble = f; 
		}
		f = static_cast<Function*>(addassign_longdouble.get());
	}
	else if (name == "^subassign_longdouble") {
		if (subassign_longdouble.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGDOUBLE)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONGDOUBLE))));
			subassign_longdouble = f; 
		}
		f = static_cast<Function*>(subassign_longdouble.get());
	}
	else if (name == "^mulassign_longdouble") {
		if (mulassign_longdouble.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGDOUBLE)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONGDOUBLE))));
			mulassign_longdouble = f; 
		}
		f = static_cast<Function*>(mulassign_longdouble.get());
	}
	else if (name == "^divassign_longdouble") {
		if (divassign_longdouble.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("a", new Parameter(Type::basic(T_LONGDOUBLE)));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_LONGDOUBLE))));
			divassign_longdouble = f; 
		}
		f = static_cast<Function*>(divassign_longdouble.get());
	}
  return f;
}

Function* Function::system(const std::string& name)
{
// memory
static ObjPtr copy_down;
static ObjPtr copy_up;
static ObjPtr move;
static ObjPtr long_to_longlong;
static ObjPtr ulong_to_ulonglong;
// types

  UnionStructType* ust = 0;
	Function* f = 0;

  if ((f = system_bool(name)) != 0) {return f;}
  if ((f = system_int(name)) != 0) {return f;}
  if ((f = system_uint(name)) != 0) {return f;}
  if ((f = system_long(name)) != 0) {return f;}
  if ((f = system_ulong(name)) != 0) {return f;}
  if ((f = system_longlong(name)) != 0) {return f;}
  if ((f = system_ulonglong(name)) != 0) {return f;}
  if ((f = system_float(name)) != 0) {return f;}
  if ((f = system_double(name)) != 0) {return f;}
  if ((f = system_longdouble(name)) != 0) {return f;}

  //************************************ float *********************************
	//if (name == "^reintptolong_float") {
	//	if (deref_longlong.is_null()) {
	//		f = new Function(0, name);
	//		f->return_type(Type::basic(T_LONG));
	//		f->add_parameter("a", new Parameter(Type::basic(T_FLOAT)));
	//		deref_longlong = f; 
	//	}
	//	f = static_cast<Function*>(deref_longlong.get());
	//}
	//else if (name == "^reintpfromlong_float") {
	//	if (reintpfromlong_float.is_null()) {
	//		f = new Function(0, name);
	//		f->return_type(Type::basic(T_FLOAT));
	//		f->add_parameter("a", new Parameter(Type::basic(T_LONG)));
	//		reintpfromlong_float = f; 
	//	}
	//	f = static_cast<Function*>(reintpfromlong_float.get());
	//}

  //************************************ memory *********************************
	if (name == "^copy_down") {
		if (copy_down.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_VOID))));
			f->add_parameter("src", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_VOID))));
			f->add_parameter("cnt", new Parameter(Type::basic(T_UINT)));
			copy_down = f; 
		}
		f = static_cast<Function*>(copy_down.get());
	}
	else if (name == "^copy_up") {
		if (copy_up.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_VOID))));
			f->add_parameter("src", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_VOID))));
			f->add_parameter("cnt", new Parameter(Type::basic(T_UINT)));
			copy_up = f; 
		}
		f = static_cast<Function*>(copy_up.get());
	}
	else if (name == "^move") {
		if (move.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_VOID));
			f->add_parameter("dest", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_VOID))));
			f->add_parameter("src", new Parameter(new ReferenceType(T_POINTER, Type::basic(T_VOID))));
			f->add_parameter("cnt", new Parameter(Type::basic(T_UINT)));
			move = f; 
		}
		f = static_cast<Function*>(move.get());
	}
	else if (name == "^long_to_longlong") {
		if (long_to_longlong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("val", new Parameter(Type::basic(T_LONG)));
			long_to_longlong = f; 
		}
		f = static_cast<Function*>(long_to_longlong.get());
	}
	else if (name == "^ulong_to_ulonglong") {
		if (ulong_to_ulonglong.is_null()) {
			f = new Function(0, name);
			f->return_type(Type::basic(T_LONGLONG));
			f->add_parameter("val", new Parameter(Type::basic(T_LONG)));
			ulong_to_ulonglong = f; 
		}
		f = static_cast<Function*>(ulong_to_ulonglong.get());
	}
  //*********************************************************************
	else {
    throw not_implemented_exception("Function::system unknow name "+name);
  }
	return f;
}

void Function::write(xml::Writer& out)
{
	size_t i;
	out.newline();
	out.out() << "<Function name='";
	out.write(name_);
	out.out() << "'>";
	out.newline(1);
	out.out() << "<Type>";
	return_type()->write(out);
	out.out() << "</Type>";
	size_t pars = parameter();
	for (i=0; i<pars; ++i) {
		parameter(i)->write(out);
	}
	if (manager_.is_not_null()) {
		manager_.get()->write(out);
	}
	if (compilation_unit_.is_not_null()) {
		out.newline();
		out.out() << "<CU>";
		out.indent(1);
		compilation_unit()->write(out);
		out.newline(-1);
		out.out() << "</CU>";
		//compilation_unit()->code().write(out);
	}
	if (!data_fixup_list_.empty()) {
		std::stringstream fixup_data;
		for (offset_list_type::iterator it=data_fixup_list_.begin(); it!=data_fixup_list_.end(); ++it) {
			fixup_data << *it << " ";
		}
		out.out() << "<DF size='" << data_fixup_list_.size() << "'>" << fixup_data.str() << "</DF>";
		out.newline();
	}
	out.newline(-1);
	out.out() << "</Function>";
}

void Function::read(xml::Iterator& it)
{
	ObjPtr obj;
	size_t limit, i, value;
	if (it.current() != xml::Element::tag) {throw exception("Function::read invalid file");}
	std::string id, tag = it.tag();
	it.next();
	while (it.current() == xml::Element::attribute) {
		if (it.key() == "name") {name_ = it.value();}
		it.next();
	}
	if ((it.current() != xml::Element::content) || (it.tag() != tag)) {throw exception("Function::read invalid file");}
	it.next_tag();


	while (it.current() == xml::Element::tag) {
		if (it.tag() == "Par") {
			Parameter* par = new Parameter();
			obj = par;
			par->read(it);
			it.next_tag();
		}
		else if (it.tag() == "Type") {
			it.next_tag();
			Type::read(it, return_type_);
			it.next_tag();
			if ((it.current() != xml::Element::end) || (it.tag() != "Type")) {throw exception("Function::read invalid file");}
			it.next_tag();
		}
		else if (it.tag() == "XRM") {
			manager()->read(it);
			it.next_tag();
		}
		else if (it.tag() == "CU") {
			compilation_unit_ = new CompilationUnit();
			it.next_tag();
			compilation_unit()->read(it);
			//it.next_tag();
			if ((it.current() != xml::Element::end) || (it.tag() != "CU")) {throw exception("Function::read invalid file");}
			it.next_tag();
		}
		else if (it.tag() == "DF") {
			limit = 0;
			it.next();
			while (it.current() == xml::Element::attribute) {
				if (it.key() == "size") {limit = boost::lexical_cast<size_t>(it.value());}
				it.next();
			}
			if (it.current() != xml::Element::content) {throw exception("Function::read invalid file");}
			if (limit > 0) {
				std::stringstream fixup_data(it.value());
				for (i=0; i<limit; ++i) {
					fixup_data >> value;
					data_fixup_list_.push_back(value);
				}
			}
		}
		else {throw exception("Function::read invalid file");}
	}
	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw exception("Function::read invalid file");}
}

void Function::link1(Linker& lnk)
{
	lnk.add_function(this);
	if (manager_.is_not_null()) {
		manager()->link1(lnk, 0);
	}
}

void Function::link2(Linker& lnk)
{
	size_t data_offset = lnk.code().size();
	lnk.code().append(compilation_unit()->data());
	size_t link_offset = lnk.code().size();
	lnk.code().append(compilation_unit()->code());
	if (manager_.is_not_null()) {
		manager()->link2(lnk, link_offset);
	}
	if (!data_fixup_list_.empty()) {
		for (offset_list_type::iterator it=data_fixup_list_.begin(); it!=data_fixup_list_.end(); ++it) {
			lnk.code().adjust(*it, data_offset);
		}
	}
}

}
