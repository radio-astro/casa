//# DisplayDataEvent.h: base class for events sent by DisplayDatas
//# Copyright (C) 2003
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
//# $Id$

#ifndef TRIALDISPLAY_DISPLAYDATAEVENT_H
#define TRIALDISPLAY_DISPLAYDATAEVENT_H

#include <casa/aips.h>
#include <display/DisplayEvents/DisplayEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class DisplayData;

// <summary>
// Class for events sent by DisplayDatas
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// "DisplayDataEvents" describes "Events" (ie. things which happen at a 
// measurable time) which are sent by DisplayDatas.
// </etymology>

// <prerequisite>
// <li> <linkto class=DisplayEvent>DisplayEvent</linkto>
// </prerequisite>

// <synopsis>
// This class adds to the information stored in the <linkto
// class=DisplayEvent>DisplayEvent</linkto> class.  It adds a pointer
// to the DisplayData itself.
// </synopsis>

// <example>
// </example>

// <motivation>
// There was a need for events that can be sent by display datas to
// signal modifications. DisplayEHs can listen to these events from
// any display data that transmits DisplayDataEvents
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/10/15">
// None.
// </todo>

class DisplayDataEvent : public DisplayEvent {

 public:

  // Constructor, taking a pointer to a constant DD.
  DisplayDataEvent(DisplayData* dd);

  // Destructor.
  virtual ~DisplayDataEvent();

  // Return a pointer to the DD that generated the event.
  virtual DisplayData *displayData() const {
    return itsDisplayData;
  }

 protected:

  // (Required) default constructor.
  DisplayDataEvent();

  // (Required) copy constructor.
  DisplayDataEvent(const DisplayDataEvent &other);
  
  // (Required) copy assignment.
  DisplayDataEvent &operator=(const DisplayDataEvent &other);

 private:

  // Store the DisplayData of the event here at construction.
  DisplayData *itsDisplayData;

};


} //# NAMESPACE CASA - END

#endif
