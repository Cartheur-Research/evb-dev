#ifndef UTILS_HPP
#define UTILS_HPP

#ifdef _WIN32
#pragma warning (disable: 4251) // http://msdn2.microsoft.com/en-gb/library/esew7y1w.aspx
#pragma warning (disable: 4996)
#endif 

#include <vector>
#include <map>
#include <string>

namespace utils {

void append_hex(
  long val,
  size_t digits,
  std::string&
);

void append_cell(
  long val,
  std::string&
);

class RefCntObj;

class Recycler {
public:
  Recycler() {}
  virtual ~Recycler();
  virtual void recycle(RefCntObj*);
};

class RefCntObj {
protected:
  mutable int   ref_cnt_;
  Recycler*     recycler_;

public:
  RefCntObj() : ref_cnt_(0), recycler_(0) {}
  RefCntObj(Recycler* r) : ref_cnt_(0), recycler_(r) {}
  virtual ~RefCntObj();

  int get_ref_cnt() const {return ref_cnt_;}
  void inc_ref() const;
  void dec_ref() const;
  bool dec_ref_no_delete() const; // return true if reference counter reaches 0 and object should be deleted

  Recycler* recycler() {return recycler_;}
  const Recycler* recycler() const {return recycler_;}
  void recycler(Recycler* value) {recycler_ = value;}

  void swap(RefCntObj&);

};

class RefCntObjPtr {
private:
  RefCntObj* ptr_;

public:
  RefCntObjPtr() : ptr_(0) {}
  RefCntObjPtr(RefCntObj*);
  RefCntObjPtr(const RefCntObjPtr&);
  RefCntObjPtr& operator=(RefCntObj*);
  RefCntObjPtr& operator=(const RefCntObjPtr&);
  ~RefCntObjPtr();

  void clear();
  void swap(RefCntObjPtr&);

  bool operator==(const RefCntObjPtr& n) const {return ptr_ == n.ptr_;}
  bool operator!=(const RefCntObjPtr& n) const {return ptr_ != n.ptr_;}

  bool is_null() const {return ptr_ == 0;}
  bool is_not_null() const {return ptr_ != 0;}
  RefCntObj* get() {return ptr_;}
  const RefCntObj* get() const {return ptr_;}
  RefCntObj* operator*() {return ptr_;}
  const RefCntObj* operator*() const {return ptr_;}
  RefCntObj* operator->() {return ptr_;}
  const RefCntObj* operator->() const {return ptr_;}
};
typedef std::vector<RefCntObjPtr> RefCntObjPtrList_type;
typedef std::map<std::string, RefCntObjPtr> RefCntObjPtrMap_type;

template <typename T>
class RefCntPtr {
private:
  T* ptr_;

public:
  RefCntPtr() : ptr_(0) {}
  RefCntPtr(T* p): ptr_(p) {if (ptr_) {ptr_->inc_ref();}}
  RefCntPtr(const RefCntPtr& n): ptr_(n.ptr_) {if (ptr_) {ptr_->inc_ref();}}

  RefCntPtr& operator=(T* p) {
    if (p) {p->inc_ref();}
    if (ptr_) {ptr_->dec_ref();}
    ptr_ = p;
    return *this;
  }

  RefCntPtr& operator=(const RefCntPtr& n){
    if (n.ptr_) {n.ptr_->inc_ref();}
    if (ptr_) {ptr_->dec_ref();}
    ptr_ = n.ptr_;
    return *this;
  }

  ~RefCntPtr() {if (ptr_) {ptr_->dec_ref(); ptr_ = 0;}}

  void reset() {if (ptr_) {ptr_->dec_ref(); ptr_ = 0;}}
  void swap(RefCntPtr& n) {T* p = ptr_; ptr_ = n.ptr_; n.ptr_ = p;}

  bool operator==(const RefCntPtr& n) const {return ptr_ == n.ptr_;}
  bool operator!=(const RefCntPtr& n) const {return ptr_ != n.ptr_;}

  bool is_null() const {return ptr_ == 0;}
  bool is_not_null() const {return ptr_ != 0;}
  T* get() {return ptr_;}
  const T* get() const {return ptr_;}
  T* operator*() {return ptr_;}
  const T* operator*() const {return ptr_;}
  T* operator->() {return ptr_;}
  const T* operator->() const {return ptr_;}
};

}

#endif
