/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version July 2010
   Maintained by ESO since 2013.

   \file almaopts.hpp

   Retrieval options for ALMA

*/
#ifndef _LIBAIR_APPS_ALMAOPTS_HPP__
#define _LIBAIR_APPS_ALMAOPTS_HPP__

namespace LibAIR2 {

  /** \brief Specifies options available for ALMA basic retrieval
   */
  struct ALMARetOpts {

    /** If true, priors tuned for the OSF will be used
     */
    bool OSFPriors;

    ALMARetOpts(void);
    

  };


}

#endif
