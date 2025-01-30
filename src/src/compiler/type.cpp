#include "cvmc.hpp"


namespace cvmc {

Type::Type()
: RefObj(OT_TYPE)
, type_type_(T_UNDEFINED)
, storage_class_mask_(0)
, qualifier_mask_(0)
{
}

Type::Type(TypeType t)
: RefObj(OT_TYPE)
, type_type_(t)
, storage_class_mask_(0)
, qualifier_mask_(0)
{
  if (t == T_STRUCT) {
    type_type_ = type_type_;
  }
}

//bool Type::add_storage_class(StorageClass sc)
//{
//	if (sc == SC_TYPEDEF) {
//		if (storage_class_mask_ != 0) {
//			return false;
//		}
//	}
//	if ((MASK(sc) & storage_class_mask_) != 0) {return false;}
//	storage_class_mask_ |= MASK(sc);
//	return true;
//}

Type* Type::base()
{
	return this;
}

void Type::type(TypeType t)
{
  if (t == T_STRUCT) {
    type_type_ = type_type_;
  }
  type_type_ = t;
}

Type* Type::deref()
{
	throw exception("Type::deref not a pointer type");
	return this;
}

bool Type::is_composed() const
{
	return false;
}

bool Type::append(ReferenceType& t)
{
	return false;
}

bool Type::is_direct() const
{
	return true;
}

bool Type::is_array() const
{
	return false;
}

bool Type::is_initializer() const
{
	return false;
}

bool Type::is_array_or_pointer() const
{
	return false;
}

bool Type::is_pointer() const
{
	return false;
}

size_t Type::depth() const
{
	return 0;
}

bool Type::is_additive_compatible_with(Type& t)
{
	return is_assignment_compatible_with(t);
}

int Type::conversion_rank() const
{
	switch (type_type_) {
	case T_VOID: return 0;
	case T_BOOL: return 1;
  case T_CHAR:
  case T_SCHAR:
  case T_UCHAR: return 2;
  case T_SHORT:
  case T_SSHORT:
  case T_USHORT: return 3;
  case T_ENUM:
  case T_INT:
  case T_SINT:
  case T_UINT: return 4;
  case T_LONG:
  case T_SLONG:
  case T_ULONG: return 5;
  case T_LONGLONG:
  case T_SLONGLONG:
  case T_ULONGLONG: return 6;
  case T_FLOAT: return 10;
  case T_DOUBLE: return 11;
  case T_LONGDOUBLE: return 12;
  default:;
  }
  return -1;
}

bool Type::is_assignment_compatible_with(Type& t)
{
	if (&t == this) {return true;}
	if (is_equivalent_with(t)) {return true;}
  if (conversion_rank() >= t.conversion_rank()) {
    return true;
  }
	//switch (type_type_) {
	//case T_VOID:
	//case T_BOOL:
	//	return type_type_ == t.type_type_;

	//case T_CHAR:
	//case T_SCHAR:
	//case T_UCHAR:
	//	switch (t.type_type_) {
	//	case T_BOOL:
	//	case T_CHAR:
	//	case T_SCHAR:
	//	case T_UCHAR:
	//		return true;
	//	}
	//	break;

	//case T_SHORT:
	//case T_SSHORT:
	//case T_USHORT:
	//	switch (t.type_type_) {
	//	case T_BOOL:
	//	case T_CHAR:
	//	case T_SCHAR:
	//	case T_UCHAR:
	//	case T_SHORT:
	//	case T_SSHORT:
	//	case T_USHORT:
	//		return true;
	//	}
	//	break;

	//case T_INT:
	//case T_SINT:
	//case T_UINT:
	//	switch (t.type_type_) {
	//	case T_BOOL:
	//	case T_CHAR:
	//	case T_SCHAR:
	//	case T_UCHAR:
	//	case T_SHORT:
	//	case T_SSHORT:
	//	case T_USHORT:
	//	case T_INT:
	//	case T_SINT:
	//	case T_UINT:
	//	case T_SIGNED:
	//	case T_UNSIGNED:
	//		return true;
	//	}
	//	break;

	//case T_LONG:
	//case T_SLONG:
	//case T_ULONG:
	//	switch (t.type_type_) {
	//	case T_BOOL:
	//	case T_CHAR:
	//	case T_SCHAR:
	//	case T_UCHAR:
	//	case T_SHORT:
	//	case T_SSHORT:
	//	case T_USHORT:
	//	case T_INT:
	//	case T_SINT:
	//	case T_UINT:
	//	case T_SIGNED:
	//	case T_UNSIGNED:
	//	case T_LONG:
	//	case T_SLONG:
	//	case T_ULONG:
	//		return true;
	//	}
	//	break;

	//case T_LONGLONG:
	//case T_SLONGLONG:
	//case T_ULONGLONG:
	//	switch (t.type_type_) {
	//	case T_BOOL:
	//	case T_CHAR:
	//	case T_SCHAR:
	//	case T_UCHAR:
	//	case T_SHORT:
	//	case T_SSHORT:
	//	case T_USHORT:
	//	case T_INT:
	//	case T_SINT:
	//	case T_UINT:
	//	case T_SIGNED:
	//	case T_UNSIGNED:
	//	case T_LONG:
	//	case T_SLONG:
	//	case T_ULONG:
	//	case T_LONGLONG:
	//	case T_SLONGLONG:
	//	case T_ULONGLONG:
	//		return true;
	//	}
	//	break;


	//case T_FLOAT:
	//case T_DOUBLE:
	//case T_LONGDOUBLE:
	//	switch (t.type_type_) {
	//	case T_BOOL:
	//	case T_CHAR:
	//	case T_SCHAR:
	//	case T_UCHAR:
	//	case T_SHORT:
	//	case T_SSHORT:
	//	case T_USHORT:
	//	case T_INT:
	//	case T_SINT:
	//	case T_UINT:
	//	case T_SIGNED:
	//	case T_UNSIGNED:
	//	case T_LONG:
	//	case T_SLONG:
	//	case T_ULONG:
	//	case T_LONGLONG:
	//	case T_SLONGLONG:
	//	case T_ULONGLONG:
	//	case T_FLOAT:
	//	case T_DOUBLE:
	//	case T_LONGDOUBLE:
	//		return true;
	//	}
	//	break;

	//case T_POINTER:
	//case T_BOUNDEDARRAY:
	//case T_ARRAY:
	//case T_REFERENCE:
	//case T_ENUM:
	//default:;
	//}
	return false;
}

bool Type::is_integer() const
{
	switch (type_type_) {
	case T_BOOL:
	case T_CHAR:
	case T_SCHAR:
	case T_UCHAR:
	case T_SHORT:
	case T_SSHORT:
	case T_USHORT:
	case T_INT:
	case T_SINT:
	case T_UINT:
	case T_LONG:
	case T_SLONG:
	case T_ULONG:
	case T_LONGLONG:
	case T_SLONGLONG:
	case T_ULONGLONG:
	case T_SIGNED:
	case T_UNSIGNED:
		return true;
	}
	return false;
}

bool Type::is_integer_or_pointer() const
{
  return is_integer();
}

bool Type::is_float() const
{
	switch (type_type_) {
	case T_FLOAT:
	case T_DOUBLE:
	case T_LONGDOUBLE:
		return true;
	}
	return false;
}

bool Type::is_equivalent_with(Type& t)
{
	if (&t == this) {return true;}
	switch (type_type_) {
	case T_VOID:
	case T_BOOL:
	case T_CHAR:
	case T_SCHAR:
	case T_UCHAR:
	case T_USHORT:
	case T_ULONG:
	case T_ULONGLONG:
	case T_FLOAT:
	case T_DOUBLE:
	case T_LONGDOUBLE:
		return type_type_ == t.type_type_;

	case T_SHORT:
	case T_SSHORT:
		return (t.type_type_ == T_SHORT) || (t.type_type_ == T_SSHORT);

	case T_UINT:
	case T_UNSIGNED:
		return (t.type_type_ == T_UINT) || (t.type_type_ == T_UNSIGNED);

	case T_INT:
	case T_SINT:
	case T_SIGNED:
		return (t.type_type_ == T_INT) || (t.type_type_ == T_SINT) || (t.type_type_ == T_SIGNED);

	case T_LONG:
	case T_SLONG:
		return (t.type_type_ == T_LONG) || (t.type_type_ == T_SLONG);

	case T_LONGLONG:
	case T_SLONGLONG:
		return (t.type_type_ == T_LONGLONG) || (t.type_type_ == T_SLONGLONG);

	case T_REFERENCE:
	case T_POINTER:
	case T_BOUNDEDARRAY:
	case T_ARRAY:
	case T_ENUM:
	default:;
	}
	return false;
}

bool Type::is_recursive_equivalent_with(Type& t, std::multimap<void*, void*>& map)
{
  return is_equivalent_with(t);
}

bool Type::is_cast_possible_with(Type& t)
{
	if (&t == this) {return true;}
	if (is_equivalent_with(t)) {return true;}
	return false;
}

//bool Type::add_qualifier(TypeQualifier tq)
//{
//	if ((MASK(tq) & qualifier_mask_) != 0) {return false;}
//	qualifier_mask_ |= MASK(tq);
//	return true;
//}

bool Type::is_unsigned() const
{
	switch (type_type_) {
	case T_BOOL:
	case T_UNSIGNED:
	case T_UCHAR:
	case T_USHORT:
	case T_UINT:
	case T_ULONG:
	case T_ULONGLONG:
	case T_POINTER:
	case T_REFERENCE:
		return true;
	default:;
	}
	return false;
}

//size_t Type::var_size() const
//{
//	return size();
//}

size_t Type::size() const
{
	switch (type_type_) {
	case T_BOOL:
	case T_CHAR:
	case T_SCHAR:
	case T_UCHAR:
	case T_SHORT:
	case T_SSHORT:
	case T_USHORT:
	case T_INT:
	case T_SINT:
	case T_UINT:
	case T_ENUM:
	case T_POINTER:
	case T_REFERENCE:
	case T_BOUNDEDARRAY:
	case T_ARRAY:
	case T_SIGNED:
	case T_UNSIGNED:
		return 1;

	case T_LONG:
	case T_SLONG:
	case T_ULONG:
	case T_FLOAT:
		return 2;

	case T_LONGLONG:
	case T_SLONGLONG:
	case T_ULONGLONG:
	case T_DOUBLE:
		return 4;

	case T_LONGDOUBLE:
		return 8;

	case T_VOID:
	default:;
	}
	return 0;
}

//bool Type::assign(Type& t)
//{
//	switch (t.type_) {
//		case T_POINTER:
//		case T_REFERENCE:
//		case T_BOUNDEDARRAY:
//		case T_ARRAY:
//			return false;
//
//		default:;
//	}
//	if (type_ == T_UNDEFINED) {
//		type_ = t.type_;
//		return true;
//	}
//	return false;
//}

Type* Type::copy()
{
	Type* res = new Type();
	*res = *this;
	return res;
}


bool Type::is_valid(bool first) const
{
	switch (type_type_) {
	case T_REFERENCE:
		if (!first) {return false;}
	case T_POINTER:
		{
			const ReferenceType* rt = dynamic_cast<const ReferenceType*>(this);
			if (rt == 0) {
				return false;
			}
			return rt->next()->is_valid(false);
		}
		break; 

	case T_BOUNDEDARRAY:
	case T_ARRAY:
		{
			const ArrayType* at = dynamic_cast<const ArrayType*>(this);
			if (at == 0) {
				return false;
			}
			return at->next()->is_valid(false);
		}
		break;

	case T_BOOL:
	case T_CHAR:
	case T_SCHAR:
	case T_UCHAR:
	case T_SHORT:
	case T_SSHORT:
	case T_USHORT:
	case T_INT:
	case T_SINT:
	case T_UINT:
	case T_ENUM:
	case T_LONG:
	case T_SLONG:
	case T_ULONG:
	case T_FLOAT:
	case T_LONGLONG:
	case T_SLONGLONG:
	case T_ULONGLONG:
	case T_DOUBLE:
	case T_LONGDOUBLE:
	case T_VOID:
	default:;
	}
	return true;
}

void Type::print(std::ostream& out)
{
	switch (type_type_) {
	case T_BOOL:		out << "bool"; break;
	case T_CHAR:		out << "char"; break;
	case T_SCHAR:		out << "signed char"; break;
	case T_UCHAR:		out << "unsigned char"; break;
	case T_SHORT:
	case T_SSHORT:		out << "short"; break;
	case T_USHORT:		out << "unsigned short"; break;
	case T_INT:
	case T_SINT:		out << "int"; break;
	case T_UINT:		out << "unsigned int"; break;
	case T_ENUM:		out << "enum"; break;
	case T_POINTER:
	case T_REFERENCE:
	case T_BOUNDEDARRAY:
	case T_ARRAY:		out << "pointer"; break;
	case T_LONG:
	case T_SLONG:		out << "long"; break;
	case T_ULONG:		out << "unsigned long"; break;
	case T_FLOAT:		out << "float"; break;
	case T_LONGLONG:
	case T_SLONGLONG:	out << "long long"; break;
	case T_ULONGLONG:	out << "unsigned long long"; break;
	case T_DOUBLE:		out << "double"; break;
	case T_LONGDOUBLE:	out << "long double"; break;
	case T_VOID:		out << "void"; break;
	default:;
	}
}

Type* Type::basic(TypeType t)
{
	switch (t) {
	case T_BOOL: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_CHAR: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_SCHAR: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_UCHAR: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_SHORT: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_SSHORT: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_USHORT: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_INT: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_SINT: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_UINT: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_ENUM: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_LONG: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_SLONG: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_ULONG: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_FLOAT: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_LONGLONG: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_SLONGLONG: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_ULONGLONG: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_DOUBLE: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_LONGDOUBLE: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	case T_VOID: {static ObjPtr obj; if (obj.is_null()) {obj = new Type(t);} return static_cast<Type*>(obj.get());}
	}
	return 0;
}


void Type::write(xml::Writer& out)
{
	switch (type_type_) {
	case T_BOOL:		out.out() << "<bool/>"; break;
	case T_CHAR:		out.out() << "<char/>"; break;
	case T_SCHAR:		out.out() << "<schar/>"; break;
	case T_UCHAR:		out.out() << "<uchar/>"; break;
	case T_SHORT:		out.out() << "<short/>"; break;
	case T_SSHORT:		out.out() << "<sshort/>"; break;
	case T_USHORT:		out.out() << "<ushort/>"; break;
	case T_INT:			out.out() << "<int/>"; break;
	case T_SIGNED:
	case T_SINT:		out.out() << "<sint/>"; break;
	case T_UNSIGNED:
	case T_UINT:		out.out() << "<uint/>"; break;
	case T_ENUM:		out.out() << "<enum/>"; break;
	case T_LONG:		out.out() << "<long/>"; break;
	case T_SLONG:		out.out() << "<slong/>"; break;
	case T_ULONG:		out.out() << "<ulong/>"; break;
	case T_LONGLONG:	out.out() << "<longlong/>"; break;
	case T_SLONGLONG:	out.out() << "<slonglong/>"; break;
	case T_ULONGLONG:	out.out() << "<ulonglong/>"; break;
	case T_FLOAT:		out.out() << "<float/>"; break;
	case T_DOUBLE:		out.out() << "<double/>"; break;
	case T_LONGDOUBLE:	out.out() << "<longdouble/>"; break;
	case T_VOID:		out.out() << "<void/>"; break;
	//case T_STRUCT:		out.out() << "<struct>" << name() << "</struct>"; break;
	default: throw exception("Type::write invalid type");
	}
}

void Type::read(xml::Iterator&)
{
	throw not_implemented_exception("Type::read");
}

void Type::read(xml::Iterator& it, ObjPtr& obj)
{
	if (it.current() == xml::Element::empty) {
		if (it.tag() == "bool") {obj = new Type(T_BOOL); return;}
		if (it.tag() == "char") {obj = new Type(T_CHAR); return;}
		if (it.tag() == "schar") {obj = new Type(T_SCHAR); return;}
		if (it.tag() == "uchar") {obj = new Type(T_UCHAR); return;}
		if (it.tag() == "short") {obj = new Type(T_SHORT); return;}
		if (it.tag() == "sshort") {obj = new Type(T_SSHORT); return;}
		if (it.tag() == "ushort") {obj = new Type(T_USHORT); return;}
		if (it.tag() == "int") {obj = new Type(T_INT); return;}
		if (it.tag() == "sint") {obj = new Type(T_SINT); return;}
		if (it.tag() == "uint") {obj = new Type(T_UINT); return;}
		if (it.tag() == "enum") {obj = new Type(T_ENUM); return;}
		if (it.tag() == "long") {obj = new Type(T_LONG); return;}
		if (it.tag() == "slong") {obj = new Type(T_SLONG); return;}
		if (it.tag() == "ulong") {obj = new Type(T_ULONG); return;}
		if (it.tag() == "longlong") {obj = new Type(T_LONGLONG); return;}
		if (it.tag() == "slonglong") {obj = new Type(T_SLONGLONG); return;}
		if (it.tag() == "ulonglong") {obj = new Type(T_ULONGLONG); return;}
		if (it.tag() == "float") {obj = new Type(T_FLOAT); return;}
		if (it.tag() == "double") {obj = new Type(T_DOUBLE); return;}
		if (it.tag() == "longdouble") {obj = new Type(T_LONGDOUBLE); return;}
		if (it.tag() == "void") {obj = new Type(T_VOID); return;}
	}
	if (it.current() == xml::Element::tag) {
		if (it.tag() == "Ptr") {
			ReferenceType* r = new ReferenceType();
			obj = r;
			r->read(it);
			return;
		}
		if (it.tag() == "Array") {
			ArrayType* a = new ArrayType();
			obj = a;
			a->read(it);
			return;
		}
		if (it.tag() == "Struct") {
			UnionStructType* t = new UnionStructType("", false);
			obj = t;
			t->read(it);
			return;
		}
		if (it.tag() == "Union") {
			UnionStructType* t = new UnionStructType("", true);
			obj = t;
			t->read(it);
			return;
		}
	}
	throw not_implemented_exception("Type::read");
}



//*************************************************************************************

ReferenceType::ReferenceType()
{
}

ReferenceType::ReferenceType(TypeType t)
: Type(t)
{
}

ReferenceType::ReferenceType(TypeType t, Type* n)
: Type(t)
, next_(n)
{
}

bool ReferenceType::is_composed() const
{
	return true;
}

Type* ReferenceType::base()
{
	return next()->base();
}

Type* ReferenceType::deref()
{
	return next();
}

Type* ReferenceType::copy()
{
	ReferenceType* res = new ReferenceType();
	*res = *this;
	return res;
}

bool ReferenceType::is_array_or_pointer() const
{
	return type_type_ == T_POINTER;
}

bool ReferenceType::is_pointer() const
{
	return type_type_ == T_POINTER;
}

bool ReferenceType::is_integer_or_pointer() const
{
	return is_pointer();
}

bool ReferenceType::is_direct() const
{
	return false;
}

size_t ReferenceType::depth() const
{
	size_t res = is_array_or_pointer() ? 1 : 0;
	res += next()->depth();
	return res;
}

void ReferenceType::print(std::ostream& out)
{
	base()->print(out);
	switch (type()) {
	case T_POINTER:		out << "*"; break;
	case T_REFERENCE:	out << "&"; break;
	case T_BOUNDEDARRAY:out << "[*]"; break;
	case T_ARRAY:		out << "[]"; break;
	default: out << "invalid pointer type";
	}
}

bool ReferenceType::append(ReferenceType& t)
{
	if (next()->append(t)) {
		return true;
	}
  if (t.is_pointer()) {
	  t.next(next());
	  next(&t);
	  return true;
  }
  return false;
}

bool ReferenceType::is_additive_compatible_with(Type& t)
{
	if (is_assignment_compatible_with(t)) {return true;}
	switch (type()) {
	case T_POINTER:
	case T_BOUNDEDARRAY:
	case T_ARRAY:
		return t.is_integer();

	default:;
	}
	return false;
}

bool ReferenceType::is_assignment_compatible_with(Type& t)
{
	if (type() == T_REFERENCE) {
		return next()->is_assignment_compatible_with(t);
	}
	if (t.type() == T_REFERENCE) {
		return false;
	}
	ReferenceType* rt = dynamic_cast<ReferenceType*>(&t);
	if (rt == 0) {
		return false;
	}
  if (next()->type() == T_VOID) {
    return true;
  }
	return next()->is_equivalent_with(*rt->next());
}

bool ReferenceType::is_equivalent_with(Type& t)
{
  if (&t == this) {return true;}
	if (type() == T_REFERENCE) {
		return next()->is_equivalent_with(t);
	}
	if (t.type() == T_REFERENCE) {
		return false;
	}
	ReferenceType* rt = dynamic_cast<ReferenceType*>(&t);
	if (rt == 0) {
		return false;
	}
	return next()->is_equivalent_with(*rt->next());
}

bool ReferenceType::is_recursive_equivalent_with(Type& t, std::multimap<void*, void*>& map)
{
  if (&t == this) {return true;}
	if (type() == T_REFERENCE) {
		return next()->is_recursive_equivalent_with(t, map);
	}
	if (t.type() == T_REFERENCE) {
		return false;
	}
	ReferenceType* rt = dynamic_cast<ReferenceType*>(&t);
	if (rt == 0) {
		return false;
	}
  std::multimap<void*, void*>::iterator it = map.find(this);
  while ((it != map.end()) && (it->first == this)) {
    if (it->second == rt) {
      return true;
    }
  }
  map.insert(std::make_pair(this, rt));
	return next()->is_recursive_equivalent_with(*rt->next(), map);
}

bool ReferenceType::is_cast_possible_with(Type& t)
{
	if (type() == T_REFERENCE) {
		return next()->is_cast_possible_with(t);
	}
	if (t.type() == T_REFERENCE) {
		return false;
	}
	ReferenceType* rt = dynamic_cast<ReferenceType*>(&t);
	if (rt == 0) {
		return false;
	}
	return next()->is_equivalent_with(*rt->next());
	return false;
}

void ReferenceType::write(xml::Writer& out)
{
	if (is_pointer()) {
		out.out() << "<Ptr>";
		base()->write(out);
		out.out() << "</Ptr>";
	}
	else if (is_reference()) {
		out.out() << "<Ref>";
		base()->write(out);
		out.out() << "</Ref>";
	}
	else {
		throw exception("ReferenceType::write invalid type");
	}
}

void ReferenceType::read(xml::Iterator& it)
{
	if (it.current() != xml::Element::tag) {throw exception("ReferenceType::read invalid file");}
	std::string tag = it.tag();
	if (tag == "Ptr") {type(T_POINTER);}
	else if (tag == "Ref") {type(T_REFERENCE);}
	else {throw exception("ReferenceType::read invalid type");}
	it.next_tag();
	Type::read(it, next_);
	it.next_tag();
	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw exception("ReferenceType::read invalid file");}
}

//*************************************************************************************

ArrayType::ArrayType()
: ReferenceType(T_ARRAY)
, limit_(0)
{
}

ArrayType::ArrayType(Type* base_type)
: ReferenceType(T_ARRAY, base_type)
, limit_(0)
{
}

ArrayType::ArrayType(Type* base_type, size_t limit)
: ReferenceType(T_BOUNDEDARRAY, base_type)
, limit_(limit)
{
	if (limit == 0) {type(T_ARRAY);}
}

ArrayType::ArrayType(TypeType t, Type* base_type, size_t limit)
: ReferenceType(t, base_type)
, limit_(limit)
{
}

size_t ArrayType::size() const
{
	if (limit_ > 0) {
		return next()->size() * limit_;
	}
	return 1; // size of a pointer
}

void ArrayType::limit(size_t val)
{
	if (is_array()) {
		if (val > 0) {
			type(T_BOUNDEDARRAY);
		}
		else {
			type(T_ARRAY);
		}
	}
	limit_ = val;
}

Type* ArrayType::copy()
{
	ArrayType* res = new ArrayType();
	*res = *this;
	return res;
}

bool ArrayType::append(ReferenceType& t)
{
	if (next()->append(t)) {
		return true;
	}
  if (t.is_array()) {
  	t.next(next());
  	next(&t);
  	return true;
  }
  return false;
}

bool ArrayType::is_direct() const
{
	return limit_ > 0;
}

bool ArrayType::is_array() const
{
	return type() != T_INITIALIZER;
}

bool ArrayType::is_array_or_pointer() const
{
	return type() != T_INITIALIZER;
}

bool ArrayType::is_initializer() const
{
	return type() == T_INITIALIZER;
}

void ArrayType::print(std::ostream& out)
{
	base()->print(out);
	out << "[";
	if (limit_ > 0) {
		out << limit_;
	}
	out << "]";
}

void ArrayType::write(xml::Writer& out)
{
	if (limit_ == 0) {
		out.out() << "<Array>";
	}
	else {
		out.out() << "<Array size='" << limit_ << "'>";
	}
	base()->write(out);
	out.out() << "</Array>";
}

void ArrayType::read(xml::Iterator& it)
{
	if (it.current() != xml::Element::tag) {throw exception("ReferenceType::read invalid file");}
	std::string tag = it.tag();
	if (tag == "Array") {type(T_ARRAY);}
	else {throw exception("ReferenceType::read invalid type");}
	it.next();
	if (it.current() == xml::Element::attribute) {
		while (it.current() == xml::Element::attribute) {
			if (it.key() == "size") {
				type(T_BOUNDEDARRAY);
				limit_ = boost::lexical_cast<size_t>(it.value());
			}
			it.next();
		}
		if (it.current() != xml::Element::content) {throw exception("ReferenceType::read invalid file");}
	}
	it.next_tag();
	Type::read(it, next_);
	it.next_tag();
	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw exception("ReferenceType::read invalid file");}
}

//*************************************************************************************



}
