/**
   \file partitionsum.cpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
   Initial version February 2008
   Maintained by ESO since 2013.

*/

#include "partitionsum.hpp"

namespace LibAIR2 {

  PartitionTable::PartitionTable(const part_table_raw & raw):
    raw(raw)
  {
  }

  size_t PartitionTable::findrow(double T) const
  {
    const size_t nrows= raw.shape()[0];
    
    size_t rlow = 0;
    size_t rhigh= nrows;
    
    while  ( rhigh > rlow+1 ) 
    {
      size_t rmid = (rlow+rhigh)/2;
      
      if ( raw[rmid][0] > T )
	rhigh=rmid;
      else
	rlow=rmid;

    } ;
    return rlow;

  }

  double PartitionTable::eval(double T, size_t i) const
  {
    const size_t rlow  = findrow(T);
    const double delta = (raw[ rlow+1][0] - T) / (raw[ rlow+1][0] - raw[ rlow][0] );

    return delta* raw[ rlow][i] + (1.0-delta)* raw[ rlow+1][i];

  }

  

}


