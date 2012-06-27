/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version November 2010. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file wvrgcalerrors.cpp

*/

#include <sstream>

#include "wvrgcalerrors.hpp"

namespace LibAIR {

  AntIDError::AntIDError(const std::string &aname,
			 const aname_t &anames):
    WVRUserError("Antenna name not recognised")
  {
    std::ostringstream msgb;
    msgb<<"Error: Antenna name "<<aname<<" does not appear in the antenna table"<<std::endl
	<<"These are the known antenna names: "<<std::endl;
    for (aname_t::right_const_iterator i=anames.right.begin();
	 i!=anames.right.end();
	 ++i)
    {
      std::string a=i->first;
      msgb<<a<<", ";
    }
    msg=msgb.str();
  }

  AntIDError::AntIDError(int a,
			 const aname_t &anames):
    WVRUserError("Antenna number not recognised")
  {
    std::ostringstream msgb;
    msgb<<"Antenna number "<<a<<" does not appear in the antenna table"<<std::endl
	<<"These are the known antenna numbers: "<<std::endl;
    for (aname_t::left_const_iterator i=anames.left.begin();
	 i!=anames.left.end();
	 ++i)
    {
      int ca=i->first;
      msgb<<ca<<", ";
    }
    msg=msgb.str();
    
  }
    

  const char *AntIDError::what() const throw()
  {
    return msg.c_str();
  }

  SPWIDError::SPWIDError(int spw,
			 size_t nspws):
    WVRUserError("SPW number not recognised")
  {
    std::ostringstream msgb;
    msgb<<"SPW number "<<spw<<" is not reckognised"<<std::endl
	<<"I am expecting numbers between 0 and "<<nspws-1<<std::endl;
    msg=msgb.str();
  }
  
  const char *SPWIDError::what() const throw()
  {
    return msg.c_str();
  }

}


