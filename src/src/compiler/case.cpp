#include "cvmc.hpp"

namespace cvmc {


/*

switch(expr) {
  case c1: [stmt1]
  case c2: [stmt2]
  case c3: [stmt3]
  case c4: [stmt4]
  ..
  case cn: [stmtn]
  default: [stmt]
  case cn+1: [stmtn+1]
  ..
  case cm: [stmtm]
};

sequential case (example):

switch(expr) {
  case 5: stmt1
  case 9: stmt2
  case 7: stmt3
  default: stmtd
  case 8: stmt4
  case 4: stmt5;
}

call ^switch_int(expr, 4, 9-4+1)
tjmp
d:  offset to stmtd
4:  offset to stmt5
5:  offset to stmt1
6:  offset to stmtd7
7:  offset to stmt3
8:  offset to stmt4
9:  offset to stmt2
stmt1
stmt2
stmt3
stmtd
stmt4
stmt5
<-- break position









*/

//*************************************************************************************

SwitchStmt::SwitchStmt(const context_type::position_type& w)
: Iteration(w)
, default_(-1)
, is_compressed_(false)
{
	init();
}

bool SwitchStmt::is_breakable() const
{
	return true;
}

void SwitchStmt::add(CaseStmt* stm)
{
	case_list_.push_back(stm);
}

void SwitchStmt::deflt(CaseStmt* stm)
{
	default_ = case_list_.size();
	add(stm);
}

void SwitchStmt::optimize(ObjPtr& ptr, int mask)
{
	if (case_list_.empty()) {
		if (value()->has_side_effect()) {
			ptr = value_;
		}
		else {
			ptr.clear();
		}
	}
	else {
		int i, limit = case_list_.size();
		for (i=0; i<limit; ++i) {
			if (i != default_) {
				case_list_[i].optimize(mask);
			}
		}
	}
}


void SwitchStmt::finish(Function* f)
{
	int min_pos, max_pos, i, limit = case_list_.size();
	//ObjPtr_list_type jump_list;
	unsigned long long umin = MAX_ULONGLONG;
	unsigned long long umax = 0;
	long long min = MAX_LONGLONG;
	long long max = MIN_LONGLONG;
	bool is_unsigned = value()->type()->is_unsigned();
	size_t size = value()->type_size();
	ObjPtr obj;
	Call* c;
	if (is_unsigned) {
    UnsignedHashFinder finder;
		is_unsigned = true;
		min_pos = max_pos = -1;
		for (i=0; i<limit; ++i) {
			CaseStmt* cstmt = static_cast<CaseStmt*>(case_list_[i].get());
			if (i != default_) {
				if (!cstmt->value()->is_const()) {
					throw analyzer_exception("SwitchStmt::finish case is not a constant");
				}
				unsigned long long val;
				if (!cstmt->value()->get_ulonglong(val)) {
					throw analyzer_exception("SwitchStmt::finish case is not an integer constant");
				}
        finder.add(val);
				if (val < umin) {umin = val; min_pos = i;}
				if (val > umax) {umax = val; max_pos = i;}
				std::map<unsigned long long, ObjPtr>::iterator it = umap_.find(val);
				if (it == umap_.end()) {
					umap_[val] = cstmt;
				}
				else {
					throw analyzer_exception("SwitchStmt::finish duplicated case constant");
				}
			}
		}
		if ((umax - umin) > 0x1000) {
			throw analyzer_exception("SwitchStmt::finish switch table exceeds limit of 4096");
		}
		//table_limit = static_cast<size_t>(umax - umin) + 1;
	}
	else {
    HashFinder finder;
		min_pos = max_pos = -1;
		for (i=0; i<limit; ++i) {
			CaseStmt* cstmt = static_cast<CaseStmt*>(case_list_[i].get());
			if (i != default_) {
				if (!cstmt->value()->is_const()) {
					throw analyzer_exception("SwitchStmt::finish case is not a constant");
				}
				long long val;
				if (!cstmt->value()->get_longlong(val)) {
					throw analyzer_exception("SwitchStmt::finish case is not an integer constant");
				}
        finder.add(val);
				if (val < min) {min = val; min_pos = i;}
				if (val > max) {max = val; max_pos = i;}
				std::map<long long, ObjPtr>::iterator it = map_.find(val);
				if (it == map_.end()) {
					map_[val] = cstmt;
				}
				else {
					throw analyzer_exception("SwitchStmt::finish duplicated case constant");
				}
			}
		}
		if ((max - min) > 0x1000) {
			throw analyzer_exception("SwitchStmt::finish switch table exceeds limit of 4096");
		}
		limit_ = static_cast<size_t>(max - min) + 1;

		switch (size) {
		case 1:
			if ((limit_ > 10) && (limit < (limit_/2))) {
				// use compressed tabel
        if (finder.find()) {
        }
				c = new Call("^compressed_switch_int");
				c->add(value());
				c->add(new IntConst(limit));
				call_ = c;
				is_compressed_ = true;
			}
			else {
				// use jump table
				min_ = static_cast<int>(min);
				c = new Call("^switch_int");
				c->add(new BinaryExpr(BIN_ADD, value(), new IntConst(-min_)));
				c->add(new IntConst(limit_));
				call_ = c;
			}
			break;

		default: throw not_implemented_exception("SwitchStmt::finish invalid size");
		}

	}
}

int SwitchStmt::compile(CompilationUnit& cu, bool push_result)
{
	bool is_unsigned = value()->type()->is_unsigned();
	size_t size = value()->type_size();
	size_t i, pos;
	CaseJump* default_jump = 0;
	ObjPtr obj;
	if (default_ >= 0) {
		CaseStmt* cstmt = static_cast<CaseStmt*>(case_list_[default_].get());
		default_jump = cstmt->jump();
	}
	if (default_jump == 0) {
		default_jump = new CaseJump(location());
		obj = default_jump;
		default_jump->destination(brk());
	}
	Call* c = static_cast<Call*>(call_.get());
	c->compile(cu, false);
	if (size == 1) {
		if (is_compressed_) {
			default_jump->compile(cu, false);
			if (!map_.empty()) {
				for (std::map<long long, ObjPtr>::iterator it = map_.begin(); it != map_.end(); ++it) {
					cu.code().append_data(static_cast<codepoint_type>(it->first));
					CaseStmt* cstmt = static_cast<CaseStmt*>(it->second.get());
					cstmt->jump()->compile(cu, false);
				}
			}
			else if (!umap_.empty()) {
				for (std::map<unsigned long long, ObjPtr>::iterator it = umap_.begin(); it != umap_.end(); ++it) {
					cu.code().append_data(static_cast<codepoint_type>(it->first));
					CaseStmt* cstmt = static_cast<CaseStmt*>(it->second.get());
					cstmt->jump()->compile(cu, false);
				}
			}
		}
		else {
			cu.code().compile_tjmp();
			pos = 0;
			if (!map_.empty()) {
				for (std::map<long long, ObjPtr>::iterator it = map_.begin(); it != map_.end(); ++it) {
					i = static_cast<int>(it->first - min_);
					while (pos < i) {
						default_jump->compile(cu, false);
						++pos;
					}
					CaseStmt* cstmt = static_cast<CaseStmt*>(it->second.get());
					cstmt->jump()->compile(cu, false);
					++pos;
				}
			}
			else if (!umap_.empty()) {
				for (std::map<unsigned long long, ObjPtr>::iterator it = umap_.begin(); it != umap_.end(); ++it) {
					i = static_cast<int>(it->first - min_);
					while (pos < i) {
						default_jump->compile(cu, false);
						++pos;
					}
					CaseStmt* cstmt = static_cast<CaseStmt*>(it->second.get());
					cstmt->jump()->compile(cu, false);
					++pos;
				}
			}
			default_jump->compile(cu, false);
			cu.code().append(0);
		}
	}
	return 0;
}


//*************************************************************************************

CaseStmt::CaseStmt(const context_type::position_type& w, SwitchStmt* s)
: Statement(w)
, switch_(s)
{
	jump(new CaseJump(w));
	label_ = new Label(w);
	jump()->destination(label());
}

void CaseStmt::optimize(ObjPtr&, int mask)
{
	value_.optimize(mask);
}

int CaseStmt::compile(CompilationUnit& cu, bool push_result)
{
	label_.compile(cu, push_result);
	return 0;
}

int CaseStmt::compile_table_entry(CompilationUnit& cu)
{
	return 0;
}


//*************************************************************************************


}
