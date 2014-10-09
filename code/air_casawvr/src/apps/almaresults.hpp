/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version November 2009
   Maintained by ESO since 2013.

   \file almaresults.hpp

*/
#ifndef _LIBAIR_APPS_ALMARESULT_HPP__
#define _LIBAIR_APPS_ALMARESULT_HPP__

#include <vector>
#include <iosfwd>
#include <boost/ptr_container/ptr_list.hpp>

#include "alma_datastruct.h"

namespace LibAIR2 {

  /** \brief Definition of quantities to be typically retrieved from
      ALMA analysis
  */
  struct ALMAResBase: 
    public ALMARes_Basic 
  {

    /**
       
     */
    ALMAResBase(void);

    virtual ~ALMAResBase();
    
    /**\brief Output the field names when doing str_inline

     */
    virtual std::ostream &header_inline(std::ostream &os) const;

    /**\brief Output the results separated by tabs and on a single line 

       \note Polymorphic as this class is used through pointers
     */
    virtual std::ostream &str_inline(std::ostream &os) const;
    
    
  };

  std::ostream &operator<<(std::ostream &os,
			   const ALMAResBase &r);

  std::ostream &operator<<(std::ostream &os,
			   const boost::ptr_list<ALMAResBase> &i);

  struct ALMAContRes:
    public ALMAResBase {
    
    /** \brief Estimate of non-water vapour continuum opacity at 183
       ghz
    */
    double tau183;

    double tau183_err;

    virtual std::ostream &header_inline(std::ostream &os) const;
    virtual std::ostream &str_inline(std::ostream &os) const;
  };

  std::ostream &operator<<(std::ostream &os,
			   const ALMAContRes &r);
    


}

#endif


