/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version November 2010. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file wvrgcalerrors.hpp

   An exception hirerachy for user errors from WVRGCal

*/
#ifndef _LIBAIR_CMDLINE_WVRGCALERRORS__
#define _LIBAIR_CMDLINE_WVRGCALERRORS__

#include <stdexcept>

#include "../casawvr/msantdata.hpp"

namespace LibAIR {

  class WVRUserError:
    public std::runtime_error
  {

  public:
    WVRUserError(const std::string &m):
      std::runtime_error(m)
    {
    }
  };

  class AntIDError:
    public WVRUserError
  {
    std::string msg;
  public:

    /// Error in an antenna name
    AntIDError(const std::string &aname,
	       const aname_t &anames);

    /// Error in antenna number
    AntIDError(int a,
	       const aname_t &anames);
    
    ~AntIDError() throw() {} ;
    
    virtual const char* 
    what() const throw();
  };

  class SPWIDError:
    public WVRUserError
  {
    std::string msg;
  public:

    /// Error in an antenna name
    SPWIDError(int spw,
	       size_t nspws);
    ~SPWIDError() throw() {} ;
    
    virtual const char* 
    what() const throw();
  };

}

#endif
