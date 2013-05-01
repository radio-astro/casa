//# DDModEvent.h: DisplayData Modified Event used to signal change in data
//# Copyright (C) 1999,2000,2003
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

#ifndef TRIALDISPLAY_DDMODEVENT_H
#define TRIALDISPLAY_DDMODEVENT_H

#include <casa/aips.h>
#include <display/DisplayEvents/DisplayDataEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class used by  DisplayDatas to signal change in data
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// "DisplayData Modified Event" describes "Events" (ie. things which
// happen at a measurable time) which signal to event handlers that
// the data of the transmitting displaydata has been modified.
// </etymology>

// <prerequisite>
// <li> <linkto class=DisplayDataEvent>DisplayDataEvent</linkto>
// </prerequisite>

// <synopsis>
// This class adds to the information stored in the <linkto
// class=DisplayDataEvent>DisplayDataEvent</linkto> class. It adds a
// pointer to a record that holds the modified data. The structure of
// the record is specific to each type of Display Data so the event
// handlers will need to know the structure they are listening for
// </synopsis>

// <example>
// <srcBlock>
// // Sending a DDModEvent from a Profile2dDD (inherits from ActiveCaching2dDD)
// Record rec;
// fillRecordWithData(rec); // DD specific function
// DDModEvent ev(this, &rec);
// ActiveCaching2dDD::handleEvent(ev); // let super classes send events
//
// // Receiving a DDModEvent from a Profile2dDD
// ...
// profile2dDD->addDisplayEventHandler(this);
// ...
// void MyClass::handleEvent(DisplayEvent &ev) {
//   DDModEvent *dev = dynamic_cast<DDModEvent*>(&ev);
//   if (dev) {
//     Profile2dDD *pdd = dynamic_cast<Profile2dDD*>(dev->displayData());
//     if (pdd) {
//     // event from Profile2dDD received!
//     }
//   }
// }
// </srcBlock>
// </example>

// <motivation>
// It's desirable to send some data with DisplayDataEvents that has
// information about the DisplayData and the data that is modified
// within it.
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/10/15">
// None.
// </todo>

class DDModEvent : public DisplayDataEvent {

 public:

  // Constructor, taking a pointer to a DisplayData and
  // a pointer to a data record.
  DDModEvent(DisplayData* dd, const Record *rec);

  // Destructor.
  virtual ~DDModEvent();

  // Return a pointer to the data record.
  virtual const Record *dataRecord() const
    { return itsRec; }

 protected:

  // (Required) default constructor.
  DDModEvent();

  // (Required) copy constructor.
  DDModEvent(const DDModEvent &other);
  
  // (Required) copy assignment.
  DDModEvent &operator=(const DDModEvent &other);

 private:

  // the data record
  const Record *itsRec;

};


} //# NAMESPACE CASA - END

#endif
