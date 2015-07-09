/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version November 2010. 
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file wvrgcalerrors.cpp

*/

#include <sstream>
#include <algorithm>
#include "wvrgcalerrors.hpp"


#if __cplusplus < 201103L
struct hack01 {
	void operator()( const std::map<size_t, std::string >::value_type &p ) {
		msgb << p.second << ", ";
	}
	hack01( std::ostringstream &m ) : msgb(m) { }
    std::ostringstream &msgb;
};
#endif
	
namespace LibAIR2 {

  AntIDError::AntIDError(const std::string &aname,
			 const aname_t &anames):
    WVRUserError("Antenna name not recognised")
  {
    std::ostringstream msgb;
    msgb<<"Error: Antenna name "<<aname<<" does not appear in the antenna table"<<std::endl
	<<"These are the known antenna names: "<<std::endl;
	std::for_each( anames.begin( ),
				   anames.end( ),
#if __cplusplus >= 201103L
				   [&]( const aname_t::value_type &p ) {
					   msgb << p.second << ", ";
				   }
#else
				   hack01(msgb)
#endif
 );
    msg=msgb.str();
  }

  AntIDError::AntIDError(int a,
			 const aname_t &anames):
    WVRUserError("Antenna number not recognised")
  {
    std::ostringstream msgb;
    msgb<<"Antenna number "<<a<<" does not appear in the antenna table"<<std::endl
	<<"These are the known antenna numbers: "<<std::endl;
	std::for_each( anames.begin( ),
				   anames.end( ),
#if __cplusplus >= 201103L
				   [&]( const aname_t::value_type &p ) {
					   msgb << p.first << ", ";
				   }
#else
				   hack01(msgb)
#endif
);
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


