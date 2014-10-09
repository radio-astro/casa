/**
   \file slice.cpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

*/

#include "slice.hpp"

#include <cmath>

#include "columns.hpp"


namespace LibAIR2 {

  Slice::Slice( double T , double P,
		double scale):
    T(T), P(P),
    scale(scale)
  {

  }

  void Slice::AddColumn (const Column & c)
  {
    cols.push_back(& c);
  }

  void Slice::ComputeTx (const std::vector<double> & f,
			 std::vector<double> & res) const 
  {
    res.resize(f.size());
    
    std::vector<double> scratch( f.size() , 0.0 );
    std::vector<double> total  ( f.size() , 0.0 );
    for ( size_t cn =0 ; cn < cols.size() ; ++cn)
    {
      cols[cn]->ComputeTau(f, *this, scratch );
      for (size_t i =0 ; i < f.size() ; ++i )
      {
	total[i] += (scratch[i]*scale);
      }
    }
    
    for (size_t i =0 ; i < f.size() ; ++i )
    {
      res[i] = exp( -1.0 * total[i] );
    }
  }

  OpaqueSlice::OpaqueSlice( double T , double P):
    Slice(T,P)
  {
  }

  void OpaqueSlice::ComputeTx (const std::vector<double> & f,
			       std::vector<double> & res) const   
  {
    res.resize(f.size());
    for (size_t i =0 ; i < f.size() ; ++i )
    {
      res[i] = 0;
    }
  }



}


