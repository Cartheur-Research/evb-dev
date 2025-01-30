#ifndef XML_HPP
#define XML_HPP

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

namespace xml {

class exception : public std::runtime_error {
public:
	explicit exception(const char* msg) : std::runtime_error(msg) {}
	explicit exception(const std::string& msg) : std::runtime_error(msg.c_str()) {}
};

class not_implemented_exception : public exception {
public:
	explicit not_implemented_exception(const char* msg) : exception(msg) {}
	explicit not_implemented_exception(const std::string& msg) : exception(msg.c_str()) {}
};

struct Element {
  enum Enum {
    undefined
,   tag		// begin tag
,   end		// end tag
,   empty		// empty tag
,   attribute
,   content
,   content_value
,   comment
,   processing_instruction
,   error		// invalid document
,   eof
,   limit
  };
};

class Writer {
	std::ostream&	              out_;
  std::map<std::string, int>  visit_map_;
	int				                  indent_;
	bool			                  print_pretty_;

public:
	Writer(std::ostream&);

	std::ostream& out() {return out_;}
	int indent() const {return indent_;}

  bool visited(const std::string&);

	void hex();
	void dec();
	void width(size_t);
	void newline(int delta = 0);
	void indent(int delta);
	void write(const std::string&);
  void write(const std::vector<long>&);
  void open(const std::string& tag, bool is_ending, bool close = false);
  void close(bool is_ending);
  void tag(const std::string& tag, const std::string& value);
  void attribute(const std::string& name, const std::string& value);
};

class Iterator {
	std::istream&	in_;
	std::string		tag_;
	std::string		key_;
	std::string		value_;
	Element::Enum	prev_;
	Element::Enum	curr_;
	wchar_t			  char_;
	bool			    char_was_escaped_;

	bool get_char();
	void get_next();
	bool get_next_unescaped_char();
	bool skip_whitespace();
	bool get_next_char(bool skip_ws);
	void assure_next_char(bool skip_ws);
	Element::Enum scan_tag();
	Element::Enum scan_end();
	Element::Enum scan_attribute();
	//Element::Enum value();
	Element::Enum scan_pi();
	Element::Enum scan_comment();

public:
	Iterator(std::istream& in);

	void reset();
	Element::Enum next();
	Element::Enum next_tag();
	Element::Enum next_content();

	bool start(bool skip_ws) {return get_next_char(skip_ws);}

	Element::Enum current() const {return curr_;}
	const std::string& tag() const {return tag_;}
	const std::string& key() const {return key_;}
	const std::string& value() const {return value_;}

  void read(std::vector<long>&);

  void begin();

	static bool is_whitespace(wchar_t);
	
};


}

#endif
