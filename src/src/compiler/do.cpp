#include "cvmc.hpp"

namespace cvmc {

//Do::Do()
//: op_(undefined)
//{
//}
//
//void Do::add_tag(Context* c, const std::string& n, Type* t)
//{
//	op_ = Operation::op_add_tag;
//	context_ = c;
//	name_ = n;
//	type_ = t;
//}
//
//void Do::add_enumerator(Context* c, const std::string& n, Constant* v)
//{
//	op_ = Operation::op_add_enumerator;
//	context_ = c;
//	name_ = n;
//	value_ = v;
//}
//
//void Do::perform()
//{
//	switch (op_) {
//	case Operation::op_add_tag: static_cast<Context*>(context_.get())->add_tag(name_, static_cast<Type*>(type_.get())); break;
//	case Operation::op_add_enumerator: {
//			Context* cntxt = static_cast<Context*>(context_.get());
//			Constant* c = static_cast<Constant*>(value_.get());
//			cntxt->add_enumerator(name_, c);
//		}
//		break;
//	}
//}

}
