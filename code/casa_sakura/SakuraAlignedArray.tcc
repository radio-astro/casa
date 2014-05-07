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
SakuraAlignedArray<T>::SakuraAlignedArray(size_t num_data) : 
  num_data_(num_data) {
  LogIO logger(LogOrigin("SakuraAlignedArray", "SakuraAlignedArray", WHERE));
  logger << LogIO::DEBUGGING << "Constructing SakuraAlignedArray..." << LogIO::POST;

  initialize();

  logger << LogIO::DEBUGGING << "Start Address [" << storage_ << "]" << LogIO::POST;
  logger << LogIO::DEBUGGING << "Aligned Address [" << data_ << "]" << LogIO::POST;
}

template<typename T>
SakuraAlignedArray<T>::SakuraAlignedArray(Vector<T> const &in_vector) : 
  num_data_(in_vector.nelements()) {
  LogIO logger(LogOrigin("SakuraAlignedArray", "SakuraAlignedArray", WHERE));
  logger << LogIO::DEBUGGING << "Constructing SakuraAlignedArray..." << LogIO::POST;

  initialize();

  T *ptr = data_;
  for (size_t i = 0; i < num_data_; ++i) {
    *ptr = in_vector(i);
    ptr++;
  }

  logger << LogIO::DEBUGGING << "Start Address [" << storage_ << "]" << LogIO::POST;
  logger << LogIO::DEBUGGING << "Aligned Address [" << data_ << "]" << LogIO::POST;
}

template<typename T>
void SakuraAlignedArray<T>::initialize() {
  size_t size_required = sizeof(T) * num_data_;
  size_t size_of_arena = size_required + LIBSAKURA_SYMBOL(GetAlignment)() - 1;
  storage_ = malloc(size_of_arena);

  data_ = (T *)LIBSAKURA_SYMBOL(AlignAny)(size_of_arena, storage_, size_required);
  assert(LIBSAKURA_SYMBOL(IsAligned)(data_));
}

template<typename T> SakuraAlignedArray<T>::~SakuraAlignedArray() {
  LogIO logger(LogOrigin("SakuraAlignedArray", "~SakuraAlignedArray", WHERE));
  logger << LogIO::DEBUGGING << "Destructing SakuraAlignedArray..." << LogIO::POST;

  free(storage_);
}

template<typename T>
Vector<T> SakuraAlignedArray<T>::getAlignedVector() {
  return Vector<T>(IPosition(1, num_data_), data_, SHARE);
}

}  // End of casa namespace.
