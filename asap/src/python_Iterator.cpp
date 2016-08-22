#include <boost/python.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

#include "STIdxIter.h"

using namespace boost::python;

namespace asap {
  namespace python {

    void python_Iterator() {
      class_<STIdxIter2>("iterator")
        .def( init < const string &, const vector<string> & > () )
#ifdef HAVE_LIBPYRAP
        .def("current", &STIdxIter2::currentValue)
#endif
        .def("pastEnd", &STIdxIter2::pastEnd)
        .def("next", &STIdxIter2::next)
        .def("getrows", &STIdxIter2::getRowsSTL)
      ;
    };

  } //namespace python
} // namespace asap
