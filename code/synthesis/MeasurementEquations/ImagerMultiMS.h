//# ImagerMultiMS.h: Imager functionality sits here; 
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#
//# $Id$


#ifndef SYNTHESIS_IMAGERMULTIMS_H
#define SYNTHESIS_IMAGERMULTIMS_H

#include <synthesis/MeasurementEquations/Imager.h>

namespace casa {

  class ImagerMultiMS : public Imager
    {

    public: 
      // Default constructor with no ms                                        

      ImagerMultiMS();
      //Copy the sub ms to memory useful when imaging only 
      //a few channels 
      virtual Bool setDataToMemory(const String& msname, const String& mode, 
				   const Vector<Int>& nchan, 
				   const Vector<Int>& start,
				   const Vector<Int>& step,
				   const Vector<Int>& spectralwindowids,
				   const Vector<Int>& fieldids,
				   const String& msSelect,
				   const String& timerng,
				   const String& fieldnames,
				   const Vector<Int>& antIndex,
				   const String& antnames,
				   const String& spwstring,
				   const String& uvdist,
				   const String& scan,
                                   const String& intent="", 
                                   const String& obs="");  // select by obs ID
      // Set the data selection on for each ms seperately
      virtual  Bool setDataPerMS(const String& msname, const String& mode, 
				 const Vector<Int>& nchan, 
				 const Vector<Int>& start,
				 const Vector<Int>& step,
				 const Vector<Int>& spectralwindowids,
				 const Vector<Int>& fieldids,
				 const String& msSelect="",
				 const String& timerng="",
				 const String& fieldnames="",
				 const Vector<Int>& antIndex=Vector<Int>(),
				 const String& antnames="",
				 const String& spwstring="",
				 const String& uvdist="",
                                 const String& scan="",
                                 const String& intent="",
                                 const String& obs="",
                                 const Bool useModel=False,
				 const Bool msreadonly=False);


       // Set image construction parameters
      virtual Bool setimage(const Int nx, const Int ny,
		const Quantity& cellx, const Quantity& celly,
		const String& stokes,
                Bool doShift,
		const MDirection& phaseCenter, 
                const Quantity& shiftx, const Quantity& shifty,
		const String& mode, const Int nchan,
                const Int start, const Int step,
		const MRadialVelocity& mStart, const MRadialVelocity& mStep,
		const Vector<Int>& spectralwindowids, const Int fieldid,
		const Int facets, const Quantity& distance);
  

      Bool selectDataChannel();
      
      // Lock the ms and its subtables
      virtual Bool lock();
      
      // Unlock the ms and its subtables
      virtual Bool unlock();

      //open sub tables
      virtual Bool openSubTables();
      
      // @copydoc Imager::mapExtent()
      virtual Bool mapExtent(const String &referenceFrame, const String &movingSource,
              const String &pointingColumn, Vector<Double> &center, Vector<Double> &blc,
              Vector<Double> &trc, Vector<Double> &extent);

    protected:
      
      Block<Vector<Int> > blockNChan_p;
      Block<Vector<Int> > blockStart_p;
      Block<Vector<Int> > blockStep_p;
      Block<Vector<Int> > blockSpw_p;
      Block<MeasurementSet> blockMSSel_p;
      Bool setDataOnThisMS(MeasurementSet& ms, const String& mode="none", 
			   const Vector<Int>& nchan=Vector<Int>(0), 
			   const Vector<Int>& start=Vector<Int>(1,0),
			   const Vector<Int>& step=Vector<Int>(1,1),
			   const Vector<Int>& spectralwindowids=Vector<Int>(0),
			   const Vector<Int>& fieldids=Vector<Int>(0),
			   const String& msSelect="",
			   const String& timerng="",
			   const String& fieldnames="",
			   const Vector<Int>& antIndex=Vector<Int>(),
			   const String& antnames="",
			   const String& spwstring="",
			   const String& uvdist="",
			   const String& scan="",
                           const String& intent="",
                           const String& obs="");
      Bool dataSet_p;

      
    };
  
} //# NAMESPACE CASA - END


#endif


