#include <cassert>
#include <iostream>

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/helpers/exception.h>

#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>

namespace casa {
template<typename T>
inline SakuraAlignedArray<T>::SakuraAlignedArray(size_t num_data) : 
  num_data_(num_data) {
  LogIO logger(LogOrigin("SakuraAlignedArray", "SakuraAlignedArray", WHERE));
  logger << LogIO::DEBUGGING << "Constructing SakuraAlignedArray..." << LogIO::POST;

  initialize();

  logger << LogIO::DEBUGGING << "  Initial Address = " << storage_ << LogIO::POST;
  logger << LogIO::DEBUGGING << "  Aligned Address = " << data_ << LogIO::POST;
}

template<typename T>
inline SakuraAlignedArray<T>::SakuraAlignedArray(Vector<T> const &in_vector) : 
  num_data_(in_vector.nelements()) {
  LogIO logger(LogOrigin("SakuraAlignedArray", "SakuraAlignedArray", WHERE));
  logger << LogIO::DEBUGGING << "Constructing SakuraAlignedArray..." << LogIO::POST;

  initialize();

  T *ptr = data_;
  for (size_t i = 0; i < num_data_; ++i) {
    ptr[i] = in_vector(i);
  }

  logger << LogIO::DEBUGGING << "  Initial Address = " << storage_ << LogIO::POST;
  logger << LogIO::DEBUGGING << "  Aligned Address = " << data_ << LogIO::POST;
}

template<typename T>
inline void SakuraAlignedArray<T>::initialize() {
  storage_ = NULL;
  data_ = NULL;

  size_t size_required = sizeof(T) * num_data_;
  size_t size_of_arena = size_required + LIBSAKURA_SYMBOL(GetAlignment)() - 1;
  storage_ = malloc(size_of_arena);
  if (storage_ == NULL) {
    data_ = NULL;
    throw std::bad_alloc();
  }
  data_ = reinterpret_cast<T *>(LIBSAKURA_SYMBOL(AlignAny)(
				size_of_arena, storage_, size_required));
  assert(data_ != NULL);
  assert(LIBSAKURA_SYMBOL(IsAligned)(data_));
}

template<typename T>
inline SakuraAlignedArray<T>::~SakuraAlignedArray() {
  LogIO logger(LogOrigin("SakuraAlignedArray", "~SakuraAlignedArray", WHERE));
  logger << LogIO::DEBUGGING << "Destructing SakuraAlignedArray..." << LogIO::POST;

  free(storage_);
}

template<typename T>
inline Vector<T> SakuraAlignedArray<T>::getAlignedVector() {
  Vector<T> res = Vector<T>(IPosition(1, num_data_), data_, SHARE);
  assert(data_ == res.cbegin());
  return res;
}

}  // End of casa namespace.
