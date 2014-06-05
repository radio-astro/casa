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

template <class DataType, class Handler>
class DataConverter
{
public:
  static PyObject *EncapsulateChunk(PyObject *obj)
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
    return tuple;
  }

  static PyObject *DecapsulateChunk(PyObject *obj)
  {
    casa::Array<DataType> arr;
    int is_tuple = PyTuple_Check(obj);
    Py_ssize_t nrow = PyTuple_Size(obj);
    casa::uInt npol = 0;
    casa::uInt nchan = 0;
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
	  sakura_PyAlignedBufferElements(buffer, dimensions, elements);
	  if (arr.empty()) {
	    npol = _npol;
	    nchan = elements[0];
	    arr.resize(casa::IPosition(3, npol, nchan, nrow));
	  }
	  Handler::ToCasa(ipol, npol, irow, elements[0], aligned, arr);
	}
      }
    }

    // reform
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
    size_t element_size = Handler::GetElementSize();
    size_t required_size = element_size * nchan;
    size_t allocation_size = required_size + element_size * (sakura_GetAlignment() - 1);
    size_t elements[] = {nchan};
    const DataType *arr_p = arr.getStorage(b);
    for (casa::uInt ipol = 0; ipol < npol; ++ipol) {
      void *storage = _malloc(allocation_size);
      void *aligned = sakura_AlignAny(allocation_size, storage, required_size);
      sakura_PyAlignedBuffer *buffer;
      PyObject *capsule;
      sakura_PyAlignedBufferCreate(Handler::GetTypeId(),
				   storage, aligned,
				   1, elements, &_free,
				   &buffer);
      sakura_PyAlignedBufferEncapsulate(buffer,
					&capsule);
      Handler::ToSakura(arr_p, npol, ipol, nchan, aligned);
      PyTuple_SetItem(tuple, (Py_ssize_t)ipol, capsule);
    }
    arr.freeStorage(arr_p, b);
    return tuple;
  }
};

static PyObject *ArgAsPyObject(PyObject *args)
{
  PyObject *obj;
  if (!PyArg_ParseTuple(args, "O", &obj)) {
    std::cout << "Failed to parse args" << std::endl;
    return NULL;
  }
  return obj;
}

template<class DataType, class Handler>
static PyObject *_tosakura(PyObject *self, PyObject *args)
{
  PyObject *obj = ArgAsPyObject(args);

  PyObject *tuple = DataConverter<DataType, Handler>::EncapsulateChunk(obj);
  return tuple;
}

template<class DataType, class Handler>
static PyObject *_tocasa(PyObject *self, PyObject *args)
{
  PyObject *obj = ArgAsPyObject(args);

  PyObject *ret = DataConverter<DataType, Handler>::DecapsulateChunk(obj);
  return ret;  
}

class FloatDataHandler
{
public:
  static PyObject *tosakura(PyObject *self, PyObject *args)
  {
    return _tosakura<casa::Float, FloatDataHandler>(self, args);
  }

  static PyObject *tocasa(PyObject *self, PyObject *args)
  {
    return _tocasa<casa::Float, FloatDataHandler>(self, args);
  }

  static casa::Array<casa::Float> asArray(casa::ValueHolder *v)
  {
    return v->asArrayFloat();
  }

  static size_t GetElementSize()
  {
    return sizeof(float);
  }

  static sakura_PyTypeId GetTypeId()
  {
    return sakura_PyTypeId_kFloat;
  }
  
  static void ToSakura(const casa::Float *in_p, const size_t increment,
		       const size_t offset, const size_t num_elements,
		       void *out_p)
  {
    float *work_p = reinterpret_cast<float *>(out_p);
    for (casa::uInt i = 0; i < num_elements; ++i) {
      work_p[i] = in_p[offset + i * increment];
    }
  }

  static void ToCasa(const size_t offset, const size_t increment,
		     const size_t row, const size_t num_elements,
		     const void *in_p, casa::Array<casa::Float> &arr)
  {
    casa::Bool b;
    const float *work_p = reinterpret_cast<const float *>(in_p);
    casa::Float *arr_p = arr.getStorage(b);
    size_t start_pos = row * (increment * num_elements);
    casa::Float *out_p = &arr_p[start_pos];
    for (size_t i = 0; i < num_elements; ++i) {
      out_p[offset + i * increment] = work_p[i];
    }
    arr.putStorage(arr_p, b);
  }
};

class ComplexDataHandler
{
public:
  static PyObject *tosakura(PyObject *self, PyObject *args)
  {
    return _tosakura<casa::Complex, ComplexDataHandler>(self, args);
  }

  static PyObject *tocasa(PyObject *self, PyObject *args)
  {
    return _tocasa<casa::Complex, ComplexDataHandler>(self, args);
  }

  static casa::Array<casa::Complex> asArray(casa::ValueHolder *v)
  {
    return v->asArrayComplex();
  }

  static size_t GetElementSize()
  {
    return sizeof(float);
  }

  static sakura_PyTypeId GetTypeId()
  {
    return sakura_PyTypeId_kFloat;
  }
  
  static void ToSakura(const casa::Complex *in_p, const size_t increment,
		       const size_t offset, const size_t num_elements,
		       void *out_p)
  {
    float *work_p = reinterpret_cast<float *>(out_p);
    for (casa::uInt i = 0; i < num_elements; ++i) {
      work_p[i] = in_p[offset + i * increment].real();
    }
  }

  static void ToCasa(const size_t offset, const size_t increment,
		     const size_t row, const size_t num_elements,
		     const void *in_p, casa::Array<casa::Complex> &arr)
  {
    casa::Bool b;
    const float *work_p = reinterpret_cast<const float *>(in_p);
    casa::Complex *arr_p = arr.getStorage(b);
    size_t start_pos = row * (increment * num_elements);
    casa::Complex *out_p = &arr_p[start_pos];
    for (size_t i = 0; i < num_elements; ++i) {
      out_p[offset + i * increment].real() = work_p[i];
      out_p[offset + i * increment].imag() = 0.0;
    }
    arr.putStorage(arr_p, b);
  }

};

class DoubleDataHandler
{
public:
  static PyObject *tosakura(PyObject *self, PyObject *args)
  {
    return _tosakura<casa::Double, DoubleDataHandler>(self, args);
  }

  static PyObject *tocasa(PyObject *self, PyObject *args)
  {
    return _tocasa<casa::Double, DoubleDataHandler>(self, args);
  }

  static casa::Array<casa::Double> asArray(casa::ValueHolder *v)
  {
    return v->asArrayDouble();
  }

  static size_t GetElementSize()
  {
    return sizeof(double);
  }

  static sakura_PyTypeId GetTypeId()
  {
    return sakura_PyTypeId_kDouble;
  }
  
  static void ToSakura(const casa::Double *in_p, const size_t increment,
		       const size_t offset, const size_t num_elements,
		       void *out_p)
  {
    double *work_p = reinterpret_cast<double *>(out_p);
    for (casa::uInt i = 0; i < num_elements; ++i) {
      work_p[i] = in_p[offset + i * increment];
    }
  }

  static void ToCasa(const size_t offset, const size_t increment,
		     const size_t row, const size_t num_elements,
		     const void *in_p, casa::Array<casa::Double> &arr)
  {
    casa::Bool b;
    const double *work_p = reinterpret_cast<const double *>(in_p);
    casa::Double *arr_p = arr.getStorage(b);
    size_t start_pos = row * (increment * num_elements);
    casa::Double *out_p = &arr_p[start_pos];
    for (size_t i = 0; i < num_elements; ++i) {
      out_p[offset + i * increment] = work_p[i];
    }
    arr.putStorage(arr_p, b);
  }

};

class BoolDataHandler
{
public:
  static PyObject *tosakura(PyObject *self, PyObject *args)
  {
    return _tosakura<casa::Bool, BoolDataHandler>(self, args);
  }

  static PyObject *tocasa(PyObject *self, PyObject *args)
  {
    return _tocasa<casa::Bool, BoolDataHandler>(self, args);
  }

  static casa::Array<casa::Bool> asArray(casa::ValueHolder *v)
  {
    return v->asArrayBool();
  }

  static size_t GetElementSize()
  {
    return sizeof(bool);
  }

  static sakura_PyTypeId GetTypeId()
  {
    return sakura_PyTypeId_kBool;
  }
  
  static void ToSakura(const casa::Bool *in_p, const size_t increment,
		       const size_t offset, const size_t num_elements,
		       void *out_p)
  {
    bool *work_p = reinterpret_cast<bool *>(out_p);
    for (casa::uInt i = 0; i < num_elements; ++i) {
      // CASA flag: valid is False, invalid is True
      // Sakura mask: valid is True, invalid is False
      work_p[i] = !(in_p[offset + i * increment]);
    }
  }

  static void ToCasa(const size_t offset, const size_t increment,
		     const size_t row, const size_t num_elements,
		     const void *in_p, casa::Array<casa::Bool> &arr)
  {
    casa::Bool b;
    const bool *work_p = reinterpret_cast<const bool *>(in_p);
    casa::Bool *arr_p = arr.getStorage(b);
    size_t start_pos = row * (increment * num_elements);
    casa::Bool *out_p = &arr_p[start_pos];
    for (size_t i = 0; i < num_elements; ++i) {
      // CASA flag: valid is False, invalid is True
      // Sakura mask: valid is True, invalid is False
      out_p[offset + i * increment] = !(work_p[i]);
    }
    arr.putStorage(arr_p, b);
  }

};

static PyMethodDef CasaSakuraMethods[] = {
  {"tosakura_float", FloatDataHandler::tosakura, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_float", FloatDataHandler::tocasa, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_double", DoubleDataHandler::tosakura, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_double", DoubleDataHandler::tocasa, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_complex", ComplexDataHandler::tosakura, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_complex", ComplexDataHandler::tocasa, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_bool", BoolDataHandler::tosakura, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_bool", BoolDataHandler::tocasa, METH_VARARGS, "convert sakura array to casa array"},
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
