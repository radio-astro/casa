// Need to replicate
// IntVec
// StringVec
// DoubleVec
// BoolVec
// ComplexVec
//
// Shapes for all the vectors need to be added
//
// Quantities
// Quantity DoubleVec
%{
#include <string>
#include <vector>
#include <complex>
#include <stdcasa/record.h>
#include <tools/swigconvert_python.h>

using casac::record;
using casac::variant;
using namespace casac;

%}
%typemap(in) string {
   $1 = string(PyString_AsString($input));
}
%typemap(typecheck) string {
   $1 = PyString_Check($input);
}
%typemap(in) string& {
   $1 = new string(PyString_AsString($input));
}
%typemap(typecheck) string& {
   $1 = PyString_Check($input);
}
%typemap(in) const string& {
   $1 = new string(PyString_AsString($input));
}
%typemap(typecheck) const string& {
   $1 = PyString_Check($input);
}

%typemap(in) StringVec {
  /* Check if is a list */
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    for (int i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
        $1.value.push_back(PyString_AsString(PyList_GetItem($input,i)));
      else {
        PyErr_SetString(PyExc_TypeError,"list must contain strings");
        return NULL;
      }
    }
  } else {
    if(PyString_Check($input)){
       $1.value.push_back(PyString_AsString($input));
    } else {
       PyErr_SetString(PyExc_TypeError,"not a list");
       return NULL;
    }
  }
}

%typemap(in) complex {
   if(PyComplex_Check($input)){
      Py_complex c = PyComplex_AsCComplex($input);
      $1 = std::complex<double>(c.real, c.imag);
      //$1 = casac::complex(c.real, c.imag);
   }
}

%typemap(in) variant {
   $1 = new varaiant(pyobj2variant($input, true));
}

%typemap(in) variant& {
   $1 = new variant(pyobj2variant($input, true));
}

%typemap(in) variant* {
   $1 = new variant(pyobj2variant($input, true));
}

%typemap(typecheck) variant {
   $1=1;
}

%typemap(typecheck) variant& {
   $1=1;
}

%typemap(in) record {
   if(PyDict_Check($input)){
      $1 = pyobj2variant($input, true).asRecord();      
   } else {
      PyErr_SetString(PyExc_TypeError,"not a dictionary");
   }
}

%typemap(in) record *{
   if(PyDict_Check($input)){
      $1 = new record(pyobj2variant($input, true).asRecord());      
   } else {
      PyErr_SetString(PyExc_TypeError,"not a dictionary");
   }
}

%typemap(in) record &{
   if(PyDict_Check($input)){
      $1 = new record(pyobj2variant($input, true).asRecord());      
   } else {
      PyErr_SetString(PyExc_TypeError,"not a dictionary");
   }
}

%typemap(typecheck) record& {
   $1 = PyDict_Check($input);
}

%typemap(in) BoolVec {
   $1 = new casac::BoolAry;
   if(pyarray_check($input)){
      numpy2vector($input, $1->value, $1->shape);
   } else {
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(in) IntVec {
   $1 = new casac::IntAry;
   if(pyarray_check($input)){
      numpy2vector($input, $1->value, $1->shape);
   } else {
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(in) DoubleVec {
   $1 = new casac::DoubleAry;
   if(pyarray_check($input)){
      numpy2vector($input, $1->value, $1->shape);
   } else {
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(in) ComplexVec {
   $1 = new casac::ComplexAry;
   if(pyarray_check($input)){
      numpy2vector($input, $1->value, $1->shape);
   } else {
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(out) complex {
   $result = PyComple_FromDouble($1.real(), $1.imag());
   //$result = PyComple_FromDouble($1.re, $1.im);
}

%typemap(out) variant {
   $result = variant2pyobj($1);
}

%typemap(out) variant& {
   $result = variant2pyobj($1);
}

%typemap(out) variant* {
   $result = variant2pyobj(*$1);
}

%typemap(out) BoolVec {
   $result = ::map_array($1.value, $1.shape);
}

%typemap(out) IntVec {
   $result = ::map_array($1.value, $1.shape);
}

%typemap(out) ComplexVec {
   $result = ::map_array($1.value, $1.shape);
}

%typemap(out) DoubleVec {
   $result = ::map_array($1.value, $1.shape);
}

%typemap(out) StringVec {
   $result = ::map_array($1.value, $1.shape);
/*
   $result = PyList_New($1.size());
   for(int i=0;i<$1.size();i++)
      PyList_SetItem($result, i, PyString_FromString($1[i].c_str()));
*/
}

%typemap(out) RecordVec {
   $result = PyList_New($1.size());
   for(int i=0;i<$1.size();i++){
      const record &val = $1[i];
      PyObject *r = record2pydict(val);
      PyList_SetItem($result, i, r);
   }
}

%typemap(out) record {
   $result = PyDict_New();
   for(record::const_iterator iter = $1.begin(); iter != $1.end(); ++iter){
      const std::string &key = (*iter).first;
      const casac::variant &val = (*iter).second;
      PyObject *v = casac::variant2pyobj(val);
      PyDict_SetItem($result, PyString_FromString(key.c_str()), v);
      Py_DECREF(v);
   }
}

%typemap(out) record& {
   $result = PyDict_New();
   for(record::const_iterator iter = $1->begin(); iter != $1->end(); ++iter){
      const std::string &key = (*iter).first;
      const casac::variant &val = (*iter).second;
      PyObject *v = casac::variant2pyobj(val);
      PyDict_SetItem($result, PyString_FromString(key.c_str()), v);
      Py_DECREF(v);
   }
}

%typemap(out) record* {
   $result = PyDict_New();
   for(record::const_iterator iter = $1->begin(); iter != $1->end(); ++iter){
      const std::string &key = (*iter).first;
      const casac::variant &val = (*iter).second;
      PyObject *v = casac::variant2pyobj(val);
      PyDict_SetItem($result, PyString_FromString(key.c_str()), v);
      Py_DECREF(v);
   }
}

%typemap(argout) record& header {
   PyObject *o = PyDict_New();
   for(record::const_iterator iter = $1->begin(); iter != $1->end(); ++iter){
      const std::string &key = (*iter).first;
      const casac::variant &val = (*iter).second;
      PyObject *v = casac::variant2pyobj(val);
      PyDict_SetItem(o, PyString_FromString(key.c_str()), v);
      Py_DECREF(v);
   }
   if((!$result) || ($result == Py_None)){
      $result = o;
   } else {
      PyObject *o2 = $result;
      if (!PyTuple_Check($result)) {
         $result = PyTuple_New(1);
         PyTuple_SetItem($result,0,o2);
      }
      PyObject *o3 = PyTuple_New(1);
      PyTuple_SetItem(o3,0,o);
      o2 = $result;
      $result = PySequence_Concat(o2,o3);
      Py_DECREF(o2);
      Py_DECREF(o3);
   }
}

%apply record& header {record &summary}
%apply record& header {record &spwInfo} 

