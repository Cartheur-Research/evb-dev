#include "cvmc.hpp"

#include "math.h"
#include "float.h"

// debug: test1.cxx out.txt			in working directory ..\bin

// --lib test1.cxx lib.xml
// --lib kraken.c errno.c math.c system.c stdlib.c poly.c stdio.c setjmp.c system.xml
// -l system.xml --dis sysdiss.txt system.cxx system.bin
// test1.cxx --dis diss.txt a.out
// -l lib.xml --dis diss.txt a.out
// -l system.xml --dis test.txt test.cxx cvm.bin
// -l system.xml --little --dis regress_test.txt regress_test.cxx cvm.bin
// -l system.xml --little --dis test2.txt test2.cxx cvm.bin
// -l system.xml --little --dis test3.txt test3.cxx cvm.bin
// --lib poly.c poly.xml
// -i test --little -l system.xml --dis test/diss.txt test/int/test.c test/testenv.c test/main.c test/cvm.bin
// -f system_lib.gen

std::string token_id_to_string(const boost::wave::token_id& tid)
{
	switch(tid) {
	case boost::wave::T_FIRST_TOKEN: return "T_FIRST_TOKEN";
	case boost::wave::T_AND: return "T_AND";
	case boost::wave::T_AND_ALT: return "T_AND_ALT";
	case boost::wave::T_ANDAND: return "T_ANDAND";
	case boost::wave::T_ANDAND_ALT: return "T_ANDAND_ALT";
	case boost::wave::T_ASSIGN: return "T_ASSIGN";
	case boost::wave::T_ANDASSIGN: return "T_ANDASSIGN";
	case boost::wave::T_ANDASSIGN_ALT: return "T_ANDASSIGN_ALT";
	case boost::wave::T_OR: return "T_OR";
	case boost::wave::T_OR_ALT: return "T_OR_ALT";
	case boost::wave::T_OR_TRIGRAPH: return "T_OR_TRIGRAPH";
	case boost::wave::T_ORASSIGN: return "T_ORASSIGN";
	case boost::wave::T_ORASSIGN_ALT: return "T_ORASSIGN_ALT";
	case boost::wave::T_ORASSIGN_TRIGRAPH: return "T_ORASSIGN_TRIGRAPH";
	case boost::wave::T_XOR: return "T_XOR";
	case boost::wave::T_XOR_ALT: return "T_XOR_ALT";
	case boost::wave::T_XOR_TRIGRAPH: return "T_XOR_TRIGRAPH";
	case boost::wave::T_XORASSIGN: return "T_XORASSIGN";
	case boost::wave::T_XORASSIGN_ALT: return "T_XORASSIGN_ALT";
	case boost::wave::T_XORASSIGN_TRIGRAPH: return "T_XORASSIGN_TRIGRAPH";
	case boost::wave::T_COMMA: return "T_COMMA";
	case boost::wave::T_COLON: return "T_COLON";
	case boost::wave::T_DIVIDE: return "T_DIVIDE";
	case boost::wave::T_DIVIDEASSIGN: return "T_DIVIDEASSIGN";
	case boost::wave::T_DOT: return "T_DOT";
	case boost::wave::T_DOTSTAR: return "T_DOTSTAR";
	case boost::wave::T_ELLIPSIS: return "T_ELLIPSIS";
	case boost::wave::T_EQUAL: return "T_EQUAL";
	case boost::wave::T_GREATER: return "T_GREATER";
	case boost::wave::T_GREATEREQUAL: return "T_GREATEREQUAL";
	case boost::wave::T_LEFTBRACE: return "T_LEFTBRACE";
	case boost::wave::T_LEFTBRACE_ALT: return "T_LEFTBRACE_ALT";
	case boost::wave::T_LEFTBRACE_TRIGRAPH: return "T_LEFTBRACE_TRIGRAPH";
	case boost::wave::T_LESS: return "T_LESS";
	case boost::wave::T_LESSEQUAL: return "T_LESSEQUAL";
	case boost::wave::T_LEFTPAREN: return "T_LEFTPAREN";
	case boost::wave::T_LEFTBRACKET: return "T_LEFTBRACKET";
	case boost::wave::T_LEFTBRACKET_ALT: return "T_LEFTBRACKET_ALT";
	case boost::wave::T_LEFTBRACKET_TRIGRAPH: return "T_LEFTBRACKET_TRIGRAPH";
	case boost::wave::T_MINUS: return "T_MINUS";
	case boost::wave::T_MINUSASSIGN: return "T_MINUSASSIGN";
	case boost::wave::T_MINUSMINUS: return "T_MINUSMINUS";
	case boost::wave::T_PERCENT: return "T_PERCENT";
	case boost::wave::T_PERCENTASSIGN: return "T_PERCENTASSIGN";
	case boost::wave::T_NOT: return "T_NOT";
	case boost::wave::T_NOT_ALT: return "T_NOT_ALT";
	case boost::wave::T_NOTEQUAL: return "T_NOTEQUAL";
	case boost::wave::T_NOTEQUAL_ALT: return "T_NOTEQUAL_ALT";
	case boost::wave::T_OROR: return "T_OROR";
	case boost::wave::T_OROR_ALT: return "T_OROR_ALT";
	case boost::wave::T_OROR_TRIGRAPH: return "T_OROR_TRIGRAPH";
	case boost::wave::T_PLUS: return "T_PLUS";
	case boost::wave::T_PLUSASSIGN: return "T_PLUSASSIGN";
	case boost::wave::T_PLUSPLUS: return "T_PLUSPLUS";
	case boost::wave::T_ARROW: return "T_ARROW";
	case boost::wave::T_ARROWSTAR: return "T_ARROWSTAR";
	case boost::wave::T_QUESTION_MARK: return "T_QUESTION_MARK";
	case boost::wave::T_RIGHTBRACE: return "T_RIGHTBRACE";
	case boost::wave::T_RIGHTBRACE_ALT: return "T_RIGHTBRACE_ALT";
	case boost::wave::T_RIGHTBRACE_TRIGRAPH: return "T_RIGHTBRACE_TRIGRAPH";
	case boost::wave::T_RIGHTPAREN: return "T_RIGHTPAREN";
	case boost::wave::T_RIGHTBRACKET: return "T_RIGHTBRACKET";
	case boost::wave::T_RIGHTBRACKET_ALT: return "T_RIGHTBRACKET_ALT";
	case boost::wave::T_RIGHTBRACKET_TRIGRAPH: return "T_RIGHTBRACKET_TRIGRAPH";
	case boost::wave::T_COLON_COLON: return "T_COLON_COLON";
	case boost::wave::T_SEMICOLON: return "T_SEMICOLON";
	case boost::wave::T_SHIFTLEFT: return "T_SHIFTLEFT";
	case boost::wave::T_SHIFTLEFTASSIGN: return "T_SHIFTLEFTASSIGN";
	case boost::wave::T_SHIFTRIGHT: return "T_SHIFTRIGHT";
	case boost::wave::T_SHIFTRIGHTASSIGN: return "T_SHIFTRIGHTASSIGN";
	case boost::wave::T_STAR: return "T_STAR";
	case boost::wave::T_COMPL: return "T_COMPL";
	case boost::wave::T_COMPL_ALT: return "T_COMPL_ALT";
	case boost::wave::T_COMPL_TRIGRAPH: return "T_COMPL_TRIGRAPH";
	case boost::wave::T_STARASSIGN: return "T_STARASSIGN";
	case boost::wave::T_ASM: return "T_ASM";
	case boost::wave::T_AUTO: return "T_AUTO";
	case boost::wave::T_BOOL: return "T_BOOL";
	case boost::wave::T_FALSE: return "T_FALSE";
	case boost::wave::T_TRUE: return "T_TRUE";
	case boost::wave::T_BREAK: return "T_BREAK";
	case boost::wave::T_CASE: return "T_CASE";
	case boost::wave::T_CATCH: return "T_CATCH";
	case boost::wave::T_CHAR: return "T_CHAR";
	case boost::wave::T_CLASS: return "T_CLASS";
	case boost::wave::T_CONST: return "T_CONST";
	case boost::wave::T_CONSTCAST: return "T_CONSTCAST";
	case boost::wave::T_CONTINUE: return "T_CONTINUE";
	case boost::wave::T_DEFAULT: return "T_DEFAULT";
	case boost::wave::T_DELETE: return "T_DELETE";
	case boost::wave::T_DO: return "T_DO";
	case boost::wave::T_DOUBLE: return "T_DOUBLE";
	case boost::wave::T_DYNAMICCAST: return "T_DYNAMICCAST";
	case boost::wave::T_ELSE: return "T_ELSE";
	case boost::wave::T_ENUM: return "T_ENUM";
	case boost::wave::T_EXPLICIT: return "T_EXPLICIT";
	case boost::wave::T_EXPORT: return "T_EXPORT";
	case boost::wave::T_EXTERN: return "T_EXTERN";
	case boost::wave::T_FLOAT: return "T_FLOAT";
	case boost::wave::T_FOR: return "T_FOR";
	case boost::wave::T_FRIEND: return "T_FRIEND";
	case boost::wave::T_GOTO: return "T_GOTO";
	case boost::wave::T_IF: return "T_IF";
	case boost::wave::T_INLINE: return "T_INLINE";
	case boost::wave::T_INT: return "T_INT";
	case boost::wave::T_LONG: return "T_LONG";
	case boost::wave::T_MUTABLE: return "T_MUTABLE";
	case boost::wave::T_NAMESPACE: return "T_NAMESPACE";
	case boost::wave::T_NEW: return "T_NEW";
	case boost::wave::T_OPERATOR: return "T_OPERATOR";
	case boost::wave::T_PRIVATE: return "T_PRIVATE";
	case boost::wave::T_PROTECTED: return "T_PROTECTED";
	case boost::wave::T_PUBLIC: return "T_PUBLIC";
	case boost::wave::T_REGISTER: return "T_REGISTER";
	case boost::wave::T_REINTERPRETCAST: return "T_REINTERPRETCAST";
	case boost::wave::T_RETURN: return "T_RETURN";
	case boost::wave::T_SHORT: return "T_SHORT";
	case boost::wave::T_SIGNED: return "T_SIGNED";
	case boost::wave::T_SIZEOF: return "T_SIZEOF";
	case boost::wave::T_STATIC: return "T_STATIC";
	case boost::wave::T_STATICCAST: return "T_STATICCAST";
	case boost::wave::T_STRUCT: return "T_STRUCT";
	case boost::wave::T_SWITCH: return "T_SWITCH";
	case boost::wave::T_TEMPLATE: return "T_TEMPLATE";
	case boost::wave::T_THIS: return "T_THIS";
	case boost::wave::T_THROW: return "T_THROW";
	case boost::wave::T_TRY: return "T_TRY";
	case boost::wave::T_TYPEDEF: return "T_TYPEDEF";
	case boost::wave::T_TYPEID: return "T_TYPEID";
	case boost::wave::T_TYPENAME: return "T_TYPENAME";
	case boost::wave::T_UNION: return "T_UNION";
	case boost::wave::T_UNSIGNED: return "T_UNSIGNED";
	case boost::wave::T_USING: return "T_USING";
	case boost::wave::T_VIRTUAL: return "T_VIRTUAL";
	case boost::wave::T_VOID: return "T_VOID";
	case boost::wave::T_VOLATILE: return "T_VOLATILE";
	case boost::wave::T_WCHART: return "T_WCHART";
	case boost::wave::T_WHILE: return "T_WHILE";
	case boost::wave::T_PP_DEFINE: return "T_PP_DEFINE";
	case boost::wave::T_PP_IF: return "T_PP_IF";
	case boost::wave::T_PP_IFDEF: return "T_PP_IFDEF";
	case boost::wave::T_PP_IFNDEF: return "T_PP_IFNDEF";
	case boost::wave::T_PP_ELSE: return "T_PP_ELSE";
	case boost::wave::T_PP_ELIF: return "T_PP_ELIF";
	case boost::wave::T_PP_ENDIF: return "T_PP_ENDIF";
	case boost::wave::T_PP_ERROR: return "T_PP_ERROR";
	case boost::wave::T_PP_LINE: return "T_PP_LINE";
	case boost::wave::T_PP_PRAGMA: return "T_PP_PRAGMA";
	case boost::wave::T_PP_UNDEF: return "T_PP_UNDEF";
	case boost::wave::T_PP_WARNING: return "T_PP_WARNING";
	case boost::wave::T_IDENTIFIER: return "T_IDENTIFIER";
	case boost::wave::T_OCTALINT: return "T_OCTALINT";
	case boost::wave::T_DECIMALINT: return "T_DECIMALINT";
	case boost::wave::T_HEXAINT: return "T_HEXAINT";
	case boost::wave::T_INTLIT: return "T_INTLIT";
	case boost::wave::T_LONGINTLIT: return "T_LONGINTLIT";
	case boost::wave::T_FLOATLIT: return "T_FLOATLIT";
	case boost::wave::T_FIXEDPOINTLIT: return "T_FIXEDPOINTLIT";
	case boost::wave::T_CCOMMENT: return "T_CCOMMENT";
	case boost::wave::T_CPPCOMMENT: return "T_CPPCOMMENT";
	case boost::wave::T_CHARLIT: return "T_CHARLIT";
	case boost::wave::T_STRINGLIT: return "T_STRINGLIT";
	case boost::wave::T_CONTLINE: return "T_CONTLINE";
	case boost::wave::T_SPACE: return "T_SPACE";
	case boost::wave::T_SPACE2: return "T_SPACE2";
	case boost::wave::T_NEWLINE: return "T_NEWLINE";
	case boost::wave::T_GENERATEDNEWLINE: return "T_GENERATEDNEWLINE";
	case boost::wave::T_POUND_POUND: return "T_POUND_POUND";
	case boost::wave::T_POUND_POUND_ALT: return "T_POUND_POUND_ALT";
	case boost::wave::T_POUND_POUND_TRIGRAPH: return "T_POUND_POUND_TRIGRAPH";
	case boost::wave::T_POUND: return "T_POUND";
	case boost::wave::T_POUND_ALT: return "T_POUND_ALT";
	case boost::wave::T_POUND_TRIGRAPH: return "T_POUND_TRIGRAPH";
	case boost::wave::T_ANY: return "T_ANY";
	case boost::wave::T_ANY_TRIGRAPH: return "T_ANY_TRIGRAPH";
	case boost::wave::T_PP_INCLUDE: return "T_PP_INCLUDE";
	case boost::wave::T_PP_QHEADER: return "T_PP_QHEADER";
	case boost::wave::T_PP_HHEADER: return "T_PP_HHEADER";
	case boost::wave::T_PP_INCLUDE_NEXT: return "T_PP_INCLUDE_NEXT";
	case boost::wave::T_PP_QHEADER_NEXT: return "T_PP_QHEADER_NEXT";
	case boost::wave::T_PP_HHEADER_NEXT: return "T_PP_HHEADER_NEXT";
	case boost::wave::T_EOF: return "T_EOF";
	case boost::wave::T_EOI: return "T_EOI";
	case boost::wave::T_PP_NUMBER: return "T_PP_NUMBER";

// MS extensions
	case boost::wave::T_MSEXT_INT8: return "T_MSEXT_INT8";
	case boost::wave::T_MSEXT_INT16: return "T_MSEXT_INT16";
	case boost::wave::T_MSEXT_INT32: return "T_MSEXT_INT32";
	case boost::wave::T_MSEXT_INT64: return "T_MSEXT_INT64";
	case boost::wave::T_MSEXT_BASED: return "T_MSEXT_BASED";
	case boost::wave::T_MSEXT_DECLSPEC: return "T_MSEXT_DECLSPEC";
	case boost::wave::T_MSEXT_CDECL: return "T_MSEXT_CDECL";
	case boost::wave::T_MSEXT_FASTCALL: return "T_MSEXT_FASTCALL";
	case boost::wave::T_MSEXT_STDCALL: return "T_MSEXT_STDCALL";
	case boost::wave::T_MSEXT_TRY: return "T_MSEXT_TRY";
	case boost::wave::T_MSEXT_EXCEPT: return "T_MSEXT_EXCEPT";
	case boost::wave::T_MSEXT_FINALLY: return "T_MSEXT_FINALLY";
	case boost::wave::T_MSEXT_LEAVE: return "T_MSEXT_LEAVE";
	case boost::wave::T_MSEXT_INLINE: return "T_MSEXT_INLINE";
	case boost::wave::T_MSEXT_ASM: return "T_MSEXT_ASM";

	case boost::wave::T_MSEXT_PP_REGION: return "T_MSEXT_PP_REGION";
	case boost::wave::T_MSEXT_PP_ENDREGION: return "T_MSEXT_PP_ENDREGION";

// import is needed to be a keyword for the C++ module Standards proposal
	case boost::wave::T_IMPORT: return "T_IMPORT";

// C++11 keywords
	case boost::wave::T_ALIGNAS: return "T_ALIGNAS";
	case boost::wave::T_ALIGNOF: return "T_ALIGNOF";
	case boost::wave::T_CHAR16_T: return "T_CHAR16_T";
	case boost::wave::T_CHAR32_T: return "T_CHAR32_T";
	case boost::wave::T_CONSTEXPR: return "T_CONSTEXPR";
	case boost::wave::T_DECLTYPE: return "T_DECLTYPE";
	case boost::wave::T_NOEXCEPT: return "T_NOEXCEPT";
	case boost::wave::T_NULLPTR: return "T_NULLPTR";
	case boost::wave::T_STATICASSERT: return "T_STATICASSERT";
	case boost::wave::T_THREADLOCAL: return "T_THREADLOCAL";
	case boost::wave::T_RAWSTRINGLIT: return "T_RAWSTRINGLIT";

	case boost::wave::T_LAST_TOKEN_ID: return "T_LAST_TOKEN_ID";
	case boost::wave::T_LAST_TOKEN: return "T_LAST_TOKEN";

// pseudo tokens to help streamlining macro replacement, these should not 
// returned from the lexer nor should these be returned from the pp-iterator
	case boost::wave::T_NONREPLACABLE_IDENTIFIER: return "T_NONREPLACABLE_IDENTIFIER";
	case boost::wave::T_PLACEHOLDER: return "T_PLACEHOLDER";
	case boost::wave::T_PLACEMARKER: return "T_PLACEMARKER";
	case boost::wave::T_PARAMETERBASE: return "T_PARAMETERBASE";
	case boost::wave::T_EXTPARAMETERBASE: return "T_EXTPARAMETERBASE";
	default: return "unknown token id";
	}
}

//typedef struct _udiv_t {
//	unsigned short quot;
//	unsigned short rem;
//} udiv_t;
//
//typedef struct _idiv_t {
//	short quot;
//	short rem;
//} idiv_t;
//
//void div_uint(unsigned short a, unsigned short b, udiv_t* res)
//{
//	if (b > a) {
//		res->rem = b;
//		res->quot = 0;
//		return;
//	}
//	if (a == b) {
//		res->rem = 0;
//		res->quot = 1;
//		return;
//	}
//	res->rem = 0;
//	res->quot = a;
//	int i=16;
//	while (i) {
//		res->rem <<= 1;
//		if (res->quot & 0x8000) {
//			++res->rem;
//		}
//		res->quot <<= 1;
//		if (res->rem >= b) {
//			res->rem -= b;
//			++res->quot;
//		}
//		--i;
//	}
//}
//
//void div_int(int a, int b, idiv_t* res)
//{
//	bool a_neg = false;
//	bool b_neg = false;
//	if (a < 0) {
//		a_neg = true; 
//		a = -a;
//	}
//	if (b < 0) {
//		b_neg = true; 
//		b = -b;
//	}
//	div_uint((unsigned int)a, (unsigned int)b, (udiv_t*)res);
//	if (a_neg) {
//		res->rem = -res->rem;
//	}
//	if (a_neg != b_neg) {
//		res->quot = -res->quot;
//	}
//}

struct CompilerContext {
	std::string entry_point;
	std::vector<std::string> include_directories;
	std::vector<std::string> sys_include_directories;
	std::vector<std::string> input_files;
	std::vector<std::string> library_files;
	std::string output_file;
	std::string disassembly_file;
	bool generate_lib;
	bool optimize;
	bool verbose;
  bool big_endian;

  CompilerContext()
  : generate_lib(false)
  , optimize(false)
  , verbose(false)
  , big_endian(true)
  { }

};

bool read_parameter(xml::Iterator& it, CompilerContext& cntxt)
{
	if (it.current() == xml::Element::empty) {
		if (it.tag() == "generate_library") {
      cntxt.generate_lib = true;
      return true;
    }
		if (it.tag() == "generate_executable") {
      cntxt.generate_lib = false;
      return true;
    }
		if (it.tag() == "big_endian") {
      cntxt.big_endian = true;
      return true;
    }
		if (it.tag() == "little_endian") {
      cntxt.big_endian = false;
      return true;
    }
		if (it.tag() == "optimize") {
      cntxt.optimize = true;
      return true;
    }
		if (it.tag() == "verbose") {
      cntxt.verbose = true;
      return true;
    }
		if (it.tag() == "unoptimize") {
      cntxt.optimize = false;
      return true;
    }
		if (it.tag() == "unverbose") {
      cntxt.verbose = false;
      return true;
    }
		throw cvmc::exception("read_parameter unknown parameter");
  }
	else if (it.current() == xml::Element::tag) {
		if (it.tag() == "include_dir") {
      if (it.next() != xml::Element::content) {throw cvmc::exception("read_parameter no include directory");}
      cntxt.include_directories.push_back(it.value());
    	if ((it.next() != xml::Element::end) || (it.tag() != "include_dir")) {throw cvmc::exception("read_parameter invalid tag 'include_dir'");}
      return true;
		}
		if (it.tag() == "system_include_dir") {
      if (it.next() != xml::Element::content) {throw cvmc::exception("read_parameter no system include directory");}
      cntxt.sys_include_directories.push_back(it.value());
    	if ((it.next() != xml::Element::end) || (it.tag() != "system_include_dir")) {throw cvmc::exception("read_parameter invalid tag 'system_include_dir'");}
      return true;
		}
		if (it.tag() == "input_file") {
      if (it.next() != xml::Element::content) {throw cvmc::exception("read_parameter no input file");}
      cntxt.input_files.push_back(it.value());
    	if ((it.next() != xml::Element::end) || (it.tag() != "input_file")) {throw cvmc::exception("read_parameter invalid tag 'input_file'");}
      return true;
		}
		if (it.tag() == "output_file") {
      if (it.next() != xml::Element::content) {throw cvmc::exception("read_parameter no output file");}
      cntxt.output_file = it.value();
    	if ((it.next() != xml::Element::end) || (it.tag() != "output_file")) {throw cvmc::exception("read_parameter invalid tag 'output_file'");}
      return true;
		}
		if (it.tag() == "disassembly_file") {
      if (it.next() != xml::Element::content) {throw cvmc::exception("read_parameter no disassembly file");}
      cntxt.disassembly_file = it.value();
    	if ((it.next() != xml::Element::end) || (it.tag() != "disassembly_file")) {throw cvmc::exception("read_parameter invalid tag 'disassembly_file'");}
      return true;
		}
		if (it.tag() == "library_file") {
      if (it.next() != xml::Element::content) {throw cvmc::exception("read_parameter no library file");}
      cntxt.library_files.push_back(it.value());
    	if ((it.next() != xml::Element::end) || (it.tag() != "library_file")) {throw cvmc::exception("read_parameter invalid tag 'library_file'");}
      return true;
		}
		throw cvmc::exception("read_parameter unknown tag");
	}
  return false;
}

void read_parameter_file(const std::string& filename, CompilerContext& cntxt)
{
	std::ifstream in(filename.c_str(), ::std::ios::binary);
	xml::Iterator it(in);
  it.begin();
	if (it.current() != xml::Element::tag) {throw cvmc::exception("read_parameter_file invalid file");}
	std::string tag = it.tag();
	it.next_tag();
  while (read_parameter(it, cntxt)) {
    it.next_tag();
  };
	//if (it.current() != xml::Element::xml::Element::tag) {throw cvmc::exception("read_parameter_file invalid file");}
	//it.next_tag();
	if ((it.current() != xml::Element::end) || (it.tag() != tag)) {throw cvmc::exception("read_parameter_file invalid file");}

}


int main(int argc, char**argv)
{
  //udiv_t dinfo;
  //div_uint(160, 15, &dinfo);
	//bool* pba[2] = {0, reinterpret_cast<bool*>(1)};
	//bool* pbap = pba[1];
	//long off0 = reinterpret_cast<long>(pbap)  -  reinterpret_cast<long>(pba);
  //fputc(0, 0);

	//bool b = {true,};
	//bool ba[1][2][3][4] = {
	//	{
	//		{
	//			{
	//				false, false, false, false
	//			},
	//			{
	//				false, false, false, false
	//			},
	//			{
	//				false, false, false, false
	//			}
	//		},
	//		{
	//			{
	//				false, false, false, false
	//			},
	//			{
	//				false, false, false, false
	//			},
	//			{
	//				false, false, false, true
	//			}
	//		}
	//	}
	//};
	//bool(*bap1)[3][4] = ba[0];
	//long off1 = reinterpret_cast<long>(bap1)  -  reinterpret_cast<long>(ba);
	//bool(*bap2)[4] = ba[0][1];
	//long off2 = reinterpret_cast<long>(bap2)  -  reinterpret_cast<long>(ba);
	//bool* bap3 = ba[0][1][2];
	//long off3 = reinterpret_cast<long>(bap3)  -  reinterpret_cast<long>(ba);
	//bool* bap4 = &ba[0][1][2][3];
	//long off4 = reinterpret_cast<long>(bap4)  -  reinterpret_cast<long>(ba);

	//short a = -1;
	//unsigned short b = 0xffff;
	//long c = a;
	//long d = b;
	//unsigned long e = a;
	//unsigned long f = b;

	//char buf[20];
	//char* a=buf;
	//*a++ = 7;
	//bool ba[] = {false, true, true, false, true,};
  //fgetpos(0,0);

  //typedef void (*fun)(int);
  //fun a, b;
  //a = b+5;

  //int* a = 0;
  //int* b = !a;

  //float a = 1.0;
  //float b = ++a;

  //int a = 14, b = 4, c, d;
  //idiv_t dinfo;
  //c = a/b;
  //d = a%b;
  //div_int(a, b, &dinfo);
  //c = a/-b;
  //d = a%-b;
  //div_int(a, -b, &dinfo);
  //c = -a/b;
  //d = -a%b;
  //div_int(-a, b, &dinfo);
  //c = -a/-b;
  //d = -a%-b;
  //div_int(-a, -b, &dinfo);

	if (argc > 1) {
		//lnk::Linker linker;
		//lib::Library library;
    CompilerContext cntxt;
		//std::string entry_point;
		//bool generate_lib = false;
		//bool optimize = false;
		//bool verbose = false;
		//bool big_endian = true;
		//std::vector<std::string> include_directories;
		//std::vector<std::string> sys_include_directories;
		//std::vector<std::string> input_files;
		//std::vector<std::string> library_files;
		//std::string output_file;
		//std::string disassembly_file;
		cvmc::ObjPtr_list_type context_list;
		cvmc::Library lib;
		cvmc::Linker lnk;
		//std::cerr << "argc:" << argc << std::endl;
		int i = 1;
		while (i<argc) {
			std::string opt(argv[i++]);
			if (cntxt.verbose) {std::cout << "arg[" << i-1 << "]:" << opt << std::endl;}
			if (!opt.empty()) {
				if (opt[0] == '-') {
					if (opt == "--lib") {
						cntxt.generate_lib = true;
					}
					else if (opt == "--opt") {
						cntxt.optimize = true;
					}
					else if (opt == "--little") {
						cntxt.big_endian = false;
					}
					else if (opt == "--verbose") {
						cntxt.verbose = true;
					}
					else if (opt == "-v") {
						cntxt.verbose = true;
					}
					else if ((opt == "--dis") && (i<argc)) {
						cntxt.disassembly_file = argv[i++];
					}
					else if ((opt == "-i") && (i<argc)) {
						cntxt.include_directories.push_back(argv[i++]);
					}
					else if ((opt == "-s") && (i<argc)) {
						cntxt.sys_include_directories.push_back(argv[i++]);
					}
					else if ((opt == "-l") && (i<argc)) {
						cntxt.library_files.push_back(argv[i++]);
					}
					else if ((opt == "-f") && (i<argc)) {
						read_parameter_file(argv[i++], cntxt);
					}
					else {
						std::cerr << "invalid option: " << opt << std::endl;
						//linker.error("invalid option "+opt);
					}
				}
				else {
					if (i<argc) {
						cntxt.input_files.push_back(opt);
					}
					else {
						cntxt.output_file = opt;
					}
				}
			}
		}
		if (!cntxt.entry_point.empty()) {
			lnk.entry_point(cntxt.entry_point);
		}
		boost::wave::util::file_position_type current_position;
    cntxt.include_directories.insert(cntxt.include_directories.begin(), ".");
    cntxt.sys_include_directories.push_back(".");

		if (!cntxt.input_files.empty()) {
			for (std::vector<std::string>::iterator itin=cntxt.input_files.begin(); itin!=cntxt.input_files.end(); ++itin) {
				if (!itin->empty()) {
					if (cntxt.verbose) {std::cout << "compiling " << *itin << std::endl;}
					try {
						std::ifstream instream(itin->c_str());
						std::string instr;
						if (!instream.is_open()) {
							std::cerr << "Could not open input file: " << itin->c_str() << std::endl;
							return -2;
						}
						instream.unsetf(std::ios::skipws);
 
						std::string input(
								std::istreambuf_iterator<char>(instream.rdbuf()),
								std::istreambuf_iterator<char>());
						cvmc::context_type ctx(input.begin(), input.end(), itin->c_str());
            for (std::vector<std::string>::iterator it=cntxt.include_directories.begin(); it!=cntxt.include_directories.end(); ++it) {
              ctx.add_include_path(it->c_str());
            }
            for (std::vector<std::string>::iterator it=cntxt.sys_include_directories.begin(); it!=cntxt.sys_include_directories.end(); ++it) {
              ctx.add_sysinclude_path(it->c_str());
            }
						cvmc::context_type::iterator_type it = ctx.begin();
						cvmc::context_type::iterator_type last = ctx.end();
						int lang = ctx.get_language();
						lang &= ~boost::wave::support_cpp;
						lang |= boost::wave::support_option_long_long;
						lang &= ~boost::wave::support_option_variadics;
						ctx.set_language(static_cast<boost::wave::language_support>(lang));

						cvmc::ContextHelper helper(it, last);
						cvmc::Context* ctxt = new cvmc::Context();
						context_list.push_back(ctxt);
						cvmc::Parser p(helper);
						p.parse(*ctxt);
						size_t err = cvmc::IssueList::error().size();
						size_t wrn = cvmc::IssueList::warning().size();

						if (cntxt.generate_lib) {
							lib.add(ctxt);
						}
						else {
							lnk.add(ctxt);
						}


						//while (helper.filter()) {
						//	boost::wave::token_id id = boost::wave::token_id(*it);
						//	std::cout <<	(*it).get_position() << "\t" << token_id_to_string(id)<< "\t: " << (*it).get_value() << std::endl;
						//	++it;
						//}


					}
					catch (boost::wave::cpplexer::lexing_exception const& e) {
					// some lexing error
						std::cerr 
								<< e.file_name() << "(" << e.line_no() << "): "
								<< e.description() << std::endl;
						return -3;
					}
					catch (std::exception const& e) {
					// use last recognized token to retrieve the error position
						std::cerr 
								<< current_position.get_file() 
								<< "(" << current_position.get_line() << "): "
								<< "exception caught: " << e.what()
								<< std::endl;
						return -4;
					}
					catch (...) {
					// use last recognized token to retrieve the error position
						std::cerr 
								<< current_position.get_file() 
								<< "(" << current_position.get_line() << "): "
								<< "unexpected exception caught." << std::endl;
						return -5;
					}
				}
			}
		}
		cvmc::IssueList::warning().print(std::cout);
		cvmc::IssueList::error().print(std::cerr);
		if (cvmc::IssueList::error().size() == 0) {
			int res = 0;
			if (cntxt.generate_lib) {
				std::ofstream out(cntxt.output_file.c_str());
				xml::Writer wr(out);
				lib.write(wr);
				out.close();
			}
			else {
				//res = linker.link(entry_point);
				if (res == 0) {
					if (!cntxt.library_files.empty()) {
						// we must import some libraries
						for (std::vector<std::string>::iterator it=cntxt.library_files.begin(); it!=cntxt.library_files.end(); ++it) {
							lnk.import_library(*it);
						}
					}
					// no errors, we can generate the file
					lnk.link();
					if (!cntxt.output_file.empty()) {
						std::ofstream out(cntxt.output_file.c_str(), std::ios_base::out | std::ios_base::binary);
						lnk.big_endian(cntxt.big_endian);
						lnk.write(out);
						out.close();
					}
					if (!cntxt.disassembly_file.empty()) {
						std::ofstream dis(cntxt.disassembly_file.c_str());
						lnk.disassembly(dis);
						dis.close();
					}
				}
			}
			return res;
		}
		return cvmc::IssueList::error().size();
	}
	std::cout << "usage:" << std::endl;
	std::cout << "\tcvmc [--lib] [--little] [--dis <file>] {-s <include_dir>} {-i <include_dir>} {-l <library_file>} {<source_file>} <target_file>" << std::endl;
	std::cout << "\t\t--lib target_file is a library" << std::endl;
	std::cout << "\t\t--little target_file is little endian (default is big endian)" << std::endl;
	std::cout << "\t\t--dis <file> generate a disassembly file (linking only)" << std::endl;
	std::cout << "\t\t-i <include_dir> add include directory" << std::endl;
	std::cout << "\t\t-s <include_dir> add system include directory" << std::endl;
	std::cout << "\t\t-l <library_file> add library for linking" << std::endl;
	std::cout << "\t\t-f <parameter_file> use file for parameter" << std::endl;
	std::cout << "\t\t<source_file> to be compiled" << std::endl;
	std::cout << "\t\t<target_file> destination file" << std::endl;
	return -1;
}
