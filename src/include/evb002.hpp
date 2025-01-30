#ifndef EVB002_HPP
#define EVB002_HPP

#include <evb.hpp>

namespace evb {



class Board002 : public evb::Board {
public:

private:


  Board002(const Board002&);
  Board002& operator=(const Board002&);


public:
  Board002();
  ~Board002();

  virtual std::string picture_file() const;
  virtual std::string version() const;

};

}

#endif
