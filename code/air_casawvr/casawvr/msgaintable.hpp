/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file msgaintable.hpp

   Notes on how to create a gain table:

   One approach is to use a  CalSet<Complex>, and add values

   
*/
#ifndef _LIBAIR_CASAWVR_MSGAINTABLE_HPP__
#define _LIBAIR_CASAWVR_MSGAINTABLE_HPP__

#include <set>
#include <string>

#include "msfwds.hpp"

namespace LibAIR2 {

  // Forward declarations
  class ArrayGains;
  class MSSpec;

  /** \brief Write out gains calculated from WVRs to a table
      
      This version writes for all the spws

      \param s Spectral windows and channels to write for

      \param reverse Reverse the sign of correction for this set of
      spw-ids (to work-around temporary ALMA problems)
      
      \param disperse Aplly dispersion correction (currently the
      correction from the default dispersion table will be applied)
   */

  void writeNewGainTbl(const ArrayGains &g,
		       const char *fnameout,
		       const MSSpec &s,
		       std::set<size_t> reverse,
		       bool disperse,
		       const std::string &msname,
		       const std::string &invocation,
		       const std::set<int> &interpolImpossibleAnts);

  /** \brief Add "History" information to a calibration table --
      really just version info about us
   */
  void addCalHistory(const char *fnameout,
		     const std::string &invocation);


}

#endif
