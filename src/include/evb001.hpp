#ifndef EVB001_HPP
#define EVB001_HPP

#include <evb.hpp>

namespace evb {



class Board001 : public evb::Board {
public:

private:


  Board001(const Board001&);
  Board001& operator=(const Board001&);


public:
  Board001();
  ~Board001();

  virtual std::string picture_file() const;
  virtual std::string version() const;

};

}

#endif
