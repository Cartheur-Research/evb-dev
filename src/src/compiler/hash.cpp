#include "cvmc.hpp"

namespace cvmc {

int HashFinder::highestBit(unsigned long long val)
{
  int res = 0;
  while (val != 0) {
    ++res;
    val >>= 1;
  }
  return res;
}

HashFinder::HashFinder()
: min_(0)
, max_(0)
, a_(0)
, b_(0)
, c_(0)
, d_(0)
{
}

void HashFinder::clear()
{
  for (map_type::iterator it = values_.begin(); it != values_.end(); ++it) {
    it->second = -1;
  }
}

void HashFinder::add(long long value)
{
  map_type::iterator it = values_.find(value);
  if (it != values_.end()) {
		throw analyzer_exception("HashFinder::add duplicate value");
  }
  if (values_.empty()) {
    min_ = max_ = value;
  }
  else {
    if (min_ > value) {min_ = value;}
    else if (max_ < value) {max_ = value;}
  }
  values_[value] = -1;
}

bool HashFinder::find()
{
  a_ = b_ = c_ = d_= 0;
  unsigned long long range = static_cast<unsigned long long>(max_ - min_);
  int bits = HashFinder::highestBit(values_.size());
  if (bits > 10) { // table too big
    return false;
  }
  unsigned short mask = (1 << bits) - 1;
  unsigned short max_range = range & mask;
  map_type map;
  d_ = mask;
  size_t cnt, limit = values_.size();
  for (int lsh=0; lsh<bits; ++lsh) {
    a_ = lsh;
    for (int rsh=0; rsh<bits; ++rsh) {
      b_ = rsh;
      for (int off=0; off<max_range; ++off) {
        c_ = off;
        map.clear();
        cnt = 0;
        for (map_type::iterator it=values_.begin(); it != values_.end(); ++it) {
          int pos = evaluate(static_cast<unsigned short>(it->first - min_));
          if (map.find(pos) != map.end()) {
            break;
          }
          ++cnt;
          map[pos] = 1;
        }
        if (cnt >= limit) {
          return true;
        }
      }
    }
  }
  ++bits;
  mask = (1 << bits) - 1;
  max_range = range & mask;
  d_ = mask;
  for (int lsh=0; lsh<bits; ++lsh) {
    a_ = lsh;
    for (int rsh=0; rsh<bits; ++rsh) {
      b_ = rsh;
      for (int off=0; off<max_range; ++off) {
        c_ = off;
        map.clear();
        cnt = 0;
        for (map_type::iterator it=values_.begin(); it != values_.end(); ++it) {
          int pos = evaluate(static_cast<unsigned short>(it->first - min_));
          if (map.find(pos) != map.end()) {
            break;
          }
          ++cnt;
          map[pos] = 1;
        }
        if (cnt >= limit) {
          return true;
        }
      }
    }
  }
  return false;
}

int HashFinder::evaluate(unsigned short value)
{
  return static_cast<int>(((value << a_) + (value >> b_) + c_) & d_);
}

int HashFinder::compile(CompilationUnit& cu)
{
  return 0;
}

//=====================================================================

UnsignedHashFinder::UnsignedHashFinder()
: min_(0)
, max_(0)
, a_(0)
, b_(0)
, c_(0)
, d_(0)
{
}

void UnsignedHashFinder::add(unsigned long long value)
{
  map_type::iterator it = values_.find(value);
  if (it != values_.end()) {
		throw analyzer_exception("HashFinder::add duplicate value");
  }
  if (values_.empty()) {
    min_ = max_ = value;
  }
  else {
    if (min_ > value) {min_ = value;}
    else if (max_ < value) {max_ = value;}
  }
  values_[value] = -1;
}


}
