#if     !defined(_DATABLOCK_H)
 
#include "stddef.h"

class DataBlock
{
 public:

  DataBlock(){ blockDataPtr_ = NULL; blockSize_ = 0; }
  DataBlock(short int* blockDataPtr, long int blockSize){ blockDataPtr_= blockDataPtr; blockSize_=blockSize; }
  ~DataBlock();

  short int*  data() {return blockDataPtr_;}
  long int    size() {return blockSize_;}     // in number of short int (units of 16 bits)

 private:
  short int*  blockDataPtr_;
  long  int   blockSize_;

};

#define _DATABLOCK_H
#endif
