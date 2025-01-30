#ifndef NODETYPE_HPP
#define NODETYPE_HPP

#include <utils.hpp>
#include <gtkmm.h>

class NodeType : public utils::RefCntObj {
public:
  NodeType();
  virtual ~NodeType();

  virtual void draw(
    const Cairo::RefPtr<Cairo::Context>& cairo,
    const Glib::RefPtr<Pango::Layout>& pango,
    Pango::FontDescription& fdesc,
    double x,
    double y,
    double w,
    double h
  );

};
typedef utils::RefCntPtr<NodeType> NodeType_ptr;


#endif
