#include <iostream>
#include <iomanip>
#include <exception>
#include <stdexcept>
#include <Python.h>
#include <numpy/arrayobject.h>

#include <casa/Containers/ValueHolder.h>
#include <casa/Utilities/PtrHolder.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayIO.h>

#include <libsakura/sakura.h>
#include <libsakura/sakura-python.h>

#ifdef PYTHON_NO_RELEASE_GIL
#define CASA_BEGIN_ALLOW_THREADS {
#define CASA_END_ALLOW_THREADS }
#else
#define CASA_BEGIN_ALLOW_THREADS Py_BEGIN_ALLOW_THREADS
#define CASA_END_ALLOW_THREADS Py_END_ALLOW_THREADS
#endif

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

template<class HandlerImpl, class SakuraDataType, class NumpyDataType, int NumpyTypeId, int SakuraTypeId>
class HandlerInterface
{
public:
  typedef SakuraDataType SDataType;
  typedef NumpyDataType PDataType;
  enum { PythonTypeId = NumpyTypeId };
  enum { SakuraPyTypeId = SakuraTypeId };  
  
  static void ToSakura(const size_t offset, const size_t increment,
		       const size_t num_elements,
		       const PDataType *in_p, void *out_p)
  {
    SDataType *work_p = reinterpret_cast<SDataType *>(out_p);
    for (size_t i = 0; i < num_elements; ++i) {
      HandlerImpl::ConvertToSakura(offset + i * increment, i,
				   in_p, work_p);
    }
  }

  static void ToCasa(const size_t offset, const size_t increment,
		     const size_t num_elements,
		     const void *in_p, PDataType *out_p)
  {
    const SDataType *work_p = reinterpret_cast<const SDataType *>(in_p);
    for (size_t i = 0; i < num_elements; ++i) {
      HandlerImpl::ConvertToCasa(i, offset + i * increment,
				 work_p, out_p);
    }
  }
};

template<class SakuraDataType, class NumpyDataType, int NumpyTypeId, int SakuraTypeId>
class DataHandler : public HandlerInterface<DataHandler<SakuraDataType, NumpyDataType, NumpyTypeId, SakuraTypeId>, SakuraDataType, NumpyDataType, NumpyTypeId, SakuraTypeId>
{
public:
  typedef HandlerInterface<DataHandler<SakuraDataType, NumpyDataType, NumpyTypeId, SakuraTypeId>, SakuraDataType, NumpyDataType, NumpyTypeId, SakuraTypeId> Base;
  typedef typename Base::SDataType SDataType;
  typedef typename Base::PDataType PDataType;
  
  static void ConvertToSakura(const size_t in_index, const size_t out_index,
			      const PDataType *in_p, SDataType *out_p)
  {
    out_p[out_index] = in_p[in_index];
  }

  static void ConvertToCasa(const size_t in_index, const size_t out_index,
			    const SDataType *in_p, PDataType *out_p)
  {
    out_p[out_index] = in_p[in_index];
  }
};

template<>
class DataHandler<float, npy_cdouble, NPY_CDOUBLE, sakura_PyTypeId_kFloat> : public HandlerInterface<DataHandler<float, npy_cdouble, NPY_CDOUBLE, sakura_PyTypeId_kFloat>, float, npy_cdouble, NPY_CDOUBLE, sakura_PyTypeId_kFloat>
{
public:
  typedef HandlerInterface<DataHandler<float, npy_cdouble, NPY_CDOUBLE, sakura_PyTypeId_kFloat>, float, npy_cdouble, NPY_CDOUBLE, sakura_PyTypeId_kFloat> Base; 
  
  static void ConvertToSakura(const size_t in_index, const size_t out_index,
			      const PDataType *in_p, SDataType *out_p)
  {
    out_p[out_index] = in_p[in_index].real;
  }

  static void ConvertToCasa(const size_t in_index, const size_t out_index,
			    const SDataType *in_p, PDataType *out_p)
  {
    out_p[out_index].real = in_p[in_index];
    out_p[out_index].imag = 0.0;
  }
};

// DataHandler for boolean data
// NOTE: Boolean values must be inverted during conversion
//       CASA flag: valid is False, invalid is True
//       Sakura mask: valid is True, invalid is False
template<>
class DataHandler<bool, npy_bool, NPY_BOOL, sakura_PyTypeId_kBool> : public HandlerInterface<DataHandler<bool, npy_bool, NPY_BOOL, sakura_PyTypeId_kBool>, bool, npy_bool, NPY_BOOL, sakura_PyTypeId_kBool>
{
public:
  typedef HandlerInterface<DataHandler<bool, npy_bool, NPY_BOOL, sakura_PyTypeId_kBool>, bool, npy_bool, NPY_BOOL, sakura_PyTypeId_kBool> Base;
  
  static void ConvertToSakura(const size_t in_index, const size_t out_index,
			      const PDataType *in_p, SDataType *out_p)
  {
    out_p[out_index] = !(in_p[in_index]);
  }

  static void ConvertToCasa(const size_t in_index, const size_t out_index,
			    const SDataType *in_p, PDataType *out_p)
  {
    out_p[out_index] = !(in_p[in_index]);
  }
};

template <class Handler>
class DataConverter
{
public:
  typedef typename Handler::SDataType SDataType;
  typedef typename Handler::PDataType PDataType;
  
  static PyObject *CreateChunkForSakura(PyObject *obj)
  {
    // Check if input PyObject is compatible with numpy array
    if (!PyArray_Check(obj)) {
      throw dataconversion_error("Input is not a numpy array");
      return NULL;
    }

    // array dimension
    int ndim = PyArray_NDIM(obj);
    npy_intp *shape = PyArray_SHAPE((PyArrayObject *)obj);
    const PDataType *pyarray_data = reinterpret_cast<const PDataType *>(PyArray_DATA(obj));

    // Check if array dimension is expected value
    if (ndim == 0 || ndim > 3) {
      throw dataconversion_error("Input array is not an expected shape");
      return NULL;
    }
    
    bool status;
	
    // Convert PyArray object data to sakura_PyAlignedBuffer
    size_t npol, nchan, nrow;
    std::vector<sakura_PyAlignedBuffer *> buffer_list;
    //Py_BEGIN_ALLOW_THREADS
    CASA_BEGIN_ALLOW_THREADS
    try {
      status = true;
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
      else { // ndim == 3
	npol = shape[0];
	nchan = shape[1];
	nrow = shape[2];
      }
      //std::cout << "ndim=" << ndim << ": npol,nchan,nrow = " << npol << "," << nchan << "," << nrow << std::endl;
      status = ReadArray(npol, nchan, nrow,
			 pyarray_data, buffer_list);
    }
    catch (...) {
      status = false;
    }
    //Py_END_ALLOW_THREADS
    CASA_END_ALLOW_THREADS

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
    if (obj == Py_None) {
      throw dataconversion_error("NoneType object is not acceptable");
    }

    // decapsulate
    std::vector<sakura_PyAlignedBuffer *> buffer_list;
    size_t nrow, npol, nchan;
    bool status = DecapsulateChunk(obj, buffer_list, npol, nchan, nrow);
    if (!status) {
      throw dataconversion_error("Failed to decapsulate chunk");
      return NULL;
    }

    // storage for returned array
    void *pyarray_data = NULL;
    
    //Py_BEGIN_ALLOW_THREADS
    CASA_BEGIN_ALLOW_THREADS
    try {
      status = true;
      size_t nelements = npol * nchan * nrow;
      pyarray_data = malloc_(sizeof(PDataType) * nelements);
      if (pyarray_data != NULL) {
	PDataType *work_p = reinterpret_cast<PDataType *>(pyarray_data);
	status = ReadAlignedBuffer(npol, nchan, nrow, buffer_list, work_p);
      }
      else {
	status = false;
      }
    }
    catch (...) {
      status = false;
    }
    //Py_END_ALLOW_THREADS
    CASA_END_ALLOW_THREADS

    if (!status) {
      if (pyarray_data != NULL) {
	free_(pyarray_data);
      }
      throw dataconversion_error("Failed to convert chunk");
      return NULL;
    }      

    int ndim;
    npy_intp *shape = NULL;

    // create PyArrayObject
    try {
      if (nrow == 1 && npol == 1) {
	ndim = 1;
	shape = reinterpret_cast<npy_intp *>(malloc_(sizeof(npy_intp) * ndim));
	shape[0] = nchan;
      }
      else if (nrow == 1) {
	ndim = 2;
	shape = reinterpret_cast<npy_intp *>(malloc_(sizeof(npy_intp) * ndim));
	shape[0] = npol;
	shape[1] = nchan;
      }
      else {
	ndim = 3;
	shape = reinterpret_cast<npy_intp *>(malloc_(sizeof(npy_intp) * ndim));
	shape[0] = npol;
	shape[1] = nchan;
	shape[2] = nrow;
      }
    }
    catch (...) {
      if (shape != NULL) {
	free_(shape);
      }
      throw dataconversion_error("Failed to construct array");
      return NULL;
    }

    //std::cout << "ndim=" << ndim << ": npol,nchan,nrow=" << npol << "," << nchan << "," << nrow << std::endl;
    
    PyArray_Descr *pyarray_desc = PyArray_DescrFromType(Handler::PythonTypeId);
    PyObject *pyarray = PyArray_NewFromDescr(&PyArray_Type,
					     pyarray_desc,
					     ndim,
					     shape,
					     NULL, // strides
					     pyarray_data,
					     NPY_ARRAY_F_CONTIGUOUS, // Fortran order
					     NULL);

    free_(shape);

    if (pyarray == NULL) {
      throw dataconversion_error("Failed to create array");
      return NULL;
    }

    if (!PyArray_Check(pyarray)) {
      Py_DECREF(pyarray);
      throw dataconversion_error("Failed to convert chunk");
      return NULL;
    }
    
    return pyarray;
  }
  
private:
  static bool DecapsulateChunk(PyObject *obj,
			       std::vector<sakura_PyAlignedBuffer *> &buffer_list,
			       size_t &npol,
			       size_t &nchan,
			       size_t &nrow)
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
    npol = num_elements;
    nchan = elements[0];
    nrow = num_tuples;

    return true;
  }

  static bool ReadAlignedBuffer(const size_t npol,
				const size_t nchan,
				const size_t nrow,
				const std::vector<sakura_PyAlignedBuffer *> &buffer_list,
				PDataType *array)
  {
    try {
      std::vector<sakura_PyAlignedBuffer *>::const_iterator iter = buffer_list.begin();
      for (size_t irow = 0; irow < nrow; ++irow) {
	for (size_t ipol = 0; ipol < npol; ++ipol) {
	  assert(iter != buffer_list.end());

	  void *aligned;
	  sakura_PyAlignedBufferAlignedAddr(*iter, &aligned);
	  size_t elements[1];
	  sakura_PyAlignedBufferElements(*iter, 1, elements);

	  assert(elements[0] == (size_t)nchan);

	  size_t start_pos = (size_t)(irow * npol * nchan);
	  Handler::ToCasa((size_t)(ipol + start_pos),
			  (size_t)npol, (size_t)nchan,
			  aligned, array);

	  iter++;
	}
      }
    }
    catch (...) {
      return false;
    }

    return true;
  }
  
  static bool ReadArray(const size_t npol,
			const size_t nchan,
			const size_t nrow,
			const PDataType *array,
			std::vector<sakura_PyAlignedBuffer *> &buffer_list)
  {
    // resize buffer_list
    buffer_list.resize(npol * nrow);

    size_t element_size = sizeof(typename Handler::SDataType);
    size_t required_size = element_size * nchan;
    size_t allocation_size = required_size + sakura_GetAlignment() - 1;
    size_t elements[] = {nchan};
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
			    npol, nchan, array, aligned);
	}
      }
    }
    catch(...) {
      return false;
    }
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
    return NULL;
  }
  catch (...) {
    // any exception occurred
    PyErr_SetString(PyExc_RuntimeError, "Failed due to unknown error");
    return NULL;
  }
    
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
    return NULL;
  }
  catch (...) {
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
  {"tosakura_float", tosakura<DataConverter<DataHandler<float, npy_double, NPY_DOUBLE, sakura_PyTypeId_kFloat> > >, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_float", tocasa<DataConverter<DataHandler<float, npy_double, NPY_DOUBLE, sakura_PyTypeId_kFloat> > >, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_double", tosakura<DataConverter<DataHandler<double, npy_double, NPY_DOUBLE, sakura_PyTypeId_kDouble> > >, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_double", tocasa<DataConverter<DataHandler<double, npy_double, NPY_DOUBLE, sakura_PyTypeId_kDouble> > >, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_complex", tosakura<DataConverter<DataHandler<float, npy_cdouble, NPY_CDOUBLE, sakura_PyTypeId_kFloat> > >, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_complex", tocasa<DataConverter<DataHandler<float, npy_cdouble, NPY_CDOUBLE, sakura_PyTypeId_kFloat> > >, METH_VARARGS, "convert sakura array to casa array"},
  {"tosakura_bool", tosakura<DataConverter<DataHandler<bool, npy_bool, NPY_BOOL, sakura_PyTypeId_kBool> > >, METH_VARARGS, "convert casa array to sakura array"},
  {"tocasa_bool", tocasa<DataConverter<DataHandler<bool, npy_bool, NPY_BOOL, sakura_PyTypeId_kBool> > >, METH_VARARGS, "convert sakura array to casa array"},
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

  // initialize numpy
  import_array();
}
