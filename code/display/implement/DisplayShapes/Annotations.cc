//# Annotations.cc : Control class for DisplayShapeInterface
//# Copyright (C) 1998,1999,2000,2001,2002,2003
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
//# $Id: 

#include <casa/aips.h>
#include <casa/Exceptions.h>

#include <display/DisplayShapes/Annotations.h>
#include <display/DisplayShapes/DisplayShape.h>

#include <casa/BasicSL/String.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/PanelDisplay.h>

#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayMath.h>

#include <casa/Containers/Block.h>
#include <display/Display/DisplayEnums.h>

#include <casa/Containers/Record.h>
#include <casa/iostream.h>


// The shapes
#include <display/DisplayShapes/DSPixelMarker.h>
#include <display/DisplayShapes/DSWorldMarker.h>
#include <display/DisplayShapes/DSScreenMarker.h>

#include <display/DisplayShapes/DSPixelText.h>
#include <display/DisplayShapes/DSWorldText.h>
#include <display/DisplayShapes/DSScreenText.h>

#include <display/DisplayShapes/DSPixelArrow.h>
#include <display/DisplayShapes/DSWorldArrow.h>
#include <display/DisplayShapes/DSScreenArrow.h>

#include <display/DisplayShapes/DSPixelEllipse.h>
#include <display/DisplayShapes/DSScreenEllipse.h>
#include <display/DisplayShapes/DSWorldEllipse.h>

#include <display/DisplayShapes/DSPixelPolyLine.h>
#include <display/DisplayShapes/DSScreenPolyLine.h>
#include <display/DisplayShapes/DSWorldPolyLine.h>

#include <display/DisplayShapes/DSPixelPoly.h>
#include <display/DisplayShapes/DSScreenPoly.h>
#include <display/DisplayShapes/DSWorldPoly.h>


namespace casa { //# NAMESPACE CASA - BEGIN

//#define SCALE True
Annotations::Annotations(PanelDisplay* panDisp, 
			 const Display::KeySym& keysym,
			 const Bool useEH) :
  //
  itsPanelDisplay(panDisp),
  itsPC(0),
  itsWCLI(0),
  itsShapes(0),
  //

  //
  itsKey(keysym),
  itsUseEH(useEH),
  //

  //
  itsState(Annotations::Nothing),
  itsCreation(Annotations::Normal),
  itsShapeMoved(False),
  itsActiveShape(-1),
  itsActiveHandle(-1),
  itsX(0), itsY(0),
  itsEnabled(True),
  itsRefreshedYet(False)
  //
  
{
  // Ok let's try and get the stuff we need 
  if (itsPanelDisplay) {

    itsPC = itsPanelDisplay->pixelCanvas();

    // Save a copy of the world canvases being used by the paneldisplay
    itsWCLI = new ListIter<WorldCanvas* >(itsWCs);
    
    //itsWCLI->toStart();
    
    itsPanelDisplay->myWCLI->toStart();
    
    while (!itsPanelDisplay->myWCLI->atEnd()) {
      itsWCLI->toEnd();
      itsWCLI->addRight(itsPanelDisplay->myWCLI->getRight());
      itsPanelDisplay->myWCLI->step();
    }

  } else {
    throw (AipsError("Annotations.cc - I got an invalid (null) PanelDisplay!"));
  }
  
  if (!itsPC) throw ("Annotations.cc - PanelDisplay returned a null "
		     "pixelcanvas");


  // Add this class as pixelCanvas everything handlers and worldCanvas 
  // refresh Handlers
  if (itsPC && itsUseEH) {
    itsPC->addRefreshEventHandler(*this);
    itsPC->addMotionEventHandler(*this);
    itsPC->addPositionEventHandler(*this);
  }
  
  if (itsWCLI && itsUseEH) {
    registerToWCs();
  }
  //
}

Annotations::~Annotations() {

  if (itsUseEH && itsPC) {
    itsPC->removeRefreshEventHandler(*this);
    itsPC->removeMotionEventHandler(*this);
    itsPC->removePositionEventHandler(*this);
  }
  
  if (itsUseEH && itsWCLI) {
    if (validateWCs()) {
      itsWCLI->toStart();
      while (!itsWCLI->atEnd()) {
	(itsWCLI->getRight())->removeRefreshEventHandler(*this);
	(*itsWCLI)++;
      }
    } else {
      // I think it's ok to just ignore this and die quietly. 
    }
  }
  delete itsWCLI;
  // Delete any shapes we are looking after
  for (uInt i=0; i<itsShapes.nelements(); i++) {
    delete itsShapes[i]; itsShapes[i] = 0;
  }

}

void Annotations::addLockedToCurrent(const uInt& addMe) {
  if (itsActiveShape!=-1) {
    itsShapes[itsActiveShape]->addLocked(itsShapes[addMe]);
  }
}

void Annotations::removeLockedFromCurrent(const uInt& removeMe) {
 if (itsActiveShape!=-1) {
    itsShapes[itsActiveShape]->removeLocked(itsShapes[removeMe]);
  }
}

void Annotations::setKey(const Display::KeySym& newSym) {
  cout << "setKey=" << newSym << endl;
  itsKey = newSym;
}

void Annotations::handleKeyUp() {
  
  // Does creation of shape require multiple clicks?
  if (itsCreation != Annotations::PolyLine && 
      itsCreation != Annotations::Polygon &&
      itsState != Annotations::Nothing) {
    
    // No, we're done:
    itsState = Annotations::Nothing;
    annotEvent("endchange");
  }
  
}


void Annotations::handleNormalCreation(const Vector<Float>& createPix) {
  cout << "handle normal" << endl;
  try {
    itsShapes[itsActiveShape]->setCenter(createPix[0] , createPix[1]);
  } catch (const AipsError& x) {
    deleteShape(itsActiveShape);
    annotEvent("endchange");
    throw (AipsError("Couldn't create new shape: " + x.getMesg()));
  }
  
  // Set the centre - now a valid shape
  if (itsShapes[itsActiveShape]->whichHandle(createPix[0],
					     createPix[1], 
					     itsActiveHandle)) {
    itsState = Annotations::Handle;
  } else if (itsShapes[itsActiveShape]->inObject(createPix[0], 
						 createPix[1])) {
    // Maybe it was clicked with a certain size
    itsState = Annotations::Move;
    itsX = Int(createPix[0]); itsY = Int(createPix[1]);
  } else {
    
    // Oh dear... have a guess. This could get ugly (shouldn't happen)
    // What has happened is someone has created a new shape with a click,
    // but that click is not on the shape. We just throw:

    itsActiveHandle = -1;
    itsState = Annotations::Nothing;
    throw (AipsError("Annotations.cc - Problem determining initial state"));
  }
}

void Annotations::handleMarkerCreation(const Vector<Float>& createPix) {
  try { 
    itsShapes[itsActiveShape]->setCenter(createPix[0], createPix[1]);
  } catch (const AipsError& x) {
    deleteShape(itsActiveShape);
    annotEvent("endchange");
    throw (AipsError("Couldn't create new shape : " + x.getMesg()));
  }
  itsX = Int(createPix[0]); itsY = Int(createPix[1]);
  itsState = Annotations::Move;
  itsCreation = Annotations::Normal;
  itsActiveHandle = -1;
}

void Annotations::handlePolyLineCreation(const Vector<Float>& createPix) {
  static Bool firstPoint = True;
  static Int lastX, lastY;
  
  if (firstPoint) {
    lastX = Int(createPix[0]);
    lastY = Int(createPix[1]);
    firstPoint = False;

    try {
      itsShapes[itsActiveShape]->addPoint(createPix);
    } catch (const AipsError& x) {
      deleteShape(itsActiveShape);
      annotEvent("endchange");
      throw (AipsError("Couldn't create new shape : " + x.getMesg()));
    }

  } else {
    if (hypot(createPix[0] - lastX, createPix[1] - lastY) > 2) {
      try {
	itsShapes[itsActiveShape]->addPoint(createPix);
      } catch(const AipsError& x) {
	deleteShape(itsActiveShape);
	annotEvent("endchange");
	throw (AipsError("Couldn't create new shape : " + x.getMesg()));
      }
    } else {
      firstPoint = True; // for next time
      
      itsState = Annotations::Nothing;
      annotEvent("endchange");
      itsCreation = Normal;
      
    }
    lastX = Int(createPix[0]);
    lastY = Int(createPix[1]);
  }
  
  itsX = Int(createPix[0]); itsY = Int(createPix[1]);
}

void Annotations::handlePolygonCreation(const Vector<Float>& createPix) {
  static Bool firstPoint = True;
  static Int firstX, firstY;
  
  if (firstPoint) {
    firstX = Int(createPix[0]);
    firstY = Int(createPix[1]);
    firstPoint = False;
    try {
      itsShapes[itsActiveShape]->addPoint(createPix);
    } catch (const AipsError& x) {
      deleteShape(itsActiveShape);
      annotEvent("endchange");
      firstPoint = True;
      throw (AipsError("Couldn't create new shape : " + x.getMesg()));
    }
  } else {
    if (hypot(createPix[0] - firstX, createPix[1] - firstY) > 2) {
      try {
	itsShapes[itsActiveShape]->addPoint(createPix);
      } catch (const AipsError& x) {
	deleteShape(itsActiveShape);
	annotEvent("endchange");
	firstPoint = True;
	throw (AipsError("Couldn't create new shape : " + x.getMesg()));
      }
    } else {
      firstPoint = True; // for next time

      polyLineToPolygon(itsActiveShape);
      annotEvent("endchange");
      itsState = Annotations::Nothing;
      itsCreation = Normal;
    }
  }  
  
  itsX = Int(createPix[0]); itsY = Int(createPix[1]);
}

void Annotations::handleArrowCreation(const Vector<Float>& createPix) {
  try {
    itsShapes[itsActiveShape]->changePoint(createPix, 0);
    itsShapes[itsActiveShape]->changePoint(createPix, 1);
    itsActiveHandle = 1;
    itsState = Annotations::Handle;
    itsCreation = Normal;
  } catch (const AipsError& x) {
    // No choice to but to cancel creation
    deleteShape(itsActiveShape);
    annotEvent("endchange");
    throw (AipsError("Arrow creation failed: " + x.getMesg()));
  }

}

void Annotations::select(const Int i) {
  cout << "select" << endl;
  if (itsActiveShape != -1) {
    itsShapes[itsActiveShape]->setDrawHandles(False);
  }

  if (i < Int(itsShapes.nelements()) && i >= 0) {
    itsActiveShape = i;
    itsShapes[itsActiveShape]->setDrawHandles(True);
  } else itsActiveShape = -1;
  itsShapeMoved = False;
}

void Annotations::handleCreation(const PCPositionEvent& ev) {

  cout << "handle creation" << endl;
  
  Vector<Float> createPix(2); 
  createPix[0] = ev.x(); createPix[1] = ev.y();
  
  switch(itsCreation) {
  case Annotations::Normal :
    // Handle normal / One click creation
    handleNormalCreation(createPix);
    break;
  case Annotations::Marker :
    handleMarkerCreation(createPix);
    break;
  case Annotations::Arrow :
    handleArrowCreation(createPix);
    break;
  case Annotations::PolyLine :
    handlePolyLineCreation(createPix);
    break;
  case Annotations::Polygon :
    handlePolygonCreation(createPix);
    break;
  }
}

Bool Annotations::determineState(const PCPositionEvent& ev) {
  Bool refresh = False;  
  itsActiveHandle = -1;
 cout << "determineState" << endl; 
  if (itsActiveShape != -1 && 
      itsShapes[itsActiveShape]->whichHandle(ev.x(), ev.y(),
					     itsActiveHandle)) {
    if (ev.modifiers() == Display::KM_Ctrl) {
      itsState = Annotations::OtherHandle;
    } else { 
      itsState = Annotations::Handle;
    }
    itsX = ev.x(); itsY = ev.y();
    
    if (itsActiveShape != -1 && itsActiveHandle != -1) {
      return False;
    }
  }
  // Ok we are either starting from scratch, or not on the current shape's
  // handles.
  
 cout << "determineState   -----------1" << endl; 
  if (itsActiveShape != -1 && !itsShapes[itsActiveShape]->inObject(ev.x(),
								   ev.y())) {
    // Current shape no longer valid
    select(-1);
    refresh = True;
  }

  // Find all the objects that we are inside of....
  Bool foundNew = False;
  
 cout << "determineState   -----------2" << endl; 
  for (uInt i=0; i<itsShapes.nelements() ; i++) {
    if (itsActiveShape == -1 && itsShapes[i]->inObject(ev.x(), ev.y())) {
      select(i);
      foundNew = True;
      break;
    } else if (Int(i) > itsActiveShape 
	       && !itsShapeMoved && itsActiveShape != -1 &&
	       itsShapes[i]->inObject(ev.x(), ev.y())) {
      select(i);
      foundNew = True;
      break;
    }
  }
  
 cout << "determineState   -----------3" << endl; 
  if (!foundNew) {
    // We haven't found a new shape, either we are not in a shape, 
    // or the index could be less than the active shape's. Check them
    // all and find the lowest
    for (uInt i=0; i<itsShapes.nelements() ; i++) {

      if (itsShapes[i]->inObject(ev.x(), ev.y()) && 
	  (!itsShapeMoved || itsActiveShape == -1) )  {

	if (Int(i) != itsActiveShape) select(i);
	refresh = True;
	break;
      }

    } 
  } else refresh = True;
  
 cout << "determineState   -----------5" << endl; 
  // Ok, at this point... either itsActiveShape == -1 or
  // it equals the correct shape id.
  if (itsActiveShape != -1) {
    if (itsShapes[itsActiveShape]->whichHandle(ev.x(), ev.y(), 
					       itsActiveHandle)) {
      if (ev.modifiers() == Display::KM_Ctrl) {
	itsState = Annotations::OtherHandle;
      } else itsState = Annotations::Handle;
      itsX = ev.x(); itsY = ev.y();
    } else if (ev.modifiers() == Display::KM_Ctrl) {
      itsState = Annotations::OtherMove;
    } else itsState = Annotations::Move;
    
    itsX = ev.x(); itsY = ev.y();
  } else {
    // Nothing found... check em all for handles
    for (uInt i = 0 ; i<itsShapes.nelements() ; i++) {
      if (itsShapes[i]->whichHandle(ev.x(), ev.y(),
						 itsActiveHandle)) {
	select(i);
	if (ev.modifiers() == Display::KM_Ctrl) {
	  itsState = Annotations::OtherHandle;
	} else itsState = Annotations::Handle;
	itsX = ev.x(); itsY = ev.y();
      }
    }
    if (itsActiveShape == -1) {
      itsState = Annotations::Nothing;
      itsActiveHandle = -1;
    }
  }
 cout << "determineState   -----------6" << endl; 
  return refresh;
}

void Annotations::operator()(const PCPositionEvent& ev) {
cout << "position------" << endl;
  Bool refresh(False);
  cout << "ev.key=" << ev.key() << " itsKey=" << itsKey << endl;
  cout << "itsEnabled=" << itsEnabled << endl;
  cout << "ev.keystate()=" << ev.keystate() << endl;
  cout << "itsState=" << itsState << endl;
  cout << "itsActiveShape=" << itsActiveShape << endl;
  if (ev.key() == itsKey && itsEnabled) {
    
    if (!ev.keystate()) {
      handleKeyUp();
    } else { 
      // Key Down

      itsState = Annotations::Creation;
      // First check if this is building a shape
      if (itsState == Annotations::Creation) { 
	handleCreation(ev);
	refresh = True;
      } else { 
	// Not creation
	if (determineState(ev)) refresh = True;
	itsShapeMoved = False;
      }

    } // END keydown
    
    if (refresh) update(ev.pixelCanvas());
    if (itsState == Annotations::Nothing && ev.keystate()) {
      annotEvent("deselect");
    }
    else if (ev.keystate()) {
      annotEvent("newselection");
    }
    
  } // END if our key
}

void Annotations::registerToWCs() {
  itsWCLI->toStart();
  while (!itsWCLI->atEnd()) {    
    (itsWCLI->getRight())->addRefreshEventHandler(*this);
    (*itsWCLI)++;
  }
}

Int Annotations::activeShape() {
  return itsActiveShape;
}

void Annotations::setShapeOptions(const uInt& toSet, 
				  const Record& newSettings) {
  Bool updateReq = False;
  if (toSet >= itsShapes.nelements() || toSet < 0 ) {
    throw (AipsError("Bad call to setShapeOptions, bad shape index"));
  } else {
    try {
      if (itsShapes[toSet]->setOptions(newSettings)) 
	updateReq = True;
    } catch (const AipsError &x) {
      throw (x);
    }
  }
  if (updateReq) {
    update(itsPC);
  }
}

Record Annotations::getAllOptions() {
  Record temp;

  if (itsShapes.nelements()) {    
    for (uInt i=0; i<itsShapes.nelements(); i++) {
      String current; 
      current = "Shape" + current.toString(i);
      temp.defineRecord(current, getShapeOptions(i));
    }
  }
  
  // Ensure a full copy
  Record toReturn = temp;
  // Don't know why but we have to refresh here, otherwise all
  // eventhandlers seem to be disabled
  itsPanelDisplay->refresh();
  return toReturn;
}


void Annotations::setAllOptions(const Record& newSettings) {
  // Ensure a full copy.
  Record copy = newSettings;

  for (uInt i = 0 ; i < itsShapes.nelements() ; i++) 
    deleteShape(i);

  itsActiveShape = -1;
  itsActiveHandle = -1;

  itsState = Annotations::Nothing;
  
  Record temp;

  for (uInt i=0; i < copy.nfields(); i++) {
    temp = copy.subRecord(i);
    try {
      newShape(temp, False);
    } catch (const AipsError &x) {
      throw (x);
    }
  }
  
  //Turn off all handles ( Should be off anyway ), and redraw:
  cancelShapes();

}



Record Annotations::getShapeOptions(const uInt& toGet) {
  if (toGet >= itsShapes.nelements()) 
    throw (AipsError("Annotations.cc-Error getting shape options, bad index!"));
  
  // Ensure full copy
  Record toReturn;
  Record tmp = itsShapes[toGet]->getOptions();
  toReturn = tmp;
  return toReturn;
}

Record Annotations::shapesSummary() {
  Record summary;
  for (uInt i=0; i<itsShapes.nelements(); i++) {
    
    Record shapeRec;
    if ((itsShapes[i]->getOptions()).isDefined("type"))
      shapeRec.define("type", (itsShapes[i]->getOptions()).asString("type"));
    else {
      shapeRec.define("type", "??? ERROR ???");
    }
    
    String name("Shape");
    name += name.toString(i);
    
    summary.defineRecord(name, shapeRec);

  }
  return summary;
}

void Annotations::operator()(const PCMotionEvent& ev) {
//cout << "motion------" << endl;
  Bool refresh(False);

  if (!itsEnabled) return;

  if (itsState == Annotations::Handle) {

    itsShapeMoved = True;
    Vector<Float> change(2);
    change[0] = ev.x();
    change[1] = ev.y();
    
    itsShapes[itsActiveShape]->changePoint(change, itsActiveHandle);
    refresh = True;
    
  } else if (itsState == Annotations::OtherHandle) {

    itsShapeMoved = True;
    Float rot(0);
    Vector<Float> pixCent(itsShapes[itsActiveShape]->getCenter());
    if (ev.x() > pixCent[0]) {
      rot = Float(((ev.x() - Int(itsX)) + (ev.y() - Int(itsY))));
    }
    else if (ev.x() < pixCent[0]) {
      rot = Float(((Int(itsX) - ev.x()) + (Int(itsY) - ev.y())));
    }
    
    itsShapes[itsActiveShape]->rotate(Float(rot));
    
    itsX = ev.x();  itsY = ev.y();
    refresh= True;

  } else if (itsState == Annotations::Move) {

    itsShapeMoved = True;
    itsShapes[itsActiveShape]->move(Float(ev.x()) - itsX, 
				    Float(ev.y()) - itsY);
    itsX = ev.x();  itsY = ev.y();
    refresh = True;

  } else if (itsState == Annotations::OtherMove) {

    itsShapeMoved = True;
    itsShapes[itsActiveShape]->scale(1 + Float(ev.y() - Int(itsY))/20);
    itsY = ev.y();
    refresh = True;

  } 

  if (refresh) {
    update(ev.pixelCanvas());
  }
}

void Annotations::changedWC() {
  for (uInt i =0 ; i < itsShapes.nelements() ; i++) {
    Record tmp = itsShapes[i]->getOptions();
    if (!tmp.isDefined("coords")) {
      throw (AipsError("Bad option record returned for shape with index : "
		       + i));
    } else {
      if (tmp.asString("coords") == "world") {
	deleteShape(i);
      }
    }
  }
  
  // Reset the list
  itsWCLI->toStart();
  while (!itsWCLI->atEnd()) {
    itsWCLI->removeRight();
  }
  
  itsPanelDisplay->myWCLI->toStart();

  while (!itsPanelDisplay->myWCLI->atEnd()) {
    if (itsUseEH) {
      itsPanelDisplay->myWCLI->getRight()->addRefreshEventHandler(*this);
    }
    itsWCLI->toEnd();
    itsWCLI->addRight(itsPanelDisplay->myWCLI->getRight());

    itsPanelDisplay->myWCLI->step();
    
  }
}


void Annotations::operator()(const PCRefreshEvent& ev) {
  cout << "refresh--------" << endl;
  itsRefreshedYet = True;
  
  if (!validateWCs()) {
    changedWC();
  }
  
  if ( ev.reason() == Display::BackCopiedToFront &&
       ev.pixelCanvas()->drawBuffer()==Display::FrontBuffer ) {
    draw(ev.pixelCanvas());
  } else {
    
    // Recalc all centers
    for (uInt i=0; i<itsShapes.nelements(); i++) {
      itsShapes[i]->recalculateScreenPosition();
    }
    
  } 
}

// TODO: Make these nested.
void Annotations::enable() {
  itsEnabled = True;
}

void Annotations::disable() {
  itsEnabled = False;
}

void Annotations::cancelShapes() {
  // If we are about to make one, kill it else reset
  if (itsState == Annotations::Creation) {
    deleteShape(itsActiveShape);
    itsActiveShape = -1;
    itsActiveHandle = -1;
    itsState = Annotations::Nothing;
  } else {
    itsState = Annotations::Nothing;
    itsActiveShape = -1;
    itsActiveHandle = -1;
  }

  for (uInt i=0; i<itsShapes.nelements(); i++) {
    itsShapes[i]->setDrawHandles(False);
  }
  
  update(itsPC);
}

ostream & operator << (ostream & os, Annotations::State st) {
  switch(st) {
  case Annotations::Nothing : os << "Nothing"; break;
  case Annotations::Creation : os << "Creation"; break;
  case Annotations::WcCreation : os << "WcCreation"; break;
  case Annotations::Handle : os << "Handle"; break;
  case Annotations::OtherHandle : os << "OtherHandle"; break;
  case Annotations::Move : os << "Move"; break;
  case Annotations::OtherMove : os << "OtherMove"; break;
  default: os << "???"; break;
  }
  return os;
}

void Annotations::createShape(Record& settings) {
  cout << "create shape" << endl;
  if (itsState == Annotations::Creation) {
    // User was about to make a shape then changed their mind
    deleteShape(itsActiveShape);
  }
  
  if (!settings.isDefined("coords")) {
    throw (AipsError("What sort of coords am I using? Please define a "
		     "\'coords\' field (\'world\', \'frac\' or \'pix\') "
		     "in the shape "
		     "record"));
  }
  
  String coords = settings.asString("coords");
  

  // Strip all fields which could confuse ( all these will be defined via
  // mouse )
  if (settings.isDefined("center")) {
    settings.removeField("center");
  }

  if (!settings.isDefined("type")) {
    throw (AipsError("No \'type\' specified for creation of new shape"));
  }
  
  // Cater for special cases;
  String type = settings.asString("type");  type.downcase();
  if (type == "arrow") {   // Should become || line
    itsCreation = Annotations::Arrow;
  } else if (type == "polyline") {
    itsCreation = Annotations::PolyLine; 
  } else if (type == "polygon") {
    itsCreation = Annotations::Polygon;
    settings.removeField("type");
    settings.define("type", "polyline");
  } else if (type == "marker") {
    itsCreation = Annotations::Marker;
  } else {
    itsCreation = Annotations::Normal;
  }

  DisplayShape* toCreate = 0;
  // Try and build the shape
  try {
    toCreate = constructShape(settings);
  } catch (const AipsError &x) {
    throw (x);
  }

  // Ok, so....
  
  // Turn off old handles
  if (itsActiveShape != -1) {
    itsShapes[itsActiveShape]->setDrawHandles(False);
  }
  
  uInt n = itsShapes.nelements();
  itsShapes.resize(uInt(n + 1), False, True);
  
  itsState = Annotations::Creation;
  
  // Normal case
  itsShapes[n] = toCreate;
  itsShapes[n]->setDrawHandles(True);
  itsActiveShape = Int(n);
}

Bool Annotations::deleteShape(const uInt& toDel) {

  if (toDel >= itsShapes.nelements() || toDel < 0) {
    throw (AipsError("Annotations.cc - Shape index for deletion out of range"));
  } else {
    
    // Delete shape
    delete itsShapes[toDel];
    itsShapes[toDel] = 0;

    itsShapes.remove(toDel, True);

    if (Int(toDel) == itsActiveShape) {
      itsActiveShape = -1;
      itsActiveHandle = -1;
      itsState = Annotations::Nothing;
    }
  }
  
  update(itsPC);
  return True;
}


Bool Annotations::revertToPix(const Int& whichOne) {
  if (whichOne < 0 || whichOne >= Int(itsShapes.nelements())) 
    throw (AipsError("Tried to change co-ord system of a shape which "
		     "was out of range."));

  Record shape = itsShapes[whichOne]->getOptions();
  
  if (!(shape.isDefined("coords") && shape.isDefined("type"))) {
    throw (AipsError("Revert to absolute (pixel) position failed, since"
		     " the requested shape returned a bad option record!"));  
  }
  
  String currentCoords = shape.asString("coords");
  String type = shape.asString("type");

  if (currentCoords == "pix") {
    return True;
  }

  return changeCoordSys(whichOne, type, currentCoords, "pix");
}


Bool Annotations::revertToFrac(const Int& whichOne) {
  if (whichOne <= -1 || whichOne >= Int(itsShapes.nelements())) 
    throw (AipsError("Tried to change co-ord system of a shape which "
		     "was out of range."));
  Record shape = itsShapes[whichOne]->getOptions();
  
  if (!(shape.isDefined("coords") && shape.isDefined("type"))) {
    throw (AipsError("Revert to fractional (relative) position failed, since"
		     " the requested shape returned a bad option record!"));
    
  }
  
  String currentCoords = shape.asString("coords");
  String type = shape.asString("type");

  if (currentCoords == "frac") {
    return True;
  }

  return changeCoordSys(whichOne, type, currentCoords, "frac");
}

Bool Annotations::lockToWC(const Int& whichOne) {
 
  if (whichOne <= -1 || whichOne >= Int(itsShapes.nelements())) {
    throw (AipsError("Tried to change co-ord system of a shape which "
		     "was out of range."));
  }
  
  Record shape = itsShapes[whichOne]->getOptions();
  
  if (!(shape.isDefined("coords") && shape.isDefined("type"))) {
    throw (AipsError("Lock to world position failed, since"
		     " the requested shape returned a bad option record!"));
    
  }
  
  String currentCoords = shape.asString("coords");
  String type = shape.asString("type");
  
  if (currentCoords == "world") {
    return True;
  }

  return changeCoordSys(whichOne, type, currentCoords, "world");
}

Bool Annotations::validShape(const Record& shape) {
  String type = shape.asString("type");
  Bool success = True;
  
  if (type == "marker") {
    success = success && shape.isDefined("center");
  } else if (type == "text") {
    success = success && shape.isDefined("center");
  } else if (type == "ellipse") {
    success = success && shape.isDefined("center");
    success = success && shape.isDefined("majoraxis");
    success = success && shape.isDefined("minoraxis");
  } else if (type == "arrow") {
    success = success && shape.isDefined("startpoint");
    success = success && shape.isDefined("endpoint");
  } else if (type == "polyline") {
    success = success && shape.isDefined("polylinepoints");
  } else if (type == "polygon") {
    success = success && shape.isDefined("polygonpoints");
  }

  return success;
}

void Annotations::newShape(Record& settings, const Bool& redraw) {
  
  if (itsState == Annotations::Creation) {
    deleteShape(itsActiveShape);
  }

  if (!settings.isDefined("coords")) {
    throw (AipsError("No coords field - specify \'world\', \'frac\', "
		     "or \'pix\' please."));
  }

  if (!settings.isDefined("type")) {
    throw (AipsError("No type field - can't make a \'newShape()\' of "
		     "unknown type"));
  }
  
  if (!validShape(settings)) {
    throw (AipsError("Bad / missing fields in request to build shape"));
  }

  // Current shapes
  uInt n = itsShapes.nelements();
  
  // Pointer to our new shape
  DisplayShape* newShape = 0;
  
  // First try to make the new shape.
  try {
    newShape = constructShape(settings);
  } catch (const AipsError &x) {
    throw (x);
  }
  // We got past there so....

  //Turn off old handles
  if (itsActiveShape != -1)  {
    itsShapes[itsActiveShape]->setDrawHandles(False);
  }
  itsShapes.resize(uInt(n + 1), False, True);
  itsShapes[n] = newShape;

  if (redraw) {
    itsShapes[n]->setDrawHandles(True);  
    update(itsPC);    
    itsActiveShape = n;
  } else {
    itsActiveShape = -1;
  }
  itsState = Annotations::Nothing;
}

void Annotations::update(PixelCanvas* pc) {
  if (itsUseEH && itsRefreshedYet) {
    pc->copyBackBufferToFrontBuffer();
    pc->setDrawBuffer(Display::FrontBuffer);
    pc->callRefreshEventHandlers(Display::BackCopiedToFront);
  } else if (!itsUseEH) {
    for (uInt i=0; i<itsShapes.nelements(); i++) {
      itsShapes[i]->draw(pc);
    }
  }
}

void Annotations::operator()(const WCRefreshEvent& ev) {
  if (ev.reason() != Display::BackCopiedToFront) {
    for (uInt i=0; i<itsShapes.nelements(); i++) {
      itsShapes[i]->recalculateScreenPosition();
    }
  } 
}

Bool Annotations::validateWCs() {
  Bool conform = True;
  if (itsWCLI->len() != itsPanelDisplay->myWCLI->len()) {
    return False;    
  } else {
    itsPanelDisplay->myWCLI->toStart();
    itsWCLI->toStart();
    while (!itsWCLI->atEnd() && conform) {      
      conform = 
	(conform && (itsPanelDisplay->myWCLI->getRight() == 
	 	     itsWCLI->getRight()));
      (*itsWCLI)++; 
      (*itsPanelDisplay->myWCLI)++; 
    } 
  }

  return conform;
}

DisplayShape* Annotations::constructShape(const Record& settings) {

  if (!(settings.isDefined("type") && settings.isDefined("coords"))) {
    throw (AipsError("When creating a new shape, you must specify its "
		    "\'type\' and also its \'coords\' (world, frac, pix)"));
  }

  DisplayShape* toReturn = 0;
  
  String shape = settings.asString("type");
  String coords = settings.asString("coords");

  if (!(coords == "pix" || coords == "frac" || coords == "world")) {
    throw (AipsError("Unknown coordinate system requested (" + coords + ")"
		    " for new shape"));
  }
  
  
  if (shape == "marker") {
    
    if (coords == "pix") {
      toReturn = new DSPixelMarker(settings);
    } else if (coords == "frac") {
      toReturn = new DSScreenMarker(settings, itsPC);
    } else if (coords == "world") {
      toReturn = new DSWorldMarker(settings, itsPanelDisplay);
    }
    
  } else if (shape == "text") {

    if (coords == "pix") {
      toReturn = new DSPixelText(settings);
    } else if (coords == "frac") {
      toReturn = new DSScreenText(settings, itsPC);
    } else if (coords == "world") {
      toReturn = new DSWorldText(settings, itsPanelDisplay);
    }

  } else if (shape == "arrow") {

    if (coords == "pix") {
      toReturn = new DSPixelArrow(settings);
    } else if (coords == "frac") {
      toReturn = new DSScreenArrow(settings, itsPC);
    } else if (coords == "world") {
      toReturn = new DSWorldArrow(settings, itsPanelDisplay);
    }
  } else if (shape == "ellipse") {
    
    if (coords == "pix") {
      toReturn = new DSPixelEllipse(settings);
    } else if (coords == "frac") {
      toReturn = new DSScreenEllipse(settings, itsPC);
    } else if (coords == "world") {
      toReturn = new DSWorldEllipse(settings, itsPanelDisplay);
    }
    
  } else if (shape == "polyline") {

    if (coords == "pix") {
      toReturn = new DSPixelPolyLine(settings);
    } else if (coords == "frac") {
      toReturn = new DSScreenPolyLine(settings, itsPC);
    } else if (coords == "world") {
      toReturn = new DSWorldPolyLine(settings, itsPanelDisplay);
    }
    
  } else if (shape == "polygon") {
    
    if (coords == "pix") {
      toReturn = new DSPixelPoly(settings);
    } else if (coords == "frac") {
      toReturn = new DSScreenPoly(settings, itsPC);
    } else if (coords == "world") {
      toReturn = new DSWorldPoly(settings, itsPanelDisplay);
    }
    
  } else {
    throw (AipsError("I'm sorry, that shape (\'" + shape + "\') is not yet"
		     " implemented using that co-ord system (\'" +
		     coords + "\')"));
  }

  return toReturn;
}


void Annotations::draw(PixelCanvas* pc, const Bool noHandles) {

  if (noHandles && itsActiveShape != -1) 
    itsShapes[itsActiveShape]->setDrawHandles(False);
  
  
  for (uInt i=0; i<itsShapes.nelements(); i++) {
    itsShapes[i]->draw(pc);
  }
  
  if (noHandles && itsActiveShape != -1) 
    itsShapes[itsActiveShape]->setDrawHandles(True);
  
}

// This is probably the messiest function around...
Bool Annotations::changeCoordSys(const Int shapeIndex, const String& type,
				 const String& currentCoords, 
				 const String& toCoords) {


  if (shapeIndex <= -1 || shapeIndex >= Int(itsShapes.nelements())) {
    throw (AipsError("Tried to change co-ord system of a shape which "
		     "was out of range."));
  }


  if (toCoords == currentCoords) {
    return True;
  }
  
  DisplayShape* oldShape = itsShapes[shapeIndex];
  DisplayShape* newShape(0);

  // Marker
  if (type == "marker") {
    
    if (currentCoords == "pix") {

      DSPixelMarker* current = dynamic_cast<DSPixelMarker*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }

      if (toCoords == "frac") {
	newShape = new DSScreenMarker(*current,itsPC);
      } else if (toCoords == "world") {
	newShape = new DSWorldMarker(*current,itsPanelDisplay);
      }
      
    } else if (currentCoords == "frac") {

      DSScreenMarker* current = dynamic_cast<DSScreenMarker*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }

      if (toCoords == "pix") {
	newShape = new DSPixelMarker(*current);
      } else if (toCoords == "world") {
	newShape = new DSWorldMarker(*current,itsPanelDisplay);
      }

    } else if (currentCoords == "world") {

      DSWorldMarker* current = dynamic_cast<DSWorldMarker*>(oldShape);
      
      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }

      if (toCoords == "frac") {
	newShape = new DSScreenMarker(*current);
      } else if (toCoords == "pix") {
	newShape = new DSPixelMarker(*current);
      }
    }

  } else if (type == "text") {
    if (currentCoords == "pix") {
      
      DSPixelText* current = dynamic_cast<DSPixelText*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }

      if (toCoords == "frac") {
	newShape = new DSScreenText(*current,itsPC);
      } else if (toCoords == "world") {
	newShape = new DSWorldText(*current,itsPanelDisplay);
      }
      
    } else if (currentCoords == "frac") {

      DSScreenText* current = dynamic_cast<DSScreenText*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }

      if (toCoords == "pix") {
	newShape = new DSPixelText(*current);
      } else if (toCoords == "world") {
	newShape = new DSWorldText(*current,itsPanelDisplay);
      }

    } else if (currentCoords == "world") {

      DSWorldText* current = dynamic_cast<DSWorldText*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }
      
      
      if (toCoords == "frac") {
	newShape = new DSScreenText(*current);
      } else if (toCoords == "pix") {
	newShape = new DSPixelText(*current);
      }
    }
    
  } else if (type == "arrow") {
    if (currentCoords == "pix") {

      DSPixelArrow* current = dynamic_cast<DSPixelArrow*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }
      
      if (toCoords == "frac") {
	newShape = new DSScreenArrow(*current,itsPC);
      } else if (toCoords == "world") {
	newShape = new DSWorldArrow(*current,itsPanelDisplay);
      }
      
    } else if (currentCoords == "frac") {

      DSScreenArrow* current = dynamic_cast<DSScreenArrow*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }
      
      if (toCoords == "pix") {
	newShape = new DSPixelArrow(*current);
      } else if (toCoords == "world") {
	newShape = new DSWorldArrow(*current,itsPanelDisplay);
      }

    } else if (currentCoords == "world") {

      DSWorldArrow* current = dynamic_cast<DSWorldArrow*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }

      if (toCoords == "frac") {
	newShape = new DSScreenArrow(*current);
      } else if (toCoords == "pix") {
	newShape = new DSPixelArrow(*current);
      }
    }
    
  } else if (type == "ellipse") {

    if (currentCoords == "pix") {
      
      DSPixelEllipse* current = dynamic_cast<DSPixelEllipse*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }
      
      if (toCoords == "frac") {
	newShape = new DSScreenEllipse(*current, itsPC);
      } else if (toCoords == "world") {
	newShape = new DSWorldEllipse(*current, itsPanelDisplay);
      }
      
    } else if (currentCoords == "frac") {

      DSScreenEllipse* current = dynamic_cast<DSScreenEllipse*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }
      
      if (toCoords == "pix") {
	newShape = new DSPixelEllipse(*current);
      } else if (toCoords == "world") {
	newShape = new DSWorldEllipse(*current,itsPanelDisplay);
      }

    } else if (currentCoords == "world") {

      DSWorldEllipse* current = dynamic_cast<DSWorldEllipse*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }

      if (toCoords == "frac") {
	newShape = new DSScreenEllipse(*current);
      } else if (toCoords == "pix") {
	newShape = new DSPixelEllipse(*current);
      }
    }

  } else if (type == "polyline") {

    if (currentCoords == "pix") {
      
      DSPixelPolyLine* current = dynamic_cast<DSPixelPolyLine*>(oldShape);
      
      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }
      
      if (toCoords == "frac") {
	newShape = new DSScreenPolyLine(*current, itsPC);
      } else if (toCoords == "world") {
	newShape = new DSWorldPolyLine(*current, itsPanelDisplay);
      }
      
    } else if (currentCoords == "frac") {

      DSScreenPolyLine* current = dynamic_cast<DSScreenPolyLine*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }
      
      if (toCoords == "pix") {
	newShape = new DSPixelPolyLine(*current);
      } else if (toCoords == "world") {
	newShape = new DSWorldPolyLine(*current,itsPanelDisplay);
      }

    } else if (currentCoords == "world") {

      DSWorldPolyLine* current = dynamic_cast<DSWorldPolyLine*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }

      if (toCoords == "frac") {
	newShape = new DSScreenPolyLine(*current);
      } else if (toCoords == "pix") {
	newShape = new DSPixelPolyLine(*current);
      }
    }
  } else if (type == "polygon") {

    if (currentCoords == "pix") {
      
      DSPixelPoly* current = dynamic_cast<DSPixelPoly*>(oldShape);
      
      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }
      
      if (toCoords == "frac") {
	newShape = new DSScreenPoly(*current, itsPC);
      } else if (toCoords == "world") {
	newShape = new DSWorldPoly(*current, itsPanelDisplay);
      }
      
    } else if (currentCoords == "frac") {

      DSScreenPoly* current = dynamic_cast<DSScreenPoly*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }
      
      if (toCoords == "pix") {
	newShape = new DSPixelPoly(*current);
      } else if (toCoords == "world") {
	newShape = new DSWorldPoly(*current,itsPanelDisplay);
      }

    } else if (currentCoords == "world") {

      DSWorldPoly* current = dynamic_cast<DSWorldPoly*>(oldShape);

      if (!current) {
	throw (AipsError("I (Annotations) couldn't down-cast a shape to "
			"allow me to change its co-ordinate system. I don't"
			" know what to do."));
      }

      if (toCoords == "frac") {
	newShape = new DSScreenPoly(*current);
      } else if (toCoords == "pix") {
	newShape = new DSPixelPoly(*current);
      }
    }

    
  } else {
    throw (AipsError("Sorry, change of co-ordinate system for shape of type "
		     + type + " is not yet implemented."));
  }
  
  delete itsShapes[shapeIndex]; itsShapes[shapeIndex] = 0;
  itsShapes[shapeIndex] = newShape;

  return True;
}

Record Annotations::availableShapes() {

  Vector<String> toReturn(6);

  toReturn(0) = "marker";
  toReturn(1) = "text";
  toReturn(2) = "arrow";
  toReturn(3) = "ellipse";
  toReturn(4) = "polyline";
  toReturn(5) = "polygon";
  
  Record rec;
  rec.define("shapes", toReturn);
  return rec;
}

void Annotations::polyLineToPolygon(const Int whichOne) {

  if (whichOne < 0 || whichOne >= Int(itsShapes.nelements())) {
    throw (AipsError("Bad shape index in call to polyLineToPolygon"));
  }
  Record shapeOptions = itsShapes[whichOne]->getOptions();

  if (!shapeOptions.isDefined("type")) {
    throw (AipsError("Bad option record returned when trying to convert "
		    "a polyline to a polygon"));
  }

  if (shapeOptions.asString("type") != "polyline") {
    throw (AipsError("Tried to convert a shape other than a polyline to "
		    " a polygon."));
  }

  if (!shapeOptions.isDefined("coords")) {
    throw (AipsError("Tried to convert a polyline with unknown coords to "
		    "a polygon."));
  }

  String coords = shapeOptions.asString("coords");

  if (coords == "pix") {
    DSPixelPoly* newShape;
    try {
      DSPixelPolyLine* current = 
	dynamic_cast<DSPixelPolyLine*>(itsShapes[whichOne]);
      newShape = new DSPixelPoly(*current);
    } catch (const AipsError& er) {
      throw (AipsError("Error going from polyline -> polygon :" 
		       + er.getMesg()));
    }
    
    delete itsShapes[whichOne]; itsShapes[whichOne] = 0;
    itsShapes[whichOne] = newShape;

  } else if (coords == "frac") {
    DSScreenPoly* newShape;
    try {
      DSScreenPolyLine* current = 
	dynamic_cast<DSScreenPolyLine*>(itsShapes[whichOne]);
      newShape = new DSScreenPoly(*current, itsPC);
    } catch (const AipsError& er) {
      throw (AipsError("Error going from polyline -> polygon :" 
		       + er.getMesg()));
    }
    
    delete itsShapes[whichOne]; itsShapes[whichOne] = 0;
    itsShapes[whichOne] = newShape;
    
  } else if (coords == "world") {
    DSWorldPoly* newShape;
    try {
      DSWorldPolyLine* current = 
	dynamic_cast<DSWorldPolyLine*>(itsShapes[whichOne]);
      newShape = new DSWorldPoly(*current);
    } catch (const AipsError& er) {
      throw (AipsError("Error going from polyline -> polygon :" 
		       + er.getMesg()));
    }
    
    delete itsShapes[whichOne]; itsShapes[whichOne] = 0;
    itsShapes[whichOne] = newShape;
    
  } else {
    throw (AipsError("Tried to convert a polyline with unknown coords to "
		    "a polygon."));
  }
  
}

} //# NAMESPACE CASA - END

