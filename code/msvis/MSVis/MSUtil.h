//# MSUtil.h: Definitions for casacore::MS utilities
//# Copyright (C) 2011
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef MSVIS_MSUTIL_H
#define MSVIS_MSUTIL_H
#include <casa/aips.h>
#include <ms/MeasurementSets/MeasurementSet.h>
namespace casa { //# NAMESPACE CASA - BEGIN

  class MSUtil{

  public:
    //Empty Constructor if needed
    MSUtil();
    //This method gives you the channels (and spws) that will cover the 
    //freq range provided in the frame the user stated. 
    //The conversion from the data frame will be done properly for finding this
    //spw, start, nchan will be resized appropriately
    static void getSpwInFreqRange(casacore::Vector<casacore::Int>& spw, casacore::Vector<casacore::Int>& start,
				  casacore::Vector<casacore::Int>& nchan,
				  const casacore::MeasurementSet& ms, 
				  const casacore::Double freqStart,
				  const casacore::Double freqEnd,
				  const casacore::Double freqStep,
				  const casacore::MFrequency::Types freqframe=casacore::MFrequency::LSRK, 
				  const casacore::Int fieldId=0);
    
    //A version of the above for all the fields in the MS
    static void getSpwInFreqRangeAllFields(casacore::Vector<casacore::Int>& spw, casacore::Vector<casacore::Int>& start,
			  casacore::Vector<casacore::Int>& nchan,
			  const casacore::MeasurementSet& ms,
			  const casacore::Double freqStart,
			  const casacore::Double freqEnd,
			  const casacore::Double freqStep,
			  const casacore::MFrequency::Types freqframe=casacore::MFrequency::LSRK);

    // The following wil provide the range of frequency convered in the frame requested by the spw, channel selection 
 
    static void getFreqRangeInSpw( casacore::Double& freqStart,
			      casacore::Double& freqEnd, 
			      const casacore::Vector<casacore::Int>& spw, 
			      const casacore::Vector<casacore::Int>& start,
			      const casacore::Vector<casacore::Int>& nchan,
			      const casacore::MeasurementSet& ms, 
			      const casacore::MFrequency::Types freqframe=casacore::MFrequency::LSRK,
				   const casacore::Int fieldId=0);
    //Return all the selected SPW types selected in the selected casacore::MS if the input ms
    //is a reference MS. Else it will return all the types in the  SPW table
    static casacore::Vector<casacore::String> getSpectralFrames(casacore::Vector<casacore::MFrequency::Types>& types, const casacore::MeasurementSet& ms);

  private:
    static void rejectConsecutive(const casacore::Vector<casacore::Double>& t, casacore::Vector<casacore::Double>& retval, casacore::Vector<casacore::Int>& indx);

  };
} //# NAMESPACE CASA - END
#endif
