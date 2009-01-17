//# Qimager.h: Perform non-transparent qimager functions; 
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

#ifndef SYNTHESIS_QIMAGER_H
#define SYNTHESIS_QIMAGER_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <ms/MeasurementSets/MeasurementSet.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class VisSet;

// <summary> Class that contains functions needed for qimager </summary>


class Qimager 
{
 public:
  // Default constructor
  Qimager(MeasurementSet& ms);

  // Copy constructor and assignment operator
  Qimager(const Qimager&);
  Qimager& operator=(const Qimager&);

  // Destructor
  virtual ~Qimager();
  
  // open all the subtables as userNoReadLock
  virtual Bool openSubTables();


  // Lock the ms and its subtables
  virtual Bool lock();

  // Unlock the ms and its subtables
  virtual Bool unlock();


  // Utility function to do channel selection

  virtual Bool selectDataChannel(VisSet& vs, Vector<Int>& spectralwindowids, 
				 String& dataMode, 
				 Vector<Int>& dataNchan, 
				 Vector<Int>& dataStart, Vector<Int>& dataStep,
				 MRadialVelocity& mDataStart, 
				 MRadialVelocity& mDataStep);

 private:
  // Private data
  MeasurementSet* ms_p;
  Table antab_p;
  Table datadesctab_p;
  Table feedtab_p;
  Table fieldtab_p;
  Table obstab_p;
  Table pointingtab_p;
  Table poltab_p;
  Table proctab_p;
  Table spwtab_p;
  Table statetab_p;
  Table dopplertab_p;
  Table flagcmdtab_p;
  Table freqoffsettab_p;
  Table historytab_p;
  Table sourcetab_p;
  Table syscaltab_p;
  Table weathertab_p;
  Int lockCounter_p;


};


} //# NAMESPACE CASA - END

#endif














