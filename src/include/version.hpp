#ifndef VERSION_HPP_
#define VERSION_HPP_

#include <boost/version.hpp>

#define GA_VERSION_MAJOR      1
#define GA_VERSION_MINOR      1
#define GA_VERSION_MINOR_SUB  0
#define GA_VERSION_STR        "1.1.0"
#define GA_VERSION_WIN_RC_STR "1.1.0\0"

#define GA_COMPILER_WARNING "use GA_COMPILER_WARNING to force a compiler warning."

// TODO check GA_ATEXIT_FIXED compilers and Version
// see: "C++ Standard Library Issues List (Revision 8)"
//    "Atexit registration during atexit() call is not described"
// url: http://anubis.dkuug.dk/jtc1/sc22/wg21/docs/lwg-issues.html#3
// url: http://gcc.gnu.org/ml/gcc/2006-01/msg00868.html
//   __cxa_atexit -fuse-cxa-exit

/** 
 * @brief GA_IS_DEBUG defined for debug compilation.
 */
#if defined(_DEBUG) || defined(IS_DEBUG)
#  define GA_IS_DEBUG
#endif


/**********************************************/
/* compilers                                  */
/**********************************************/
/** 
 * @brief compiler defines
@verbatim
for each compiler the following two defines are defined:
 GA_COMPILER_NAME               the compiler as string
 GA_COMPILER_VERSION            the version which define from the compiler
the following compiler defines are defined:
 GA_IS_COMPILER_ACC             HP aCC C++ compiler
 GA_IS_COMPILER_BORLANDC        Borland C++ compiler
 GA_IS_COMPILER_COMO            Comeau C++ compiler
 GA_IS_COMPILER_DMC             Digital Mars C++ compiler
 GA_IS_COMPILER_GCC             GNU C++ compiler
 GA_IS_COMPILER_IBMCPP          IBM Visual Age C++ compiler
 GA_IS_COMPILER_ICC             Intel ICC compiler
 GA_IS_COMPILER_KCC             Kai C++ compiler
 GA_IS_COMPILER_MWERKS          Metrowerks C++ compiler
 GA_IS_COMPILER_MSVC            Microsoft Visual C++ compiler
 GA_IS_COMPILER_SUNPROCC        Sun C++ compiler
@endverbatim
 */

/* Borland C++ compiler */
#if defined(__BORLANDC__)
#  define GA_IS_COMPILER_BORLANDC __BORLANDC__
#  define GA_COMPILER_NAME        "Borland C++"
#  define GA_COMPILER_VERSION     __BORLANDC__

/* Comeau C++ compiler */
#elif defined(__COMO_VERSION__)
#  define GA_IS_COMPILER_COMO     __COMO_VERSION__
#  define GA_COMPILER_NAME        "Comeau C++"
#  define GA_COMPILER_VERSION     __COMO_VERSION__

/* Digital Mars C++ compiler */
#elif defined(__DMC__)
#  define GA_IS_COMPILER_DMC      __DMC__
#  define GA_COMPILER_NAME        "Digital Mars C++"
#  define GA_COMPILER_VERSION     __DMC__

/* GNU C++ compiler */
#elif defined(__GNUC__)
#  define GA_IS_COMPILER_GCC      __GNUC__
#  define GA_COMPILER_NAME        "GNU C++"
#  define GA_COMPILER_VERSION     __GNUC__

/* HP aCC C++ compiler */
#elif defined(__HP_aCC)
#  define GA_IS_COMPILER_ACC      __HP_aCC
#  define GA_COMPILER_NAME        "HP aCC C++"
#  define GA_COMPILER_VERSION     __HP_aCC

/* Intel compiler */
#elif defined(__INTEL_COMPILER)
#  define GA_IS_COMPILER_ICC      __INTEL_COMPILER
#  define GA_COMPILER_NAME        "Intel ICC"
#  define GA_COMPILER_VERSION     __INTEL_COMPILER

/* Kai C++ compiler */
#elif defined(__KCC_VERSION)
#  define GA_IS_COMPILER_KCC      __KCC_VERSION
#  define GA_COMPILER_NAME        "Kai C++"
#  define GA_COMPILER_VERSION     __KCC_VERSION

/* Metrowerks C++ compiler */
#elif defined(__MWERKS__)
#  define GA_IS_COMPILER_MWERKS   __MWERKS__
#  define GA_COMPILER_NAME        "Metrowerks C++"
#  define GA_COMPILER_VERSION     __MWERKS__

/* Sun C++ compiler */
#elif defined(__SUNPRO_CC)
#  define GA_IS_COMPILER_SUNPROCC __SUNPRO_CC
#  define GA_COMPILER_NAME        "Sun C++"
#  define GA_COMPILER_VERSION     __SUNPRO_CC

/* IBM Visual Age C++ compiler */
#elif defined(__IBMCPP__)
#  define GA_IS_COMPILER_IBMCPP   __IBMCPP__
#  define GA_COMPILER_NAME        "IBM Visual Age C++"
#  define GA_COMPILER_VERSION     __IBMCPP__

/* Microsoft Visual C++ compiler */
#elif defined(_MSC_VER)
#  if _MSC_VER < 1300 // for example VC++ 6.0 and eVC++4
#    error "your VC++ is to old and not supported please use VC++ >= 7.0"
#  endif
#  define GA_IS_COMPILER_MSVC     _MSC_VER
#  define GA_COMPILER_NAME        "MS Visual C++"
#  define GA_COMPILER_VERSION     _MSC_VER

/* Microsoft Resource compiler */
#elif defined(RC_INVOKED)
#  define GA_IS_COMPILER_RC     _MSC_VER
#  define GA_COMPILER_NAME        "MS Resource Compiler"
#  define GA_COMPILER_VERSION     _MSC_VER

/* unknown compiler */
#else
#  define BCONFIG__UNKNOWN_COMPILER__PLEASE_FIX_ME 1
#  define BCONFIG__UNKNOWN_COMPILER__PLEASE_FIX_ME 0
#  define GA_IS_COMPILER_UNKNOWN  0
#  define GA_COMPILER_NAME        "compiler unknown"
#  define GA_COMPILER_VERSION     0
#endif

/**********************************************/
/* os                                         */
/**********************************************/
/** 
 * @brief OS defines
@verbatim
for each OS the following defines are defined:

IBM AIX:
 GA_IS_OS_AIX
 GA_OS_NAME "AIX"
 GA_IS_UNIX

AmigaOS:
 GA_IS_OS_AMIGAOS
 GA_OS_NAME "AmigaOS"

BeOS:
 GA_IS_OS_BEOS
 GA_OS_NAME "BeOS"

FreeBSD:
 GA_IS_OS_FREEBSD
 GA_OS_NAME "FreeBSD"
 GA_IS_BSD
 GA_IS_UNIX

Cygwin:
 GA_IS_OS_CYGWIN
 GA_OS_NAME "Cygwin"

DragonFly:
 GA_IS_OS_DRAGONFLY
 GA_OS_NAME "DragonFly"
 GA_IS_BSD
 GA_IS_UNIX

HP Unix:
 GA_IS_OS_HPUX
 GA_OS_NAME "HP-UX"
 GA_IS_UNIX

Linux:
 GA_IS_OS_LINUX
 GA_OS_NAME "Linux"
 GA_IS_LINUX
 GA_IS_UNIX

MacOS:
 GA_IS_OS_MACINTOSH
 GA_OS_NAME "Macintosh"

NetBSD:
 GA_IS_OS_NETBSD
 GA_OS_NAME "NetBSD"
 GA_IS_BSD
 GA_IS_UNIX

OpenBSD:
 GA_IS_OS_OPENBSD
 GA_OS_NAME "OpenBSD"
 GA_IS_BSD
 GA_IS_UNIX

SGI Irix:
 GA_IS_OS_IRIX
 GA_OS_NAME "Irix"
 GA_IS_UNIX

Solaris:
 GA_IS_OS_SOLARIS
 GA_OS_NAME "Solaris"
 GA_IS_UNIX

QNX:
 GA_IS_OS_QNX
 GA_OS_NAME "QNX"
 GA_IS_UNIX

Microsoft Windows:
 GA_IS_OS_WINDOWS
 GA_OS_NAME "Windows"
 GA_IS_WINDOWS

unkown OS:
 GA_IS_OS_UNKNOWN
 GA_OS_NAME "OS unknown"
@endverbatim
 */

/* IBM AIX */
#if defined(_AIX)
#  define GA_IS_OS_AIX
#  define GA_OS_NAME "AIX"
#  define GA_IS_UNIX

/* AmigaOS */
#elif defined(__amigaos__)
#  define GA_IS_OS_AMIGAOS
#  define GA_OS_NAME "AmigaOS"

/* BeOS */
#elif defined(__BEOS__)
#  define GA_IS_OS_BEOS
#  define GA_OS_NAME "BeOS"

/* FreeBSD */
#elif defined(__FreeBSD__)
#  define GA_IS_OS_FREEBSD
#  define GA_OS_NAME "FreeBSD"
#  define GA_IS_BSD
#  define GA_IS_UNIX

/* Cygwin */
#elif defined(__CYGWIN__)
#  define GA_IS_OS_CYGWIN
#  define GA_OS_NAME "Cygwin"

/* DragonFly */
#elif defined(__DragonFly__)
#  define GA_IS_OS_DRAGONFLY
#  define GA_OS_NAME "DragonFly"
#  define GA_IS_BSD
#  define GA_IS_UNIX

/* HP unix */
#elif defined(__hpux)
#  define GA_IS_OS_HPUX
#  define GA_OS_NAME "HP-UX"
#  define GA_IS_UNIX

/* linux */
#elif defined(linux) || defined(__linux) || defined(__linux__)
#  define GA_IS_OS_LINUX
#  define GA_OS_NAME "Linux"
#  define GA_IS_LINUX
#  define GA_IS_UNIX

/* MacOS */
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#  define GA_IS_OS_MACINTOSH
#  define GA_OS_NAME "Macintosh"

/* NetBSD */
#elif defined(__NetBSD__)
#  define GA_IS_OS_NETBSD
#  define GA_OS_NAME "NetBSD"
#  define GA_IS_BSD
#  define GA_IS_UNIX

/* OpenBSD */
#elif defined(__OpenBSD__)
#  define GA_IS_OS_OPENBSD
#  define GA_OS_NAME "OpenBSD"
#  define GA_IS_BSD
#  define GA_IS_UNIX

/* SGI Irix */
#elif defined(__sgi)
#  define GA_IS_OS_IRIX
#  define GA_OS_NAME "Irix"
#  define GA_IS_UNIX

/* Solaris */
#elif defined(__sun)
#  if defined(GA_OS_VERSION)
#    define GA_IS_OS_SOLARIS GA_OS_VERSION
#  else
#    define GA_IS_OS_SOLARIS
#  endif
#  define GA_OS_NAME "Solaris"
#  define GA_IS_UNIX

/* QNX */
#elif defined(__QNXNTO__)
#  define GA_IS_OS_QNX
#  define GA_OS_NAME "QNX"
#  define GA_IS_UNIX

/* Windows */
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32) \
   || defined(_WIN64) || defined(__WIN64__) || defined(WIN64)
#  define GA_IS_OS_WINDOWS
#  define GA_OS_NAME "Windows"
#  define GA_IS_WINDOWS

/* unkown OS */
#else
#  define BCONFIG__UNKNOWN_OS__PLEASE_FIX_ME 1
#  define BCONFIG__UNKNOWN_OS__PLEASE_FIX_ME 0
#  define GA_IS_OS_UNKNOWN
#  define GA_OS_NAME "OS unknown"
#endif


// Nicht Fuer den Microsoft Resource compiler
#ifndef RC_INVOKED

/**********************************************/
/* GNU C++ compiler settings                  */
/**********************************************/
/** 
 * @brief architecture defines
@verbatim
architecture:
 GA_IS_ARCH_X86
 GA_IS_ARCH_SPARC

windos:
 GA_IS_WIN32
 GA_IS_WIN64

bit:
 GA_IS_32BIT
 GA_IS_64BIT

endian:
 GA_IS_LITTLE_ENDIAN
 GA_IS_BIG_ENDIAN
@endverbatim
 */

#if defined(GA_IS_COMPILER_GCC)
#  if defined(__x86_64)
#    define GA_IS_ARCH_X86
#    define GA_IS_LITTLE_ENDIAN
#    define GA_IS_64BIT
#  elif defined(__i386)
#    define GA_IS_ARCH_X86
#    define GA_IS_LITTLE_ENDIAN
#    define GA_IS_32BIT
#  elif defined(__sparcv9)
#    define GA_IS_ARCH_SPARC
#    define GA_IS_BIG_ENDIAN
#    define GA_IS_64BIT
#  elif defined(__sparc)
#    define GA_IS_ARCH_SPARC
#    define GA_IS_BIG_ENDIAN
#    define GA_IS_32BIT
#  else
#    define BCONFIG__MISSING_DEFINE_FOR_COMPILER_GCC__PLEASE_FIX_ME 1
#    define BCONFIG__MISSING_DEFINE_FOR_COMPILER_GCC__PLEASE_FIX_ME 0
#  endif
#endif

/**********************************************/
/* SUN C++ compiler settings                  */
/**********************************************/
#if defined(GA_IS_COMPILER_SUNPROCC)
#  if defined(__x86_64)
#    define GA_IS_ARCH_X86
#    define GA_IS_LITTLE_ENDIAN
#    define GA_IS_64BIT
#  elif defined(__i386)
#    define GA_IS_ARCH_X86
#    define GA_IS_LITTLE_ENDIAN
#    define GA_IS_32BIT
#  elif defined(__sparcv9)
#    define GA_IS_ARCH_SPARC
#    define GA_IS_BIG_ENDIAN
#    define GA_IS_64BIT
#  elif defined(__sparc)
#    define GA_IS_ARCH_SPARC
#    define GA_IS_BIG_ENDIAN
#    define GA_IS_32BIT
#  else
#    define BCONFIG__MISSING_DEFINE_FOR_COMPILER_SUNPROCC__PLEASE_FIX_ME 1
#    define BCONFIG__MISSING_DEFINE_FOR_COMPILER_SUNPROCC__PLEASE_FIX_ME 0
#  endif
#endif

/**********************************************/
/* Microsoft Visual C++ compiler settings     */
/**********************************************/
#if defined(GA_IS_WINDOWS) && defined(GA_IS_COMPILER_MSVC)
#  if ! defined(NOMINMAX)
#    define NOMINMAX
#  endif
// see: http://msdn.microsoft.com/en-us/library/aa383745(VS.85).aspx?ppud=4
#  ifdef GA_DEFINE_WIN32_WINNT
#    define _WIN32_WINNT GA_DEFINE_WIN32_WINNT
#  endif
#  if ! defined(_WIN32_WINNT)
#    define _WIN32_WINNT 0x0502
#  endif
#  if defined(_WIN64)
#    define GA_IS_ARCH_X86
#    define GA_IS_LITTLE_ENDIAN
#    define GA_IS_WIN64
#    define GA_IS_64BIT
#  elif defined(_WIN32)
#    define GA_IS_ARCH_X86
#    define GA_IS_LITTLE_ENDIAN
#    define GA_IS_WIN32
#    define GA_IS_32BIT
#  else
#    define BCONFIG__MISSING_DEFINE_FOR_COMPILER_MSVC__PLEASE_FIX_ME 1
#    define BCONFIG__MISSING_DEFINE_FOR_COMPILER_MSVC__PLEASE_FIX_ME 0
#  endif
#  if defined(GA_COMPILE_STRICT)
#    pragma warning(default: 4242) /**< the types are different. Type conversion may result in loss of data. */
#    pragma warning(default: 4254) /**< a larger bit field was assigned to a smaller bit field. There could be a loss of data. */
#    pragma warning(default: 4263) /**< a class function definition has the same name as a virtual function in a base class but not the same number or type of arguments. This effectively hides the virtual function in the base class. */
#    pragma warning(default: 4264) /**< 'virtual_function' : no override available for virtual member function from base 'class'; function is hidden */
#    pragma warning(default: 4265) /**< ISO/IEC 14882 5.3.5 [expr.delete]: 'class' : class has virtual functions, but destructor is not virtual */
#    pragma warning(default: 4296) //*< an unsigned variable was used in a comparison operation with zero. */
#    pragma warning(default: 4302) //*< the compiler detected a conversion from a larger type to a smaller type. Information may be lost. */
#    pragma warning(default: 4365) //*< 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch */
#    pragma warning(default: 4623) //*< 'derived class' : default constructor could not be generated because a base class default constructor is inaccessible */
#    pragma warning(default: 4625) //*< 'derived class' : copy constructor could not be generated because a base class copy constructor is inaccessible */
#    pragma warning(default: 4626) //*< 'derived class' : assignment operator could not be generated because a base class assignment operator is inaccessible */
#    pragma warning(default: 4668) //*< 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives' */
#    pragma warning(default: 4946) //*< reinterpret_cast used between related classes: 'class1' and 'class2' */
#    pragma warning(error:   4001) /**< nonstandard extension 'single line comment' was used (disallow "//" C++ comments) */
#    pragma warning(error:   4002) /**< too many actual parameters for macro 'identifier' */
#    pragma warning(error:   4003) /**< not enough actual parameters for macro 'identifier' */
#    pragma warning(error:   4004) /**< incorrect construction after 'defined' */
//   pragma warning(error:   4005) /**< disabled for the redefinition warning hack: macro redefinition */
#    pragma warning(error:   4006) /**< #undef expected an identifier */
#    pragma warning(error:   4009) /**< string too big; trailing characters truncated */
#    pragma warning(error:   4013) /**< 'function' undefined; assuming extern returning int */
#    pragma warning(error:   4015) /**< 'identifier' : type of bit field must be integral */
#    pragma warning(error:   4016) /**< 'function' : no function return type; using int as default */
#    pragma warning(error:   4018) /**< 'expression' : signed/unsigned mismatch */
#    pragma warning(error:   4020) /**< 'function' : too many actual parameters */
#    pragma warning(error:   4021) /**< 'function' : too few actual parameters */
#    pragma warning(error:   4022) /**< 'function' : pointer mismatch for actual parameter 'number' */
#    pragma warning(error:   4023) /**< 'symbol' : based pointer passed to unprototyped function : parameter number */
#    pragma warning(error:   4024) /**< 'function' : different types for formal and actual parameter 'number' */
#    pragma warning(error:   4025) /**< 'number' : based pointer passed to function with variable arguments: parameter number */
#    pragma warning(error:   4026) /**< function declared with formal parameter list */
#    pragma warning(error:   4027) /**< function declared without formal parameter list */
#    pragma warning(error:   4028) /**< formal parameter 'number' different from declaration */
#    pragma warning(error:   4029) /**< declared formal parameter list different from definition */
#    pragma warning(error:   4030) /**< first formal parameter list longer than the second list */
#    pragma warning(error:   4031) /**< second formal parameter list longer than the first list */
#    pragma warning(error:   4033) /**< 'function' must return a value */
#    pragma warning(error:   4035) /**< 'function' : no return value */
#    pragma warning(error:   4036) /**< unnamed 'type' as actual parameter */
#    pragma warning(error:   4045) /**< 'identifier' : array bounds overflow */
#    pragma warning(error:   4047) /**< 'identifier1' : 'operator' : different levels of indirection from 'identifier2' */
#    pragma warning(error:   4049) /**< compiler limit : terminating line number emission */
#    pragma warning(error:   4051) /**< type conversion; possible loss of data */
#    pragma warning(error:   4053) /**< one void operand for '?:' */
#    pragma warning(error:   4054) /**< 'conversion' : from function pointer 'type1' to data pointer 'type2' */
#    pragma warning(error:   4057) /**< 'operator' : 'identifier1' indirection to slightly different base types from 'identifier2' */
#    pragma warning(error:   4059) /**< pascal string too big, length byte is length % 256 */
#    pragma warning(error:   4063) /**< case 'identifier' is not a valid value for switch of enum 'identifier' */
#    pragma warning(error:   4064) /**< switch of incomplete enum 'identifier' */
#    pragma warning(error:   4071) /**< 'function' : no function prototype given */
#    pragma warning(error:   4072) /**< 'function' : no function prototype on 'convention' function */
#    pragma warning(error:   4078) /**< case constant 'value' too big for the type of the switch expression */
#    pragma warning(error:   4081) /**< expected 'token1'; found 'token2' */
#    pragma warning(error:   4087) /**< 'function' : declared with 'void' parameter list */
#    pragma warning(error:   4088) /**< 'function' : pointer mismatch in actual parameter 'number', formal parameter 'number' */
#    pragma warning(error:   4089) /**< 'function' : different types in actual parameter 'number', formal parameter 'number' */
#    pragma warning(error:   4098) /**< 'function' : void function returning a value */
#    pragma warning(error:   4113) /**< 'identifier1' differs in parameter lists from 'identifier2' */
#    pragma warning(error:   4129) /**< 'character' : unrecognized character escape sequence */
#    pragma warning(error:   4133) /**< 'type' : incompatible types - from 'type1' to 'type2' */
#    pragma warning(error:   4150) /**< deletion of pointer to incomplete type 'type'; no destructor called */
#    pragma warning(error:   4172) /**< returning address of local variable or temporary */
#    pragma warning(error:   4221) /**< nonstandard extension used : 'identifier' : cannot be initialized using address of automatic variable */
#    pragma warning(error:   4223) /**< nonstandard extension used : non-lvalue array converted to pointer */
#    pragma warning(error:   4224) /**< nonstandard extension used : formal parameter 'identifier' was previously defined as a type */
#    pragma warning(error:   4390) /**< ';' : empty controlled statement found; is this what was intended?" */
#    pragma warning(error:   4508) /**< 'function' : function should return a value; void return type assumed */
#    pragma warning(error:   4541) /**< 'identifier' used on polymorphic type 'type' with /GR-; unpredictable behavior may result */
#    pragma warning(error:   4551) /**< function call missing argument list */
#    pragma warning(error:   4553) /**< 'operator' : operator has no effect; did you intend 'operator'? */
#    pragma warning(error:   4700) /**< local variable 'name' used without having been initialized */
#    pragma warning(error:   4706) /**< assignment within conditional expression */
#    pragma warning(error:   4715) /**< 'function' : not all control paths return a value */
#    pragma warning(error:   4761) /**< integral size mismatch in argument : conversion supplied */
#    pragma warning(disable: 4996) /**< This function or variable may be unsafe. */
// #    pragma warning(disable: 4503) boost::spirit
// #    pragma warning(disable: 4242)
#  endif
#endif

/**********************************************/
/* checks                                     */
/**********************************************/
#if ! defined(GA_IS_LITTLE_ENDIAN) && ! defined(GA_IS_BIG_ENDIAN)
#  define BCONFIG__MISSING_DEFINE_FOR_LITTLE_ENDIAN_OR_BIG_ENDIAN__PLEASE_FIX_ME 1
#  define BCONFIG__MISSING_DEFINE_FOR_LITTLE_ENDIAN_OR_BIG_ENDIAN__PLEASE_FIX_ME 0
#endif
#if ! defined(GA_IS_32BIT) && ! defined(GA_IS_64BIT)
#  define BCONFIG__MISSING_DEFINE_FOR_32BIT_OR_64BIT__PLEASE_FIX_ME 1
#  define BCONFIG__MISSING_DEFINE_FOR_32BIT_OR_64BIT__PLEASE_FIX_ME 0
#endif

/**********************************************/
/* GA_HAS_TERMIOS                              */
/**********************************************/
#if defined(GA_IS_UNIX)
#  define GA_HAS_TERMIOS
#endif

/**********************************************/
/* GA_LACKS_SETENV                             */
/**********************************************/
#if defined(GA_IS_OS_SOLARIS)
#  if (GA_OS_VERSION <= 0x0508)
#    define GA_LACKS_SETENV
#  endif
#endif

#if (BOOST_VERSION >= 104400)
#  define GA_BOOST_HAS_RANDOM_DEVICE
#endif

#endif /* ! RC_INVOKED */

#endif
