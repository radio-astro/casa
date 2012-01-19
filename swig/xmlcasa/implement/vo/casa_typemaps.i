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
%}
%typemap(in) string {
   $1 = string(PyString_AsString($input));
}
%typemap(in) StringVec {
  /* Check if is a list */
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    for (int i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
        $1.push_back(PyString_AsString(PyList_GetItem($input,i)));
      else {
        PyErr_SetString(PyExc_TypeError,"list must contain strings");
        return NULL;
      }
    }
  } else {
    if(PyString_Check($input)){
       $1.push_back(PyString_AsString($input));
    } else {
       PyErr_SetString(PyExc_TypeError,"not a list");
       return NULL;
    }
  }
}
%typemap(out) StringVec {
   $result = PyList_New($1.size());
   for(int i=0;i<$1.size();i++)
      PyList_SetItem($result, i, PyString_FromString($1[i].c_str()));
}
