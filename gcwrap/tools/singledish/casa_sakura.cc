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

template<class HandlerImpl, class CasaDataType, class SakuraDataType, int TypeId>
class HandlerInterface
{
public:
  typedef CasaDataType CDataType;
  typedef SakuraDataType SDataType;
  enum { SakuraPyTypeId = TypeId };  
  
  static casa::Array<CDataType> AsArray(casa::ValueHolder *v)
  {
    return ValueHolderToArray<CDataType>(v);
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
class DataHandler : public HandlerInterface<DataHandler<CasaDataType, SakuraDataType, TypeId>, CasaDataType, SakuraDataType, TypeId>
{
public:
  typedef HandlerInterface<DataHandler<CasaDataType, SakuraDataType, TypeId>, CasaDataType, SakuraDataType, TypeId> Base;
  typedef typename Base::CDataType CDataType;
  typedef typename Base::SDataType SDataType;
  
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
class DataHandler<casa::Complex, float, sakura_PyTypeId_kFloat> : public HandlerInterface<DataHandler<casa::Complex, float, sakura_PyTypeId_kFloat>, casa::Complex, float, sakura_PyTypeId_kFloat>
{
public:
  typedef HandlerInterface<DataHandler<casa::Complex, float, sakura_PyTypeId_kFloat>, casa::Complex, float, sakura_PyTypeId_kFloat> Base; 
  
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
class DataHandler<casa::Bool, bool, sakura_PyTypeId_kBool> : public HandlerInterface<DataHandler<casa::Bool, bool, sakura_PyTypeId_kBool>, casa::Bool, bool, sakura_PyTypeId_kBool>
{
public:
  typedef HandlerInterface<DataHandler<casa::Bool, bool, sakura_PyTypeId_kBool>, casa::Bool, bool, sakura_PyTypeId_kBool> Base;
  
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
    // decapsulate
    std::vector<sakura_PyAlignedBuffer *> buffer_list;
    casa::uInt nrow, npol, nchan;
    bool status = DecapsulateChunk(obj, buffer_list, npol, nchan, nrow);
    if (!status) {
      // TODO: throw exception
      return NULL;
    }

    // convert sakura_PyAlignedBuffer to casa::Array
    casa::Array<CDataType> array;
    Py_BEGIN_ALLOW_THREADS
    status = ReadAlignedBuffer(npol, nchan, nrow, buffer_list, array);
    Py_END_ALLOW_THREADS

    if (!status) {
      // TODO: throw exception
      return NULL;
    }

    // convert casa::Array to casac::variant
    casac::variant *v = NULL;
    Py_BEGIN_ALLOW_THREADS
    v = ToVariant(array, npol, nchan, nrow);
    Py_END_ALLOW_THREADS
    
    return casac::variant2pyobj(*v);
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

  static bool DecapsulateChunk(PyObject *obj,
			       std::vector<sakura_PyAlignedBuffer *> &buffer_list,
			       casa::uInt &npol,
			       casa::uInt &nchan,
			       casa::uInt &nrow)
  {
    if (PyTuple_Check(obj) == 0 || PyTuple_Size(obj) == 0) {
      return false;
    }
    
    Py_ssize_t num_tuples = PyTuple_Size(obj);

    // Access first element to obtain num_elements (=npol)
    PyObject *first_element = PyTuple_GetItem(obj, 0);
    if (first_element == NULL) {
      // TODO: throw exception
      return false;
    }

    if (PyTuple_Check(first_element) == 0 || PyTuple_Size(first_element) == 0) {
      return false;
    }
    
    Py_ssize_t num_elements = PyTuple_Size(first_element);

    buffer_list.resize((size_t)(num_tuples * num_elements));

    std::vector<sakura_PyAlignedBuffer *>::iterator iter = buffer_list.begin();
    for (Py_ssize_t irow = 0; irow < num_tuples; ++irow) {
      PyObject *tuple = PyTuple_GetItem(obj, irow);
      for (Py_ssize_t ipol = 0; ipol < num_elements; ++ipol) {
	PyObject *capsule = PyTuple_GetItem(tuple, ipol);
	sakura_PyAlignedBuffer *buffer;
	if (sakura_PyAlignedBufferDecapsulate(capsule, &buffer) != sakura_Status_kOK) {
	  return false;
	}

	assert(iter != buffer_list.end());
	
	*iter = buffer;
	iter++;
      }
    }

    // now buffer_list contains sakura_PyAlignedBuffer in order of
    // [(pol0,row0), (pol1,row0), (pol0,row1), (pol1,row1), ...]

    // get elements[0] which is nchan
    size_t elements[1];
    sakura_PyAlignedBufferElements(buffer_list[0], 1, elements);

    // finally, set npol, nchan, and nrow
    npol = (casa::uInt)num_elements;
    nchan = (casa::uInt)elements[0];
    nrow = (casa::uInt)num_tuples;

    return true;
  }

  static bool ReadAlignedBuffer(const casa::uInt npol,
				const casa::uInt nchan,
				const casa::uInt nrow,
				const std::vector<sakura_PyAlignedBuffer *> &buffer_list,
				casa::Array<CDataType> &array)
  {
    // resize array
    array.resize(casa::IPosition(3, npol, nchan, nrow));

    // get raw pointer from array
    casa::Bool b;
    CDataType *array_p = array.getStorage(b);

    try {
      std::vector<sakura_PyAlignedBuffer *>::const_iterator iter = buffer_list.begin();
      for (casa::uInt irow = 0; irow < nrow; ++irow) {
	for (casa::uInt ipol = 0; ipol < npol; ++ipol) {
	  assert(iter != buffer_list.end());

	  void *aligned;
	  sakura_PyAlignedBufferAlignedAddr(*iter, &aligned);
	  size_t elements[1];
	  sakura_PyAlignedBufferElements(*iter, 1, elements);

	  assert(elements[0] == (size_t)nchan);

	  size_t start_pos = (size_t)(irow * npol * nchan);
	  CDataType *out_p = &array_p[start_pos];
	  Handler::ToCasa((size_t)ipol, (size_t)npol, (size_t)nchan,
			  aligned, out_p);

	  iter++;
	}
      }
    }
    catch (...) {
      // finalization
      array.putStorage(array_p, b);
      
      return false;
    }

    // finalization
    array.putStorage(array_p, b);
    
    return true;
  }
  
  static casac::variant *ToVariant(const casa::Array<CDataType> &array,
				   const casa::uInt npol,
				   const casa::uInt nchan,
				   const casa::uInt nrow)
  {
    // reform
    // Shape of returned array will be same as input
    casa::Array<CDataType> reshaped_array;
    if (npol == 1 && nrow == 1) {
      reshaped_array = array.reform(casa::IPosition(1, nchan));
    }
    else if (nrow == 1) {
      reshaped_array = array.reform(casa::IPosition(2, npol, nchan));
    }
    else {
      reshaped_array = array;
    }

    // Array to ValueHolder
    casa::ValueHolder val(reshaped_array);

    // ValueHolder to variant, then return
    return casa::fromValueHolder(val);
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

  PyObject *tuple = NULL;

  try {
    tuple = Handler::CreateChunkForSakura(obj);
  }
  catch (...) {
    // any exception occurred
    Py_RETURN_NONE;
  }
    
  RETURN_NONE_IF_NULL(tuple)

  return tuple;
}

template<class Handler>
static PyObject *tocasa(PyObject *self, PyObject *args)
{
  PyObject *obj = ArgAsPyObject(args);

  RETURN_NONE_IF_NULL(obj)

  PyObject *ret = NULL;
  try {
    ret = Handler::CreateChunkForCasa(obj);
  }
  catch (...) {
    // any exception occurred
    Py_RETURN_NONE;
  }
  
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
