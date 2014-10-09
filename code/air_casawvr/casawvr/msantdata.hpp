/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version September 2010. 
   Maintained by ESO since 2013.
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file mswvrdata.hpp

   Extraction of information related to antennas
*/
#ifndef _LIBAIR_CASAWVR_MSANTDATA_HPP__
#define _LIBAIR_CASAWVR_MSANTDATA_HPP__

#include <string>
#include <boost/bimap.hpp>

#include "../src/apps/antennautils.hpp"
#include "msfwds.hpp"

namespace LibAIR2 {

  /** \brief Load positions of all antennas
   */
  void getAntPos(const casa::MeasurementSet &ms,
		 antpos_t &res);

  /// Association between antenna numbers and names
  typedef boost::bimap<size_t, std::string > aname_t;

  aname_t getAName(const casa::MeasurementSet &ms);
  

}

#endif
