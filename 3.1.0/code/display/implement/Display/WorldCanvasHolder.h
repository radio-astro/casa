//# WorldCanvasHolder.h: Interface between DisplayDatas and a WorldCanvas
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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

#ifndef TRIALDISPLAY_WORLDCANVASHOLDER_H
#define TRIALDISPLAY_WORLDCANVASHOLDER_H

#include <casa/aips.h>
#include <casa/Containers/List.h>
#include <display/DisplayEvents/WCRefreshEH.h>
#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayEvents/WCMotionEH.h>
#include <display/DisplayCanvas/WCSizeControlHandler.h>
#include <display/DisplayCanvas/WCCoordinateHandler.h>
#include <display/Display/AttributeBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class Attribute;
class DisplayData;
class WorldCanvas;
class String;

// <summary>
// A holder to interface between DisplayDatas and a WorldCanvas 
// </summary>
//
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <prerequisite>
//   <li> Attribute
//   <li> AttributeBuffer
//   <li> WorldCanvas
//   <li> DisplayData
// </prerequisite>
//
// <etymology>
// The WorldCanvasHolder "holds" a WorldCanvas and some number of
// DisplayDatas which are "registered" on the WorldCanvas.  It actually
// registers itself to handle the WC events, and passes the events on to
// the DDs.
// </etymology>
//
// <synopsis> 
// </synopsis>
//
// <example> 
// </example>
//
// <motivation> 
// </motivation>

class WorldCanvasHolder : public WCRefreshEH,
			  public WCMotionEH,
			  public WCPositionEH,
			  public WCSizeControlHandler,
			  public WCCoordinateHandler {

 public:

  // Constructor.  A WorldCanvas must be provided for the constructed
  // WorldCanvasHolder to "hold".
  WorldCanvasHolder(WorldCanvas *canvas);

  // Destructor.
  virtual ~WorldCanvasHolder();

  // Return the WorldCanvas which is held by this WorldCanvasHolder.
  virtual WorldCanvas *worldCanvas() const
    { return itsWorldCanvas; }

  // Add a DisplayData object to the list of DisplayDatas registered
  // on the held WorldCanvas by this WorldCanvasHolder.
  virtual void addDisplayData(DisplayData *dData); 

  // Remove a DisplayData from the list of DisplayDatas which are
  // registered by this WorldCanvasHolder for display on the held
  // WorldCanvas. <src>ignoreRefresh</src> tells the DD not to refresh
  // just to clean up DMs
  virtual void removeDisplayData(DisplayData& dData, 
				 Bool ignoreRefresh = False); 

  // How many DisplayDatas are registered?
  virtual const uInt nDisplayDatas() const;

  // Install a single restriction, or a buffer of restrictions, on the
  // WorldCanvasHolder which DisplayData must match in order that they
  // be allowed to draw themselves.
  // <group>
  virtual void setRestriction(const Attribute& restriction);
  virtual void setRestrictions(const AttributeBuffer& resBuff);
  // </group>

  // Check if a named restriction exists.
  virtual const Bool existRestriction(const String& name) const;
  
  // Remove the named restriction, or all restrictions, from the
  // WorldCanvasHolder.
  // <group>
  virtual void removeRestriction(const String& restrictionNAme);
  virtual void removeRestrictions();
  // </group>
  
  // Determine whether the restrictions installed on the
  // WorldCanvasHolder match the given restriction or buffer of
  // restrictions.
  // <group>
  virtual Bool matchesRestriction(const Attribute& restriction) const;
  virtual Bool matchesRestrictions(const AttributeBuffer& buffer) const;
  // </group>
 
  // Return the buffer of restrictions installed on this
  // WorldCanvasHolder.
  virtual const AttributeBuffer *restrictionBuffer() const;
  
  // Invoke a refresh on the WorldCanvas, ie. this is a shorthand for
  // WorldCanvasHolder->worldCanvas()->refresh(reason);.
  virtual void refresh(const Display::RefreshReason &reason = 
		       Display::UserCommand,
		       const Bool &explicitrequest = True);

  // Handle size control requests originating from the WorldCanvas.
  virtual Bool executeSizeControl(WorldCanvas *wCanvas);     

  // Distribute a WCPositionEvent originating from the held
  // WorldCanvas over the DisplayDatas.
  virtual void operator()(const WCPositionEvent &ev);
  
  // Distribute a WCRefreshEvent originating from the held WorldCanvas
  // over the DisplayDatas.
  virtual void operator()(const WCRefreshEvent &ev);

  // Distribute a  WCMotionEvent originating from the held WorldCanvas
  // over the DisplayDatas.
  virtual void operator()(const WCMotionEvent &ev);

  // Handle other, generic types of events.  As with the handlers above,
  // WCH handles these new events by simply passing them on to the
  // DisplayDatas registered on it.  WorldCanvasHolder inherits this
  // new-style event handling interface from DisplayEH, via WCRefreshEH.
  virtual void handleEvent(DisplayEvent& ev);

  // Coordinate conversion routines, handled for the WorldCanvas.  
  // In future, they should be handled on the WC itself, via its own CS.
  // At present, these requests are forwarded to the CSmaster DD, which
  // should be equivalent in most cases.
  // <group>
  virtual Bool linToWorld(Vector<Double>& world, const Vector<Double>& lin);
  virtual Bool worldToLin(Vector<Double>& lin, const Vector<Double>& world);
  
  //# (these latter two are merely to stop a compiler whine...)
  virtual Bool linToWorld(Matrix<Double> & world, Vector<Bool> & failures,
                          const Matrix<Double> & lin) {
    return WCCoordinateHandler::linToWorld(world, failures, lin);  }
  
  virtual Bool worldToLin(Matrix<Double> & lin, Vector<Bool> & failures,
                          const Matrix<Double> & world) {
    return WCCoordinateHandler::worldToLin(lin, failures, world);  }
  // </group> 

  // Return the names and units of the world coordinate axes.
  // <group>
  virtual Vector<String> worldAxisNames();
  virtual Vector<String> worldAxisUnits();
  // </group>

  // Return the number of world axes, which is hard-wired to 2.
  virtual uInt nWorldAxes() const {
    return 2;
  }

  // Maximum number of animation frames of all registered
  // DDs which are valid for the WC's current CS state.
  virtual const uInt nelements();
  
  // Force a cleanup of all the DisplayDatas which are registered with
  // this WorldCanvasHolder.
  virtual void cleanup();
  
  // The DD in charge of setting WC coordinate state (0 if none).
  virtual const DisplayData* csMaster() const { return itsCSmaster;  }

  // Is the specified DisplayData the one in charge of WC state?
  // (During DD::sizeControl() execution, it means instead that the
  // DD has permission to become CSmaster, if it can).
  virtual Bool isCSmaster(const DisplayData *dd) const {
    return dd==csMaster() && dd!=0;  }
    
  // Was the passed DD the last CS master (or, if no DD passed, was
  // there any CS master)?  For convenience of the DDs during the next
  // sizeControl execution, in determining whether a CS master change is
  // occurring, and whether anyone was master before.  This affects
  // whether any old zoom window is retained or completely reset.
  virtual Bool wasCSmaster(DisplayData* dd=0) const { 
    return (dd==0)? itsLastCSmaster!=0 : itsLastCSmaster==dd;  }
    
  // used by PanelDisplay on new WCHs to keep a consistent CS master on
  // all its main display WC[H]s.  Sets [default] CS master dd to that of
  // passed wch (if that dd is registered here), and gets it to reset WC
  // coordinate state.
  virtual Bool syncCSmaster(const WorldCanvasHolder* wch);


private: 

  // The WorldCanvas that is held by this WorldCanvasHolder.
  WorldCanvas *itsWorldCanvas;

  // A list containing the DisplayDatas that are registered on this
  // WorldCanvasHolder.
  List<DisplayData *> itsDisplayList;

  // A buffer to contain the restrictions that DisplayDatas must match
  // if they are to be allowed to draw themselves.
  AttributeBuffer itsRestrictions;
  
  // [First] responder to 'sizeControl', responsible for setting
  // WC CS, zoom window and draw area.  It will be 0 initially, and
  // whenever the old master is unregistered (until a new master responds).
  // This is a further attempt toward a coherent sense of 'who's in charge'
  // of WC[H] state (there is more to do).
  //
  // Some day, the WC CS should be directly responsible for all the Canvas's
  // coordinate conversions.  For now at least we'll know that they're done
  // by the DD below (which should be equivalent).
  DisplayData* itsCSmaster;
  
  // The CS master in effect after executeSizeControl was last run (0 if none).
  // For determining (via wasCSmaster(), above) whether a CS master change is
  // occurring, and whether anyone was master before.  This affects whether
  // the old zoom window is retained or completely reset.
  // (Note void*, rather than DD*, type: it is not intended to be dereferenced,
  // just compared for equality.  The original DD may not even exist by the
  // time it is used).
  void* itsLastCSmaster;

};


} //# NAMESPACE CASA - END

#endif
