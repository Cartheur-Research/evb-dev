#ifndef CVMC_HPP
#define CVMC_HPP

#ifdef _WIN32
#pragma warning (disable: 4251) // http://msdn2.microsoft.com/en-gb/library/esew7y1w.aspx
#pragma warning (disable: 4996)
#endif 

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <exception>

#include <boost/lexical_cast.hpp>

///////////////////////////////////////////////////////////////////////////////
//	Include Wave itself
#include <boost/wave.hpp>

///////////////////////////////////////////////////////////////////////////////
// Include the lexer stuff
#include <boost/wave/cpplexer/cpp_lex_token.hpp>		// token class
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp> // lexer class

#include <xml.hpp>


#define BIT_PER_INT		16
#define INT_MASK		0xffff

#define MIN_INT			(-0x8000)
#define MAX_INT			0x7fff
#define MAX_UINT		0x7fff

#define MIN_LONG		(-0x80000000L)
#define MAX_LONG		0x7fffffffL
#define MAX_ULONG		0xffffffffUL

#define MIN_LONGLONG		(-0x8000000000000000LL)
#define MAX_LONGLONG		0x7fffffffffffffffLL
#define MAX_ULONGLONG		0xffffffffffffffffULL

namespace cvmc {

class exception : public std::runtime_error {
public:
	explicit exception(const char* msg) : std::runtime_error(msg) {}
	explicit exception(const std::string& msg) : std::runtime_error(msg.c_str()) {}
};

class parser_exception : public exception {
public:
	explicit parser_exception(const char* msg) : exception(msg) {}
	explicit parser_exception(const std::string& msg) : exception(msg.c_str()) {}
};

class analyzer_exception : public exception {
public:
	explicit analyzer_exception(const char* msg) : exception(msg) {}
	explicit analyzer_exception(const std::string& msg) : exception(msg.c_str()) {}
};

class generator_exception : public exception {
public:
	explicit generator_exception(const char* msg) : exception(msg) {}
	explicit generator_exception(const std::string& msg) : exception(msg.c_str()) {}
};

class not_implemented_exception : public exception {
public:
	explicit not_implemented_exception(const char* msg) : exception(msg) {}
	explicit not_implemented_exception(const std::string& msg) : exception(msg.c_str()) {}
};

extern std::string token_id_to_string(const boost::wave::token_id& tid);

typedef unsigned short codepoint_type;
typedef std::vector<codepoint_type> code_type;


typedef boost::wave::cpplexer::lex_iterator<boost::wave::cpplexer::lex_token<> >lex_iterator_type;
typedef boost::wave::context<std::string::iterator, lex_iterator_type> context_type;
typedef std::deque<context_type::token_type> token_stack_type;

class ContextHelper {
	std::string						identifier_;
	context_type::iterator_type&	iterator_;
	context_type::iterator_type&	last_;
	boost::wave::token_id			current_id_;
	token_stack_type				backlog_;
	size_t							backlog_position_;

	void update();
	void handle_exception(boost::wave::preprocess_exception& ex);
	void handle_exception(boost::wave::macro_handling_exception& ex);

public:
	ContextHelper(context_type::iterator_type& it, context_type::iterator_type& last);

	bool is_left_paren();
	bool is_right_paren();
	bool is_left_bracket();
	bool is_right_bracket();
	bool is_left_brace();
	bool is_right_brace();

	context_type::token_type& current() {return backlog_.at(backlog_position_);}
	const context_type::position_type& location() {return current().get_position();}

	boost::wave::token_id current_token_id() {return current_id_;}

	const std::string& identifier();
  //void remove_identifier() {identifier_.clear();}

	bool filter();
	void next();
	bool finished() const;
	size_t mark() const {return backlog_position_;}
	void go_back_to(size_t pos);
	void flush();
};

#define MASK(x)	 (1 << x)

#define PAR_LIMIT 0xff
#define LOC_LIMIT 0xff
#define GLOB_LIMIT 0x1ff
#define PARAMETER_LIMIT 0xff
#define LOCALS_LIMIT 0xff
#define OFFSET_LIMIT 0xff

enum ScopeType {
	ST_UNDEFINED
,	ST_EXTERN
,	ST_FILE
,	ST_PROTOTYP
,	SC_BLOCK
};


enum FunctionSpecifier {
	FS_UNDEFINED
,	FS_INLINE
,	FS_NORETURN
};

enum TypeQualifier {
	TQ_UNDEFINED
,	TQ_CONST
,	TQ_RESTRICT
,	TQ_ATOMIC
,	TQ_VOLATILE
};

enum StorageClass {
	SC_UNDEFINED
,	SC_TYPEDEF
,	SC_EXTERN
,	SC_STATIC
,	SC_AUTO
,	SC_REGISTER
,	SC_THREAD_LOCAL
,	SC_SYSTEM
};

enum ObjType {
	OT_UNDEFINED
,	OT_ERROR
,	OT_PARSER
,	OT_CONTEXT
,	OT_TYPE
,	OT_FUNCTION
,	OT_VARIABLE
,	OT_STATEMENT
,	OT_CODE
};

enum ExprType {
	ET_UNDEFINED
,	ET_UNARY
,	ET_BINARY
,	ET_TERNARY
,	ET_ASSIGN
,	ET_CONST
};


enum TypeType {
	T_UNDEFINED
,	T_INVALID
,	T_VOID
,	T_BOOL
,	T_CHAR
,	T_SCHAR
,	T_UCHAR
,	T_SHORT
,	T_SSHORT
,	T_USHORT
,	T_INT
,	T_SINT
,	T_UINT
,	T_LONG
,	T_SLONG
,	T_ULONG
,	T_LONGLONG
,	T_SLONGLONG
,	T_ULONGLONG
,	T_FLOAT
,	T_DOUBLE
,	T_LONGDOUBLE
,	T_SIGNED
,	T_UNSIGNED
,	T_ENUM
,	T_STRUCT
,	T_UNION
,	T_BOUNDEDARRAY
,	T_ARRAY
,	T_INITIALIZER
,	T_POINTER
,	T_REFERENCE
,	T_ELLIPSIS
};

enum TerOp {
	TER_UNDEFINED
,	TER_QC
};

enum BinOp {
	BIN_UNDEFINED
,	BIN_ARRAY
,	BIN_ADD
,	BIN_SUB
,	BIN_MUL
,	BIN_DIV
,	BIN_MOD
,	BIN_BITAND
,	BIN_BITXOR
,	BIN_BITOR
,	BIN_LEFT
,	BIN_RIGHT
,	BIN_AND
,	BIN_OR
,	BIN_EQ
,	BIN_NE
,	BIN_LT
,	BIN_LE
,	BIN_GT
,	BIN_GE
,	BIN_COMMA
};

enum UnOp {
	UN_UNDEFINED
,	UN_PLUS
,	UN_NEGATE
,	UN_BITNOT
,	UN_NOT
,	UN_REF
,	UN_DEREF
,	UN_INC
,	UN_DEC
,	UN_MUL2
,	UN_DIV2
,	UN_PREINC
,	UN_PREDEC
,	UN_POSTINC
,	UN_POSTDEC
,	UN_SIZEOF
,	UN_ALIGNOF
,	UN_EQ0
,	UN_NE0
,	UN_LT0
,	UN_LE0
,	UN_GT0
,	UN_GE0
,	UN_OFFSET		// add with constant 0..1ff
};

enum AssOp {
	AS_UNDEFINED
,	AS_NORMAL
,	AS_ADD
,	AS_SUB
,	AS_MUL
,	AS_DIV
,	AS_MOD
,	AS_BITAND
,	AS_BITXOR
,	AS_BITOR
,	AS_LEFT
,	AS_RIGHT
};

enum ExtRefType {
	EXR_UNDEFINED
,	EXR_STRING
,	EXR_STATIC
,	EXR_FUNCTION
};

enum OptimizeType {
	OPTM_UNDEFINED
,	OPTM_CONST_FOLDING
,	OPTM_PEEPHOLE
,	OPTM_DEADCODE
};


class ArrayAccess;
class Call;
class Code;
class CompilationUnit;
class Constant;
class Context;
class Expression;
class Function;
class Jump;
class Linker;
class ReferenceType;
class Statement;
class Static;
class StringAssembler;
class Type;
class Variable;
class XRefManager;


class RefObj {
private:
	int			  ref_cnt_;
	ObjType		objtype_;
	ScopeType	scope_type_;

protected:
	void objtype(ObjType t) {objtype_ = t;}

public:
	RefObj();
	RefObj(ObjType t);
	virtual ~RefObj();

	ScopeType scope_type() const {return scope_type_;}
	void scope_type(ScopeType st) {scope_type_ = st;}

	virtual void print(std::ostream&);
	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

	virtual void link1(Linker&);	// linking pass 1
	virtual void link2(Linker&);	// linking pass 2

	ObjType objtype() const {return objtype_;}

	void inc_ref() {++ref_cnt_;}
	void dec_ref();

	virtual XRefManager* manager();
};

class ObjPtr {
private:
	RefObj*	ptr_;

public:
	ObjPtr() : ptr_(0) {}
	ObjPtr(RefObj* o);
	~ObjPtr();
	ObjPtr(const ObjPtr& op);
	ObjPtr& operator=(const ObjPtr& op);
	ObjPtr& operator=(RefObj* o);

	void clear();

	bool is_null() const {return ptr_ == 0;}
	bool is_not_null() const {return ptr_ != 0;}

	RefObj* get() {return ptr_;}
	//Type* get_type() {return reinterpret_cast<Type*>(ptr_);}
	//Statement* get_stmt() {return reinterpret_cast<Statement*>(ptr_);}
	//Expression* get_expr() {return reinterpret_cast<Expression*>(ptr_);}
	RefObj* operator*() {return ptr_;}
	RefObj* operator->() {return ptr_;}

	const RefObj* get() const {return ptr_;}
	const RefObj* operator*() const {return ptr_;}
	const RefObj* operator->() const {return ptr_;}

	Type* as_type();
	Constant* as_const();
	Function* as_function();
	Static* as_static();
	Variable* as_variable();
	Expression* as_expression();
	Statement* as_statement();

	void swap(ObjPtr& op) {RefObj* tmp = ptr_; ptr_ = op.ptr_; op.ptr_ = tmp;}
	void optimize(int mask);
	int compile(CompilationUnit&, bool push_result);
};

typedef std::vector<ObjPtr>	ObjPtr_list_type;
typedef std::vector<std::string>	string_list_type;
typedef std::vector<size_t>	offset_list_type;
typedef std::map<std::string, ObjPtr>	ObjPtr_map_type;
typedef std::map<int, ObjPtr>	ObjPtr_int_map_type;
typedef std::map<RefObj*, size_t> resolve_map_type;
typedef std::map<size_t, std::string> xref_map_type;
typedef std::map<std::string, size_t> string_map_type;
typedef std::map<size_t, size_t> remap_type;

class Code : public RefObj {
	ObjPtr		manager_;
	code_type	data_;
	size_t		opt_pos_;
	int			  opt_value_;
	bool		  is_data_;
	bool		  optimize_;
	bool		  optimize_space_;
	bool		  last_was_literal_;
	bool		  last_was_load_;

	void no_last();

	void disassemble_line(std::ostream&, size_t pos);


public:
	Code();
	
	virtual XRefManager* manager();
	void manager(XRefManager* t);

	virtual void print(std::ostream&);

	bool is_data() const {return is_data_;}
	void is_data(bool v) {is_data_ = v;}

	bool optimize() const {return optimize_;}
	void optimize(bool v) {optimize_ = v;}

	bool optimize_space() const {return optimize_space_;}
	void optimize_space(bool v) {optimize_space_ = v;}


	//void assemble(const std::string&);

	void disassemble_data(std::ostream&, size_t& pos, size_t len);
	void disassemble_code(std::ostream&, size_t& pos, size_t len, Linker*);
	void disassemble_string_literal(std::ostream&, size_t& pos, size_t len, Linker*);
	void disassemble_global(std::ostream&, size_t& pos, size_t len, Linker*);

	void clear();
	void append_data(codepoint_type);
	void append(codepoint_type);
	void data(size_t, codepoint_type);
	codepoint_type data(size_t) const;

	void fixup_string_literal(size_t pos, size_t string_pos);
	void fixup_global(size_t pos, size_t string_pos);
	void fixup_call(size_t pos, size_t string_pos);

	void append(const Code&);
	void adjust(size_t pos, int value);

	const code_type& data() const {return data_;}
	code_type& data() {return data_;}
	size_t size() const {return data_.size();}

	void flush_opt() {opt_pos_ = data_.size();}

	int compile_nop();						// do nothing
//	int compile_new();						// allocate on the heap


	int compile_xs();						// exchange r with stack pointer
	int compile_xp();						// exchange r with program counter
	int compile_xf();						// exchange r with frame pointer
	//int compile_xn();						// exchange r with new pointer
	int compile_xg();						// exchange r with global pointer
	int compile_xd();						// exchange r with d
	int compile_xt();						// exchange r with t
	int compile_xpt();						// exchange r with port register

	int compile_cx();						// compare and exchange

	int compile_push();						// push R onto the stack, return the change in stack size
	int compile_load_push();				// load word from mem[r] and push it on the stack. r is unchangeds
	int compile_pop();						// pop R from the stack, return the change in stack size
	//int compile_push2();					// push D,R onto the stack, return the change in stack size
	//int compile_pop2();						// pop D,R from the stack, return the change in stack size
	int compile_push_opt();					// pop R from the stack
	int compile_pop_opt();					// pop R from the stack
	int compile_inc();						// R += 1
	int compile_dec();						// R -= 1
	int compile_add();						// R = pop() + R
	int compile_add_opt(int val);			// R = R + val, optimized
	int compile_sub_opt(int val);			// R = R - val, optimized
	int compile_mul_opt(int val);			// R = R * val, optimized
	//int compile_add2();						// D,R = pop(),pop() + D,R
	int compile_sub();						// R = pop() - R
	int compile_and();						// R = pop() & R
	int compile_or();						// R = pop() | R
	int compile_xor();						// R = pop() ^ R
	int compile_mul();						// R = pop() * R
	int compile_negate();					// R = -R
	int compile_not();						// R = ~R
	int compile_bitcnt();						// R = bitcount1(R)
	int compile_mul2();						// R <<= 1
	int compile_div2();						// R >>= 1
	int compile_udiv2();					// R >>= 1, unsigned
	int compile_load();						// R = mem(R)
	int compile_store();					// mem(R) = pop()
	int compile_literal(int val, bool optimze=true);			// 16 bit literal
	int compile_literal_short(int val);	// 16 bit short literal
	int compile_literal_long(long val);		// 32 bit literal
	int compile_push_literal(int val);		// push 16 bit literal on the stack
	int compile_push_long(long val);		// push 32 bit literal on the stack (push lo word first, then hi word)
	//int compile_push_literal2(int val1, int val2);	// push 2x16 bit literal on the stack. val2 is new top of stack

	int compile_addc();						// carry,R = pop() + R + carry
	int compile_lshc();						// carry,R = R,carry << 1
	int compile_rshc();						// R,carry = carry,R >> 1
	int compile_umul();						// R,D = pop() * R
	int compile_sext();						// D = sign(R) (either 0 or -1)
	int compile_zext();						// D = 0

	int compile_in();						// read 18 bit from port
	int compile_out();						// write 18 bit to port
	int compile_ldlo();						// 
	int compile_ldhi();						// 
	int compile_stlo();						// 
	int compile_sthi();						// 

	int compile_ldt();						// move t to r
	int compile_stt();						// move r to t

	int compile_ldx();						// 
	int compile_stx();						// 
	int compile_stmk();						// 

  int compile_std();						// move r to d
  int compile_ldd();						// move d to r

	int compile_true();						// R = 1
	int compile_false();					// R = 0

  int compile_sl();						// R = pop() << R
  int compile_sr();						// R = pop() >> R
  int compile_usr();						// R = pop() >> R

	int compile_eq();						// R = pop() == R
	int compile_ne();						// R = pop() != R
	int compile_gt();						// R = pop() > R
	int compile_ge();						// R = pop() >= R
	int compile_lt();						// R = pop() < R
	int compile_le();						// R = pop() <= R
	int compile_ugt();						// R = pop() > R (unsigned)
	int compile_uge();						// R = pop() >= R (unsigned)
	int compile_ult();						// R = pop() < R (unsigned)
	int compile_ule();						// R = pop() <= R (unsigned)
	int compile_gt0();						// R = R > 0
	int compile_ge0();						// R = R >= 0
	int compile_lt0();						// R = R < 0
	int compile_le0();						// R = R <= 0
	int compile_eq0();						// R = !R (or R == 0)
	int compile_ne0();						// R = !!R (or R != 0)

	int compile_tjmp();						// PC = PC + mem(PC+R) + 1

	int compile_push_long();				// push D, R onto the stack
	int compile_pop_long();					// pop D, R from the stack
	int compile_push_long_opt(bool final_xd=true);			// push D, R onto the stack (optimized)
	int compile_pop_long_opt();				// pop D, R from the stack (optimized)
	//int compile_exit();						// exit stack frame
	int compile_return();					// return from subroutine
	int compile_branch();
	int compile_cond_branch();

	void fixup_branch(size_t pos, size_t target_pos);
	int compile_call(size_t adr);
	int compile_terminate();
	int compile_wait();

	int compile_load_local(size_t offset);
	int compile_store_local(size_t offset);
	int compile_adr_local(size_t offset);
	int compile_load_parameter(size_t offset);
	int compile_store_parameter(size_t offset);
	int compile_adr_parameter(size_t offset);
	//int compile_load_global(size_t offset);
	//int compile_store_global(size_t offset);
	int compile_adr_global(size_t offset);

	int compile_load_offset(size_t offset);
	int compile_store_offset(size_t offset);
	int compile_adr_offset(size_t offset);

	int compile_reserve(size_t words_on_stack_to_be_reserved);
	int compile_enter(size_t local_space);
	int compile_exit(size_t parameter_space);	// add argument_space to stack pointer

	void write(std::ostream& out, bool big_endian);
	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);


};

class CompilationUnit : public RefObj {
protected:
	Function*	func_;
	Code		  code_;
	Code		  data_;

public:
	CompilationUnit();
	CompilationUnit(Function*);

	Function* function() {return func_;}

	Code& code() {return code_;}
	Code& data() {return data_;}

	int compile_condition(Expression*, bool push_result);

	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);
};

class Issue : public RefObj {
protected:
	context_type::position_type	where_;
	std::string						      what_;
	bool							          has_where_;
	bool							          is_error_;

public:
	Issue(const context_type::position_type& w1, const std::string& w2, bool is_error);
	Issue(const std::string& w2, bool is_error);

	void print(std::ostream& out);
};

class ExtRef : public RefObj {
protected:
	ExtRefType			  type_;
	size_t				    link_position_;
	size_t				    offset_;
	std::string			  name_;
	offset_list_type  fixup_positions_;

public:
	ExtRef();
	ExtRef(ExtRefType);
	ExtRef(ExtRefType, const std::string&);
	ExtRef(ExtRefType, const std::string&, size_t offset);

	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

	ExtRefType type() {return type_;}

	//void link_position(size_t t) {link_position_ = t;}
	size_t link_position() const {return link_position_;}

	void name(const std::string& t) {name_ = t;}
	const std::string& name() const {return name_;}

	void add_fixup_position(size_t pos) {fixup_positions_.push_back(pos);}
	size_t fixup_positions() const {return fixup_positions_.size();}
	size_t fixup_position(size_t pos) const {return fixup_positions_[pos];}

	void link1(Linker&, size_t offset);	// linking pass 1
	void link2(Linker&, size_t offset);	// linking pass 2
};

class XRefManager : public RefObj {
protected:
	ObjPtr_map_type		xref_map_;
	ObjPtr_list_type	xref_list_;
	ObjPtr_map_type		string_literal_map_;

public:
	XRefManager();

	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

	bool empty() const;

	size_t xrefs() const {return xref_list_.size();}
	ExtRef* xref(size_t pos) {return static_cast<ExtRef*>(xref_list_[pos].get());}

	void add(ExtRef*);
	void add_string_literal(const std::string&, size_t pos);
	void add_function(const std::string&);
	void add_call(const std::string&, size_t pos);
	void add_static(const std::string&);
	void add_static_ref(const std::string&, size_t pos);
	void add_static_ref(const std::string&, size_t pos, size_t offset);

	void link1(Linker&, size_t offset);	// linking pass 1
	void link2(Linker&, size_t offset);	// linking pass 2
};

class Statement : public RefObj {
protected:
	context_type::position_type		where_;

public:
	Statement();
	Statement(const context_type::position_type& w);

	context_type::position_type& location() {return where_;}
	void location(const context_type::position_type& val) {where_ = val;}

	virtual bool last_is_return() const;
	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual int compile_condition(CompilationUnit& cu, bool push_result);

	static Statement* readFrom(xml::Iterator&);

};

class HashFinder {
private:
  typedef std::map<long long, short> map_type;
  map_type            values_;
  long long           min_;
  long long           max_;
  unsigned short  a_;
  unsigned short  b_;
  unsigned short  c_;
  unsigned short  d_;

  void clear();
  int evaluate(unsigned short value);

public:
  HashFinder();

  static int highestBit(unsigned long long);

  void add(long long);
  bool find();
	int compile(CompilationUnit&);
	int compile_table(CompilationUnit&);
};

class UnsignedHashFinder {
private:
  typedef std::map<unsigned long long, short> map_type;
  map_type            values_;
  unsigned long long  min_;
  unsigned long long  max_;
  unsigned long long  a_;
  unsigned long long  b_;
  unsigned long long  c_;
  unsigned long long  d_;

public:
  UnsignedHashFinder();

  void add(unsigned long long);
  bool find();
	int compile(CompilationUnit&);
};

class Block : public Statement {
protected:
	ObjPtr_list_type	list_;

public:
	Block();
	Block(const context_type::position_type& w);

	void add(Statement* stmt) {list_.push_back(stmt);}
	size_t size() const {return list_.size();}
	Statement* statement(size_t pos) {return static_cast<Statement*>(list_[pos].get());}
	const Statement* statement(size_t pos) const {return static_cast<const Statement*>(list_[pos].get());}

	virtual bool last_is_return() const;
	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
};

class Type : public RefObj {
protected:
	TypeType	type_type_;
	int				storage_class_mask_;
	int				qualifier_mask_;

public:
	Type();
	Type(TypeType t);

	virtual Type* base();
	virtual Type* deref();

	TypeType type() const {return type_type_;}
	void type(TypeType t);

	int storage_class_mask() const {return storage_class_mask_;}
	void storage_class_mask(int val) {storage_class_mask_ = val;}
	//bool add_storage_class(StorageClass sc);
	bool has_storage_class(StorageClass sc) {return (MASK(sc) & storage_class_mask_) != 0;}

	int qualifier_mask() const {return qualifier_mask_;}
	void qualifier_mask(int val) {qualifier_mask_ = val;}
	//bool add_qualifier(TypeQualifier tq);
	bool has_qualifier(TypeQualifier tq) {return (MASK(tq) & qualifier_mask_) != 0;}

	bool is_unsigned() const;

	bool is_indirection() const {return (type_type_ == T_POINTER) || (type_type_ == T_REFERENCE);}
	bool is_reference() const {return type_type_ == T_REFERENCE;};

	bool is_simple() const {return (type_type_ >= T_VOID) || (type_type_ <= T_UNSIGNED);}

  int conversion_rank() const;

	virtual bool is_direct() const;
	virtual bool is_integer() const;
	virtual bool is_integer_or_pointer() const;
	virtual bool is_float() const;
	virtual bool is_array() const;
	virtual bool is_initializer() const;
	virtual bool is_pointer() const;
	virtual bool is_array_or_pointer() const;
	virtual bool is_valid(bool first=true) const;
	virtual bool is_composed() const;
	virtual bool is_assignment_compatible_with(Type&);
	virtual bool is_additive_compatible_with(Type&);
	virtual bool is_equivalent_with(Type&);
	virtual bool is_recursive_equivalent_with(Type&, std::multimap<void*, void*>& map);
	virtual bool is_cast_possible_with(Type&);

	virtual size_t size() const;
	//virtual size_t var_size() const; // size of variable holding a member of this type
	virtual size_t depth() const;

	virtual bool append(ReferenceType& t);
	virtual Type* copy();
	virtual void print(std::ostream&);
	
	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

	static void read(xml::Iterator&, ObjPtr&);
	static Type* basic(TypeType);
};

class ReferenceType : public Type {
protected:
	ObjPtr			next_;

public:
	ReferenceType();
	ReferenceType(TypeType t);
	ReferenceType(TypeType t, Type* n);

	virtual bool is_composed() const;
	virtual bool is_additive_compatible_with(Type&);
	virtual bool is_assignment_compatible_with(Type&);
	virtual bool is_equivalent_with(Type&);
	virtual bool is_recursive_equivalent_with(Type&, std::multimap<void*, void*>& map);
	virtual bool is_cast_possible_with(Type&);

	virtual bool is_direct() const;
	virtual bool is_pointer() const;
	virtual bool is_integer_or_pointer() const;
	virtual bool is_array_or_pointer() const;

	void next(Type* t) {next_ = t;}
	Type* next() {return static_cast<Type*>(next_.get());}
	const Type* next() const {return static_cast<const Type*>(next_.get());}

	virtual bool append(ReferenceType& t);
	virtual Type* copy();
	virtual Type* base();
	virtual void print(std::ostream&);
	virtual Type* deref();
	virtual size_t depth() const;

	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

};

class ArrayType : public ReferenceType {
	size_t		limit_;

public:
	ArrayType();
	ArrayType(Type* base_type);
	ArrayType(Type* base_type, size_t limit);
	ArrayType(TypeType t, Type* base_type, size_t limit);

	virtual bool append(ReferenceType& t);
	virtual bool is_direct() const;
	virtual bool is_array() const;
	virtual bool is_initializer() const;
	virtual bool is_array_or_pointer() const;
	size_t limit() const {return limit_;}
	void limit(size_t val);
	virtual void print(std::ostream&);
	virtual Type* copy();
	virtual size_t size() const;

	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

};

class Expression : public Statement {
protected:
	ObjPtr			type_;

public:
	Expression();
	Expression(Type* t);
	Expression(const context_type::position_type& w);
	Expression(const context_type::position_type& w, Type* t);

	virtual void set_type(Type* val);
	Type* type();
	const Type* type() const;
	virtual void build_type();
	virtual Type* lvalue_type();

	virtual size_t type_size();
	//virtual void append(ObjPtr&, ArrayAccess& acc);

	virtual ExprType expr_type() const;
	virtual bool is_const() const;
	virtual bool is_negative() const;
	virtual bool is_lvalue() const;
	virtual bool has_side_effect() const;

	virtual bool get_int(int&) const;				// only constant expressions should return a true
	virtual bool get_long(long&) const;				// only constant expressions should return a true
	virtual bool get_longlong(long long&) const;	// only constant expressions should return a true

	bool get_uint(unsigned int&) const;	// only constant expressions should return a true
	bool get_ulong(unsigned long&) const;	// only constant expressions should return a true
	bool get_ulonglong(unsigned long long&) const;	// only constant expressions should return a true

	virtual size_t initializer_depth() const;

	virtual bool is_unary(UnOp) const;
	virtual bool is_binary(BinOp) const;

	virtual void compile_data(CompilationUnit& cu);

	virtual int compile_push(CompilationUnit& cu);
	virtual int compile_pop(CompilationUnit& cu);
	virtual int compile_push(CompilationUnit& cu, size_t offset);
	virtual int compile_pop(CompilationUnit& cu, size_t offset);


	virtual int compile_load(CompilationUnit& cu, bool push_result);
	virtual int compile_store(CompilationUnit& cu);
	virtual int compile_load_word(CompilationUnit& cu, size_t offset, bool push_result);
	virtual int compile_store_word(CompilationUnit& cu, size_t offset);

	virtual int compile_adr(CompilationUnit& cu, size_t offset=0);
};

class Initializer : public Expression {
protected:
	ObjPtr_list_type	list_;


public:
	Initializer();
	Initializer(const context_type::position_type& w);

	virtual bool is_const() const;
	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();

	virtual size_t initializer_depth() const;
	size_t size() const {return list_.size();}
	Expression* child(size_t pos) {return static_cast<Expression*>(list_.at(pos).get());}
	const Expression* child(size_t pos) const {return static_cast<const Expression*>(list_.at(pos).get());}
	virtual void compile_data(CompilationUnit& cu);

	void add(Expression*);
};

class Assembler : public Expression {
protected:
	StringAssembler*  ass_;
	std::string				target_;
	std::string				source_;

public:
	Assembler();
	~Assembler();
	Assembler(const context_type::position_type& w);
	Assembler(const context_type::position_type& w, Type* t);

	void clear();
	virtual void build_type();

	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);

	void target(const std::string& t);
	const std::string& target() const {return target_;}

	void source(const std::string& t) {source_ = t;}
	const std::string& source() const {return source_;}
	std::string& source() {return source_;}

};

class ArrayAccess : public Expression {
protected:
	ObjPtr  base_;
	ObjPtr  index_;
	bool	  is_optimized_;
	bool	  is_direct_;

public:
	ArrayAccess(Expression* base = 0);
	ArrayAccess(const context_type::position_type& w, Expression* base = 0);

	void base(Expression* val);
	Expression* base() {return static_cast<Expression*>(base_.get());}
	const Expression* base() const {return static_cast<const Expression*>(base_.get());}

	Expression* index() {return static_cast<Expression*>(index_.get());}
	const Expression* index() const {return static_cast<const Expression*>(index_.get());}

	bool is_optimized() const {return is_optimized_;}

	virtual void build_type();
	virtual void optimize(ObjPtr&, int mask);
	virtual Type* lvalue_type();
	virtual bool is_const() const;
	virtual bool is_lvalue() const;
	//virtual void append(ObjPtr&, ArrayAccess& acc);

	void add_index(Expression* index);

	virtual int compile_push(CompilationUnit& cu);
	virtual int compile_pop(CompilationUnit& cu);
	virtual int compile_push(CompilationUnit& cu, size_t offset);
	virtual int compile_pop(CompilationUnit& cu, size_t offset);


	virtual int compile_load(CompilationUnit& cu, bool push_result);
	virtual int compile_store(CompilationUnit& cu);
	virtual int compile_load_word(CompilationUnit& cu, size_t offset, bool push_result);
	virtual int compile_store_word(CompilationUnit& cu, size_t offset);

	virtual int compile_adr(CompilationUnit& cu, size_t offset=0);

	virtual int compile(CompilationUnit& cu, bool push_result);

};

class ArrayIndex : public Expression {
protected:
	ObjPtr  base_;
	ObjPtr  index_;
	bool	  is_optimized_;
	bool	  is_direct_;

public:
	ArrayIndex(Expression* base = 0);
	ArrayIndex(const context_type::position_type& w, Expression* base = 0);

	void base(Expression* val);
	Expression* base() {return static_cast<Expression*>(base_.get());}
	const Expression* base() const {return static_cast<const Expression*>(base_.get());}

	Expression* index() {return static_cast<Expression*>(index_.get());}
	const Expression* index() const {return static_cast<const Expression*>(index_.get());}

	bool is_optimized() const {return is_optimized_;}

	virtual void build_type();
	virtual void optimize(ObjPtr&, int mask);
	//virtual Type* lvalue_type();
	virtual bool is_const() const;
	virtual int calculate_offset() const;
	//virtual bool is_lvalue() const;
	//virtual void append(ObjPtr&, ArrayIndex& acc);

	void add_index(Expression* index);

	virtual int compile_push(CompilationUnit& cu);
	virtual int compile_pop(CompilationUnit& cu);
	virtual int compile_push(CompilationUnit& cu, size_t offset);
	virtual int compile_pop(CompilationUnit& cu, size_t offset);


	virtual int compile_load(CompilationUnit& cu, bool push_result);
	virtual int compile_store(CompilationUnit& cu);
	virtual int compile_load_word(CompilationUnit& cu, size_t offset, bool push_result);
	virtual int compile_store_word(CompilationUnit& cu, size_t offset);

	virtual int compile_adr(CompilationUnit& cu, size_t offset=0);

	virtual int compile(CompilationUnit& cu, bool push_result);

};

class OffsetAccess : public Expression {
protected:
	ObjPtr  base_;
	ObjPtr  offset_;
	bool	  deref_;

public:
	OffsetAccess(Expression* base, Type* type, bool deref);
	OffsetAccess(const context_type::position_type& w, Expression* base, Type* type, bool deref);

	void deref(bool val) {deref_ = val;}
	bool deref() const {return deref_;}

	void base(Expression* val);
	Expression* base() {return static_cast<Expression*>(base_.get());}
	const Expression* base() const {return static_cast<const Expression*>(base_.get());}

	Expression* offset() {return static_cast<Expression*>(offset_.get());}
	const Expression* offset() const {return static_cast<const Expression*>(offset_.get());}

	virtual void build_type();
	virtual void optimize(ObjPtr&, int mask);
	virtual Type* lvalue_type();
	virtual bool is_const() const;
	virtual int calculate_offset() const;
	virtual bool is_lvalue() const;
	//virtual void append(ObjPtr&, ArrayAccess& acc);

	void add_offset(Expression* offset);

	virtual int compile_push(CompilationUnit& cu);
	virtual int compile_pop(CompilationUnit& cu);
	virtual int compile_push(CompilationUnit& cu, size_t offset);
	virtual int compile_pop(CompilationUnit& cu, size_t offset);


	virtual int compile_load(CompilationUnit& cu, bool push_result);
	virtual int compile_store(CompilationUnit& cu);
	virtual int compile_load_word(CompilationUnit& cu, size_t offset, bool push_result);
	virtual int compile_store_word(CompilationUnit& cu, size_t offset);

	virtual int compile_adr(CompilationUnit& cu, size_t offset=0);

	virtual int compile(CompilationUnit& cu, bool push_result);

};

class Cast : public Expression {
protected:
	ObjPtr	from_;
	ObjPtr	to_;

public:
	Cast() {}
	Cast(const context_type::position_type& w) : Expression(w) {}
	Cast(const context_type::position_type& w, Type*, Expression*);

	void from(Expression* val) {from_ = val;}
	Expression* from() {return static_cast<Expression*>(from_.get());}
	const Expression* from() const {return static_cast<const Expression*>(from_.get());}

	void to(Type* val) {to_ = val;}
	Type* to() {return static_cast<Type*>(to_.get());}
	const Type* to() const {return static_cast<const Type*>(to_.get());}

  virtual bool is_const() const;
	virtual bool is_lvalue() const;
	virtual bool has_side_effect() const;

	virtual int compile_push(CompilationUnit& cu);
	virtual int compile_pop(CompilationUnit& cu);
	virtual int compile_push(CompilationUnit& cu, size_t offset);
	virtual int compile_pop(CompilationUnit& cu, size_t offset);

	virtual int compile_load(CompilationUnit& cu, bool push_result);
	virtual int compile_store(CompilationUnit& cu);
	virtual int compile_load_word(CompilationUnit& cu, size_t offset, bool push_result);
	virtual int compile_store_word(CompilationUnit& cu, size_t offset);

	virtual int compile_adr(CompilationUnit& cu, size_t offset=0);

	virtual void build_type();
	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);

};

class AutoCast : public Cast {
public:
	enum CastType {
		normal,
		additive
	};
private:
	CastType	cast_type_;

public:
	AutoCast(CastType = normal);
	AutoCast(const context_type::position_type& w, CastType = normal);
	AutoCast(const context_type::position_type& w, Type*, Expression*, CastType = normal);

	void cast_type(CastType val) {cast_type_ = val;}
	CastType cast_type() const {return cast_type_;}

	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual void compile_data(CompilationUnit& cu);

};

class Constant : public Expression {
	std::string	name_;
	size_t	    literal_;

public:
	Constant();
	Constant(const context_type::position_type& w);
	
	void literal(size_t t) {literal_ = t;}
	size_t literal() const {return literal_;}

	void name(const std::string& t) {name_ = t;}
	const std::string& name() const {return name_;}

	//virtual bool is_negative() const;
	virtual Type* lvalue_type();
	virtual void build_type();
	virtual ExprType expr_type() const;
	virtual bool is_const() const;
};


class DataOffset : public Constant {
	unsigned int	value_;
public:
	DataOffset(unsigned int val, Type* t);

	void value(unsigned int val) {value_ = val;}
	unsigned int value() const {return value_;}

	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();

};

class GlobalOffset : public Constant {
	unsigned int	value_;
public:
	GlobalOffset(unsigned int val) : value_(val) {}

	void value(unsigned int val) {value_ = val;}
	unsigned int value() const {return value_;}

	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();

};

class IntConst : public Constant {
	int	value_;
public:
	IntConst(int val) : value_(val) {}
	IntConst(const context_type::position_type& w, int val) : Constant(w), value_(val) {}

	void value(int val) {value_ = val;}
	int value() const {return value_;}

	virtual bool is_negative() const;
	virtual bool get_int(int& val) const;
	virtual bool get_long(long& val) const;
	virtual bool get_longlong(long long& val) const;

	virtual void compile_data(CompilationUnit& cu);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();

};

class UIntConst : public Constant {
	unsigned int	value_;
public:
	UIntConst(unsigned int val) : value_(val) {}
	UIntConst(const context_type::position_type& w, unsigned int val) : Constant(w), value_(val) {}

	void value(unsigned int val) {value_ = val;}
	unsigned int value() const {return value_;}

	virtual bool get_int(int& val) const;
	virtual bool get_long(long& val) const;
	virtual bool get_longlong(long long& val) const;

	virtual void compile_data(CompilationUnit& cu);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();

};

class LongConst : public Constant {
	long	value_;
public:
	LongConst(long val) : value_(val) {}
	LongConst(const context_type::position_type& w, long val) : Constant(w), value_(val) {}

	void value(long val) {value_ = val;}
	long value() const {return value_;}

	virtual bool is_negative() const;
	virtual bool get_int(int& val) const;
	virtual bool get_long(long& val) const;
	virtual bool get_longlong(long long& val) const;

	virtual void compile_data(CompilationUnit& cu);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();
};

class ULongConst : public Constant {
	unsigned long	value_;
public:
	ULongConst(unsigned long val) : value_(val) {}
	ULongConst(const context_type::position_type& w, unsigned long val) : Constant(w), value_(val) {}

	void value(unsigned long val) {value_ = val;}
	unsigned long value() const {return value_;}

	virtual bool get_int(int& val) const;
	virtual bool get_long(long& val) const;
	virtual bool get_longlong(long long& val) const;

	virtual void compile_data(CompilationUnit& cu);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();
};

class LongLongConst : public Constant {
	long long	value_;
public:
	LongLongConst(long long val) : value_(val) {}
	LongLongConst(const context_type::position_type& w, long long val) : Constant(w), value_(val) {}

	void value(long long val) {value_ = val;}
	long long value() const {return value_;}

	virtual bool is_negative() const;
	virtual bool get_int(int& val) const;
	virtual bool get_long(long& val) const;
	virtual bool get_longlong(long long& val) const;

	
	virtual void compile_data(CompilationUnit& cu);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();
};

class ULongLongConst : public Constant {
	unsigned long long	value_;
public:
	ULongLongConst(unsigned long long val) : value_(val) {}
	ULongLongConst(const context_type::position_type& w, unsigned long long val) : Constant(w), value_(val) {}

	void value(unsigned long long val) {value_ = val;}
	unsigned long long value() const {return value_;}

	virtual bool get_int(int& val) const;
	virtual bool get_long(long& val) const;
	virtual bool get_longlong(long long& val) const;

	virtual void compile_data(CompilationUnit& cu);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();
};

class StringConst : public Constant {
	std::string	value_;
public:
	StringConst();
	StringConst(const context_type::position_type& w, const std::string& val);

	void value(const std::string& val) {value_ = val;}
	const std::string& value() const {return value_;}

	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();

	virtual void write(xml::Writer&);
	virtual void disassembly(std::ostream&);
	virtual void read(xml::Iterator&);

	virtual void link1(Linker&);	// linking pass 1
	virtual void link2(Linker&);	// linking pass 2

  static bool hex_to_int(char, int&);
  static bool oct_to_int(char, int&);
};

class BoolConst : public Constant {
	bool	value_;
public:
	BoolConst(const context_type::position_type& w, bool val) : Constant(w), value_(val) {}

	void value(bool val) {value_ = val;}
	bool value() const {return value_;}

	virtual bool get_int(int& val) const;
	virtual bool get_long(long& val) const;
	virtual bool get_longlong(long long& val) const;
	
	virtual void compile_data(CompilationUnit& cu);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();
};

class FloatConst : public Constant {
	float	value_;
public:
	FloatConst(const context_type::position_type& w, float val) : Constant(w), value_(val) {}

	virtual void compile_data(CompilationUnit& cu);
	void value(float val) {value_ = val;}
	float value() const {return value_;}
};

class DoubleConst : public Constant {
	double	value_;
public:
	DoubleConst(const context_type::position_type& w, double val) : Constant(w), value_(val) {}

	virtual void compile_data(CompilationUnit& cu);
	void value(double val) {value_ = val;}
	double value() const {return value_;}
};

class LongDoubleConst : public Constant {
	long double	value_;
public:
	LongDoubleConst(const context_type::position_type& w, long double val) : Constant(w), value_(val) {}

	virtual void compile_data(CompilationUnit& cu);
	void value(long double val) {value_ = val;}
	long double value() const {return value_;}
};

class ConstData : public Constant {
	ObjPtr_list_type		data_list_;

public:
	ConstData();
	ConstData(const context_type::position_type& w);

	size_t size() const {return data_list_.size();}

	void add(Constant*);

	Constant* next(size_t pos) {return static_cast<Constant*>(data_list_.at(pos).get());}
	const Constant* next(size_t pos) const {return static_cast<const Constant*>(data_list_.at(pos).get());}

};

class Variable : public Expression {
protected:
	std::string	name_;
	size_t			offset_;
	bool			  initialized_;
	bool			  defined_;

public:
	Variable();
	Variable(const context_type::position_type& w);
	Variable(const context_type::position_type& w, Type* type);
	Variable(Type* type);

	bool is_initialized() const {return initialized_;}
	virtual void initialize();

	bool is_defined() const {return defined_;}
	void define() {defined_ = true;}

	size_t offset() const {return offset_;}
	void offset(size_t val) {offset_ = val;}
	
	void name(const std::string& t) {name_ = t;}
	const std::string& name() const {return name_;}

	virtual bool is_lvalue() const;
	virtual void build_type();
	virtual Type* lvalue_type();

	virtual int compile(CompilationUnit&, bool push_result);
};

class EnumType : public Type {
protected:
	ObjPtr_list_type		member_list_;
	ObjPtr_map_type			members_;
	ObjPtr_int_map_type	values_;
	size_t					    size_;
	int						      next_value_;
	int						      min_;
	int						      max_;

public:
	EnumType();

	size_t no_of_member() const {return member_list_.size();}
	int min() const {return min_;}
	int max() const {return max_;}
	virtual bool is_integer() const;
	virtual bool is_integer_or_pointer() const;

	virtual Type* copy();
	size_t size() const;
	virtual void add(const std::string& name, IntConst* value);
	IntConst* add(const std::string& name);
	IntConst* member(size_t pos);
	IntConst* member(const std::string& name);
	IntConst* value(int);
};

class UnionStructMember : public Constant {
protected:
	size_t		offset_;

public:
	UnionStructMember(const context_type::position_type& w);
	UnionStructMember(const context_type::position_type& w, Type* type);
	UnionStructMember(Type* type);

	size_t offset() const {return offset_;}
	void offset(size_t val) {offset_ = val;}

	bool is_recursive_equivalent_with(UnionStructMember&, std::multimap<void*, void*>& map);

	virtual bool is_negative() const;
	virtual bool get_int(int& val) const;

	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

};

class UnionStructType : public Type {
protected:
  std::string       name_;
	ObjPtr_list_type	member_list_;
	ObjPtr_map_type		members_;
	size_t				    size_;
	bool				      is_union_;

public:
	UnionStructType(const std::string& name, bool is_union);

  void name(const std::string& val) {name_ = val;}
  const std::string& name() const {return name_;}

	size_t no_of_member() const {return member_list_.size();}

	virtual void add(const std::string& name, UnionStructMember*);
	UnionStructMember* member(size_t pos);
	UnionStructMember* member(const std::string& name);

	virtual Type* copy();
	size_t size() const;
	bool is_union() const {return is_union_;}
	bool is_struct() const {return !is_union_;}

	virtual bool is_equivalent_with(Type&);
	virtual bool is_recursive_equivalent_with(Type&, std::multimap<void*, void*>& map);

	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

};

class Label : public Expression {
protected:
	ObjPtr_list_type	fixups_;
	size_t				    value_;
	bool				      is_declared_;
	bool				      is_compiled_;

public:
	Label(const context_type::position_type& w);

	bool is_declared() const {return is_declared_;}
	void declare() {is_declared_ = true;}

	virtual void add(CompilationUnit& cu, Jump*);
	virtual int compile(CompilationUnit&, bool push_result);


};

class Parameter : public Variable {
protected:
  //size_t  size_;

public:
	Parameter();
	Parameter(Type* type);
	Parameter(const context_type::position_type& w, Type* type);

	//virtual size_t type_size();
	virtual void set_type(Type* val);
	
	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);
	
	virtual int compile_push(CompilationUnit& cu);
	virtual int compile_pop(CompilationUnit& cu);
	virtual int compile_push(CompilationUnit& cu, size_t offset);
	virtual int compile_pop(CompilationUnit& cu, size_t offset);

	virtual int compile_load(CompilationUnit& cu, bool push_result);
	virtual int compile_store(CompilationUnit& cu);
	virtual int compile_load_word(CompilationUnit& cu, size_t off, bool push_result);
	virtual int compile_store_word(CompilationUnit& cu, size_t off);
	virtual int compile_adr(CompilationUnit& cu, size_t off=0);
	virtual void print(std::ostream&);
};

class Local : public Variable {
public:
	Local(const context_type::position_type& w, Type* type);
	
	virtual int compile_push(CompilationUnit& cu);
	virtual int compile_pop(CompilationUnit& cu);
	virtual int compile_push(CompilationUnit& cu, size_t offset);
	virtual int compile_pop(CompilationUnit& cu, size_t offset);

	virtual int compile_load(CompilationUnit& cu, bool push_result);
	virtual int compile_store(CompilationUnit& cu);
	virtual int compile_load_word(CompilationUnit& cu, size_t off, bool push_result);
	virtual int compile_store_word(CompilationUnit& cu, size_t off);
	virtual int compile_adr(CompilationUnit& cu, size_t off=0);
	virtual void print(std::ostream&);
};

class Static : public Variable {
protected:
	Context*			context_;
	ObjPtr				manager_;
	ObjPtr				init_statement_;
	ObjPtr				init_cu_;

	void add_code_fixup(Code&, size_t offset);

public:
	Static(Context*);
	Static(Type* type, Context*, const std::string& name);
	Static(const context_type::position_type& w, Type* type, Context*, const std::string& name);
	
	virtual XRefManager* manager();

	Context* context() {return context_;}
	bool is_external() const {return !name_.empty() && (name_[0] == '_');}

	void init_statement(Statement* val);
	Statement* init_statement() {return static_cast<Statement*>(init_statement_.get());}

	CompilationUnit* init() {return static_cast<CompilationUnit*>(init_cu_.get());}

	virtual void link1(Linker&);	// linking pass 1
	virtual void link2(Linker&);	// linking pass 2
	
	virtual int compile_push(CompilationUnit& cu);
	virtual int compile_pop(CompilationUnit& cu);
	virtual int compile_push(CompilationUnit& cu, size_t offset);
	virtual int compile_pop(CompilationUnit& cu, size_t offset);

	virtual int compile_load(CompilationUnit& cu, bool push_result);
	virtual int compile_store(CompilationUnit& cu);
	virtual int compile_load_word(CompilationUnit& cu, size_t off, bool push_result);
	virtual int compile_store_word(CompilationUnit& cu, size_t off);
	virtual int compile_adr(CompilationUnit& cu, size_t off=0);
	virtual void print(std::ostream&);

	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

};

class Function : public Type {
	Context*			    context_;
	std::string			  name_;
	ObjPtr				    manager_;
	ObjPtr				    return_type_;
	ObjPtr				    content_;
	ObjPtr_list_type	parameter_list_;
	ObjPtr_list_type	local_list_;
	ObjPtr_list_type	label_list_;
	size_t				    local_size_;
	size_t				    max_local_size_;
	size_t				    parameter_size_;
	int					      specifier_mask_;
	// code
	ObjPtr_map_type		labels_;
	ObjPtr_map_type		parameter_;
	ObjPtr_map_type		locals_;
	ObjPtr				    compilation_unit_;
	// fixup
	offset_list_type	data_fixup_list_;
	//ObjPtr_list_type	fixup_list_;
	// literal
	//ObjPtr_list_type	literal_list_;

	bool				      is_defined_;

  //static UnionStructType* get_umul_t();
  static UnionStructType* get_ulmul_t();
  static UnionStructType* get_ullmul_t();
  static UnionStructType* get_div_t();
  static UnionStructType* get_udiv_t();
  static UnionStructType* get_ldiv_t();
  static UnionStructType* get_uldiv_t();
  static UnionStructType* get_lldiv_t();
  static UnionStructType* get_ulldiv_t();
  static UnionStructType* get_float_t();
  static UnionStructType* get_double_t();
  static UnionStructType* get_longdouble_t();

  static Function* system_bool(const std::string&);
  static Function* system_int(const std::string&);
  static Function* system_uint(const std::string&);
  static Function* system_long(const std::string&);
  static Function* system_ulong(const std::string&);
  static Function* system_longlong(const std::string&);
  static Function* system_ulonglong(const std::string&);
  static Function* system_float(const std::string&);
  static Function* system_double(const std::string&);
  static Function* system_longdouble(const std::string&);

public:
	Function(Context*);
	Function(Context*, const std::string&);

	Context* context() {return context_;}
	virtual XRefManager* manager();

	void name(const std::string& t);
	const std::string& name() const {return name_;}

	virtual void link1(Linker&);	// linking pass 1
	virtual void link2(Linker&);	// linking pass 2

	void add_data_fixup(size_t pos) {data_fixup_list_.push_back(pos);}

	void add_statement(Statement* value);
	//void add_literal(Constant*);

	void define() {is_defined_ = true;}
	void defined(bool val=true) {is_defined_ = val;}
	bool is_defined() const {return is_defined_;}

	bool is_external() const;
	bool is_system() const;
	
	int specifier_mask() const {return specifier_mask_;}
	void specifier_mask(int val) {specifier_mask_ = val;}
	bool has_specifier_mask(FunctionSpecifier v) {return (MASK(v) & specifier_mask_) != 0;}

	void return_type(Type* val) {return_type_ = val;}
	Type* return_type() const {return const_cast<Type*>(static_cast<const Type*>(return_type_.get()));}

	void compilation_unit(CompilationUnit* val) {compilation_unit_ = val;}
	CompilationUnit* compilation_unit() const {return const_cast<CompilationUnit*>(static_cast<const CompilationUnit*>(compilation_unit_.get()));}

	virtual void print(std::ostream&);

	//const ObjPtr_list_type& parameter_list() const {return parameter_list_;}
	//void parameter_list(const ObjPtr_list_type& val) {parameter_list_ = val;}

	size_t parameter() const {return parameter_list_.size();}
	Parameter* parameter(size_t pos) {return static_cast<Parameter*>(parameter_list_.at(pos).get());}
	const Parameter* parameter(size_t pos) const {return static_cast<const Parameter*>(parameter_list_.at(pos).get());}
	
	size_t parameter_size() const {return parameter_size_;}

	void content(Statement* t) {content_ = t;}
	Statement* content() {return static_cast<Statement*>(content_.get());}

	size_t max_local_size() const {return max_local_size_;}

	size_t local_size() const {return local_size_;}
	void local_size(size_t val) {local_size_ = val;}

	void add_local(const std::string& name, Local* loc);
	Local* lookup_local(const std::string& name) const;
	void add_parameter(const std::string& name, Parameter* value);
	void set_parameter(const std::string& name, Parameter* value);
	void clear_parameter_names();
	Parameter* lookup_parameter(const std::string& name) const;
	void add_label(const std::string& name, Label* lbl);
	Label* lookup_label(const std::string& name) const;

	virtual int compile();
	virtual void optimize(ObjPtr&, int mask);
	virtual void optimize(int mask);
	virtual int compile(CompilationUnit&, bool push_result);

	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

	static Function* system(const std::string&);
  Call* to_call() const;
};

class Call : public Expression {
	std::string			  name_;
	ObjPtr_list_type	argument_list_;
	size_t				    argument_size_;
	ObjPtr				    func_;
	bool				      is_system_;
	bool				      is_external_;
	
public:
	Call(const std::string& name);
	Call(Function* type, const std::string& name);
	Call(const context_type::position_type& w, Function* type, const std::string& name);
	//Call(Function* type);
	//Call(const context_type::position_type& w, Function* type);

	bool is_system() const {return is_system_;}
	bool is_external() const {return is_external_;}

	void external() {is_external_ = true;}

	void func(Function* val) {func_ = val;}
	Function* func() {return static_cast<Function*>(func_.get());}

	const std::string& name() const {return name_;}

	void add(Expression*);
	size_t size() const {return argument_list_.size();}
	Expression* argument(size_t pos) {return static_cast<Expression*>(argument_list_.at(pos).get());}

	virtual void build_type();
	virtual void optimize(ObjPtr&, int mask);
	virtual int compile_argument(CompilationUnit& cu, Expression*);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual bool has_side_effect() const;

};


class Return : public Statement {
	ObjPtr		return_type_;
	ObjPtr		expr_;
	Function*	func_;

public:
	Return(const context_type::position_type& w, Function*);
	Return(Expression*, Function*);

	void return_type(Type* val) {return_type_ = val;}
	Type* return_type() const {return const_cast<Type*>(static_cast<const Type*>(return_type_.get()));}

	void expr(Expression* val) {expr_ = val;}
	Expression* expr() const {return const_cast<Expression*>(static_cast<const Expression*>(expr_.get()));}

	virtual bool last_is_return() const;
	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
};

class IfStmt : public Statement {
	ObjPtr		cond_;
	ObjPtr		true_;
	ObjPtr		false_;

public:
	IfStmt(const context_type::position_type& w) : Statement(w) {}
	IfStmt(Expression* cond, Statement* true_part, Statement* false_part=0);

	void cond(Expression* val) {cond_ = val;}
	Expression* cond() const {return const_cast<Expression*>(static_cast<const Expression*>(cond_.get()));}

	void true_part(Statement* val) {true_ = val;}
	Statement* true_part() const {return const_cast<Statement*>(static_cast<const Statement*>(true_.get()));}

	void false_part(Statement* val) {false_ = val;}
	Statement* false_part() const {return const_cast<Statement*>(static_cast<const Statement*>(false_.get()));}

	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
};

class Jump : public Statement {
	ObjPtr				destination_;
	size_t				value_;

public:
	Jump(const context_type::position_type& w) : Statement(w), value_(0) {}

	void value(size_t val) {value_ = val;}
	size_t value() const {return value_;}

	void destination(Label* val) {destination_ = val;}
	Label* destination() const {return const_cast<Label*>(static_cast<const Label*>(destination_.get()));}

	virtual void fixup(Code& code, size_t destination);
	virtual int compile(CompilationUnit&, bool push_result);
};


class CaseJump : public Jump {
	std::vector<size_t>	locations_;
	bool				is_added_;

public:
	CaseJump(const context_type::position_type& w);

	virtual void fixup(Code& code, size_t destination);
	virtual int compile(CompilationUnit&, bool push_result);
};

class Iteration : public Statement {
protected:
	ObjPtr		continue_;
	ObjPtr		break_;

	void init();

public:
	Iteration(const context_type::position_type& w);

	bool has_continue() const {return continue_.is_not_null();}
	bool has_break() const {return break_.is_not_null();}

	virtual Label* cont() {return const_cast<Label*>(static_cast<const Label*>(continue_.get()));}
	virtual Label* brk() {return const_cast<Label*>(static_cast<const Label*>(break_.get()));}

	virtual bool is_continuable() const;
	virtual bool is_breakable() const;
};

class LoopStmt : public Iteration {
	ObjPtr		block_;

public:
	LoopStmt(const context_type::position_type& w);

	void block(Statement* val) {block_ = val;}
	Statement* block() const {return const_cast<Statement*>(static_cast<const Statement*>(block_.get()));}

	virtual bool is_continuable() const;
	virtual bool is_breakable() const;

	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
};


class WhileStmt : public Iteration {
	ObjPtr		cond_;
	ObjPtr		block_;

public:
	WhileStmt(const context_type::position_type& w);

	void cond(Expression* val) {cond_ = val;}
	Expression* cond() const {return const_cast<Expression*>(static_cast<const Expression*>(cond_.get()));}

	void block(Statement* val) {block_ = val;}
	Statement* block() const {return const_cast<Statement*>(static_cast<const Statement*>(block_.get()));}

	virtual bool is_continuable() const;
	virtual bool is_breakable() const;

	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
};

class DoWhileStmt : public Iteration {
	ObjPtr		cond_;
	ObjPtr		block_;
	bool		  inverted_;

public:
	DoWhileStmt(const context_type::position_type& w);

	void cond(Expression* val) {cond_ = val; inverted_ = false;}
	Expression* cond() const {return const_cast<Expression*>(static_cast<const Expression*>(cond_.get()));}

	void block(Statement* val) {block_ = val;}
	Statement* block() const {return const_cast<Statement*>(static_cast<const Statement*>(block_.get()));}

	virtual bool is_continuable() const;
	virtual bool is_breakable() const;

	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
};

class ForStmt : public Iteration {
	ObjPtr		init_;
	ObjPtr		cond_;
	ObjPtr		post_;
	ObjPtr		block_;
	bool		  is_endless_;

public:
	ForStmt(const context_type::position_type& w);

	void init(Statement* val) {init_ = val;}
	Statement* init() const {return const_cast<Statement*>(static_cast<const Statement*>(init_.get()));}

	void cond(Expression* val) {cond_ = val;}
	Expression* cond() const {return const_cast<Expression*>(static_cast<const Expression*>(cond_.get()));}

	void post(Expression* val) {post_ = val;}
	Expression* post() const {return const_cast<Expression*>(static_cast<const Expression*>(post_.get()));}

	void block(Statement* val) {block_ = val;}
	Statement* block() const {return const_cast<Statement*>(static_cast<const Statement*>(block_.get()));}

	virtual bool is_continuable() const;
	virtual bool is_breakable() const;

	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
};

class SwitchStmt;

class CaseStmt : public Statement {
  HashFinder  finder_;
	ObjPtr		  value_;
	ObjPtr		  label_;
	ObjPtr		  jump_;
	SwitchStmt*	switch_;
  bool        use_finder_;

public:
	CaseStmt(const context_type::position_type& w, SwitchStmt*);
	
	void value(Expression* val) {value_ = val;}
	Expression* value() const {return const_cast<Expression*>(static_cast<const Expression*>(value_.get()));}

	void label(Label* val) {label_ = val;}
	Label* label() const {return const_cast<Label*>(static_cast<const Label*>(label_.get()));}

	void jump(CaseJump* val) {jump_ = val;}
	CaseJump* jump() const {return const_cast<CaseJump*>(static_cast<const CaseJump*>(jump_.get()));}

	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual int compile_table_entry(CompilationUnit& cu);
};



class SwitchStmt : public Iteration {
	ObjPtr				    value_;
	ObjPtr				    call_;
	ObjPtr_list_type	case_list_;
	std::map<unsigned long long, ObjPtr> umap_;
	std::map<long long, ObjPtr> map_;
	int					      default_;
	int					      min_;
	int					      limit_;
	bool				      is_compressed_;

public:
	SwitchStmt(const context_type::position_type& w);
		
	void finish(Function* owner);


	void value(Expression* val) {value_ = val;}
	Expression* value() const {return const_cast<Expression*>(static_cast<const Expression*>(value_.get()));}

	virtual void add(CaseStmt*);
	virtual void deflt(CaseStmt*);

	virtual bool is_breakable() const;

	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
};



class UnaryExpr : public Expression {
	UnOp	  op_;
	ObjPtr	arg1_;
	ObjPtr	call_;
	int		  value_;

	void assert_lvalue();

public:
	UnaryExpr(const context_type::position_type& w, UnOp op);
	UnaryExpr(const context_type::position_type& w, UnOp op, Expression* arg1);
	UnaryExpr(const context_type::position_type& w, UnOp op, Expression* arg1, int value);
	UnaryExpr(UnOp op, Expression* arg1);

	virtual ExprType expr_type() const;
	void call(Call* val) {call_ = val;}
	Call* call() {return static_cast<Call*>(call_.get());}
	void arg1(Expression* val) {arg1_ = val;}
	Expression* arg1() const {return const_cast<Expression*>(static_cast<const Expression*>(arg1_.get()));}

	//virtual void initialize(); // mark as initialized

	virtual size_t type_size();

	virtual bool is_unary(UnOp) const;

	virtual bool has_side_effect() const;
	virtual bool is_const() const;
	virtual bool is_lvalue() const;
	virtual void optimize(ObjPtr&, int mask);
	virtual int compile_load(CompilationUnit& cu, bool push_result);
	virtual int compile_store(CompilationUnit& cu);
	virtual int compile_adr(CompilationUnit& cu, size_t offset=0);
	virtual Type* lvalue_type();

	virtual int compile(CompilationUnit&, bool push_result);
	virtual void build_type();
};

class BinaryExpr : public Expression {
	BinOp	  op_;
	ObjPtr	arg1_;
	ObjPtr	arg2_;
	ObjPtr	call_;


public:
	BinaryExpr(const context_type::position_type& w, BinOp op);
	BinaryExpr(const context_type::position_type& w, BinOp op, Expression* arg1, Expression* arg2);
	BinaryExpr(BinOp op, Expression* arg1, Expression* arg2);

	virtual ExprType expr_type() const;
  virtual Call* assert_call(const char*);
	
	void call(Call* val) {call_ = val;}
	Call* call() {return static_cast<Call*>(call_.get());}

	void arg1(Expression* val) {arg1_ = val;}
	Expression* arg1() const {return const_cast<Expression*>(static_cast<const Expression*>(arg1_.get()));}

	void arg2(Expression* val) {arg2_ = val;}
	Expression* arg2() const {return const_cast<Expression*>(static_cast<const Expression*>(arg2_.get()));}

  bool is_shift() const;

	virtual bool is_binary(BinOp) const;

	virtual bool has_side_effect() const;
	virtual bool is_const() const;
	virtual void optimize(ObjPtr&, int mask);
	virtual int compile(CompilationUnit&, bool push_result);
	virtual int compile_condition(CompilationUnit& cu, bool push_result);
	virtual void build_type();
};

class TernaryExpr : public Expression {
	TerOp	  op_;
	ObjPtr	arg1_;
	ObjPtr	arg2_;
	ObjPtr	arg3_;
	ObjPtr	call_;

public:
	TernaryExpr(const context_type::position_type& w, TerOp op);

	virtual ExprType expr_type() const;
	void call(Call* val) {call_ = val;}
	Call* call() {return static_cast<Call*>(call_.get());}

	void arg1(Expression* val) {arg1_ = val;}
	Expression* arg1() const {return const_cast<Expression*>(static_cast<const Expression*>(arg1_.get()));}

	void arg2(Expression* val) {arg2_ = val;}
	Expression* arg2() const {return const_cast<Expression*>(static_cast<const Expression*>(arg2_.get()));}

	void arg3(Expression* val) {arg3_ = val;}
	Expression* arg3() const {return const_cast<Expression*>(static_cast<const Expression*>(arg3_.get()));}

	virtual bool has_side_effect() const;
	virtual bool is_const() const;
	virtual void optimize(ObjPtr&, int mask);
	virtual void build_type();
	virtual int compile(CompilationUnit&, bool push_result);
	virtual int compile_condition(CompilationUnit& cu, bool push_result);
};

class Assignment : public Expression {
	AssOp	  op_;
	ObjPtr	lhs_;
	ObjPtr	rhs_;

public:
	Assignment(const context_type::position_type& w, AssOp op);
	Assignment(const context_type::position_type& w, AssOp op, Expression* lhs, Expression* rhs);

	virtual ExprType expr_type() const;

	void lhs(Expression* val) {lhs_ = val;}
	Expression* lhs() const {return const_cast<Expression*>(static_cast<const Expression*>(lhs_.get()));}

	void rhs(Expression* val) {rhs_ = val;}
	Expression* rhs() const {return const_cast<Expression*>(static_cast<const Expression*>(rhs_.get()));}

	virtual bool has_side_effect() const;
	virtual bool is_const() const;
	virtual int compile(CompilationUnit&, bool push_result);
	virtual void optimize(ObjPtr&, int mask);
	virtual void build_type();
};

class InitAssign : public Assignment {
	ObjPtr				call_;

	Call* call() {return static_cast<Call*>(call_.get());}

public:
	InitAssign(const context_type::position_type& w);

	virtual int compile(CompilationUnit&, bool push_result);

};


class Context : public RefObj {
	// global scope
	ObjPtr_map_type	tags_;
	ObjPtr_map_type	typedefs_;
//	ObjPtr_map_type	enumerators_;
	// local & global scope
	ObjPtr_map_type	objects_; // contains functions, static variables and enumerators

	ObjPtr_map_type	temp_tags_;
	ObjPtr_map_type	temp_typedefs_;
	ObjPtr_map_type	temp_objects_; // contains functions, static variables and enumerators


public:
	Context();

	void write(xml::Writer&);
	void read(xml::Iterator&);

	void add_enumerator(const std::string& name, Constant* value);
	Constant* lookup_enumerator(const std::string& name) const;

	void add_typedef(const std::string& name, Type* value);
	Type* lookup_typedef(const std::string& name) const;

	void add_tag(const std::string& name, Type* value);
	Type* lookup_tag(const std::string& name) const;

	void add(Function* value);
	void add(Static* value);
	ObjPtr lookup(const std::string& name, bool check_external) const;

  void forget_temp();
  void save_temp();

};

class Library : public RefObj {
	cvmc::ObjPtr_list_type context_list_;

public:
	Library();

	void add(Context*);
	size_t context() const {return context_list_.size();}
	Context* context(size_t pos) const {return const_cast<Context*>(static_cast<const Context*>(context_list_.at(pos).get()));}

	virtual ObjPtr lookup(const std::string& name) const;

	virtual void write(xml::Writer&);
	virtual void read(xml::Iterator&);

};

class Linker : public Library {
	ObjPtr_list_type	library_list_;
	std::string				entry_point_;
	std::string				exit_point_;
	// linking
	ObjPtr_map_type		object_map_;
	ObjPtr_list_type	pending_list_;
	Code						  code_;	// compiled code
	resolve_map_type	pending_;
	resolve_map_type	static_map_;
	resolve_map_type	function_map_;
	resolve_map_type	function_data_map_;
	string_map_type		string_map_;
	xref_map_type			string_xref_map_;
	xref_map_type			static_xref_map_;
	xref_map_type			function_xref_map_;
	size_t						resolved_pos_;
	size_t						code_size_;
	size_t						data_size_;
	size_t						literal_size_;
	size_t						setup_size_;
	size_t						file_size_;
	size_t						literal_base_;
	size_t						setup_base_;
	size_t						code_base_;
	size_t						data_base_;
	bool						big_endian_;

public:
	Linker();

	void entry_point(const std::string& v) {entry_point_ = v;}
	const std::string& entry_point() const {return entry_point_;}

	void big_endian(bool val) {big_endian_ = val;}
	bool big_endian() const {return big_endian_;}

	Code& code() {return code_;}

	size_t code_size() const {return code_size_;}
	size_t data_size() const {return data_size_;}
	size_t literal_size() const {return literal_size_;}
	size_t setup_size() const {return setup_size_;}

	size_t code_base() const {return code_base_;}
	size_t data_base() const {return data_base_;}

	size_t add_string_literal(const std::string&);	// return the index of the linked string

	xref_map_type& string_xref_map() {return string_xref_map_;}
	xref_map_type& static_xref_map() {return static_xref_map_;}
	xref_map_type& function_xref_map() {return function_xref_map_;}

	void add_static(Static*);
	void add_function(Function*);

	ExtRefType get_adr_info(size_t adr, std::string& name, size_t& offset);

	void add_pending(RefObj* obj);

	void add_library(Library*);

	void import_library(const std::string& filename);
	size_t library() const {return library_list_.size();}
	Library* library(size_t pos) const {return const_cast<Library*>(static_cast<const Library*>(library_list_.at(pos).get()));}

  virtual ObjPtr resolve(ObjPtr ptr);

	virtual RefObj* find(const std::string& name);
	virtual ObjPtr lookup(const std::string& name) const;
	virtual size_t resolve(Static*);
	virtual size_t resolve(Function*);
	virtual size_t resolve_data(Function*);

	virtual void link();
	virtual void write(std::ostream&);
	virtual void disassembly(std::ostream&);
	virtual void read(xml::Iterator&);

};

class Declarator : public RefObj {
private:
	std::string		name_;
	ObjPtr				context_;
	ObjPtr				base_type_;
	ObjPtr				type_;

public:
	Declarator();

	void context(Context& cntxt) {context_ = &cntxt;}
	Context* context() {return static_cast<Context*>(context_.get());}

	void name(const std::string& t) {name_ = t;}
	const std::string& name() const {return name_;}

	void type(Type* val) {type_ = val;}
	Type* type() {return static_cast<Type*>(type_.get());}

	void base_type(Type* val) {base_type_ = val;}
	Type* base_type() {return static_cast<Type*>(base_type_.get());}

	void declare();
};

//class Do {
//public:
//	enum Operation {
//		undefined,
//		op_add_tag,
//		op_add_enumerator
//	};
//
//private:
//	Operation	op_;
//	ObjPtr		context_;
//	ObjPtr		type_;
//	ObjPtr		value_;
//	std::string name_;
//
//public:
//	Do();
//
//	void add_tag(Context*, const std::string&, Type*);
//	void add_enumerator(Context*, const std::string&, Constant*);
//
//	void perform();
//};
//
//class Updater {
//	std::vector<Do>		list_;
//
//public:
//	Updater();
//	
//	void append(const Do& d);
//
//	void perform();
//	void clear();
//};

class Parser : public RefObj {
	ContextHelper&	helper_;
	Context*			  context_;
	//Updater				updater_;
	bool				    optimize_;
	bool				    const_folding_;
	bool				    is_typedef_;
	int					    block_level_;	// indicating how many blocks have been nested
	int					    storage_class_mask_;
	int					    type_qualifier_mask_;
	int					    function_specifier_mask_;
	int					    alignment_specifier_mask_;

	ObjPtr				  manager_;	// current xref manager
	ObjPtr				  swtch_;	// current switch
	ObjPtr				  cont_;	// current continuable iteration
	ObjPtr				  brk_;	// current breakable iteration
	ObjPtr				  block_;	// current type declaration
	std::string			name_;	// current identifier
	std::string			func_name_;	// current identifier

	void next();
	void error(const std::string& txt);
	void warning(const std::string& txt);

	void end_manager();
	void end_name();

	void name(const std::string& t) {name_ = t;}
	const std::string& name() const {return name_;}

	void func_name(const std::string& t) {func_name_ = t;}
	const std::string& func_name() const {return func_name_;}

	void manager(XRefManager* t) {manager_ = t;}

	void swtch(SwitchStmt* t) {swtch_ = t;}
	SwitchStmt* swtch() {return static_cast<SwitchStmt*>(swtch_.get());}

	void cont(Iteration* t) {cont_ = t;}
	Iteration* cont() {return static_cast<Iteration*>(cont_.get());}

	void brk(Iteration* t) {brk_ = t;}
	Iteration* brk() {return static_cast<Iteration*>(brk_.get());}

	void block(Block* t) {block_ = t;}
	Block* block() {return static_cast<Block*>(block_.get());}

public:
	Parser(ContextHelper& hlp);

 	static bool parse_number_constant(const char*& warning, const std::string&, ObjPtr& e, bool with_sign=false);

	virtual XRefManager* manager();

	void block_level(int t) {block_level_ = t;}
	int block_level() const {return block_level_;}
	void enter_block() {++block_level_;}
	void exit_block() {--block_level_;}

	void const_folding(bool t) {const_folding_ = t;}
	bool const_folding() const {return const_folding_;}

	void optimize(bool t) {optimize_ = t;}
	bool optimize() const {return optimize_;}

	//void print_errors(std::ostream& out);

	bool parse_primary_expression(Function* owner, ObjPtr& e, ObjPtr& funcall_type);
	bool parse_string(ObjPtr& e);
	bool parse_generic_selection(Function* owner, ObjPtr& e);
	bool parse_generic_assoc_list(Function* owner, ObjPtr& e);
	bool parse_generic_association(Function* owner, ObjPtr& e);
	bool parse_postfix_expression(Function* owner, ObjPtr& e);
	bool parse_postfix_expression_body(Function* owner, ObjPtr& e, ObjPtr& funcall_type);
	bool parse_argument_expression_list(Function* owner, Call&);
	bool parse_unary_expression(Function* owner, ObjPtr& e);
	UnOp parse_unary_operator();
	bool parse_cast_expression(Function* owner, ObjPtr& e);
	bool parse_multiplicative_expression(Function* owner, ObjPtr& e);
	bool parse_additive_expression(Function* owner, ObjPtr& e);
	bool parse_shift_expression(Function* owner, ObjPtr& e);
	bool parse_relational_expression(Function* owner, ObjPtr& e);
	bool parse_equality_expression(Function* owner, ObjPtr& e);
	bool parse_and_expression(Function* owner, ObjPtr& e);
	bool parse_exclusive_or_expression(Function* owner, ObjPtr& e);
	bool parse_inclusive_or_expression(Function* owner, ObjPtr& e);
	bool parse_logical_and_expression(Function* owner, ObjPtr& e);
	bool parse_logical_or_expression(Function* owner, ObjPtr& e);
	bool parse_conditional_expression(Function* owner, ObjPtr lhs, ObjPtr& e);
	bool parse_assignment_expression(Function* owner, ObjPtr& e);
	AssOp parse_assignment_operator();
	bool parse_expression(Function* owner, ObjPtr& e);
	bool parse_constant_expression(Function* owner, ObjPtr& e);
	bool parse_statement(Function* owner, ObjPtr& s);
	bool parse_labeled_statement(Function* owner, ObjPtr& s);
	bool parse_selection_statement(Function* owner, ObjPtr& s);
	bool parse_iteration_statement(Function* owner, ObjPtr& s);
	bool parse_jump_statement(Function* owner, ObjPtr& s);
	bool parse_block_item_list(Function* owner, Block&);
	bool parse_block_item(Function* owner, Block&);
	//bool parse_parameter_type_list(bool is_new);
	bool parse_parameter_list(Function* owner, Function* f, bool is_new);
	bool parse_parameter_declaration(Function* owner, ObjPtr& t);
	bool parse_identifier_list();
	bool parse_direct_declarator(Function* owner, ObjPtr& t);
	bool parse_direct_declarator_body(Function* owner, ObjPtr& t);
	bool parse_alignment_specifier();
	bool parse_function_specifier();
	bool parse_storage_class_specifier();
	bool parse_initializer(Function* owner, Type*, ObjPtr& e, Initializer* init);
	bool parse_initializer_list(Function* owner, Type*, ObjPtr& e, Initializer* init);
	bool parse_direct_abstract_declarator(ObjPtr& t);
	bool parse_type_name(Function* owner, ObjPtr& t);
	bool parse_abstract_declarator(ObjPtr& t);
	bool parse_type_qualifier();
	bool parse_specifier_qualifier_list(Function* owner, ObjPtr& t);
	bool parse_simple(ObjPtr& t, TypeType tt);
	bool parse_type_specifier(Function* owner, ObjPtr& t, Declarator& name);
	bool parse_type_qualifier_list();
	bool parse_static_assert_declaration();
	bool parse_pointer(ObjPtr& t);
	bool parse_init_declarator_list(Function* owner, ObjPtr t);
	bool parse_expression_statement(Function* owner, ObjPtr& s, bool eat_semicolon = true);
	bool parse_compound_statement(Function* owner, ObjPtr& s);
	bool parse_declaration_list(Function* owner);
	bool parse_declarator(Function* owner, ObjPtr& t);
	bool parse_declaration_specifiers(Function* owner, ObjPtr& t, Declarator& name);
	bool parse_declaration(Function* owner, bool eat_semicolon = true);
	bool parse_init_declarator(Function* owner, ObjPtr& t);
	bool parse_function_definition(Function* owner);
	bool parse_external_declaration();
	void parse(Context& cntxt);
	bool parse_if(Function* owner, ObjPtr& s);
	bool parse_while(Function* owner, ObjPtr& s);
	bool parse_do_while(Function* owner, ObjPtr& s);
	bool parse_for(Function* owner, ObjPtr& s);
	bool parse_return(Function* owner, ObjPtr& s);
	bool parse_goto(Function* owner, ObjPtr& s);
	bool parse_continue(Function* owner, ObjPtr& s);
	bool parse_break(Function* owner, ObjPtr& s);
	bool parse_switch(Function* owner, ObjPtr& s);
	bool parse_case(Function* owner, ObjPtr& s);
	bool parse_default(Function* owner, ObjPtr& s);
	bool parse_label(Function* owner, ObjPtr& s);
	bool parse_struct_or_union_specifier(Function* owner, ObjPtr& t, Declarator& name);
	bool parse_struct_or_union();
	bool parse_struct_declaration_list(Function* owner, UnionStructType&);
	bool parse_struct_declaration(Function* owner, UnionStructType&);
	bool parse_struct_declarator_list(Function* owner, UnionStructType&, ObjPtr& t);
	bool parse_struct_declarator(Function* owner, UnionStructType&, ObjPtr& t);
	bool parse_enum_specifier(Function* owner, ObjPtr& t);
	bool parse_enumerator_list(Function* owner, EnumType&);
	bool parse_enumerator(Function* owner, EnumType&);
	bool parse_atomic_type_specifier();
	bool parse_designation(Function* owner, ObjPtr& e);
	bool parse_designator_list(Function* owner, ObjPtr& e);
	bool parse_designator(Function* owner, ObjPtr& e);
	bool parse_();

};

class IssueList {
	ObjPtr_list_type		data_;

	IssueList();

public:
	static IssueList& error();
	static IssueList& warning();

	void add(Issue*);

	ObjPtr_list_type& data() {return data_;}
	size_t size() const {return data_.size();}

	Issue* issue(size_t pos) {return static_cast<Issue*>(data_.at(pos).get());}

	void print(std::ostream& out);

	static void error(const context_type::position_type&, const std::string&);
	static void warning(const context_type::position_type&, const std::string&);
	static void error(const std::string&);
	static void warning(const std::string&);
};



}

#endif // !CVMC_HPP
