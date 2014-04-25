#ifndef _CASA_SAKURA_ALIGNED_ARRAY_H_
#define _CASA_SAKURA_ALIGNED_ARRAY_H_

#include <iostream>
#include <memory>
#include <string>

//#include <libsakura/sakura.h>

#include <casa/aipstype.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template<typename T>
class SakuraAlignedArray {
public:
  SakuraAlignedArray(size_t num_data);
  SakuraAlignedArray(Vector<T> const &in_vector);
  ~SakuraAlignedArray();
  Vector<T> getAlignedVector();
private:
  void initialize();
  size_t num_data_; // number of data to be stored.
  //std::tr1::shared_ptr<void> storage_; // starting address of allocated memory. (unaligned)
  void *storage_; // starting address of allocated memory. (unaligned)
  T *data_;       // aligned pointer to store data.
};

} //# NAMESPACE CASA - END

#include <casa_sakura/SakuraAlignedArray.tcc>
  
#endif /* _CASA_SAKURA_ALIGNED_ARRAY_H_ */
