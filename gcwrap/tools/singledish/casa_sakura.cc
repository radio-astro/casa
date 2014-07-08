#include <iostream>
#include <iomanip>
#include <exception>
#include <stdexcept>
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

class dataconversion_error : public std::runtime_error
{
public:
  explicit dataconversion_error(const std::string &what_arg)
    : std::runtime_error(what_arg)
  {}
};

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
    // Convert PyObject to casac::variant
    casac::variant v = casac::pyobj2variant(obj, true);

    bool status;
	
    // Convert casac::variant to casa::Array,
    // then casa::Array to sakura_PyAlignedBuffer
    casa::Array<CDataType> arr;
    casa::uInt npol, nchan, nrow;
    std::vector<sakura_PyAlignedBuffer *> buffer_list;
    Py_BEGIN_ALLOW_THREADS
    try {
      status = true;
      arr = ToCasaArray(v, npol, nchan, nrow);

      if (arr.empty()) {
	throw dataconversion_error("");
      }

      status = ReadCasaArray((size_t)npol, (size_t)nchan, (size_t)nrow,
			     arr, buffer_list);
    }
    catch (...) {
      status = false;
    }
    Py_END_ALLOW_THREADS

    if (!status) {
      throw dataconversion_error("Failed to read CASA array");
      return NULL;
    }

    // encapsulate
    PyObject *tuple = EncapsulateChunk((Py_ssize_t)nrow, (Py_ssize_t)npol,
				       buffer_list);

    if (tuple == NULL) {
      throw dataconversion_error("Failed to encapsulate chunk");
      return NULL;
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
      throw dataconversion_error("Failed to decapsulate chunk");
      return NULL;
    }

    // Convert sakura_PyAlignedBuffer to casa::Array,
    // then casa::Array to casac::variant
    casa::Array<CDataType> array;
    casac::variant *v = NULL;
    Py_BEGIN_ALLOW_THREADS
    try {
      status = ReadAlignedBuffer(npol, nchan, nrow, buffer_list, array);

      if (status == true) {
	v = ToVariant(array, npol, nchan, nrow);
      }
    }
    catch (...) {
      status = false;
    }
    Py_END_ALLOW_THREADS

    if (!status) {
      throw dataconversion_error("Failed to convert chunk");
      return NULL;
    }      
    
    return casac::variant2pyobj(*v);
  }
  
private:
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

  static casa::Array<CDataType> ToCasaArray(const casac::variant &v,
					    casa::uInt &npol,
					    casa::uInt &nchan,
					    casa::uInt &nrow)
  {
    casa::PtrHolder<casa::ValueHolder> var(casa::toValueHolder(v));
    casa::Array<CDataType> arr = Handler::AsArray(var.ptr());
    casa::IPosition shape = arr.shape();
    casa::uInt ndim = shape.size();
    if (ndim == 1) {
      npol = 1;
      nchan = shape[0];
      nrow = 1;
    }
    else if (ndim == 2) {
      npol = shape[0];
      nchan = shape[1];
      nrow = 1;
    }
    else if (ndim == 3) {
      npol = shape[0];
      nchan = shape[1];
      nrow = shape[2];
    }
    else {
      assert(false);
      throw dataconversion_error("Input Array is not in expected shape");
    }

    return arr;
  }
  
  static bool ReadCasaArray(const size_t npol,
			    const size_t nchan,
			    const size_t nrow,
			    const casa::Array<CDataType> &array,
			    std::vector<sakura_PyAlignedBuffer *> &buffer_list)
  {
    // resize buffer_list
    buffer_list.resize(npol * nrow);

    casa::Bool b;
    size_t element_size = sizeof(typename Handler::SDataType);
    size_t required_size = element_size * nchan;
    size_t allocation_size = required_size + sakura_GetAlignment() - 1;
    size_t elements[] = {nchan};
    const CDataType *arr_p = array.getStorage(b);
    try {
      for (size_t irow = 0; irow < nrow; ++irow) {
	for (size_t ipol = 0; ipol < npol; ++ipol) {
	  void *storage = malloc_(allocation_size);
	  if (storage == NULL) {
	    return false;
	  }
	  void *aligned = sakura_AlignAny(allocation_size, storage, required_size);
	  sakura_PyAlignedBuffer *buffer;
	  if (sakura_PyAlignedBufferCreate((sakura_PyTypeId)Handler::SakuraPyTypeId,
					   storage, aligned,
					   1, elements, &free_,
					   &buffer) != sakura_Status_kOK) {
	    free_(storage);
	    return false;
	  }
	  buffer_list[npol * irow + ipol] = buffer;
	  Handler::ToSakura(irow * npol * nchan + ipol,
			    npol, nchan, arr_p, aligned);
	}
      }
    }
    catch(...) {
      array.freeStorage(arr_p, b);
      return false;
    }
    array.freeStorage(arr_p, b);
    return true;
  }

  static PyObject *EncapsulateChunk(const Py_ssize_t num_tuples,
				    const Py_ssize_t num_elements,
				    std::vector<sakura_PyAlignedBuffer *> &buffer_list)
  {
    PyObject *tuple = PyTuple_New(num_tuples);
    for (Py_ssize_t i = 0; i < num_tuples; ++i) {
      PyObject *cell = PyTuple_New(num_elements);
      for (Py_ssize_t j = 0; j < num_elements; ++j) {
	PyObject *capsule = NULL;
	Py_ssize_t index = num_elements * i + j;
	if (sakura_PyAlignedBufferEncapsulate(buffer_list[index],
					      &capsule) != sakura_Status_kOK) {
	  Py_DECREF(tuple);
	  Py_DECREF(cell);
	  return NULL;
	}
	PyTuple_SetItem(cell, j, capsule);
      }
      PyTuple_SetItem(tuple, i, cell);
    }
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

template<class Handler>
static PyObject *tosakura(PyObject *self, PyObject *args)
{
  PyObject *obj = ArgAsPyObject(args);

  if (obj == NULL) {
    PyErr_SetString(PyExc_RuntimeError, "Failed to parse args.");
    return NULL;
  }

  PyObject *tuple = NULL;

  try {
    tuple = Handler::CreateChunkForSakura(obj);
  }
  catch (const std::bad_alloc &e) {
    PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory");
    return NULL;
  }
  catch (const casa::AipsError &e) {
    std::string s = "Failed due to CASA related error: " + e.getMesg();
    PyErr_SetString(PyExc_RuntimeError, s.c_str());
    return NULL;
  }
  catch (const dataconversion_error &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
  }
  catch (...) {
    // any exception occurred
    PyErr_SetString(PyExc_RuntimeError, "Failed due to unknown error");
    return NULL;
  }
    
  //RETURN_NONE_IF_NULL(tuple)
  if (tuple == NULL) {
    PyErr_SetString(PyExc_RuntimeError, "Failed due to unknown error. Return value is empty.");
    return NULL;
  }

  return tuple;
}

template<class Handler>
static PyObject *tocasa(PyObject *self, PyObject *args)
{
  PyObject *obj = ArgAsPyObject(args);

  if (obj == NULL) {
    PyErr_SetString(PyExc_RuntimeError, "Failed to parse args.");
    return NULL;
  }

  PyObject *ret = NULL;
  try {
    ret = Handler::CreateChunkForCasa(obj);
  }
  catch (const std::bad_alloc &e) {
    PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory");
    return NULL;
  }
  catch (const casa::AipsError &e) {
    std::string s = "Failed due to CASA related error" + e.getMesg();
    PyErr_SetString(PyExc_RuntimeError, s.c_str());
    return NULL;
  }
  catch (const dataconversion_error &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
  }
  catch (...) {
    // any exception occurred
    PyErr_SetString(PyExc_RuntimeError, "Failed due to unknown error");
    return NULL;
  }
  
  if (ret == NULL) {
    PyErr_SetString(PyExc_RuntimeError, "Failed due to unknown error. Return value is empty.");
    return NULL;
  }

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
