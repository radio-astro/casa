#include <iostream>
#include <iomanip>
#include <Python.h>

#include <casa/Containers/ValueHolder.h>
#include <casa/Utilities/PtrHolder.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayIO.h>

#include <libsakura/sakura.h>
#include <libsakura/sakura-python.h>

#include <stdcasa/variant.h>
#include <stdcasa/StdCasa/CasacSupport.h>
#include <tools/swigconvert_python.h>

inline void *_malloc(size_t size)
{
  void *p = malloc(size);
  std::cout << "allocate " << std::hex << p << std::dec << std::endl;
  return p;
}

inline void _free(void *p)
{
  std::cout << "free " << std::hex << p << std::dec << std::endl;
  free(p);
}

inline casa::ValueHolder *ToValueHolder(PyObject *obj)
{
  return casa::toValueHolder(casac::pyobj2variant(obj, true));
}

inline PyObject *ToPyObject(casa::ValueHolder val)
{
  casac::variant *v = casa::fromValueHolder(val);
  return casac::variant2pyobj(*v);
}

template <class ADataType, class Handler>
class HandlerBase
{
public:
  typedef ADataType DataType;
  
  static PyObject *CreateChunkForSakura(PyObject *obj)
  {
    casa::PtrHolder<casa::ValueHolder> var(ToValueHolder(obj));
    const casa::Array<DataType> arr = Handler::asArray(var.ptr());
    casa::IPosition shape = arr.shape();
    casa::uInt ndim = shape.size();
    PyObject *tuple = Py_None;
    if (ndim == 1) {
      tuple = PyTuple_New(1);
      PyTuple_SetItem(tuple, 0, EncapsulateCell(arr));
    }
    else if (ndim == 2) {
      tuple = PyTuple_New(1);
      PyTuple_SetItem(tuple, 0, EncapsulateCell(arr));
    }
    else if (ndim == 3) {
      casa::uInt npol = shape[0];
      casa::uInt nchan = shape[1];
      casa::uInt nrow = shape[2];
      tuple = PyTuple_New((Py_ssize_t)nrow);
      for (casa::uInt irow = 0; irow < nrow; ++irow) {
	casa::IPosition start(3, 0, 0, irow);
	casa::IPosition end(3, npol-1, nchan-1, irow);
	casa::Array<DataType> slice = arr(start, end);
	slice.removeDegenerate();
	PyTuple_SetItem(tuple, (Py_ssize_t)irow,
			EncapsulateCell(slice));
      }
    }
    else {
      // increment reference count for Py_None object
      Py_INCREF(tuple);
    }
    return tuple;
  }

  static PyObject *CreateChunkForCasa(PyObject *obj)
  {
    casa::Array<DataType> arr;
    int is_tuple = PyTuple_Check(obj);
    Py_ssize_t nrow = PyTuple_Size(obj);
    casa::uInt npol = 0;
    casa::uInt nchan = 0;

    casa::Bool b;
    DataType *arr_p = NULL;
    for (Py_ssize_t irow = 0; irow < nrow; ++irow) {
      PyObject *tuple = PyTuple_GetItem(obj, irow);
      Py_ssize_t _npol = PyTuple_Size(tuple);
      for (Py_ssize_t ipol = 0; ipol < _npol; ++ipol) {
	PyObject *capsule = PyTuple_GetItem(tuple, ipol);
	sakura_PyAlignedBuffer *buffer;
	sakura_Status status = sakura_PyAlignedBufferDecapsulate(capsule,
								 &buffer);
	if (status == sakura_Status_kOK) {
	  void *aligned;
	  sakura_PyAlignedBufferAlignedAddr(buffer, &aligned);
	  size_t dimensions;
	  sakura_PyAlignedBufferDimensions(buffer, &dimensions);
	  size_t elements[1];
	  sakura_PyAlignedBufferElements(buffer, 1, elements);
	  if (arr.empty()) {
	    npol = _npol;
	    nchan = elements[0];
	    arr.resize(casa::IPosition(3, npol, nchan, nrow));
	    arr_p = arr.getStorage(b);		
	  }
	  const float *p = reinterpret_cast<const float *>(aligned);
	  size_t start_pos = irow * (_npol * elements[0]);
	  DataType *out_p = &arr_p[start_pos];
	  Handler::ToCasa(ipol, npol, irow, elements[0], aligned, out_p);
	}
      }
    }
    arr.putStorage(arr_p, b);

    // reform
    // TBD: what is the best shape for output array?
    casa::Array<DataType> new_arr;
    if (npol == 1 && nrow == 1) {
      new_arr = arr.reform(casa::IPosition(1, nchan));
    }
    else if (nrow == 1) {
      new_arr = arr.reform(casa::IPosition(2, npol, nchan));
    }
    else {
      new_arr = arr;
    }
    casa::ValueHolder val(new_arr);
    return ToPyObject(val);
  }
  
private:
  static PyObject *EncapsulateCell(const casa::Array<DataType> &arr)
  {
    // arr should be two-dimensional (npol,nchan) or one-dimensional (nchan)
    Py_ssize_t ndim = arr.ndim();
    casa::IPosition shape = arr.shape();
    casa::uInt npol = (ndim == 2) ? shape[0] : 1;
    casa::uInt nchan = (ndim == 2) ? shape[1] : shape[0];
    PyObject *tuple = PyTuple_New((Py_ssize_t)npol);
    casa::Bool b;
    size_t element_size = sizeof(typename Handler::CDataType);
    size_t required_size = element_size * nchan;
    size_t allocation_size = required_size + sakura_GetAlignment() - 1;
    size_t elements[] = {nchan};
    const DataType *arr_p = arr.getStorage(b);
    for (casa::uInt ipol = 0; ipol < npol; ++ipol) {
      void *storage = _malloc(allocation_size);
      void *aligned = sakura_AlignAny(allocation_size, storage, required_size);
      sakura_PyAlignedBuffer *buffer;
      sakura_PyAlignedBufferCreate((sakura_PyTypeId)Handler::SakuraPyTypeId,
				   storage, aligned,
				   1, elements, &_free,
				   &buffer);
      PyObject *capsule;
      sakura_PyAlignedBufferEncapsulate(buffer,
					&capsule);
      Handler::ToSakura(arr_p, npol, ipol, nchan, aligned);
      PyTuple_SetItem(tuple, (Py_ssize_t)ipol, capsule);
    }
    arr.freeStorage(arr_p, b);
    return tuple;
  }
};

static inline PyObject *ArgAsPyObject(PyObject *args)
{
  PyObject *obj;
  if (!PyArg_ParseTuple(args, "O", &obj)) {
    std::cout << "Failed to parse args" << std::endl;
    return NULL;
  }
  return obj;
}

class FloatDataHandler : public HandlerBase<casa::Float, FloatDataHandler>
{
public:
  typedef float CDataType;
  
  enum { SakuraPyTypeId=sakura_PyTypeId_kFloat };
  
  static casa::Array<DataType> asArray(casa::ValueHolder *v)
  {
    return v->asArrayFloat();
  }

  static void ToSakura(const DataType *in_p, const size_t increment,
		       const size_t offset, const size_t num_elements,
		       void *out_p)
  {
    CDataType *work_p = reinterpret_cast<CDataType *>(out_p);
    for (casa::uInt i = 0; i < num_elements; ++i) {
      work_p[i] = in_p[offset + i * increment];
    }
  }

  static void ToCasa(const size_t offset, const size_t increment,
		     const size_t row, const size_t num_elements,
		     const void *in_p, DataType *out_p)
  {
    const CDataType *work_p = reinterpret_cast<const CDataType *>(in_p);
    for (size_t i = 0; i < num_elements; ++i) {
      out_p[offset + i * increment] = work_p[i];
    }
  }
};

class ComplexDataHandler : public HandlerBase<casa::Complex, ComplexDataHandler>
{
public:
  typedef float CDataType;
  
  enum { SakuraPyTypeId=sakura_PyTypeId_kFloat };
  
  static casa::Array<DataType> asArray(casa::ValueHolder *v)
  {
    return v->asArrayComplex();
  }

  static void ToSakura(const DataType *in_p, const size_t increment,
		       const size_t offset, const size_t num_elements,
		       void *out_p)
  {
    CDataType *work_p = reinterpret_cast<CDataType *>(out_p);
    for (casa::uInt i = 0; i < num_elements; ++i) {
      work_p[i] = in_p[offset + i * increment].real();
    }
  }

  static void ToCasa(const size_t offset, const size_t increment,
		     const size_t row, const size_t num_elements,
		     const void *in_p, DataType *out_p)
  {
    const CDataType *work_p = reinterpret_cast<const CDataType *>(in_p);
    for (size_t i = 0; i < num_elements; ++i) {
      out_p[offset + i * increment].real() = work_p[i];
      out_p[offset + i * increment].imag() = 0.0;
    }
  }
};

class DoubleDataHandler : public HandlerBase<casa::Double, DoubleDataHandler>
{
public:
  typedef double CDataType;
  
  enum { SakuraPyTypeId=sakura_PyTypeId_kDouble };

  static casa::Array<DataType> asArray(casa::ValueHolder *v)
  {
    return v->asArrayDouble();
  }

  static void ToSakura(const DataType *in_p, const size_t increment,
		       const size_t offset, const size_t num_elements,
		       void *out_p)
  {
    CDataType *work_p = reinterpret_cast<CDataType *>(out_p);
    for (casa::uInt i = 0; i < num_elements; ++i) {
      work_p[i] = in_p[offset + i * increment];
    }
  }

  static void ToCasa(const size_t offset, const size_t increment,
		     const size_t row, const size_t num_elements,
		     const void *in_p, DataType *out_p)
  {
    const CDataType *work_p = reinterpret_cast<const CDataType *>(in_p);
    for (size_t i = 0; i < num_elements; ++i) {
      out_p[offset + i * increment] = work_p[i];
    }
  }
};

class BoolDataHandler : public HandlerBase<casa::Bool, BoolDataHandler>
{
public:
  typedef bool CDataType;
 
  enum { SakuraPyTypeId=sakura_PyTypeId_kBool };

  static casa::Array<DataType> asArray(casa::ValueHolder *v)
  {
    return v->asArrayBool();
  }

  static void ToSakura(const DataType *in_p, const size_t increment,
		       const size_t offset, const size_t num_elements,
		       void *out_p)
  {
    CDataType *work_p = reinterpret_cast<CDataType *>(out_p);
    for (casa::uInt i = 0; i < num_elements; ++i) {
      // CASA flag: valid is False, invalid is True
      // Sakura mask: valid is True, invalid is False
      work_p[i] = !(in_p[offset + i * increment]);
    }
  }

  static void ToCasa(const size_t offset, const size_t increment,
		     const size_t row, const size_t num_elements,
		     const void *in_p, DataType *out_p)
  {
    const CDataType *work_p = reinterpret_cast<const CDataType *>(in_p);
    for (size_t i = 0; i < num_elements; ++i) {
      // CASA flag: valid is False, invalid is True
      // Sakura mask: valid is True, invalid is False
      out_p[offset + i * increment] = !(work_p[i]);
    }
  }
};

template<class Handler>
static PyObject *tosakura(PyObject *self, PyObject *args)
{
  PyObject *obj = ArgAsPyObject(args);

  PyObject *tuple = Handler::CreateChunkForSakura(obj);
  return tuple;
}

template<class Handler>
static PyObject *tocasa(PyObject *self, PyObject *args)
{
  PyObject *obj = ArgAsPyObject(args);

  PyObject *ret = Handler::CreateChunkForCasa(obj);
  return ret;  
}

static PyMethodDef CasaSakuraMethods[] = {
  {"tosakura_float", tosakura<FloatDataHandler>, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_float", tocasa<FloatDataHandler>, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_double", tosakura<DoubleDataHandler>, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_double", tocasa<DoubleDataHandler>, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_complex", tosakura<ComplexDataHandler>, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_complex", tocasa<ComplexDataHandler>, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_bool", tosakura<BoolDataHandler>, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_bool", tocasa<BoolDataHandler>, METH_VARARGS, "convert sakura array to casa array"},
  {NULL, NULL, 0, NULL} /* Sentinel */
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
init_casasakura(void)
{
  PyObject *m;
  m = Py_InitModule("_casasakura", CasaSakuraMethods);
}
