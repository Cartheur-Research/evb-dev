#include "xml.hpp"
#include <iostream>
#include <sstream>

namespace xml {

Iterator::Iterator(std::istream& in)
: in_(in)
, prev_(Element::undefined)
, curr_(Element::undefined)
, char_(0)
, char_was_escaped_(false)
{
}

void Iterator::begin()
{
	if (!start(true)) {throw exception("xml::Iterator::begin invalid file");}
	next_tag();
	if (current() == Element::processing_instruction) {
		next_tag();
		while (current() == Element::attribute) {
			next_tag();
		}
		if (current() != Element::end) {throw exception("xml::Iterator::begin invalid parameter");}
		next_tag();
	}
}

bool Iterator::is_whitespace(wchar_t ch)
{
	if (ch <= ' ') {
		if (ch == ' ') {return true;}
		if (ch == 0x09) {return true;}
		if (ch == 0x0a) {return true;}
		if (ch == 0x0d) {return true;}
		throw exception("xml::Iterator::is_whitespace invalid control character");
	}
	return false;
}

void Iterator::get_next()
{
	unsigned char ch;
	char val;
	if (!in_.get(val)) {throw exception("xml::Iterator::get_next eof");}
	ch = static_cast<unsigned char>(val);
	if ((ch <= 0x7f) || (ch >= 0xc0)) {throw exception("xml::Iterator::get_next no continuation byte");}
	char_ <<= 6;
	char_ |= ch & 0x3f;
}

bool Iterator::get_char()
{
	if (curr_ < Element::error) {
		if (in_.eof()) {
			curr_ = Element::eof;
			return false;
		}
		char val;
		if (!in_.get(val)) {return false;}
		unsigned char ch = static_cast<unsigned char>(val);
		if (ch <= 0x7f) {
			char_ = ch;
			return true;
		}
		if (ch < 0xc0) {
			curr_ = Element::error;
			return false;
		}
		if (ch < 0xe0) {
			char_ = ch & 0x1f;
			get_next();
			return true;
		}
		if (ch < 0xf0) {
			char_ = ch & 0x0f;
			get_next();
			get_next();
			return true;
		}
		if (ch < 0xf8) {
			char_ = ch & 0x07;
			get_next();
			get_next();
			get_next();
			return true;
		}
		curr_ = Element::error;
	}
	return false;
}

bool Iterator::skip_whitespace()
{
	if (curr_ < Element::error) {
		while (get_char()) {
			if (!is_whitespace(char_)) {
				return true;
			}
		}
	}
	if (curr_ != Element::error) {
		curr_ = Element::eof;
	}
	return false;
}


bool Iterator::get_next_char(bool skip_ws)
{
	char_was_escaped_ = false;
	if (get_char()) {
		while (is_whitespace(char_)) {
			if (!skip_ws) {return true;}
			if (!get_char()) {return false;}
		}
		if (char_ != '&') {return true;}
		if (!get_char()) {throw exception("xml::Iterator::get_next_non_ws_char invalid escape");}
		char_was_escaped_ = true;
		std::string name;
    name.push_back(char_);
		while (get_char()) {
			if (char_ == ';') {
				if (name == "lt") {char_ = '<'; return true;}
				if (name == "gt") {char_ = '>'; return true;}
				if (name == "amp") {
          char_ = '&'; return true;
        }
				if (name == "quot") {char_ = '"'; return true;}
				if (name == "apos") {char_ = '\''; return true;}
				throw exception("xml::Iterator::get_next_non_ws_char invalid escape sequence");
			}
			else {
				if ((char_ & 0x7f) != char_) {throw exception("xml::Iterator::get_next_non_ws_char invalid escape character");}
				name += static_cast<char>(char_);
			}
		}
		throw exception("xml::Iterator::get_next_non_ws_char invalid escape");
	}
	return false;
}

void Iterator::assure_next_char(bool skip_ws)
{
	if (!get_next_char(skip_ws)) {throw exception("xml::Iterator::assure_next_char premature end of file");}
}

void Iterator::reset()
{
	curr_ = Element::undefined;
	char_ = 0;
	tag_.clear();
	key_.clear();
	value_.clear();
}

Element::Enum Iterator::scan_end()
{
	try {
		if ((curr_ != Element::empty) && (curr_ != Element::end) && (curr_ != Element::content) && (curr_ != Element::comment) && (curr_ != Element::processing_instruction)) {
			throw exception("xml::Iterator::tag invalid tag end");
		}
		tag_.clear();
		key_.clear();
		value_.clear();
		for (;;) {
			if (is_whitespace(char_)) {throw exception("xml::Iterator::tag invalid tag end");}
			if (char_ == '>' && !char_was_escaped_) {
  			get_next_char(false);
				return curr_ = Element::end;
			}
			tag_ += static_cast<char>(char_);
			assure_next_char(false);
		}
	}
	catch (exception& ex){
	}
	return curr_ = Element::error;
}

Element::Enum Iterator::scan_tag()
{
	try {
		//if ((curr_ != Element::empty) && (curr_ != Element::end) && (curr_ != Element::content) && (curr_ != Element::undefined) && (curr_ != Element::comment) && (curr_ != Element::processing_instruction)) {
		//	throw exception("xml::Iterator::tag invalid tag position");
		//}
    Element::Enum res = Element::tag;
    bool first = true;
		tag_.clear();
		key_.clear();
		value_.clear();
		while (!is_whitespace(char_)) {
			if (char_ == '/' && !char_was_escaped_) {
				assure_next_char(false);
				if (char_ == '>' && !char_was_escaped_) {
					get_next_char(false);
					return curr_ = Element::empty;
				}
        if (first) {res = Element::end;}
			}
      first = false;
			if (char_ == '>' && !char_was_escaped_) {
				//get_next_char(false);
        curr_ = res;
		    return res;
			}
			tag_ += static_cast<char>(char_);
			assure_next_char(false);
		}
		assure_next_char(true);
    curr_ = res;
		return res;
	}
	catch (exception& ex){
	}
	return curr_ = Element::error;


	//tag_ = static_cast<char>(char_);
	//for (;;) {
	//	if (!get_char(in)) {throw exception("xml::Iterator::tag invalid tag");}
	//	if (is_whitespace(char_)) {
	//		if (!get_next_char(in, true)) {
	//			return Element::error;
	//		}
	//		if (!char_was_escaped_ && char_== '/') {
	//			if (!get_char(in)) {throw exception("xml::Iterator::tag invalid tag");}
	//			if (char_ != '>' || char_was_escaped_) {throw exception("xml::Iterator::tag invalid empty tag");}
	//			return Element::empty;
	//		}
	//		return Element::tag;
	//	}
	//	if (char_was_escaped_) {throw exception("xml::Iterator::tag escaped character");}
	//	tag_ += static_cast<char>(char_);
	//}
	//return Element::undefined;

	//if (char_ == '/') {
	//	if (!skip_whitespace(in)) {throw exception("xml::Iterator::tag invalid tag");}
	//	while (!is_whitespace(char_)) {
	//		if (char_ == '>') {
	//			if (!skip_whitespace(in)) {throw exception("xml::Iterator::tag invalid tag");}
	//			if (char_ != '>') {throw exception("xml::Iterator::tag invalid tag");}\
	//			return Element::empty;
	//		}
	//		tag_.append(1, static_cast<char>(char_));
	//		if (!get_char(in)) {throw exception("xml::Iterator::tag invalid tag");}
	//	}
	//	if (char_ != '>') {throw exception("xml::Iterator::tag invalid tag");}\
	//	return Element::end;
	//}
	//while (!is_whitespace(char_)) {
	//	if (char_ == '/') {
	//		if (!skip_whitespace(in)) {throw exception("xml::Iterator::tag invalid tag");}
	//		if (char_ != '>') {throw exception("xml::Iterator::tag invalid tag");}\
	//		return Element::empty;
	//	}
	//	if (char_ == '>') {
	//		return Element::tag;
	//	}
	//	tag_.append(1, static_cast<char>(char_));
	//	if (!get_char(in)) {throw exception("xml::Iterator::tag invalid tag");}
	//}
	//if (tag_.empty()) {throw exception("xml::Iterator::tag invalid empty tag");}
	//return Element::tag;
}

Element::Enum Iterator::scan_attribute()
{
	try {
		if ((curr_ != Element::tag) && (curr_ != Element::processing_instruction) && (curr_ != Element::attribute)) {
			throw exception("xml::Iterator::tag invalid tag position");
		}
		if (curr_ != Element::attribute) {prev_ = curr_;}
		key_.clear();
		value_.clear();
		while (!is_whitespace(char_)) {
			if (char_ == '=' && !char_was_escaped_) {
				break;
			}
			if (char_ == '>' && !char_was_escaped_) {
				if (key_.empty()) {
					if (prev_ != Element::tag) {throw exception("xml::Iterator::tag invalid tag ending");}
					assure_next_char(false);
          return next_content();
					//return curr_ = Element::content;
          //return next();
				}
				if (key_ == "?") {
					if (prev_ != Element::processing_instruction) {throw exception("xml::Iterator::tag invalid processing instruction ending");}
					assure_next_char(true);
					return curr_ = Element::end;
				}
				throw exception("xml::Iterator::tag invalid ending");
			}
			key_ += static_cast<char>(char_);
			assure_next_char(false);
		}
		if (is_whitespace(char_)) {
			assure_next_char(true);
		}

		if (char_ == '=' && !char_was_escaped_) {
			assure_next_char(true);
			if ((char_ != '"') && (char_ != '\'')) {
				throw exception("xml::Iterator::tag invalid string");
			}
			wchar_t del = char_;
			assure_next_char(false);
			while (char_ != del) {
				value_ += static_cast<char>(char_);
				assure_next_char(false);
			}
			assure_next_char(true);
		}
		return curr_ = Element::attribute;
	}
	catch (exception& ex){
	}
	return curr_ = Element::error;
}
//
//Element::Enum xml::Iterator::value(std::istream& in)
//{
//	try {
//	}
//	catch (exception& ex){
//	}
//	return Element::error;
//}

Element::Enum Iterator::scan_pi()
{
	try {
		if ((curr_ != Element::content) && (curr_ != Element::undefined)) {
			throw exception("xml::Iterator::pi invalid tag position");
		}
		tag_.clear();
		key_.clear();
		value_.clear();
		while (!is_whitespace(char_)) {
			if (char_ == '?' && !char_was_escaped_) {
				assure_next_char(false);
				if (char_ == '>' && !char_was_escaped_) {
					get_next_char(false);
					return curr_ = Element::processing_instruction;
				}
			}
			tag_ += static_cast<char>(char_);
			assure_next_char(false);
		}
		assure_next_char(true);
		return curr_ = Element::processing_instruction;
	}
	catch (exception& ex){
	}
	return curr_ = Element::error;
}

Element::Enum Iterator::scan_comment()
{
	try {
		value_.clear();
		for (;;) {
			assure_next_char(false);
			if (char_was_escaped_ || (char_ != '-')) {
				value_ += static_cast<char>(char_);
			}
			else {
				assure_next_char(false);
				if (char_was_escaped_ || (char_ != '-')) {
					value_ += '-';
					value_ += static_cast<char>(char_);
				}
				else {
					assure_next_char(false);
					if (char_was_escaped_ || (char_ != '>')) {
						value_ += "--";
						value_ += static_cast<char>(char_);
					}
					else {
						get_next_char(false);
						return curr_ = Element::comment;
					}
				}
			}
		}
	}
	catch (exception& ex){
	}
	return curr_ = Element::error;
}

Element::Enum Iterator::next_content()
{
  prev_ = curr_;
	try {
    if ((char_ == '<') && !char_was_escaped_) {
			assure_next_char(false);
      return scan_tag();
    }
		value_.clear();
		value_ += static_cast<char>(char_);
		for (;;) {
			assure_next_char(false);
      if ((char_ == '<') && !char_was_escaped_) {
        return curr_ = Element::content;
      }
  		value_ += static_cast<char>(char_);
    }
	}
	catch (exception& ex){
	}
	return curr_ = Element::error;
}

Element::Enum Iterator::next()
{
	try {
		while (curr_ < Element::error) {
			if (!char_was_escaped_) {
				if (char_ == '<') {
					//if ((curr_ == Element::content) || (curr_ == Element::tag)) {
					//	if ((prev_ != Element::content) && !value_.empty()) {
					//		return prev_ = curr_ = Element::content;
					//	}
					//}
					assure_next_char(false);
					if (!char_was_escaped_) {
						if (char_ == '/') {
							assure_next_char(false);
							return scan_end();
						}
						if (char_ == '?') {
							assure_next_char(false);
							return scan_pi();
						}
						if (char_ == '!') {
							assure_next_char(false);
							if (char_was_escaped_ || (char_ != '-')) {throw exception("xml::Iterator::next invalid comment");}
							assure_next_char(false);
							if (char_was_escaped_ || (char_ != '-')) {throw exception("xml::Iterator::next invalid comment");}
							assure_next_char(false);
							return scan_comment();
						}
						return scan_tag();
					}
				}
				else if (char_ == '/') {
					if (curr_ == Element::attribute) {
						assure_next_char(false);
						if (char_was_escaped_ || (char_ != '>')) {throw exception("xml::Iterator::next invalid comment");}
						assure_next_char(false);
						return curr_ = Element::empty;
					}
				}
			}
			switch (curr_) {
			case Element::tag:
			case Element::attribute:
			case Element::processing_instruction:
				return scan_attribute();
			default:
				value_ += static_cast<char>(char_);
			}
			assure_next_char(false);
		}
		return curr_ = Element::eof;
	}
	catch (exception& ex){
	}
	return curr_ = Element::error;
}

Element::Enum Iterator::next_tag()
{
	Element::Enum elem;
again:
	elem = next();
	switch (elem) {
	case Element::comment:
	case Element::content:
		goto again;
	default:;
	}
	return elem;
}

//Element::Enum RefObj::next_xml(std::istream& in, std::string& tag, std::string& key, std::string& value, Element::Enum prev)
//{
//	char ch;
//	switch (prev) {
//	default:;
//		while(!in.eof()) {
//			ch >> in;
//		}
//	}
//}


void Iterator::read(std::vector<long>& data)
{
  std::stringstream ss(value_);
  long long val;
  size_t limit;
  ss >> limit;
  data.clear();
  data.reserve(limit);
  for (size_t i=0; i<limit; ++i) {
    ss >> std::hex >> val;
    data.push_back(static_cast<long>(val));
  }
}

//*************************************************************************************

Writer::Writer(std::ostream& out)
: out_(out)
, indent_(0)
, print_pretty_(true)
{
}

bool Writer::visited(const std::string& id)
{
  std::map<std::string, int>::iterator it = visit_map_.find(id);
  if (it == visit_map_.end()) {
    visit_map_[id] = 1;
    return false;
  }
  ++it->second;
  return true;
}

void Writer::hex()
{
	out_ << std::hex;
}

void Writer::dec()
{
	out_ << std::dec;
}

void Writer::width(size_t w)
{
	out_ << std::setw(w);
}

void Writer::newline(int delta)
{
	if (print_pretty_) {out_ << "\n";}
	indent(delta);
	if (print_pretty_) {
		for (int i=0; i<indent_; ++i) {
			out_ << "\t";
		}
	}
}

void Writer::indent(int delta)
{
	indent_ += delta;
	if (indent_ < 0) {indent_ = 0;}
}

void Writer::write(const std::vector<long>& data)
{
  size_t limit = data.size();
  out_ << limit << ' ';
  for (size_t i=0; i<limit; ++i) {
    out_ << std::hex << data[i] << ' ';
  }
  out_ << std::dec;
}

void Writer::write(const std::string& txt)
{
	size_t limit = txt.size();
	unsigned int ch;
	for (size_t i=0; i<limit; ++i) {
		ch = static_cast<unsigned char>(txt.at(i));
		if (ch <= 0x1f) {
			switch (ch) {
			case 0x09:
			case 0x0a:
			case 0x0d:
				out_ << static_cast<char>(ch);
				break;
			default:throw exception("xml::Writer::write invalid character");
			}
		}
		else if (ch <= 0x7f) {
			switch (ch) {
			case '&': out_ << "&amp;"; break;
			case '<': out_ << "&lt;"; break;
			case '>': out_ << "&gt;"; break;
			case '"': out_ << "&quot;"; break;
			case '\'': out_ << "&apos;"; break;
			default: out_ << static_cast<char>(ch);
			}
		}
		else if (ch <= 0x7ff) {
			out_ << static_cast<char>((ch >> 6) | 0xc0);
			out_ << static_cast<char>((ch & 0x3f) | 0x80);
		}
		else if (ch <= 0xffff) {
			out_ << static_cast<char>((ch >> 12) | 0xe0);
			out_ << static_cast<char>(((ch >> 6) & 0x3f) | 0x80);
			out_ << static_cast<char>((ch & 0x3f) | 0x80);
		}
		else if (ch <= 0x10ffff) {
			out_ << static_cast<char>((ch >> 18) | 0xe0);
			out_ << static_cast<char>(((ch >> 12) & 0x3f) | 0x80);
			out_ << static_cast<char>(((ch >> 6) & 0x3f) | 0x80);
			out_ << static_cast<char>((ch & 0x3f) | 0x80);
		}
		else {
			throw exception("xml::Writer::write invalid character");
		}
	}
}

void Writer::open(const std::string& tag, bool is_ending, bool close)
{
  out_ << '<';
  if (is_ending) {
    out_ << '/';
  }
  out_ << tag;
  if (close) {
    out_ << '>';
  }
}

void Writer::close(bool is_ending)
{
  if (is_ending) {
    out_ << '/';
  }
  out_ << '>';
}

void Writer::tag(const std::string& tag, const std::string& value)
{
  if (value.empty()) {
    open(tag, false);
    close(true);
  }
  else {
    open(tag, false, true);
    write(value);
    open(tag, true, true);
  }
}

void Writer::attribute(const std::string& name, const std::string& value)
{
  out_ << ' ' << name << "=\"" << value << '"';
}


}
