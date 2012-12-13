/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file msutils.hpp

   Utilties for dealing with measurement sets and functions to extract
   non-WVR data from them. For handling of WVR data see mswvrdata.hpp
*/
#ifndef _LIBAIR_CASAWVR_MSUTILS_HPP__
#define _LIBAIR_CASAWVR_MSUTILS_HPP__

#include <vector>
#include <string>
#include <map>
#include <set>

#include <boost/bimap.hpp>

#include "msfwds.hpp"

namespace LibAIR {

  /** Channel frequencies for spectral window spw     
   */
  void spwChannelFreq(const casa::MeasurementSet &ms,
		      size_t spw,
		      std::vector<double> &fres);


  /** Retrieve row times, field IDs, and Source IDs
   */
  void fieldIDs(const casa::MeasurementSet &ms,
		std::vector<double> &time,
		std::vector<int> &fieldID,
		std::vector<int> &sourceID,
		const std::vector<size_t> &sortedI
		);

  /** \brief Connection between state_id and the ScanIntent (or
      ObsMode) 
   */
  struct StateIntentMap:
    public std::map<size_t, std::string>
  {

  public:

  };

  std::ostream & operator<<(std::ostream &o, 
			    const StateIntentMap &t);

  /** \brief Load the map between state ids and scan intents from
      measurement set*/
  void scanIntents(const casa::MeasurementSet &ms,
		   StateIntentMap &mi);

  /** Returns the set of State IDs that are known to have the WVR
      looking at the sky
   */
  std::set<size_t> skyStateIDs(const casa::MeasurementSet &ms);

  /// Association between field number and field names
  typedef boost::bimap<size_t, std::string > field_t;
  field_t getFieldNames(const casa::MeasurementSet &ms);


  /// Association between source IDs and source names
  boost::bimap<size_t, std::string > getSourceNames(const casa::MeasurementSet &ms);

  /** Get all fields associated with a particular source name
   */
  std::set<size_t> getSrcFields(const casa::MeasurementSet &ms,
				const std::string &source);

  /** Map from field IDs to Source Ids. One way only as multiple
      fields can correspond to one source (thats the way CASA does
      it).
   */
  std::map<size_t, size_t> getFieldSrcMap(const casa::MeasurementSet &ms);
      


}

#endif
