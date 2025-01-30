#include <evb002.hpp>
#include <connection.hpp>

#include <gtkmm.h>

/**

pf 2281 contains code for async writing back data.

*/

namespace evb {



// ======================================================================================

Board002::Board002()
{


}

Board002::~Board002()
{
}

std::string Board002::picture_file() const
{
  return "pict/evb002_board.png";
}

std::string Board002::version() const
{
  return "002";
}


}
