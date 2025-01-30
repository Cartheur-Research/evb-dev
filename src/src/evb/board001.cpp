#include <evb001.hpp>
#include <connection.hpp>

#include <gtkmm.h>

/**

pf 2281 contains code for async writing back data.

*/

namespace evb {



// ======================================================================================

Board001::Board001()
{


}

Board001::~Board001()
{
}

std::string Board001::picture_file() const
{
  return "pict/evb001_board.png";
}

std::string Board001::version() const
{
  return "001";
}


}
