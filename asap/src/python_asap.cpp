//#---------------------------------------------------------------------------
//# python_asap.cc: python module for single dish package asap
//#---------------------------------------------------------------------------
//# Copyright (C) 2004-2012
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id: python_asap.cpp 2921 2014-04-07 00:28:37Z TakeshiNakazato $
//#---------------------------------------------------------------------------

#include <boost/python.hpp>
#include <boost/python/exception_translator.hpp>

#include <string>
#include <vector>

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include "ScantableWrapper.h"

#ifndef HAVE_LIBPYRAP
  #include "pyconversions.h"
#else
  #include <pyrap/Converters/PycExcp.h>
  #include <pyrap/Converters/PycBasicData.h>
  #include <pyrap/Converters/PycValueHolder.h>
  #include <pyrap/Converters/PycRecord.h>
#endif

#include "python_asap.h"

#ifndef HAVE_LIBPYRAP
namespace asap {
  namespace python {

void translate_ex(const casa::AipsError& e)
{
  // Use the Python 'C' API to set up an exception object
  PyErr_SetString(PyExc_RuntimeError, e.what());
}

  }
}
#endif

using namespace boost::python;

BOOST_PYTHON_MODULE(_asap) {
#ifdef ENABLE_PLOTTER2
  asap::python::python_Plotter2();
#endif //ENABLE_PLOTTER2
  asap::python::python_Scantable();
  asap::python::python_STFiller();
  asap::python::python_Filler();
  asap::python::python_MSFiller();
  asap::python::python_STSelector();
  asap::python::python_STMath();
  asap::python::python_Fitter();
  asap::python::python_STLineFinder();
  asap::python::python_STFitEntry();
  asap::python::python_STWriter();
  asap::python::python_MSWriter();
  asap::python::python_LineCatalog();
  asap::python::python_LogSink();
  asap::python::python_STCoordinate();
  asap::python::python_STAtmosphere();
  asap::python::python_SrcType();
  asap::python::python_STGrid();
  asap::python::python_Iterator();
  asap::python::python_EdgeMarker();
  asap::python::python_PlotHelper();
  asap::python::python_STSideBandSep();
  asap::python::python_CalibrationManager();

#ifndef HAVE_LIBPYRAP
  // Use built-in pyconversions.h
  register_exception_translator<casa::AipsError>(&asap::python::translate_ex);
  from_python_sequence < std::vector< asap::ScantableWrapper >,
    variable_capacity_policy > ();
  std_vector_to_tuple < int > ();
  from_python_sequence < std::vector < int >,
    variable_capacity_policy > ();
  std_vector_to_tuple < uint > ();
  from_python_sequence < std::vector < uint >,
    variable_capacity_policy > ();
  std_vector_to_tuple < float > ();
  from_python_sequence < std::vector < float >,
    variable_capacity_policy > ();
  std_vector_to_tuple < double > ();
  from_python_sequence < std::vector < double >,
    variable_capacity_policy > ();
  std_vector_to_tuple < std::string > ();
  from_python_sequence < std::vector < std::string >,
    variable_capacity_policy > ();
  std_vector_to_tuple < bool> ();
  from_python_sequence < std::vector < bool >,
    variable_capacity_policy > ();
#else
  casa::pyrap::register_convert_excp();
  casa::pyrap::register_convert_basicdata();
  casa::pyrap::register_convert_std_vector<asap::ScantableWrapper>();
  casa::pyrap::register_convert_std_vector<int>();
  casa::pyrap::register_convert_std_vector<uint>();
  casa::pyrap::register_convert_std_vector<float>();
  casa::pyrap::register_convert_std_vector<double>();
  casa::pyrap::register_convert_std_vector<std::string>();
//////////////see error below//////////////////////////////
//casa::pyrap::register_convert_std_vector<bool>();
///////////////////////////////////////////////////////////
  casa::pyrap::register_convert_casa_valueholder();
  casa::pyrap::register_convert_casa_record();

#endif
}
//[ 79%] Building CXX object src/CMakeFiles/_asap.dir/python_asap.cpp.o
//cd ...trunk/asap/build/src && /usr/bin/clang++   -DAIPS_AUTO_STL -DAIPS_DEBUG -DAIPS_STDLIB -DCASACORE_NEEDS_RETHROW -DCASA_USECASAPATH -DENABLE_PLOTTER2 -DHAVE_LIBPYRAP -DNOPKSMS -DUSE_CASAPY -DWITHOUT_ACS -D_GNU_SOURCE -D_asap_EXPORTS -pipe -Wall -Wextra -Wno-non-template-friend -Wcast-align -Wno-comment -ggdb3 -O0 -fno-omit-frame-pointer -fPIC -I...trunk/darwin/include/casacore -I...trunk/darwin/include/casacore/.. -I/opt/casa/01/include -I/opt/casa/01/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 -I/opt/casa/01/Library/Frameworks/Python.framework/Versions/2.7/include -I/opt/casa/01/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/numpy/core/include -I...trunk/asap/src -I...trunk/asap/external-alma -I...trunk/asap/external/libpyrap/pyrap-0.3.2    -o CMakeFiles/_asap.dir/python_asap.cpp.o -c ...trunk/asap/src/python_asap.cpp
//warning: unknown warning option '-Wno-non-template-friend'; did you mean '-Wno-unsupported-friend'? [-Wunknown-warning-option]
//In file included from ...trunk/asap/src/python_asap.cpp:32:
//In file included from /opt/casa/01/include/boost/python.hpp:29:
//In file included from /opt/casa/01/include/boost/python/exec.hpp:9:
///opt/casa/01/include/boost/python/str.hpp:185:57: warning: unused parameter 'end' [-Wunused-parameter]
//    long count(T1 const& sub,T2 const& start, T3 const& end) const
//                                                        ^
//In file included from ...trunk/asap/src/python_asap.cpp:32:
//In file included from /opt/casa/01/include/boost/python.hpp:49:
///opt/casa/01/include/boost/python/opaque_pointer_converter.hpp:172:1: warning: missing field 'tp_version_tag' initializer [-Wmissing-field-initializers]
//};
//^
//In file included from ...trunk/asap/src/python_asap.cpp:32:
//In file included from /opt/casa/01/include/boost/python.hpp:52:
//In file included from /opt/casa/01/include/boost/python/overloads.hpp:11:
///opt/casa/01/include/boost/python/detail/defaults_def.hpp:92:30: warning: unused parameter 'kw' [-Wunused-parameter]
//      , keyword_range const& kw // ignored
//                             ^
//In file included from ...trunk/asap/src/python_asap.cpp:46:
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:95:9: warning: cast from
//      'boost::python::converter::rvalue_from_python_stage1_data *' to 'boost::python::converter::rvalue_from_python_storage<String> *' increases required alignment
//      from 8 to 16 [-Wcast-align]
//        (boost::python::converter::rvalue_from_python_storage<String>*)
//        ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:424:9: warning: cast from
//      'boost::python::converter::rvalue_from_python_stage1_data *' to 'rvalue_from_python_storage<std::__1::vector<asap::ScantableWrapper,
//      std::__1::allocator<asap::ScantableWrapper> > > *' increases required alignment from 8 to 16 [-Wcast-align]
//        (rvalue_from_python_storage<ContainerType>*)
//        ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:367:10: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<asap::ScantableWrapper, std::__1::allocator<asap::ScantableWrapper> >,
//      casa::pyrap::stl_variable_capacity_policy>::construct' requested here
//        &construct,
//         ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:543:2: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<asap::ScantableWrapper, std::__1::allocator<asap::ScantableWrapper> >,
//      casa::pyrap::stl_variable_capacity_policy>::from_python_sequence' requested here
//        from_python_sequence < std::vector < T >,
//        ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:550:30: note: in instantiation of member function
//      'casa::pyrap::convert_std_vector<asap::ScantableWrapper>::reg' requested here
//    { convert_std_vector<T>::reg(); }
//                             ^
//...trunk/asap/src/python_asap.cpp:122:16: note: in instantiation of function template specialization
//      'casa::pyrap::register_convert_std_vector<asap::ScantableWrapper>' requested here
//  casa::pyrap::register_convert_std_vector<asap::ScantableWrapper>();
//               ^
//In file included from ...trunk/asap/src/python_asap.cpp:46:
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:424:9: warning: cast from
//      'boost::python::converter::rvalue_from_python_stage1_data *' to 'rvalue_from_python_storage<std::__1::vector<int, std::__1::allocator<int> > > *' increases
//      required alignment from 8 to 16 [-Wcast-align]
//        (rvalue_from_python_storage<ContainerType>*)
//        ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:367:10: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<int, std::__1::allocator<int> >, casa::pyrap::stl_variable_capacity_policy>::construct' requested here
//        &construct,
//         ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:543:2: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<int, std::__1::allocator<int> >, casa::pyrap::stl_variable_capacity_policy>::from_python_sequence'
//      requested here
//        from_python_sequence < std::vector < T >,
//        ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:550:30: note: in instantiation of member function
//      'casa::pyrap::convert_std_vector<int>::reg' requested here
//    { convert_std_vector<T>::reg(); }
//                             ^
//...trunk/asap/src/python_asap.cpp:123:16: note: in instantiation of function template specialization
//      'casa::pyrap::register_convert_std_vector<int>' requested here
//  casa::pyrap::register_convert_std_vector<int>();
//               ^
//In file included from ...trunk/asap/src/python_asap.cpp:46:
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:424:9: warning: cast from
//      'boost::python::converter::rvalue_from_python_stage1_data *' to 'rvalue_from_python_storage<std::__1::vector<unsigned int, std::__1::allocator<unsigned int> >
//      > *' increases required alignment from 8 to 16 [-Wcast-align]
//        (rvalue_from_python_storage<ContainerType>*)
//        ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:367:10: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<unsigned int, std::__1::allocator<unsigned int> >, casa::pyrap::stl_variable_capacity_policy>::construct'
//      requested here
//        &construct,
//         ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:543:2: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<unsigned int, std::__1::allocator<unsigned int> >,
//      casa::pyrap::stl_variable_capacity_policy>::from_python_sequence' requested here
//        from_python_sequence < std::vector < T >,
//        ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:550:30: note: in instantiation of member function
//      'casa::pyrap::convert_std_vector<unsigned int>::reg' requested here
//    { convert_std_vector<T>::reg(); }
//                             ^
//...trunk/asap/src/python_asap.cpp:124:16: note: in instantiation of function template specialization
//      'casa::pyrap::register_convert_std_vector<unsigned int>' requested here
//  casa::pyrap::register_convert_std_vector<uint>();
//               ^
//In file included from ...trunk/asap/src/python_asap.cpp:46:
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:424:9: warning: cast from
//      'boost::python::converter::rvalue_from_python_stage1_data *' to 'rvalue_from_python_storage<std::__1::vector<float, std::__1::allocator<float> > > *'
//      increases required alignment from 8 to 16 [-Wcast-align]
//        (rvalue_from_python_storage<ContainerType>*)
//        ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:367:10: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<float, std::__1::allocator<float> >, casa::pyrap::stl_variable_capacity_policy>::construct' requested here
//        &construct,
//         ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:543:2: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<float, std::__1::allocator<float> >, casa::pyrap::stl_variable_capacity_policy>::from_python_sequence'
//      requested here
//        from_python_sequence < std::vector < T >,
//        ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:550:30: note: in instantiation of member function
//      'casa::pyrap::convert_std_vector<float>::reg' requested here
//    { convert_std_vector<T>::reg(); }
//                             ^
//...trunk/asap/src/python_asap.cpp:125:16: note: in instantiation of function template specialization
//      'casa::pyrap::register_convert_std_vector<float>' requested here
//  casa::pyrap::register_convert_std_vector<float>();
//               ^
//In file included from ...trunk/asap/src/python_asap.cpp:46:
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:424:9: warning: cast from
//      'boost::python::converter::rvalue_from_python_stage1_data *' to 'rvalue_from_python_storage<std::__1::vector<double, std::__1::allocator<double> > > *'
//      increases required alignment from 8 to 16 [-Wcast-align]
//        (rvalue_from_python_storage<ContainerType>*)
//        ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:367:10: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<double, std::__1::allocator<double> >, casa::pyrap::stl_variable_capacity_policy>::construct' requested
//      here
//        &construct,
//         ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:543:2: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<double, std::__1::allocator<double> >, casa::pyrap::stl_variable_capacity_policy>::from_python_sequence'
//      requested here
//        from_python_sequence < std::vector < T >,
//        ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:550:30: note: in instantiation of member function
//      'casa::pyrap::convert_std_vector<double>::reg' requested here
//    { convert_std_vector<T>::reg(); }
//                             ^
//...trunk/asap/src/python_asap.cpp:126:16: note: in instantiation of function template specialization
//      'casa::pyrap::register_convert_std_vector<double>' requested here
//  casa::pyrap::register_convert_std_vector<double>();
//               ^
//In file included from ...trunk/asap/src/python_asap.cpp:46:
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:424:9: warning: cast from
//      'boost::python::converter::rvalue_from_python_stage1_data *' to 'rvalue_from_python_storage<std::__1::vector<std::__1::basic_string<char>,
//      std::__1::allocator<std::__1::basic_string<char> > > > *' increases required alignment from 8 to 16 [-Wcast-align]
//        (rvalue_from_python_storage<ContainerType>*)
//        ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:367:10: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<std::__1::basic_string<char>, std::__1::allocator<std::__1::basic_string<char> > >,
//      casa::pyrap::stl_variable_capacity_policy>::construct' requested here
//        &construct,
//         ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:543:2: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<std::__1::basic_string<char>, std::__1::allocator<std::__1::basic_string<char> > >,
//      casa::pyrap::stl_variable_capacity_policy>::from_python_sequence' requested here
//        from_python_sequence < std::vector < T >,
//        ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:550:30: note: in instantiation of member function
//      'casa::pyrap::convert_std_vector<std::__1::basic_string<char> >::reg' requested here
//    { convert_std_vector<T>::reg(); }
//                             ^
//...trunk/asap/src/python_asap.cpp:127:16: note: in instantiation of function template specialization
//      'casa::pyrap::register_convert_std_vector<std::__1::basic_string<char> >' requested here
//  casa::pyrap::register_convert_std_vector<std::string>();
//               ^
//In file included from ...trunk/asap/src/python_asap.cpp:32:
//In file included from /opt/casa/01/include/boost/python.hpp:11:
//In file included from /opt/casa/01/include/boost/python/args.hpp:25:
//In file included from /opt/casa/01/include/boost/python/object_core.hpp:14:
//In file included from /opt/casa/01/include/boost/python/call.hpp:15:
///opt/casa/01/include/boost/python/converter/arg_to_python.hpp:209:9: error: no matching constructor for initialization of
//      'boost::python::converter::detail::arg_to_python_base'
//      : arg_to_python_base(&x, registered<T>::converters)
//        ^                  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///opt/casa/01/include/boost/python/converter/arg_to_python.hpp:256:7: note: in instantiation of member function
//      'boost::python::converter::detail::value_arg_to_python<std::__1::__bit_const_reference<std::__1::vector<bool, std::__1::allocator<bool> > >
//      >::value_arg_to_python' requested here
//    : base(x)
//      ^
///opt/casa/01/include/boost/python/object_core.hpp:393:33: note: in instantiation of member function
//      'boost::python::converter::arg_to_python<std::__1::__bit_const_reference<std::__1::vector<bool, std::__1::allocator<bool> > > >::arg_to_python' requested here
//          return python::incref(converter::arg_to_python<T>(x).get());
//                                ^
///opt/casa/01/include/boost/python/object_core.hpp:312:10: note: in instantiation of function template specialization
//      'boost::python::api::object_initializer_impl<false, false>::get<std::__1::__bit_const_reference<std::__1::vector<bool, std::__1::allocator<bool> > > >'
//      requested here
//      >::get(
//         ^
///opt/casa/01/include/boost/python/object_core.hpp:334:23: note: in instantiation of function template specialization
//      'boost::python::api::object_base_initializer<std::__1::__bit_const_reference<std::__1::vector<bool, std::__1::allocator<bool> > > >' requested here
//        : object_base(object_base_initializer(x))
//                      ^
///opt/casa/01/include/boost/python/list.hpp:72:22: note: in instantiation of function template specialization
//      'boost::python::api::object::object<std::__1::__bit_const_reference<std::__1::vector<bool, std::__1::allocator<bool> > > >' requested here
//        base::append(object(x));
//                     ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:219:9: note: (skipping 3 contexts in backtrace; use
//      -ftemplate-backtrace-limit=0 to see all)
//        result.append(*i);
//               ^
///opt/casa/01/include/boost/python/to_python_converter.hpp:88:22: note: in instantiation of member function
//      'boost::python::converter::as_to_python_function<std::__1::vector<bool, std::__1::allocator<bool> >, casa::pyrap::to_list<std::__1::vector<bool,
//      std::__1::allocator<bool> > > >::convert' requested here
//        &normalized::convert
//                     ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:317:7: note: in instantiation of member function
//      'boost::python::to_python_converter<std::__1::vector<bool, std::__1::allocator<bool> >, casa::pyrap::to_list<std::__1::vector<bool, std::__1::allocator<bool>
//      > >, false>::to_python_converter' requested here
//      boost::python::to_python_converter < std::vector < T >,
//      ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:542:2: note: in instantiation of member function
//      'casa::pyrap::std_vector_to_list<bool>::std_vector_to_list' requested here
//        std_vector_to_list < T > ();
//        ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:550:30: note: in instantiation of member function
//      'casa::pyrap::convert_std_vector<bool>::reg' requested here
//    { convert_std_vector<T>::reg(); }
//                             ^
//...trunk/asap/src/python_asap.cpp:128:16: note: in instantiation of function template specialization
//      'casa::pyrap::register_convert_std_vector<bool>' requested here
//  casa::pyrap::register_convert_std_vector<bool>();
//               ^
///opt/casa/01/include/boost/python/converter/arg_to_python_base.hpp:20:7: note: candidate constructor not viable: no known conversion from
//      '__bit_iterator<std::__1::vector<bool, std::__1::allocator<bool> >, true>' to 'const volatile void *' for 1st argument
//      arg_to_python_base(void const volatile* source, registration const&);
//      ^
///opt/casa/01/include/boost/python/converter/arg_to_python_base.hpp:15:28: note: candidate constructor (the implicit copy constructor) not viable: requires 1
//      argument, but 2 were provided
//  struct BOOST_PYTHON_DECL arg_to_python_base
//                           ^
//In file included from ...trunk/asap/src/python_asap.cpp:46:
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:424:9: warning: cast from
//      'boost::python::converter::rvalue_from_python_stage1_data *' to 'rvalue_from_python_storage<std::__1::vector<bool, std::__1::allocator<bool> > > *' increases
//      required alignment from 8 to 16 [-Wcast-align]
//        (rvalue_from_python_storage<ContainerType>*)
//        ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:367:10: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<bool, std::__1::allocator<bool> >, casa::pyrap::stl_variable_capacity_policy>::construct' requested here
//        &construct,
//         ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:543:2: note: in instantiation of member function
//      'casa::pyrap::from_python_sequence<std::__1::vector<bool, std::__1::allocator<bool> >, casa::pyrap::stl_variable_capacity_policy>::from_python_sequence'
//      requested here
//        from_python_sequence < std::vector < T >,
//        ^
//...trunk/asap/external/libpyrap/pyrap-0.3.2/pyrap/Converters/PycBasicData.h:550:30: note: in instantiation of member function
//      'casa::pyrap::convert_std_vector<bool>::reg' requested here
//    { convert_std_vector<T>::reg(); }
//                             ^
//...trunk/asap/src/python_asap.cpp:128:16: note: in instantiation of function template specialization
//      'casa::pyrap::register_convert_std_vector<bool>' requested here
//  casa::pyrap::register_convert_std_vector<bool>();
//               ^
//12 warnings and 1 error generated.
//make[2]: *** [src/CMakeFiles/_asap.dir/python_asap.cpp.o] Error 1
//make[1]: *** [src/CMakeFiles/_asap.dir/all] Error 2
//make: *** [all] Error 2
//
//
