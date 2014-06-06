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

inline void *malloc_(size_t size)
{
  void *p = malloc(size);
  // std::cout << "allocate " << std::hex << p << std::dec << std::endl;
  return p;
}

inline void free_(void *p)
{
  // std::cout << "free " << std::hex << p << std::dec << std::endl;
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

template<class DataType>
inline casa::Array<DataType> ValueHolderToArray(casa::ValueHolder *v)
{
  // default is to convert to casa::Array<casa::Float>
  return v->asArrayFloat();
}

// specialization for casa::Double, casa::Complex, and casa::Bool
#define SPECIALIZE_VALUEHOLDER_TO_ARRAY(dtype) \
template<> \
inline casa::Array<casa::dtype> ValueHolderToArray(casa::ValueHolder *v) \
{ \
 return v->asArray##dtype(); \
}

SPECIALIZE_VALUEHOLDER_TO_ARRAY(Double)
SPECIALIZE_VALUEHOLDER_TO_ARRAY(Complex)
SPECIALIZE_VALUEHOLDER_TO_ARRAY(Bool)

template<class HandlerImpl, class CasaDataType, class SakuraDataType>
class HandlerInterface
{
public:
  typedef CasaDataType CDataType;
  typedef SakuraDataType SDataType;
  
  static casa::Array<CDataType> AsArray(casa::ValueHolder *v)
  {
    return HandlerImpl::AsArrayImpl(v);
  }

  static void ToSakura(const size_t offset, const size_t increment,
		       const size_t num_elements,
		       const CDataType *in_p, void *out_p)
  {
    SDataType *work_p = reinterpret_cast<SDataType *>(out_p);
    for (casa::uInt i = 0; i < num_elements; ++i) {
      HandlerImpl::ConvertToSakura(offset + i * increment, i,
				   in_p, work_p);
    }
  }

  static void ToCasa(const size_t offset, const size_t increment,
		     const size_t num_elements,
		     const void *in_p, CDataType *out_p)
  {
    const SDataType *work_p = reinterpret_cast<const SDataType *>(in_p);
    for (size_t i = 0; i < num_elements; ++i) {
      HandlerImpl::ConvertToCasa(i, offset + i * increment,
				 work_p, out_p);
    }
  }
};

template<class CasaDataType, class SakuraDataType, int TypeId>
class DataHandler : public HandlerInterface<DataHandler<CasaDataType, SakuraDataType, TypeId>, CasaDataType, SakuraDataType>
{
public:
  typedef CasaDataType CDataType;
  typedef SakuraDataType SDataType;
  enum { SakuraPyTypeId = TypeId };  
  
  static casa::Array<CDataType> AsArrayImpl(casa::ValueHolder *v)
  {
    return ValueHolderToArray<CDataType>(v);
  }

  static void ConvertToSakura(const size_t in_index, const size_t out_index,
			      const CDataType *in_p, SDataType *out_p)
  {
    out_p[out_index] = in_p[in_index];
  }

  static void ConvertToCasa(const size_t in_index, const size_t out_index,
			    const SDataType *in_p, CDataType *out_p)
  {
    out_p[out_index] = in_p[in_index];
  }
};

template<>
class DataHandler<casa::Complex, float, sakura_PyTypeId_kFloat> : public HandlerInterface<DataHandler<casa::Complex, float, sakura_PyTypeId_kFloat>, casa::Complex, float>
{
public:
  typedef casa::Complex CDataType;
  typedef float SDataType;
  enum { SakuraPyTypeId = sakura_PyTypeId_kFloat };  
  
  static casa::Array<CDataType> AsArrayImpl(casa::ValueHolder *v)
  {
    return ValueHolderToArray<CDataType>(v);
  }

  static void ConvertToSakura(const size_t in_index, const size_t out_index,
			      const CDataType *in_p, SDataType *out_p)
  {
    out_p[out_index] = in_p[in_index].real();
  }

  static void ConvertToCasa(const size_t in_index, const size_t out_index,
			    const SDataType *in_p, CDataType *out_p)
  {
    out_p[out_index].real() = in_p[in_index];
    out_p[out_index].imag() = 0.0;
  }
};

// DataHandler for boolean data
// NOTE: Boolean values must be inverted during conversion
//       CASA flag: valid is False, invalid is True
//       Sakura mask: valid is True, invalid is False
template<>
class DataHandler<casa::Bool, bool, sakura_PyTypeId_kBool> : public HandlerInterface<DataHandler<casa::Bool, bool, sakura_PyTypeId_kBool>, casa::Bool, bool>
{
public:
  typedef casa::Bool CDataType;
  typedef bool SDataType;
  enum { SakuraPyTypeId = sakura_PyTypeId_kBool };  
  
  static casa::Array<CDataType> AsArrayImpl(casa::ValueHolder *v)
  {
    return ValueHolderToArray<CDataType>(v);
  }

  static void ConvertToSakura(const size_t in_index, const size_t out_index,
			      const CDataType *in_p, SDataType *out_p)
  {
    out_p[out_index] = !(in_p[in_index]);
  }

  static void ConvertToCasa(const size_t in_index, const size_t out_index,
			    const SDataType *in_p, CDataType *out_p)
  {
    out_p[out_index] = !(in_p[in_index]);
  }
};

template <class Handler>
class DataConverter
{
public:
  typedef typename Handler::CDataType CDataType;
  typedef typename Handler::SDataType SDataType;
  enum { SakuraPyTypeId = Handler::SakuraPyTypeId };
  
  static PyObject *CreateChunkForSakura(PyObject *obj)
  {
    casa::PtrHolder<casa::ValueHolder> var(ToValueHolder(obj));
    const casa::Array<CDataType> arr = Handler::AsArray(var.ptr());
    casa::IPosition shape = arr.shape();
    casa::uInt ndim = shape.size();
    PyObject *tuple = NULL;
    if (ndim == 1 || ndim == 2) {
      PyObject *cell = EncapsulateCell(arr);
      if (cell == NULL) {
	return NULL;
      }
      tuple = PyTuple_New(1);
      PyTuple_SetItem(tuple, 0, cell);
    }
    else if (ndim == 3) {
      casa::uInt npol = shape[0];
      casa::uInt nchan = shape[1];
      casa::uInt nrow = shape[2];
      tuple = PyTuple_New((Py_ssize_t)nrow);
      for (casa::uInt irow = 0; irow < nrow; ++irow) {
	casa::IPosition start(3, 0, 0, irow);
	casa::IPosition end(3, npol-1, nchan-1, irow);
	casa::Array<CDataType> arr_slice = arr(start, end);
	arr_slice.removeDegenerate();
	PyObject *slice = EncapsulateCell(arr_slice);
	if (slice == NULL) {
	  Py_DECREF(tuple);
	  return NULL;
	}
	PyTuple_SetItem(tuple, (Py_ssize_t)irow, slice);
      }
    }
    return tuple;
  }

  static PyObject *CreateChunkForCasa(PyObject *obj)
  {
    casa::Array<CDataType> arr;

    if (PyTuple_Check(obj) == 0) {
      return NULL;
    }
    
    Py_ssize_t nrow = PyTuple_Size(obj);
    casa::uInt npol = 0;
    casa::uInt nchan = 0;

    casa::Bool b;
    CDataType *arr_p = NULL;
    for (Py_ssize_t irow = 0; irow < nrow; ++irow) {
      PyObject *tuple = PyTuple_GetItem(obj, irow);
      Py_ssize_t _npol = PyTuple_Size(tuple);
      for (Py_ssize_t ipol = 0; ipol < _npol; ++ipol) {
	PyObject *capsule = PyTuple_GetItem(tuple, ipol);
	sakura_PyAlignedBuffer *buffer;
	if (sakura_PyAlignedBufferDecapsulate(capsule, &buffer) != sakura_Status_kOK) {
	  if (!arr.empty()) {
	    arr.putStorage(arr_p, b);
	  }
	  return NULL;
	}

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
	size_t start_pos = irow * (_npol * elements[0]);
	CDataType *out_p = &arr_p[start_pos];
	Handler::ToCasa(ipol, npol, elements[0], aligned, out_p);
      }
    }
    arr.putStorage(arr_p, b);

    // reform
    // Shape of returned array will be same as input
    casa::Array<CDataType> new_arr;
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
  static PyObject *EncapsulateCell(const casa::Array<CDataType> &arr)
  {
    // arr should be two-dimensional (npol,nchan) or one-dimensional (nchan)
    Py_ssize_t ndim = arr.ndim();
    casa::IPosition shape = arr.shape();
    casa::uInt npol = (ndim == 2) ? shape[0] : 1;
    casa::uInt nchan = (ndim == 2) ? shape[1] : shape[0];
    PyObject *tuple = PyTuple_New((Py_ssize_t)npol);
    casa::Bool b;
    size_t element_size = sizeof(typename Handler::SDataType);
    size_t required_size = element_size * nchan;
    size_t allocation_size = required_size + sakura_GetAlignment() - 1;
    size_t elements[] = {nchan};
    const CDataType *arr_p = arr.getStorage(b);
    for (casa::uInt ipol = 0; ipol < npol; ++ipol) {
      void *storage = malloc_(allocation_size);
      if (storage == NULL) {
	Py_DECREF(tuple);
	return NULL;
      }
      void *aligned = sakura_AlignAny(allocation_size, storage, required_size);
      sakura_PyAlignedBuffer *buffer;
      if (sakura_PyAlignedBufferCreate((sakura_PyTypeId)Handler::SakuraPyTypeId,
				       storage, aligned,
				       1, elements, &free_,
				       &buffer) != sakura_Status_kOK) {
	Py_DECREF(tuple);
	return NULL;
      }
      PyObject *capsule;
      if (sakura_PyAlignedBufferEncapsulate(buffer,
					    &capsule) != sakura_Status_kOK) {
	Py_DECREF(tuple);
	return NULL;
      }
      Handler::ToSakura(ipol, npol, nchan, arr_p, aligned);
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

#define RETURN_NONE_IF_NULL(obj) \
  if ((obj) == NULL) { \
    Py_RETURN_NONE; \
  }

template<class Handler>
static PyObject *tosakura(PyObject *self, PyObject *args)
{
  PyObject *obj = ArgAsPyObject(args);

  RETURN_NONE_IF_NULL(obj)
  
  PyObject *tuple = Handler::CreateChunkForSakura(obj);

  RETURN_NONE_IF_NULL(tuple)
  
  return tuple;
}

template<class Handler>
static PyObject *tocasa(PyObject *self, PyObject *args)
{
  PyObject *obj = ArgAsPyObject(args);

  RETURN_NONE_IF_NULL(obj)
  
  PyObject *ret = Handler::CreateChunkForCasa(obj);

  RETURN_NONE_IF_NULL(ret)
  
  return ret;  
}

static PyMethodDef CasaSakuraMethods[] = {
  {"tosakura_float", tosakura<DataConverter<DataHandler<casa::Float, float, sakura_PyTypeId_kFloat> > >, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_float", tocasa<DataConverter<DataHandler<casa::Float, float, sakura_PyTypeId_kFloat> > >, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_double", tosakura<DataConverter<DataHandler<casa::Double, double, sakura_PyTypeId_kDouble> > >, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_double", tocasa<DataConverter<DataHandler<casa::Double, double, sakura_PyTypeId_kDouble> > >, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_complex", tosakura<DataConverter<DataHandler<casa::Complex, float, sakura_PyTypeId_kFloat> > >, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_complex", tocasa<DataConverter<DataHandler<casa::Complex, float, sakura_PyTypeId_kFloat> > >, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_bool", tosakura<DataConverter<DataHandler<casa::Bool, bool, sakura_PyTypeId_kBool> > >, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_bool", tocasa<DataConverter<DataHandler<casa::Bool, bool, sakura_PyTypeId_kBool> > >, METH_VARARGS, "convert sakura array to casa array"},
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
