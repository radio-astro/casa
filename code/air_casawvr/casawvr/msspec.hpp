/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version June 2010.
   Maintained by ESO since 2013.
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file msspec.hpp 
   
   Tools for dealing with measurement set spectral setups, i.e.,
   spectral windows, frequencies, etc
   
*/
#ifndef _LIBAIR_CASAWVR_MSSPEC__
#define _LIBAIR_CASAWVR_MSSPEC__

#include <ostream>
#include <vector>
#include <map>

#include <ms/MeasurementSets/MeasurementSet.h>

namespace LibAIR2 {

  /// Information about an individual spectral window
  struct MSSPW
  {
    int spwid;
    /// Frequency of each channel in the spectral window
    std::vector<double> chf;
  };

  /// The essential information about the spectral setup that we need
  struct MSSpec {
    /// Spectral windows
    std::vector<MSSPW> spws;
  };

  /** Load the information about the spectral setup from the
      measuremnt set
   */
  void loadSpec(const casa::MeasurementSet &ms,
		const std::vector<int> &spws,
		MSSpec &s);

  std::ostream & 
  operator<<(std::ostream &os,
	     const MSSpec &s);

  /** \brief Create a map from spectral window ID to data description
      ID
      
      \note This mapping may not be unique! 
   */
  std::map<size_t, size_t>
  SPWDataDescMap(const casa::MeasurementSet &ms);

  /** \brief Create a map from data description ID to spectral window
      number
   */
  std::map<size_t, size_t>
  DataDescSPWMap(const casa::MeasurementSet &ms);

  /** \brief Return the SPW of each row in main table
   */
  void dataSPWs(const casa::MeasurementSet &ms,
		std::vector<size_t> &spw,
		const std::vector<size_t> &sortedI);

  /** \brief Total number of SPWs in the MS
   */
  size_t numSPWs(const casa::MeasurementSet &ms);

}

#endif

