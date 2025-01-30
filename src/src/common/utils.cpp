#include "utils.hpp"
#include "f18a.hpp"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>

namespace utils {

void append_hex(
  long val,
  size_t digits,
  std::string& res
)
{
  std::stringstream ss;
  ss << std::setfill ('0') << std::setw(digits)  << std::hex << val;
  res += ss.str();
}

void append_cell(long val, std::string& txt)
{
  size_t base = txt.size();
  append_hex(val, 5, txt);
  txt.push_back(' ');
  long dval = val;
  if (((F18A_CELL_SIZE/2) & val) != 0) {
    dval |= ~F18A_CELL_MASK;
  }
  std::string tmp = boost::lexical_cast<std::string>(dval);
  txt.resize(base+14-tmp.size(), ' ');
  txt += tmp;
}


// ****************************************************************************

Recycler::~Recycler()
{
}

void Recycler::recycle(RefCntObj* obj)
{
  delete obj;
}


// ****************************************************************************

RefCntObj::~RefCntObj()
{
}

void RefCntObj::swap(RefCntObj& obj)
{
  Recycler* r = recycler_;
  recycler_ = obj.recycler_;
  obj.recycler_ = r;
}

void RefCntObj::inc_ref() const
{
  ++ref_cnt_;
}

void RefCntObj::dec_ref() const
{
  if (ref_cnt_ > 0) {
    if (--ref_cnt_ == 0) {
      if (recycler_) {
        recycler_->recycle(const_cast<RefCntObj*>(this));
      }
      else {
        delete this;
      }
    }
  }
}

bool RefCntObj::dec_ref_no_delete() const
{
  if (ref_cnt_ > 0) {
    if (--ref_cnt_ == 0) {
      return true;
    }
  }
  return false;
}

// ****************************************************************************

RefCntObjPtr::RefCntObjPtr(RefCntObj* p)
: ptr_(p)
{
  if (ptr_) {ptr_->inc_ref();}
}

RefCntObjPtr::RefCntObjPtr(const RefCntObjPtr& n)
: ptr_(n.ptr_)
{
  if (ptr_) {ptr_->inc_ref();}
}

RefCntObjPtr& RefCntObjPtr::operator=(RefCntObj* p)
{
  if (p) {p->inc_ref();}
  if (ptr_) {ptr_->dec_ref();}
  ptr_ = p;
  return *this;
}

RefCntObjPtr& RefCntObjPtr::operator=(const RefCntObjPtr& n)
{
  if (n.ptr_) {n.ptr_->inc_ref();}
  if (ptr_) {ptr_->dec_ref();}
  ptr_ = n.ptr_;
  return *this;
}

RefCntObjPtr::~RefCntObjPtr()
{
  if (ptr_) {
    ptr_->dec_ref();
    ptr_ = 0;
  }
}

void RefCntObjPtr::clear()
{
  if (ptr_) {
    ptr_->dec_ref();
    ptr_ = 0;
  }
}

void RefCntObjPtr::swap(RefCntObjPtr& n)
{
// boost::shared_ptr<int> a;

  RefCntObj* p = ptr_;
  ptr_ = n.ptr_;
  n.ptr_ = p;
}

// ****************************************************************************


}
