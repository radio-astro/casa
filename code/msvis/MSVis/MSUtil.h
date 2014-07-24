//# MSUtil.h: Definitions for MS utilities
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
    static void getSpwInFreqRange(Vector<Int>& spw, Vector<Int>& start,
				  Vector<Int>& nchan,
				  const MeasurementSet& ms, 
				  const Double freqStart,
				  const Double freqEnd,
				  const Double freqStep,
				  const MFrequency::Types freqframe=MFrequency::LSRK, 
				  const Int fieldId=0);
    
    //A version of the above for all the fields in the MS
    static void getSpwInFreqRangeAllFields(Vector<Int>& spw, Vector<Int>& start,
			  Vector<Int>& nchan,
			  const MeasurementSet& ms,
			  const Double freqStart,
			  const Double freqEnd,
			  const Double freqStep,
			  const MFrequency::Types freqframe=MFrequency::LSRK);

    // The following wil provide the range of frequency convered in the frame requested by the spw, channel selection 
 
    static void getFreqRangeInSpw( Double& freqStart,
			      Double& freqEnd, 
			      const Vector<Int>& spw, 
			      const Vector<Int>& start,
			      const Vector<Int>& nchan,
			      const MeasurementSet& ms, 
			      const MFrequency::Types freqframe=MFrequency::LSRK,
				   const Int fieldId=0);
    //Return all the selected SPW types selected in the selected MS if the input ms
    //is a reference MS. Else it will return all the types in the  SPW table
    static Vector<String> getSpectralFrames(Vector<MFrequency::Types>& types, const MeasurementSet& ms);

  private:
    static void rejectConsecutive(const Vector<Double>& t, Vector<Double>& retval, Vector<Int>& indx);

  };
} //# NAMESPACE CASA - END
#endif
