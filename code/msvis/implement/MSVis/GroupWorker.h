//# GroupWorker.h: Base classes for objects that process VisBuffGroups
//# as fed to them by GroupProcessor.
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#ifndef MSVIS_GROUPWORKER_H
#define MSVIS_GROUPWORKER_H

#include <casa/aips.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBufferComponents.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forward decl
class VisBuffGroup;

//<summary>ROGroupWorkers process VisBuffGroups</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisBuffGroup">VisBuffGroup</linkto>
//   <li> <linkto class="GroupProcessor">GroupProcessor</linkto>
// </prerequisite>
//
// <etymology>
// ROGroupWorker works on VisBuffGroups.
// </etymology>
//
//<synopsis>
// This class cannot be directly used, but it defines an interface so that its
// derived classes may be called by
// <linkto class="GroupProcessor">GroupProcessor</linkto>.
//
// The interface used by GroupProcessor is process(VisBuffGroup&), which
// derived classes would define to use or process the given VisBuffGroup.
// Any information that process(VisBuffGroup&) needs which is not included
// in the VisBuffGroup must be given to the derived class before c'ting the
// GroupProcessor.
//</synopsis>
//
//<todo>
// <li> 
//</todo>
class ROGroupWorker
{
public:
  // Create empty ROGroupWorker you can assign to or attach.
  ROGroupWorker() {}

  //// Copy construct
  //ROGroupWorker(const ROGroupWorker& other) {}

  // Destructor
  virtual ~ROGroupWorker() {}

  //// Assignment
  //virtual ROGroupWorker& operator=(const ROGroupWorker& gw) {}

  // // Returns which columns need to be prefetched for process to work.
  virtual const asyncio::PrefetchColumns *prefetchColumns() const;

  // This is where all the work gets done!
  virtual Bool process(VisBuffGroup& vbg) = 0;

protected:
  asyncio::PrefetchColumns prefetchColumns_p;
};

//<summary>A base class for GroupWorkers that can modify their input MS.</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="ROGroupWorker">ROGroupWorker</linkto>
// </prerequisite>
//
// <etymology>
//   Its derived classes work on VisBuffGroups.
// </etymology>
//
//<synopsis>
// This class cannot be directly used, but it provides a starting point for
// derived GroupWorkers.
//</synopsis>
//
//<todo>
// <li> 
//</todo>
class GroupWorker : public ROGroupWorker 
{
public:
  GroupWorker(VisibilityIterator& vi);

  //// Copy construct
  //GroupWorker(const GroupWorker& gw) {}

  // Destructor
  virtual ~GroupWorker() {}

  //// Assignment
  //virtual GroupWorker& operator=(const GroupWorker& gw) {}
protected:
  VisibilityIterator vi_p;
private:
  // Disable default c'tor.
  GroupWorker() {}
};

//<summary>A base class for ROGroupWorkers that write to a new MS.</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="ROGroupWorker">ROGroupWorker</linkto>
// </prerequisite>
//
// <etymology>
//      Its derived classes are ROGroupWorkers that write to a new MS.
// </etymology>
//
//<synopsis>
// This class cannot be directly used, but it provides a starting point for
// derived ROGroupWorkers that write to a new MS.
//</synopsis>
//
//<todo>
// <li> 
//</todo>
class GroupWriteToNewMS : public ROGroupWorker 
{
public:
  GroupWriteToNewMS(MeasurementSet& outms);

  //GroupWriteToNewMS(GroupWriteToNewMS& other);
  virtual ~GroupWriteToNewMS() {}

protected:
  // Writes vb to outms_p, and returns a success estimate.
  Bool write(const VisBuffer& vb);

  MeasurementSet outms_p;

private:
  // Disable default construction.
  GroupWriteToNewMS();
};

} //# NAMESPACE CASA - END

#endif

