/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version November 2009
   Maintained by ESO since 2013.

   \file almaresults.hpp

*/

#include <iostream>

#include <boost/foreach.hpp>

#include "almaresults.hpp"


namespace LibAIR2 {

  ALMAResBase::ALMAResBase(void)
  {
    ev=-1;
    c=-1;
  }

  ALMAResBase::~ALMAResBase()
  {
  }

  std::ostream &ALMAResBase::header_inline(std::ostream &os) const
  {
    os<<"Evidence"<<"\t"
      <<"PWV"<<"\t"<<"PWV Error"<<"\t";
    for(size_t i=1; i<5; ++i)
      os<<"dT"<<i<<"dL"<<"\t";
    return os;

  }
  std::ostream &ALMAResBase::str_inline(std::ostream &os) const
  {
    os<<ev<<"\t"
      <<c<<"\t"<<c_err<<"\t";
    BOOST_FOREACH(const double &x, dTdL)
      os<<x<<"\t";
    return os;
  }

  std::ostream &ALMAContRes::header_inline(std::ostream &os) const
  {
    os<<"Evidence"<<"\t"
      <<"PWV"<<"\t"<<"PWV Error"<<"\t";
    for(size_t i=1; i<5; ++i)
      os<<"dT"<<i<<"dL"<<"\t";
    os<<"Tau183"<<"\t"
      <<"Tau183 err";    
    return os;

  }

  std::ostream &ALMAContRes::str_inline(std::ostream &os) const
  {
    os<<ev<<"\t"
      <<c<<"\t"<<c_err<<"\t";
    BOOST_FOREACH(const double &x, dTdL)
      os<<x<<"\t";
    os<<tau183<<"\t"
      <<tau183_err;
    return os;
  }

  static void printdTdL(std::ostream &os,
			const ALMAResBase &r)
  {
    os<<" -- Phase Correction Coefficients -- "<<std::endl;
    for(size_t j=0; j<4; ++j)
    {
      os<<"dTdL"<<j<<": "<<r.dTdL[j]
	<<" +/- "<<r.dTdL_err[j]
	<<std::endl;
    }
  }

  std::ostream &operator<<(std::ostream &os,
			   const ALMAResBase &r)
  {
    os<<"Evidence: "<<r.ev<<std::endl
      <<" -- Parameters -- "<<std::endl
      <<"    c: "<<r.c<<" +/- "<<r.c_err<<std::endl;
    printdTdL(os, r);
    return os;
  }

  std::ostream &operator<<(std::ostream &os,
			   const ALMAContRes &r)
  {
    os<<"Evidence: "<<r.ev<<std::endl
      <<" -- Parameters -- "<<std::endl
      <<"    c: "<<r.c<<" +/- "<<r.c_err<<std::endl
      <<"    tau183: "<<r.tau183<<" +/- "<<r.tau183_err<<std::endl;
    printdTdL(os, r);    
    return os;
  }

  std::ostream &operator<<(std::ostream &os,
			   const boost::ptr_list<ALMAResBase> &i)
  {
    i.begin()->header_inline(os)<<std::endl;
    BOOST_FOREACH(const ALMAResBase &x, i)
    {
      x.str_inline(os)<<std::endl;
    }
    return os;
  }


}




