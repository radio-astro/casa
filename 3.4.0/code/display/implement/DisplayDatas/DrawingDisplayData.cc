//# DrawingDisplayData.cc: interactive drawing DisplayData
//# Copyright (C) 1999,2000,2001,2002
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

#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Containers/Record.h>
#include <casa/Logging/LogIO.h>
#include <casa/System/AipsrcValue.h>
#include <display/DisplayDatas/DrawingDisplayMethod.h>
#include <display/DisplayDatas/DrawingDisplayData.h>
#include <display/DisplayDatas/DDDRectangle.h>
#include <display/DisplayDatas/DDDEllipse.h>
#include <display/DisplayDatas/DDDPolygon.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DrawingDisplayData::DrawingDisplayData(const Display::KeySym keysym) :
  PassiveCachingDD(),
  itsKeySym(keysym),
  itsObjectWhichIsShowingHandles(0) {
  setCaching(False);
  try {
    itsKeyModifier = Display::keyModifierFromKeySym(itsKeySym);
  } catch(AipsError x) {
    if (&x) {
      itsKeyModifier = (Display::KeyModifier)0;
    }
  } 
  AipsrcValue<Double>::find(itsDoubleClickInterval,
			    "display.controls.doubleclickinterval",
			    Double(0.5));
  installDefaultOptions();
  itsDDDOListIter = new ListIter<void *>(&itsDDDOList);
}

DrawingDisplayData::~DrawingDisplayData() {
  delete itsDDDOListIter;
}

void DrawingDisplayData::setDefaultOptions() {
  PassiveCachingDD::setDefaultOptions();
  installDefaultOptions();
}

Bool DrawingDisplayData::setOptions(Record &rec, Record &recOut) {
  Bool ret = PassiveCachingDD::setOptions(rec, recOut);
  Bool localchange = False, error;

  localchange = (readOptionRecord(itsOptionsLabelPosition, error, rec, 
					"labelposition") || localchange);

  ret = (ret || localchange);
  return ret;
}

Record DrawingDisplayData::getOptions() {
  Record rec = PassiveCachingDD::getOptions();

  Record labelposition;
  labelposition.define("dlformat", "labelposition");
  labelposition.define("listname", "Label position");
  labelposition.define("ptype", "choice");
  Vector<String> vlabelposition(2);
  vlabelposition(0) = "none";
  vlabelposition(1) = "centre";
  labelposition.define("popt", vlabelposition);
  labelposition.define("default", "none");
  labelposition.define("value", itsOptionsLabelPosition);
  labelposition.define("allowunset", False);
  rec.defineRecord("labelposition", labelposition);

  return rec;
}

CachingDisplayMethod *DrawingDisplayData::newDisplayMethod(
    WorldCanvas *worldCanvas,
    AttributeBuffer *wchAttributes,
    AttributeBuffer *ddAttributes,
    CachingDisplayData *dd) {
  return new DrawingDisplayMethod(worldCanvas, wchAttributes, 
				  ddAttributes, dd);
}

AttributeBuffer DrawingDisplayData::optionsAsAttributes() {
  AttributeBuffer buffer = PassiveCachingDD::optionsAsAttributes();
  return buffer;
}
void DrawingDisplayData::refreshEH(const WCRefreshEvent &ev) {
  itsDDDOListIter->toStart();
  DDDObject* temp= 0;
  while (!itsDDDOListIter->atEnd()) {
    temp = (DDDObject *)itsDDDOListIter->getRight();
    temp->operator()(ev);
    (*itsDDDOListIter)++;
  }
  PassiveCachingDD::refreshEH(ev);
}

void DrawingDisplayData::addObject(const Record &description) {
  // YTBI: need to check that no existing object in the list has the
  // same id as this new one
  if (!description.isDefined("type")) {
    throw(AipsError("No 'type' field in object description Record"));
  }
  String type;
  description.get("type", type);
  DDDObject* dddObject= 0;
  if (type == "rectangle") {
    dddObject = new DDDRectangle(description, this);
  } else if (type == "ellipse"){
    dddObject = new DDDEllipse(description, this);
  } else if (type == "polygon"){
    dddObject = new DDDPolygon(description, this);
  } else {
    throw(AipsError("Unknown 'type' field in object description Record"));
  }

  // preferentially add to start of list for fast access to most
  // recently added object
  itsDDDOListIter->toStart();
  itsDDDOListIter->addRight((void *)dddObject);

  // install event handlers
  addPositionEventHandler(dddObject);

  // call refresh
  refresh();

}

Record DrawingDisplayData::description(const Int objectID) 
{
  Record rec;
//
  itsDDDOListIter->toStart();
  Bool found = False;
  DDDObject* temp;

  while (!itsDDDOListIter->atEnd() && !found) {
    temp = (DDDObject *)itsDDDOListIter->getRight();
    found = (objectID == temp->objectID());
    if (found) {
      rec = temp->description();
      break;
    }
//
    if (!itsDDDOListIter->atEnd()) {
      (*itsDDDOListIter)++;
    }
  }
  if (!found) {
    throw(AipsError("Couldn't find object with given id"));
  }
//
  return rec;
}

void DrawingDisplayData::setDescription(const Int objectID, 
					const Record &rec) {
  itsDDDOListIter->toStart();
  Bool found = False;
  DDDObject *temp = 0;
  while (!itsDDDOListIter->atEnd() && !found) {
    temp = (DDDObject *)itsDDDOListIter->getRight();
    found = (objectID == temp->objectID());
    if (found) {
      temp->setDescription(rec);
      break;
    }
    (*itsDDDOListIter)++;
  }
  if (!found) {
     LogIO os(LogOrigin("DrawingDisplayDatas", "setDescription(...)",
               WHERE));
     os << LogIO::WARN << "Could not find object with given id" << LogIO::POST;
  }
}

void DrawingDisplayData::removeObject(const Int objectID) {
  itsDDDOListIter->toStart();
  Bool found = False;
  DDDObject *temp = 0;
  while (!itsDDDOListIter->atEnd() && !found) {
    temp = (DDDObject *)itsDDDOListIter->getRight();
    found = (objectID == temp->objectID());
    if (found) {
      itsDDDOListIter->removeRight();
      if (itsObjectWhichIsShowingHandles == temp) {
	itsObjectWhichIsShowingHandles = 0;
      }
      temp->showHandles(False, False); // this removes motion EH      
      removePositionEventHandler(*temp);
      break;
    }
    if (!itsDDDOListIter->atEnd()) {
      (*itsDDDOListIter)++;
    }
  }
  if (!found) {
     LogIO os(LogOrigin("DrawingDisplayDatas", "removeObject(...)",
               WHERE));
     os << LogIO::WARN << "Could not find object with given id" << LogIO::POST;
  } else {
    delete temp;temp=0;
   // call refresh
     refresh();
  }
}

void DrawingDisplayData::setHandleState(DDDObject *item, const Bool state) {
  itsDDDOListIter->toStart();
  Bool found = False;
  DDDObject *temp = 0;
  while (!itsDDDOListIter->atEnd() && !found) {
    temp = (DDDObject *)itsDDDOListIter->getRight();
    found = (temp == item);
    (*itsDDDOListIter)++;
  }
  if (!found) {
    throw(AipsError("Cannot find object in list"));
  } 
  if (state) {
    if (itsObjectWhichIsShowingHandles) {
      // switch other off
      itsObjectWhichIsShowingHandles->showHandles(False, False);
    }
    itsObjectWhichIsShowingHandles = item;
  } else {
    if (itsObjectWhichIsShowingHandles == item) {
      itsObjectWhichIsShowingHandles = 0;
    }
  }
  refresh();
}

// Set the key to catch.
void DrawingDisplayData::setKey(const Display::KeySym &keysym) {
  itsKeySym = keysym;
  try {
    itsKeyModifier = Display::keyModifierFromKeySym(itsKeySym);
  } catch (AipsError x) {
    if (&x) {
      itsKeyModifier = (Display::KeyModifier)0;
    }
  } 
}

void DrawingDisplayData::doubleClick(const Int objectID) {
  cerr << "double click on object " << objectID << " detected" << endl;
}

DrawingDisplayData::DrawingDisplayData(const DrawingDisplayData &other) {
}

void DrawingDisplayData::operator=(const DrawingDisplayData &other) {
}

void DrawingDisplayData::installDefaultOptions() {
  itsOptionsLabelPosition = "none";
}

} //# NAMESPACE CASA - END

