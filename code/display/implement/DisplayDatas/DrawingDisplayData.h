//# DrawingDisplayData.h: interactive drawing DisplayData
//# Copyright (C) 1999,2000,2002
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

#ifndef TRIALDISPLAY_DRAWINGDISPLAYDATA_H
#define TRIALDISPLAY_DRAWINGDISPLAYDATA_H

#include <casa/aips.h>
#include <casa/Containers/List.h>
#include <display/Display/DisplayEnums.h>
#include <display/DisplayDatas/PassiveCachingDD.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WorldCanvas;
class WorldCanvasHolder;
class AttributeBuffer;
class DrawingDisplayMethod;
class DDDObject;

// <summary>
// DisplayData which provides interactive drawing capabilities.
// </summary>

// <synopsis>
// This class provides an implementation of a PassiveCachingDD which
// provides the ability for the programmer to add various objects
// to the display (eg. rectangles), and then modify them, or allow
// the user to interactively modify the objects.
// </synopsis>

class DrawingDisplayData : public PassiveCachingDD {

 public:

  // Constructor.
  DrawingDisplayData(const Display::KeySym keysym = 
		     Display::K_Pointer_Button1);

  // Destructor.
  virtual ~DrawingDisplayData();

  // Install the default options for this DisplayData.
  virtual void setDefaultOptions();

  // Apply options stored in <src>rec</src> to the DisplayData.  A
  // return value of <src>True</src> means a refresh is needed.
  // <src>recOut</src> contains any fields which were implicitly 
  // changed as a result of the call to this function.
  virtual Bool setOptions(Record &rec, Record &recOut);

  // Retrieve the current and default options and parameter types.
  virtual Record getOptions();

  // Return the type of this DisplayData.
  virtual Display::DisplayDataType classType()
    { return Display::CanvasAnnotation; }

  // Create a new AxesDisplayMethod for drawing on the given
  // WorldCanvas when the AttributeBuffers are suitably matched to the
  // current state of this DisplayData and of the WorldCanvas/Holder.
  // The tag is a unique number used to identify the age of the newly
  // constructed CachingDisplayMethod.
  virtual CachingDisplayMethod *newDisplayMethod(WorldCanvas *worldCanvas,
				 AttributeBuffer *wchAttributes,
				 AttributeBuffer *ddAttributes,
						 CachingDisplayData *dd);
 
  // Return the current options of this DisplayData as an
  // AttributeBuffer.
  virtual AttributeBuffer optionsAsAttributes();

  virtual void refreshEH(const WCRefreshEvent &ev);

  // Add an object to the drawing as described in the given Record.
  virtual void addObject(const Record &description);

  // Get the description of the object with the given id.
  virtual Record description(const Int objectID);

  // Set the description of the object with the given id.
  virtual void setDescription(const Int objectID, const Record &rec);

  // Remove the object with the given id.
  virtual void removeObject(const Int objectID);

  // Set the handle state of the particular item.
  virtual void setHandleState(DDDObject *item, const Bool state);

  // Return the current setting of the labelposition option.
  virtual String labelPosition()
    { return itsOptionsLabelPosition; }
  
  // Set/get which key to catch.
  // <group>
  virtual void setKey(const Display::KeySym &keysym);
  virtual Display::KeySym key() const
    { return itsKeySym; }
  virtual Display::KeyModifier keyModifier() const
    { return itsKeyModifier; }
  // </group>

  // Double click interval.
  virtual Double doubleClickInterval() const
    { return itsDoubleClickInterval; }

  // Called when a double click occurred for the identified object.
  // Null implementation in this class, derived classes can over-ride.
  virtual void doubleClick(const Int objectID);

 protected:

  // (Required) copy constructor.
  DrawingDisplayData(const DrawingDisplayData &other);

  // (Required) copy assignment.
  void operator=(const DrawingDisplayData &other);

 private:

  friend class DrawingDisplayMethod;

  // The key to handle.
  Display::KeySym itsKeySym;

  // The modifier mask for the key.
  Display::KeyModifier itsKeyModifier;

  // Double click interval (seconds)
  Double itsDoubleClickInterval;

  // List containing the DDDObjects for this DrawingDisplayData.
  List<void *> itsDDDOList;

  // Iterator for itsDDDOList
  ListIter<void *> *itsDDDOListIter;

  // Obvious!
  DDDObject *itsObjectWhichIsShowingHandles;

  // Install the default options for this DisplayData.
  void installDefaultOptions();

  // Option: position for labels
  String itsOptionsLabelPosition;

};


} //# NAMESPACE CASA - END

#endif
