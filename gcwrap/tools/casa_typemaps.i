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
#include <memory>
#include <string>
#include <vector>
#include <complex>
#include <stdcasa/record.h>
#include <tools/swigconvert_python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

using casac::record;
using casac::variant;
using namespace casac;

%}

%typemap(in) long long {
  if(!(PyString_Check($input) || PyFloat_Check($input) || PyDict_Check($input) || PyList_Check($input))){
     $1 = PyInt_AsLong($input);
  } else {
     cerr << "Failed here " << $input->ob_type->tp_name << endl;
     PyErr_SetString(PyExc_TypeError,"argument $1_name must be an integer");
     return NULL;
  }
}
%typemap(in) long {
  if(!(PyString_Check($input) || PyFloat_Check($input) || PyDict_Check($input) || PyList_Check($input))){
     $1 = PyInt_AsLong($input);
  } else {
     cerr << "Failed here " << $input->ob_type->tp_name << endl;
     PyErr_SetString(PyExc_TypeError,"argument $1_name must be an integer");
     return NULL;
  }
}

%typemap(in) int {
  if(!(PyString_Check($input) || PyFloat_Check($input) || PyDict_Check($input) || PyList_Check($input))){
     $1 = PyInt_AsLong($input);
  } else {
     cerr << "Failed here " << $input->ob_type->tp_name << endl;
     PyErr_SetString(PyExc_TypeError,"argument $1_name must be an integer");
     return NULL;
  }
}

%typemap(in) float {
   $1 = PyFloat_AsDouble($input);
}

%typemap(in) double {
   $1 = PyFloat_AsDouble($input);
}

%typemap(in) string {
   if(PyString_Check($input)){
      $1 = string(PyString_AsString($input));
   } else {
      // Can't throw an exception here as it's not in a try catch block
      //
      //throw casa::AipsError("Not a String");
        PyErr_SetString(PyExc_TypeError,"argument $1_name must be a string");
        return NULL;
   }
}
%typemap(typecheck) string {
   $1 = PyString_Check($input);
}
%typemap(in) string& (std::unique_ptr<string> deleter) {
   if(PyString_Check($input)){
       if(!$1){
	  
	   deleter.reset (new string(PyString_AsString($input)));
	   $1 = deleter.get();

       } else {
        *$1 = string(PyString_AsString($input));
       }
   } else {
        PyErr_SetString(PyExc_TypeError,"argument $1_name must be a string");
        return NULL;
   }
}

%typemap(typecheck) string& {
   $1 = PyString_Check($input);
}
%typemap(in) const string& (std::unique_ptr<string> deleter){
   if(PyString_Check($input)){
       if(!$1){
	   deleter.reset (new string(PyString_AsString($input)));
	   $1 = deleter.get();
       }else {
	   *$1 = string(PyString_AsString($input));
       }
   } else {
        PyErr_SetString(PyExc_TypeError,"argument $1_name must be a string");
        return NULL;
   }
}
%typemap(typecheck) const string& {
   $1 = PyString_Check($input);
}

%typemap(feeearg) const string& columnname{
if($1){
  delete $1;
}
}

%typemap(in) StringVec {
  /* Check if is a list */
  if (PyList_Check($input)) {
    Py_ssize_t size = PyList_Size($input);
    for (Py_ssize_t i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
        $1.value.push_back(PyString_AsString(PyList_GetItem($input,i)));
      else {
        PyErr_SetString(PyExc_TypeError,"list $1_name must contain strings");
        return NULL;
      }
    }
  } else {
    if(PyString_Check($input)){
       $1.value.push_back(PyString_AsString($input));
    } else {
       PyErr_SetString(PyExc_TypeError,"$1_name is not a list");
       return NULL;
    }
  }
}

%typemap(in) std::vector<std::string> & (std::unique_ptr<std::vector<std::string> > deleter) {
  /* Check if is a list */
  if (PyList_Check($input)) {
    Py_ssize_t size = PyList_Size($input);
    if(!$1){
	deleter.reset (new std::vector<std::string>(size));
        $1 = deleter.get();
    }
    for (Py_ssize_t i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
        if(i < (Py_ssize_t)($1->size()))
          (*$1)[i] = PyString_AsString(PyList_GetItem($input,i));
        else
          $1->push_back(PyString_AsString(PyList_GetItem($input,i)));
      else {
        PyErr_SetString(PyExc_TypeError,"list $1_name must contain strings");
        return NULL;
      }
    }
  } else {
    if(PyString_Check($input)){
	if(!$1){
	    deleter.reset (new std::vector<std::string>(1));
	    $1 = deleter.get();
	}
       if(!$1->size())
         $1->push_back(PyString_AsString($input));
       else
          (*$1)[0] = PyString_AsString($input);
    } else {
       PyErr_SetString(PyExc_TypeError,"$1_name is not a list");
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
    if ($1){
        (* $1) = variant(pyobj2variant($input, true));
    } else {
	PyErr_SetString (PyExc_RuntimeError, 
                         "BugCheck: Argument not initialized???");
	return nullptr;
    }
}

// Create a new value here and add a method-scope unique_ptr to
// delete the object after the method exits.  The name will be
// suffixed with an integer by swig.

%typemap(in) variant & (std::unique_ptr<variant> deleter) {

    deleter.reset (new variant (pyobj2variant ($input, true)));
    $1 = deleter.get ();

}

/* %typemap(freearg) variant& { */
/*    delete $1; */
/* } */

//%typemap(freearg) record& {
   //delete $1;
//}

%typemap(in) variant* (std::unique_ptr<variant> deleter){
    deleter.reset (new variant (pyobj2variant ($input, true)));
    $1 = deleter.get ();
}

//%typemap(typecheck) variant {
 //  $1=1;
//}

//%typemap(typecheck) variant& {
   //$1=1;
//}


%typemap(in) Quantity {
   if(PyDict_Check($input)){
      PyObject *theUnits = PyDict_GetItemString($input, "unit");
      PyObject *theVal = PyDict_GetItemString($input, "value");
      if( theUnits && theVal){
         std::vector<int> shape;
         std::vector<double> myVals;
         if(casac::pyarray_check(theVal)){
            casac::numpy2vector((PyArrayObject*)theVal, myVals, shape);
         } else {
             if (PyString_Check(theVal)){
                myVals.push_back(-1);
             } else if (PyInt_Check(theVal)){
                myVals.push_back(double(PyInt_AsLong(theVal)));
             } else if (PyLong_Check(theVal)){
                myVals.push_back(PyLong_AsDouble(theVal));
             } else if (PyFloat_Check(theVal)){
                myVals.push_back(PyFloat_AsDouble(theVal));
             } else {
               shape.push_back(PyList_Size(theVal));
                casac::pylist2vector(theVal,  myVals, shape);
             }
         }
         $1 = Quantity(myVals, PyString_AsString(theUnits));
      }
   } else if (PyString_Check($input)) {
        std::string inpstring(PyString_AsString($input));
        double val;
        std::string units;
        istringstream iss(inpstring);
        iss >> val >> units;
        myVals.push_back(val);
        $1 = Quantity(myVals,units.c_str());
   } else {
      PyErr_SetString(PyExc_TypeError,"$1_name is not a dictionary Dictionary");
      return NULL;
   }
}

%typemap(in) Quantity * (std::unique_ptr<Quantity> deleter){
   if(PyDict_Check($input)){
      PyObject *theUnits = PyDict_GetItemString($input, "unit");
      PyObject *theVal = PyDict_GetItemString($input, "value");
      if( theUnits && theVal){
         std::vector<int> shape;
         std::vector<double> myVals;
         if(casac::pyarray_check(theVal)){
            casac::numpy2vector((PyArrayObject*)theVal, myVals, shape);
         } else {
             if (PyString_Check(theVal)){
                myVals.push_back(-1);
             } else if (PyInt_Check(theVal)){
                myVals.push_back(double(PyInt_AsLong(theVal)));
             } else if (PyLong_Check(theVal)){
                myVals.push_back(PyLong_AsDouble(theVal));
             } else if (PyFloat_Check(theVal)){
                myVals.push_back(PyFloat_AsDouble(theVal));
             } else {
               shape.push_back(PyList_Size(theVal));
                casac::pylist2vector(theVal,  myVals, shape);
             }
         }
         $1 = new Quantity(myVals,PyString_AsString(theUnits));
      }
   } else if (PyString_Check($input)) {
        std::string inpstring(PyString_AsString($input));
        double val;
        std::string units;
        istringstream iss(inpstring);
        iss >> val >> units;
        myVals.push_back(val);
        deleter.reset (new Quantity(myVals,units.c_str()));
        $1 = deleter.get();
   } else {
      PyErr_SetString(PyExc_TypeError,"$1_name is not a dictionary");
      return NULL;
   }
}

%typemap(in) Quantity & (std::unique_ptr<Quantity> deleter){
   if(PyDict_Check($input)){
      PyObject *theUnits = PyDict_GetItemString($input, "unit");
      PyObject *theVal = PyDict_GetItemString($input, "value");
      if( theUnits && theVal){
         std::vector<int> shape;
         std::vector<double> myVals;
         if(casac::pyarray_check(theVal)){
            casac::numpy2vector((PyArrayObject*)theVal, myVals, shape);
         } else {
             if (PyString_Check(theVal)){
                myVals.push_back(-1);
             } else if (PyInt_Check(theVal)){
                myVals.push_back(double(PyInt_AsLong(theVal)));
             } else if (PyLong_Check(theVal)){
                myVals.push_back(PyLong_AsDouble(theVal));
             } else if (PyFloat_Check(theVal)){
                myVals.push_back(PyFloat_AsDouble(theVal));
             } else {
               shape.push_back(PyList_Size(theVal));
                casac::pylist2vector(theVal,  myVals, shape);
             }
         }
	 deleter.reset (new Quantity(myVals, PyString_AsString(theUnits)));
         $1 = deleter.get();
      }
   } else if (PyString_Check($input)) {
        std::vector<double> myVals;
        std::string inpstring(PyString_AsString($input));
        double val;
        std::string units;
        istringstream iss(inpstring);
        iss >> val >> units;
        myVals.push_back(val);
	deleter.reset (new Quantity(myVals,units.c_str()));
        $1 = deleter.get();
   } else {
      PyErr_SetString(PyExc_TypeError,"$1_name is not a dictionary");
      return NULL;
   }
}

%typemap(in) record {
   if(PyDict_Check($input)){
      $1 = pyobj2variant($input, true).asRecord();      
   } else {
      PyErr_SetString(PyExc_TypeError,"$1_name is not a dictionary");
      return NULL;
   }
}

%typemap(in) record * (std::unique_ptr<record> deleter){
   if(PyDict_Check($input)){
       
       deleter.reset (new record(pyobj2variant($input, true).asRecord()));      
       $1 = deleter.get();
   } else {
      PyErr_SetString(PyExc_TypeError,"$1_name is not a dictionary");
      return NULL;
   }
}

%typemap(in) record & (std::unique_ptr<record> deleter){
   if(PyDict_Check($input)){

       // Put value into unique_ptr so it gets free when method exits
       deleter.reset (new record(pyobj2variant($input, true).asRecord()));
       $1 = deleter.get(); 
   } else {
      PyErr_SetString(PyExc_TypeError,"$1_name is not a dictionary");
      return NULL;
   }
}

%typemap(typecheck) record& {
   if($input)
     $1 = PyDict_Check($input);
   else
     $1 = 1
}

%typemap(in) BoolVec (std::unique_ptr<BoolVec> deleter){
   deleter.reset (new casac::BoolAry);
   $1 = deleter.get();
   if(pyarray_check($input)){
      numpy2vector((PyArrayObject*)$input, $1->value, $1->shape);
   } else {
         shape.push_back(PyList_Size($input));
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(in) BoolVec& (std::unique_ptr<BoolVec> deleter){
   deleter.reset (new casac::BoolAry);
   $1 = deleter.get();
   if(pyarray_check($input)){
      numpy2vector((PyArrayObject*)$input, $1->value, $1->shape);
   } else {
         shape.push_back(PyList_Size($input));
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(in) IntVec (std::unique_ptr<IntVec> deleter){
   deleter.reset (new casac::IntAry);
   $1 = deleter.get();
   if(pyarray_check($input)){
      numpy2vector((PyArrayObject*)$input, $1->value, $1->shape);
   } else {
         shape.push_back(PyList_Size($input));
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(in) IntVec& (std::unique_ptr<IntVec> deleter) {
   deleter.reset (new casac::IntAry);
   $1 = deleter.get();
   if(pyarray_check($input)){
      numpy2vector((PyArrayObject*)$input, $1->value, $1->shape);
   } else {
         shape.push_back(PyList_Size($input));
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(in) DoubleVec (std::unique_ptr<DoubleVec> deleter){

   deleter.reset (new casac::DoubleAry);
   $1 = deleter.reset();
   if(pyarray_check($input)){
      numpy2vector((PyArrayObject*)$input, $1->value, $1->shape);
   } else {
         shape.push_back(PyList_Size($input));
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(in) DoubleVec& (std::unique_ptr<DoubleVec> deleter){
    if(!$1){
	deleter.reset (new casac::DoubleAry);
	$1 = deleter.reset();
    }
   if(pyarray_check($input)){
      numpy2vector((PyArrayObject*)$input, $1->value, $1->shape);
   } else {
         shape.push_back(PyList_Size($input));
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(in) std::vector<double> & (std::unique_ptr<std::vector<double> > deleter){
   std::vector<int> shape;
  
   if(!$1){
       deleter.reset (new std::vector<double>(0));
       $1 = deleter.get();
   }
   else
      $1->resize(0);
   if(casac::pyarray_check($input)){
      //cerr << "numpy2vec" << endl;
      casac::numpy2vector((PyArrayObject*)$input, *$1, shape);
   } else {
       if (PyString_Check($input)){
          $1->push_back(-1);
       } else if (PyInt_Check($input)){
          $1->push_back(double(PyInt_AsLong($input)));
       } else if (PyLong_Check($input)){
          $1->push_back(PyLong_AsDouble($input));
       } else if (PyFloat_Check($input)){
          $1->push_back(PyFloat_AsDouble($input));
       } else {
          shape.push_back(PyList_Size($input));
          casac::pylist2vector($input,  *$1, shape);
       }
   }
}

%typemap(in) std::vector<bool> & (std::unique_ptr<std::vector<bool> > deleter) {
    if(!$1){
	deleter.reset (new std::vector<bool>(0));
        $1 = deleter.get();
    }
   else
      $1->resize(0);
   std::vector<int> shape;

   if(casac::pyarray_check($input)){
      casac::numpy2vector((PyArrayObject*)$input, *$1, shape);
   } else {
      if (PyString_Check($input)){
         $1->push_back(0);
         PyErr_SetString(PyExc_TypeError,"argument $1_name must be a string");
         return NULL;
      } else if (PyBool_Check($input)){
         $1->push_back(bool(PyInt_AsLong($input)));
      } else if (PyInt_Check($input)){
         $1->push_back(bool(PyInt_AsLong($input)));
      } else if (PyLong_Check($input)){
         $1->push_back(bool(PyLong_AsLong($input)));
      } else if (PyFloat_Check($input)){
         $1->push_back(bool(PyInt_AsLong(PyNumber_Int($input))));
      } else {
         shape.push_back(PyList_Size($input));
         casac::pylist2vector($input,  *$1, shape);
      }
   }
}


%typemap(in) std::vector<int> & (std::unique_ptr<std::vector<int> > deleter) {
    if(!$1){
	deleter.reset (new std::vector<int>(0));
	$1 = deleter.get();
    }
   else
      $1->resize(0);
   std::vector<int> shape;

   if(casac::pyarray_check($input)){
      casac::numpy2vector((PyArrayObject*)$input, *$1, shape);
   } else {
      if (PyString_Check($input)){
         $1->push_back(-1);
         PyErr_SetString(PyExc_TypeError,"argument $1_name must not be a string");
         return NULL;
      } else if (PyInt_Check($input)){
         $1->push_back(int(PyInt_AsLong($input)));
      } else if (PyLong_Check($input)){
         $1->push_back(PyLong_AsLong($input));
      } else if (PyFloat_Check($input)){
         $1->push_back(PyInt_AsLong(PyNumber_Int($input)));
      } else {
         shape.push_back(PyList_Size($input));
         casac::pylist2vector($input,  *$1, shape);
      }
   }
}

%typemap(in) std::vector<long long> & (std::unique_ptr<std::vector<long long> > deleter) {
    if(!$1){
	deleter.reset (new std::vector<long long>(0));
	$1 = deleter.get();
    }
   else
      $1->resize(0);
   std::vector<int> shape;

   if(casac::pyarray_check($input)){
      casac::numpy2vector((PyArrayObject*)$input, *$1, shape);
   } else {
      if (PyString_Check($input)){
         $1->push_back(-1);
         PyErr_SetString(PyExc_TypeError,"argument $1_name must not be a string");
         return NULL;
      } else if (PyInt_Check($input)){
         $1->push_back(int(PyInt_AsLong($input)));
      } else if (PyLong_Check($input)){
         $1->push_back(PyLong_AsLong($input));
      } else if (PyFloat_Check($input)){
         $1->push_back(PyInt_AsLong(PyNumber_Int($input)));
      } else {
         shape.push_back(PyList_Size($input));
         casac::pylist2vector($input,  *$1, shape);
      }
   }
}


%typemap(in) std::vector<long> & (std::unique_ptr<std::vector<long> > deleter){
    if(!$1){
	deleter.reset (new std::vector<long>(0));
	$1 = deleter.get():
    }
   else
      $1->resize(0);
   std::vector<int> shape;

   if(casac::pyarray_check($input)){
      casac::numpy2vector((PyArrayObject*)$input, *$1, shape);
   } else {
      if (PyString_Check($input)){
         $1->push_back(-1);
         PyErr_SetString(PyExc_TypeError,"argument $1_name must not be a string");
         return NULL;
      } else if (PyInt_Check($input)){
         $1->push_back(int(PyInt_AsLong($input)));
      } else if (PyLong_Check($input)){
         $1->push_back(PyLong_AsLong($input));
      } else if (PyFloat_Check($input)){
         $1->push_back(PyInt_AsLong(PyNumber_Int($input)));
      } else {
         shape.push_back(PyList_Size($input));
         casac::pylist2vector($input,  *$1, shape);
      }
   }
}

/* %typemap(freearg) const std::vector<long long>& rownr { */
/*    delete $1; */
/* } */

/* %typemap(freearg) const std::vector<long>& rownr { */
/*    delete $1; */
/* } */

/* %typemap(freearg) const std::vector<int>& rownr { */
/*    delete $1; */
/* } */


%typemap(in) ComplexVec (std::unique_ptr<ComplexVec> deleter) {
    deleter.reset (new casac::ComplexAry);
    $1 = deleter.get();
   if(pyarray_check($input)){
      numpy2vector((PyArrayObject*)$input, $1->value, $1->shape);
   } else {
      shape.push_back(PyList_Size($input));
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(in) ComplexVec& (std::unique_ptr<ComplexVec> deleter) {
    deleter.reset (new casac::ComplexAry);
    $1 = deleter.get();
   if(pyarray_check($input)){
      numpy2vector((PyArrayObject*)$input, $1->value, $1->shape);
   } else {
         shape.push_back(PyList_Size($input));
      pylist2vector($input,  $1->value, $1->shape);
   }
}

%typemap(out) complex {
   $result = PyComplex_FromDouble($1.real(), $1.imag());
}

%typemap(out) variant {
   $result = variant2pyobj($1);
}

%typemap(out) variant& {
   $result = variant2pyobj($1);
}

%typemap(out) string {
   $result = PyString_FromString($1.c_str());
}

%typemap(out) string* {
   if($1)
     $result = PyString_FromString($1->c_str());
   else
     $result = Py_None;
   delete $1;
}

%typemap(out) variant* {
   if ($1)
      $result = variant2pyobj(*$1);
   else{
     variant temp_v;
     $result = variant2pyobj(temp_v);
   }
   delete $1;
}

%typemap(out) Quantity& {
   $result = PyDict_New();
   PyDict_SetItem($result, PyString_FromString("unit"), PyString_FromString($1.units.c_str()));
   PyObject *v = casac::map_vector($1.value);
   PyDict_SetItem($result, PyString_FromString("value"), v);
   Py_DECREF(v);
}

%typemap(out) Quantity {
   $result = PyDict_New();
   PyDict_SetItem($result, PyString_FromString("unit"), PyString_FromString($1.units.c_str()));
   PyObject *v = casac::map_vector($1.value);
   PyDict_SetItem($result, PyString_FromString("value"), v);
   Py_DECREF(v);
}

%typemap(out) Quantity* {
   $result = PyDict_New();
   PyDict_SetItem($result, PyString_FromString("unit"), PyString_FromString($1->units.c_str()));
   PyObject *v = casac::map_vector($1->value);
   PyDict_SetItem($result, PyString_FromString("value"), v);
   Py_DECREF(v);
   delete $1;
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
   $result = PyList_New($1.size());
   for(StringVec::size_type i=0;i<$1.size();i++)
      PyList_SetItem($result, i, PyString_FromString($1[i].c_str()));
}

%typemap(out) std::vector<std::string> {
   $result = PyList_New($1.size());
   for(std::vector<std::string>::size_type i=0;i<$1.size();i++)
      PyList_SetItem($result, i, PyString_FromString($1[i].c_str()));
}

%typemap(out) std::vector<bool> {
   $result = casac::map_vector($1);
}

%typemap(out) std::vector<bool>& {
   $result = casac::map_vector($1);
}

%typemap(out) std::vector<int> {
   $result = casac::map_vector($1);
}

%typemap(out) std::vector<int>& {
   $result = casac::map_vector($1);
}

%typemap(out) std::vector<long long> {
   $result = casac::map_vector($1);
}

%typemap(out) std::vector<long long>& {
   $result = casac::map_vector($1);
}

%typemap(out) std::vector<double> {
   $result = casac::map_vector($1);
}

%typemap(out) std::vector<double>& {
   $result = casac::map_vector($1);
}

%typemap(out) RecordVec {
   $result = PyList_New($1.size());
   for(RecordVec::size_type i=0;i<$1.size();i++){
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
   if($1){
      for(record::const_iterator iter = $1->begin(); iter != $1->end(); ++iter){
         const std::string &key = (*iter).first;
         const casac::variant &val = (*iter).second;
         PyObject *v = casac::variant2pyobj(val);
         PyDict_SetItem($result, PyString_FromString(key.c_str()), v);
         Py_DECREF(v);
      }
      delete $1;
   }
}

%typemap(argout) int& OUTARGINT{
   PyObject *o = PyLong_FromLong(*$1);
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

%typemap(argout) double& OUTARGDBL{
   PyObject *o = PyFloat_FromDouble(*$1);
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

%typemap(argout) string& OUTARGSTR{
   PyObject *o = PyString_FromString($1->c_str());
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

%typemap(argout) std::string& OUTARGSTR{
   PyObject *o = PyString_FromString($1->c_str());
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


%typemap(argout) std::vector<std::string>& OUTARGVEC{
   PyObject *o = PyList_New($1.size());
   for(std::vector<std::string>::size_type i=0;i<$1.size();i++)
      PyList_SetItem($result, i, PyString_FromString($1[i].c_str()));
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

%typemap(argout) std::vector<int>& OUTARGVEC {
   PyObject *o = casac::map_vector(*$1);
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

%typemap(argout) std::vector<double>& OUTARGVEC {
   PyObject *o= casac::map_vector(*$1);
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

%typemap(argout) record& OUTARGREC {
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

%typemap(argout) Quantity& OUTARGQUANTITY{
   PyObject *o = PyDict_New();
   PyDict_SetItem(o, PyString_FromString("unit"), PyString_FromString($1->units.c_str()));
   PyObject *v = casac::map_vector($1->value);
   PyDict_SetItem(o, PyString_FromString("value"), v);
   Py_DECREF(v);
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
