#include <boost/python.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

#include "STIdxIter.h"

using namespace boost::python;

namespace asap {
  namespace python {

    void python_Iterator() {
      class_<STIdxIterAcc>("iterator")
        .def( init < const string &, const vector<string> & > () )
        .def("current", &STIdxIterAcc::currentSTL)
        .def("pastEnd", &STIdxIterAcc::pastEnd)
        .def("next", &STIdxIterAcc::next)
        .def("getrows", &STIdxIterAcc::getRowsSTL)
      ;
    };

  } //namespace python
} // namespace asap
