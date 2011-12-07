//# GroupWorker.cc: Partial implementation for base classes of objects that
//# process VisBuffGroups as fed to them by GroupProcessor.
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

#include <msvis/MSVis/GroupWorker.h>
#include <msvis/MSVis/VisBufferComponents.h>
//#include <casa/Exceptions/Error.h>
//#include <casa/Logging/LogIO.h>

namespace casa {

const asyncio::PrefetchColumns *ROGroupWorker::prefetchColumns() const
{
  return &prefetchColumns_p;
}

GroupWorker::GroupWorker(VisibilityIterator& vi) :
  vi_p(vi)
{
}

// GroupWorker& GroupWorker::operator=(const GroupWorker &other)
// {
//   // trivial so far.
//   vi_p = other.vi_p;
//   return *this;
// }

GroupWriteToNewMS::GroupWriteToNewMS(MeasurementSet& outms) :
  outms_p(outms)
{
}

} // end namespace casa
