#include <cassert>
#include <iostream>

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
  logger << LogIO::DEBUGGING << "  Aligned Address = " << data << LogIO::POST;
}

template<typename T>
inline SakuraAlignedArray<T>::SakuraAlignedArray(Vector<T> const &in_vector) : 
  num_data_(in_vector.nelements()) {
  LogIO logger(LogOrigin("SakuraAlignedArray", "SakuraAlignedArray", WHERE));
  logger << LogIO::DEBUGGING << "Constructing SakuraAlignedArray..." << LogIO::POST;

  initialize();

  T *ptr = data;
  for (size_t i = 0; i < num_data_; ++i) {
    ptr[i] = in_vector(i);
  }

  logger << LogIO::DEBUGGING << "  Initial Address = " << storage_ << LogIO::POST;
  logger << LogIO::DEBUGGING << "  Aligned Address = " << data << LogIO::POST;
}

template<typename T>
inline void SakuraAlignedArray<T>::initialize() {
  storage_ = nullptr;
  data = nullptr;
  casaVector = nullptr;

  size_t size_required = sizeof(T) * num_data_;
  size_t size_of_arena = size_required + LIBSAKURA_SYMBOL(GetAlignment)() - 1;
  storage_ = malloc(size_of_arena);
  if (storage_ == nullptr) {
    data = nullptr;
    throw std::bad_alloc();
  }
  data = reinterpret_cast<T *>(LIBSAKURA_SYMBOL(AlignAny)(
				size_of_arena, storage_, size_required));
  assert(data != nullptr);
  assert(LIBSAKURA_SYMBOL(IsAligned)(data));

  Vector<T> alignedCasaVector = Vector<T>(IPosition(1, num_data_), data, SHARE);
  assert(data == alignedCasaVector.cbegin());
  casaVector = &alignedCasaVector;
}

template<typename T>
inline SakuraAlignedArray<T>::~SakuraAlignedArray() {
  LogIO logger(LogOrigin("SakuraAlignedArray", "~SakuraAlignedArray", WHERE));
  logger << LogIO::DEBUGGING << "Destructing SakuraAlignedArray..." << LogIO::POST;

  free(storage_);
}

}  // End of casa namespace.
