//# DisplayData.h: base class for display objects
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
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef TRIALDISPLAY_DISPLAYDATA_H
#define TRIALDISPLAY_DISPLAYDATA_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Containers/List.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <display/Utilities/DisplayOptions.h>
#include <display/Utilities/DlTarget.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/AttValBase.h>
#include <display/DisplayEvents/DisplayEH.h>

#include <tr1/memory>

namespace casa { //# NAMESPACE CASA - BEGIN

	class Attribute;
	class AttributeBuffer;
	class Unit;
	class WorldCanvas;
	class Colormap;
	class String;
	class WCPositionEH;
	class WCMotionEH;
	class WCRefreshEH;
	class WCPositionEvent;
	class WCMotionEvent;
	class WCRefreshEvent;
	class Record;
	class ImageAnalysis;
	template <class T> class ImageInterface;

// <summary>
// Base class for display objects.
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> WorldCanvas
//   <li> WorldCanvasHolder
//   <li> Attribute
//   <li> AttributBuffer
// </prerequisite>
//
// <etymology>
// A class having "DisplayData" as its base is used to generate
// "Display"s out of "Data."
// </etymology>
//
// <synopsis> The basic drawing devices in the Display Library are the
// PixelCanvas and the WorldCanvas. These devices know nothing about what real
// data look like, what kind of object will draw on these devices and in what
// kind of circumstances these devices will be used. The only thing they
// define is the interface of how to draw on them and the way they communicate
// with other objects (the event handlers).  Building in no assumptions in the
// Canvases on how they will be used should give larger flexibility in how one
// can actually use these Canvases.  Since the Canvases know nothing about how
// real data looks, a class is needed to transform data into objects the
// Canvases do understand. These are the DisplayData. A DisplayData generates,
// based on data and some algorithm to represent that data, a number of
// primitives that it draws on the WorldCanvas. So in a way, 'the data draw
// themselves'. The definition of how data is represented (image, contour,
// rendering, list of symbols from a catalogue, etc, etc) is entirely defined
// by these DisplayData and as long it can be done using the primitives of the
// WorldCanvas, there are no restrictions. If one finds a new way of
// representing data, the only thing one has to do is to write a new
// DisplayData that generates this representation and draws it on the
// WorldCanvas using the primitives of the WorldCnvas.
//
// To do the administration of a number of DisplayDatas on one WorldCanvas, a
// bookkeeping class is needed, this is the WorldCnvasHolder.
//
// Some DisplayData will consist of a sequence of display object (eg a set of
// channels in a data cube). The DisplayData are build with sequences in mind
// (as is clear from the interface), and the Display Library is designed very
// strongly with movies in mind. The programmer is free to define what a
// sequence really means, but it is probably best to keep the structure of the
// sequence in a DisplayData fairly logical. But there is not requirement on
// the structure of the sequence.
//
// Before a display object is displayed (ie when a refresh happens), a size
// control step is performed. The WorldCanvas call a sizeControl event
// handler. Normally this will be a handler that is installed by the
// WorldCanvasHolder. All the WorldCanvasHolder does is to call the
// sizeControl member function of all displayData registered with the
// WorldCanvasHolder to do the sizeControl. The purpose of the size control is
// to put the WorldCanvas in a correct state before the objects are actually
// drawn. For example, for images there can be restrictions on the size of the
// output array (eg the image is expanded using pixelreplication which means
// that the draw area of the WorldCanvas must by an integral of the size of
// the image). For each DisplayData the WorldCanvasHolder calls the sizeControl
// function, supplying an AttributeBuffer where the DisplayData should set the
// Attributes it needs to be set. THERE IS ONE IMPORTANT RULE IN THIS: a
// DisplayData should never overwrite an Attribute that is already set in this
// buffer. If the WorldCanvas cannot be put in a correct state during the size
// control, a DisplayData should turn itself off. DisplayData are also
// responsible for the linear coordinate system of the WorldCanvas and usually
// setting that up will be done in this step.


// (dk 6/04 note on 'size control').  The important WC state set during
// sizeControl() defines a series of coordinate transformations, ultimately
// between screen (PixelCanvas) pixels and coordinates in world space.
// First, there are the limits of the WC's 'draw area', inside the labelling
// margins, on the PC ('canvasDraw{X,Y}{Size,Offset}' WC  Attributes).
// Second is the 'zoom window': the area of 'linear coordinate' space
// currently mapped to the draw area ('lin{X,Y}{Min,Max}' WC Attributes).
// Also stored in linear terms are the maximum extents of the data
// ('lin{X,Y}{Min,Max}Limit' Attributes), which are used to set the zoom
// window initially or when 'unzoom' (zoom-to-extent) is requested.
//
// 'Linear coordinates' often correspond to indices within the data
// being displayed, although in principle this is not necessary.  They
// serve to isolate the simple linear scaling/translation (zoom and pan)
// transformations from the final transformation (often a nonlinear sky
// projection) between 'linear coordinates' and world space (which is
// changed less often).
//
// (Also, linear coordinates define an entire plane, although a given
// sky-coordinate projection to that plane may not be defined everywhere.
// Keeping zoom windows defined in linear coordinates allows natural
// display of all-sky images even when sky coordinates are not defined
// everywhere in the display area.  In other words, the existing
// scheme allows all-sky images to be displayed easily _in principle_;
// it does not work currently due to inappropriate insistence of
// WC::drawImage() that its draw area be given in world coordinates;
// this is high on the list of bugfix priorities...).
//
// The final transformation makes use of the Coordinate classes (which
// ultimately use wcslib for any needed sky projection).  It defines the
// current 2D surface on display within some nD world space, as parameterized
// by the X and Y linear coordinates.  State for this purpose includes the
// 'WorldCanvas DisplayCoordinateSystem' or 'WCCS' (WorldCanvas::itsCoordinateSystem)
// plus 'axis codes' (which have the odd Attribute names
// '{x,y}axiscode (required match)').  They are intended to define the
// coordinate values and types for the world space currently of interest.
//
// This latter transformation is not as simple or well-defined as it might
// appear, however (and in my opinion this area still needs work).  First of
// all, the WCCS is a rather late addition to WC interface, and in most cases
// is _not even used_ to do WC linear-to-world transformations.  Instead, this
// chore is handed off (via the older WCCoordinateHandler interface) to one
// of the DDs currently registered on WC's companion object, WorldCanvasHolder.
// Also, the axis codes do not always provide enough information about the
// current world space for DDs to determine whether they are designed to
// draw in that space (which, in my opinion, should be the codes' purpose).
// They also implicitly assume a 1-to-1 correspondence between linear and
// world coordinates, which is not very general.
//
// Back to how it works now, though.  During the 'sizeControl' step of
// refresh, the WCH will attempt to determine a 'CS master DD'.  (Usually the
// same DD will remain 'in charge' as long as it is registered).  Within
// the sizeControl routine, DDs should test isCSMaster(wch) to determine
// whether they have permission to become the CS master.  If so, and if they
// are willing and able to become CS master, they must set all of the WC
// state above and return True; in all other cases they should return False.
// Only the CSmaster should set the WC CS or axis codes, and it will be
// responsible for performing the WC's linToWorld, etc. transformation chores.
// In other words, it is entirely in charge of the final transformation to
// world coordinates.  In principle, other DDs may perform certain 'slave
// sizeControl' tweaks (such as aligning the zoom window on data pixel
// boundaries or redefining maximum zoom extents).  No current implementations
// do anything but return False if they are not the master, however.
//
// Major implementation examples for sizeControl() can be found in the
// ActiveCaching2dDD and PrincipalAxesDD classes.  They should be consulted
// as well for the conventions for processing WC 'zoom/unzoom' order
// attributes, another CSmaster responsibility.  (<\dk>)



// The control of what is displayed in a display application is done with
// setting restrictions on the WorldCanvasHolder (and possibly on the
// Displaydata). Restrictions are implemented as Attributes and stored in
// AttributeBuffers. To define what is displayed, one sets one or more
// restrictions on the WorldCanvasHolder and calls a refresh on the
// WorldCanvas. Most DisplayData will have pre-defined restrictions, for
// example in an ImageDisplayData each image has defined the restriction
// zIndex (set to the pixelindex of the 'third' axis in the data set) and
// zValue (set to the 'third' worldcoordinate of the image). So to display
// channel 13 of a data cube, one only has to set on the WorldCanvasHolder a
// restriction called zIndex with value 13 and call refresh:
//
// <srcblock>
// worldCanvasHolder.setRestriction("zIndex", 13);
// worldCanvas.refresh();
// </srcblock>
//
// Movies can be made using the Animator class. One way it to use indices, but
// there is a generic way of defining movies using restrictions. So a sequence
// does not have to correspond to a 'logical' or 'physical' sequence in some
// datastructure (channels in a cube for example), but can be made of images
// from different datasets (e.g. blinking) and display data in different
// forms. This system is very flexible and there are no real limits to what a
// movie really means.
//
// The DisplayData also define the coordinate system of the WorldCanvas. If
// the WorldCanvas has to do a coordinate transformation, it asks a coordinate
// handler to do this. When a WorldCanvasHolder is created for a WorldCanvas,
// the WorldCanvasHolder install a coordinate handler on the WorldCanvas. What
// this coordinate handlers does is to ask the DisplayData that is first in
// the list of the WorldcanvasHolder to do the transformation. This is quite
// an indirect way of doing the transformation, but it is very flexible (and
// the WorldCanvas does not have to know what an Aips++ coordinate system is,
// or what a DisplayData is, , which makes the WorldCanvas more generic). Most
// DisplayData will use the coordinate system of the data belonging to this
// DisplayData to do the transformation, but this is not a requirement. As
// long as you produce something that can be used as a coordinate system, it
// does not matter how you compute it. The most important thing is that the
// DisplayData are responsible for this. At the moment, only the Vector
// version should be implemented, since efficient transformation of a series
// of positions is not available in AIPS++. The Matrix versions are handled at
// the WorldCanvas level at the moment.  Most DisplayData will assume that the
// linear coordinate system of the WorldCanvas corresponds to some underlying
// pixel array. The way the linear coordinate system is used is that it is the
// input for the coordinate transformation to world coordinates. The
// DisplayData are responsible for keeping the linear system correct.

// Other event handlers that the DisplayData have to implement are the
// position- and the motion event handlers. (PositionEH and MotionEH). These
// are called by the WorldCanvasHolder in response to an event on the
// WorldCanvas. It is up to the DisplayData what to do with these events.
//
// One can also register position- and motion event handlers on a
// DisplayData. What kind of event a DisplayData generates in response to an
// event on the WorldCanvas (passed to the DisplayData by the
// WorldCanvasHolder by calling PositionEH/MotionEH) is entirely up to the
// DisplayData, as long as the handlers are derived from WCPositionEH or
// WCMotionEH. One can also install a refresh handler on a DisplayData,
// although I have not thought of any use for that (but allowed for it to keep
// symmetry with the other events).
//
// (1/02) DisplayEH interface has also been added for handling generic
// DisplayEvents through handleEvent().  It is expected that DDs will handle
// these events themselves, as needed, or pass them on in an ad-hoc manner.
// However, nothing prevents implementing a dispatching list for passing
// on these events too, if needed.
//
// (3/03) One can also register itself as a DisplayEventHandler on a
// DisplayData. The handleEvent function of DisplayEH is implemented
// to forward any DisplayEvents its receives to these
// handlers. Therefore, all DisplayDatas that overide the handleEvent
// function, should call the handleEvent function of its super class
// so this forwarding can take place.
//
// A DisplayData also has to implement a refreshEH. This function is called by
// the WorldCanvasHolder in response to a refresh request of the WorldCanvas.
// This is a very important function: here the actual drawing has
// to happen, using the draw primitives of the WorldCanvas. It is a good idea
// to check the state of the WorldCanvas before the draw is actually done, and
// decide not to draw if the state (for whatever reason) is not ok. Also be
// aware that it is a requirement that one DisplayData can work for more than
// one WorldCanvasHolders at a time, so the DisplayData has to do some
// administration for that (which WorldCanvasHolders am I working for and
// which one am I drawing on at the moment, things like that). See
// ImageDisplayData for an example of that.
//
// DisplayData also have Attributes. These can be used to store whatever
// information on a DisplayData. The Attributes give a standard interface to
// do this.
// </synopsis>
//
// <example>
// <srcBlock>
// </srcBlock>
// </example>
//

// <motivation>
// An abstract interface between data and canvases was needed
//</motivation>
//
// <todo>
//
//   <li> When efficient implementations of the Matrix versions of the
//   coordinate transformations become available in AIPS++, these should be
//   implemented in DisplayData
//
// </todo>
//

	class DisplayData : public DisplayOptions, public DisplayEH, public DlTarget {

	public:

		enum DisplayState { DISPLAYED, UNDISPLAYED, LIMBO };

		// (Required) default constructor.
		DisplayData();

		// required destructor
		virtual ~DisplayData();

		// Coordinate transformations, called by WorldCanvasHolder (Matrix versions
		// not implemented)
		virtual Bool linToWorld(Vector<Double>& world,
		                        const Vector<Double>& lin) = 0;
		virtual Bool worldToLin(Vector<Double>& lin,
		                        const Vector<Double>& world) = 0;

		virtual std::string errorMessage( ) const = 0;

		// Format a string containing coordinate information at
		// the given world coordinate
		virtual String showPosition(const Vector<Double> &world,
		                            const Bool &displayAxesOnly = False) = 0;

		// Format a string containing value information at the
		// given world coordinate
		virtual String showValue(const Vector<Double> &world) = 0;


		virtual void setDisplayState( DisplayState s ) {
			displaystate = s;
		}
		virtual DisplayState getDisplayState( ) const {
			return displaystate;
		}

		virtual bool isDisplayable( ) const {
			return true;
		}

		// Some routines that give info on the axes names, units etc. I am not sure
		// this is the right way of doing it.
		// <group>
		virtual Vector<String> worldAxisNames() const = 0;
		virtual Vector<String> worldAxisUnits() const = 0;
		// </group>

		// Returns the number of elements in this DisplayData (mainly for movie
		// purposes).  First one is no. of elements for specific WCanvas.
		virtual const uInt nelements(const WorldCanvasHolder &wcHolder) const = 0;
		// and non-specific
		virtual const uInt nelements() const = 0;

		// Add general restrictions or a restriction for item <src>itemNum</src> of
		// this DisplayData. Note that the item versions of the restriction
		// interface are not implemented. I am not sure the item versions belong in
		// DisplayData and instead they should only appear in some derived classes.
		// <group>
		virtual void addRestrictions(AttributeBuffer& otherBuf);
		virtual void addRestriction(Attribute& newRestriction, Bool permanent);
		virtual void addElementRestrictions(const uInt itemNum,
		                                    AttributeBuffer& other);
		virtual void addElementRestriction(const uInt itemNum,
		                                   Attribute& newRestriction,
		                                   Bool permanent);
		// </group>

		// Set general restrictions or a restriction for item <src>itemNum</src> of
		// this DisplayData. Note that the item versions of the restriction
		// interface are not implemented.
		// <group>
		virtual void setRestrictions(AttributeBuffer& otherBuf);
		virtual void setRestriction(Attribute& newRestriction);
		virtual void setElementRestrictions(const uInt itemNum,
		                                    AttributeBuffer& other);
		virtual void setElementRestriction(const uInt itemNum,
		                                   Attribute& newRestriction);
		// </group>

		// Remove a general restriction or a restriction from item <src>itemNum</src>
		// <group>
		virtual void removeRestriction(const String& name);
		virtual void removeElementRestriction(const uInt itemNum,
		                                      const String& name);
		// </group>

		// Clear all general restrictions or all restrictions of item
		// <src>itemNum</src> (except the ones that are permanent of course...)
		// <group>
		virtual void clearRestrictions();
		virtual void clearElementRestrictions(const uInt itemNum);
		// </group>

		// Check if a general restriction or a restriction for item
		// <src>itemNum</src> with name <src>name</src> exists.
		// <group>
		virtual Bool existRestriction(const String& name);
		virtual Bool existElementRestriction(const uInt itemNum,
		                                     const String& name);
		// </group>

		// Get a handle to the buffer of general restrictions or of the buffer of
		// restrictions for item <src>itemNum</src>
		// <group>
		virtual AttributeBuffer *restrictionBuffer();
		virtual AttributeBuffer *elementRestrictionBuffer(const uInt itemNum);
		// </group>


		// Check whether the DD is is compatible with all WC[H]
		// state, including its coordinate state, restrictions, and zIndex (if any).
		// It also assures that the DD is 'focused' on this WC[H] and its zindex
		// for purposes of drawing or event handling.
		// <group>
		virtual Bool conformsTo(const WorldCanvas *wCanvas) {
			rstrsConformed_ = csConformed_ = zIndexConformed_ = False;
			return (wCanvas!=0 && conformsTo(*wCanvas));
		}

		virtual Bool conformsTo(const WorldCanvas& wc) {
			conformsToRstrs(wc);
			conformsToCS(wc);
			conformsToZIndex(wc);
			return conformed();
		}
		// </group>

		// Determine whether DD restrictions are in conformance with restrictions
		// on the given WCH.  (Note: this will include blink index, if any,
		// but _not_ zIndex.  zIndex is an individual DM restriction, not an
		// overall DD restriction).
		virtual Bool conformsToRstrs(const WorldCanvas& wc) {
			rstrsConformed_ = wc.matchesRestrictions(restrictions);
			return rstrsConformed_;
		}

		// Determine whether DD is compatible with the WC[H]'s current
		// world coordinates.  Derived DDs can override according to their
		// individual capabilities (PADD and ACDD match axis codes).
		// Overriding DDs should set csConformed_ to the value returned.
		virtual Bool conformsToCS(const WorldCanvas& /*wc*/) {
			csConformed_ = True;
			return csConformed_;
		}

		// Determine whether DD is compatible with the current canvas
		// animation (zIndex) position.  (This usually means that it lies
		// within the current number of DD animation frames).
		// (Generally, DDs should probably override setActiveZIndex_()
		// rather than this method).
		virtual Bool conformsToZIndex(const WorldCanvas& wc) {
			Int zindex = 0;	// (default in case no zIndex exists).
			const AttributeBuffer *rstrs = wc.restrictionBuffer();
			if (rstrs->exists("zIndex")) rstrs->getValue("zIndex",zindex);
			return setActiveZIndex_(zindex);
		}

		// Retrieve 'Active' zIndex.  Not likely to be meaningful/useful
		// unless conformsTo(wch) (or conformsToZIndex(wch)) has been called
		// just prior (and has returned a True result).  Those calls make
		// wch the 'active' one; zIndex varies from one wch to another.
		// You can pass a wch, which will force a call to conformsToZIndex(wch).
		virtual Int activeZIndex(const WorldCanvas* wc=0) {
			if(wc!=0) conformsToZIndex(*wc);
			return activeZIndex_;
		}



		// Set firstZIndex to minimum zIndex setting from all canvases where
		// this DD is registered.  (In the usual case where the DD is registered
		// on one [multi]panel, this will return its animator 'frame #' setting).
		// The routine will return false (and firstZIndex remain unchanged) if
		// there are no registered canvases with zIndex below axZrng--the total
		// number of frames on the Z axis.  axZrng can be supplied; the default
		// means 'use nelements()'.
		// (Note: to get the zindex from the 'currently active' wch instead,
		// a DD should check activeZIndex_.  Or, if the desired wch is known,
		// it can retrieve the zIndex itself from wch.restrictionBuffer()).
		virtual Bool getFirstZIndex(int& firstZIndex, Int axZrng=-1) const {
			Block<Int> zInds = allZIndices(axZrng);
			if(zInds.nelements()==0) return False;
			firstZIndex=zInds[0];
			return True;
		}



		// Add event handlers on the DisplayData. I am not sure there is also a need
		// for a refresh handler on a DisplayData, but allowing for it  makes
		// things 'symmetric'. These member functions throw an AipsError if a null
		// pointer is passed.
		// <group>
		virtual void addPositionEventHandler(WCPositionEH *positionHandler);
		virtual void addMotionEventHandler(WCMotionEH *motionHandler);
		virtual void addRefreshEventHandler(WCRefreshEH *refreshHandler);
		virtual void addDisplayEventHandler(DisplayEH *displayHandler);
		// </group>

		// Remove eventhandlers
		// <group>
		virtual void removePositionEventHandler(WCPositionEH& positionHandler);
		virtual void removeMotionEventHandler(WCMotionEH& motionHandler);
		virtual void removeRefreshEventHandler(WCRefreshEH& refreshHandler);
		virtual void removeDisplayEventHandler(DisplayEH& displayHandler);
		// </group>

		// Set/remove/get a ColourMap (sorry, ColorMap) for this DisplayData
		// setColormap() throw an AipsError is a null pointer is passed. colormap()
		// returns 0 if no Colormap is registered.
		// <group>
		virtual void setColormap(Colormap *cmap, Float weight);
		virtual void removeColormap();
		virtual Colormap *colormap() const {
			return itsColormap;
		}
		// </group>


		// set an Attribute or Attributes
		// <group>
		void setAttribute(Attribute& at);
		void setAttributes(AttributeBuffer& at);
		// </group>

		// User interface to get value from the attribute buffer
		// <group>

		Bool getAttributeValue(const String& name, uInt& newValue) ;
		Bool getAttributeValue(const String& name, Int& newValue) ;
		Bool getAttributeValue(const String& name, Float& newValue) ;
		Bool getAttributeValue(const String& name, Double& newValue) ;
		Bool getAttributeValue(const String& name, Bool& newValue) ;
		Bool getAttributeValue(const String& name, String& newValue) ;
		Bool getAttributeValue(const String& name, Vector<uInt>& newValue) ;
		Bool getAttributeValue(const String& name, Vector<Int>& newValue) ;
		Bool getAttributeValue(const String& name, Vector<Float>& newValue) ;
		Bool getAttributeValue(const String& name, Vector<Double>& newValue) ;
		Bool getAttributeValue(const String& name, Vector<Bool>& newValue) ;
		Bool getAttributeValue(const String& name, Vector<String>& newValue) ;
		// </group>

		// Check if a certain Attribute exists
		Bool existsAttribute(String& name) ;

		// Remove an  Attribute
		void removeAttribute(String& name);

		// Get the type of the Attribute
		AttValue::ValueType attributeType(String& name) ;

		// Set an attribute on any WorldCanvas for which this DD is CS master
		void setAttributeOnPrimaryWCHs(Attribute &at);

		// <group>
		virtual void notifyRegister(WorldCanvasHolder *wcHolder) ;
		// <src>ignoreRefresh</src> tells the DD not to refresh just to clean
		// up DMs
		virtual void notifyUnregister(WorldCanvasHolder& wcHolder,
		                              Bool ignoreRefresh = False) ;
		// </group>

		// remove this DD everywhere--will stop any more refresh handling by
		// the DD.  It is a good idea for top-level DDs to call this first
		// in their destructor.
		virtual void removeFromAllWCHs();

		// install the default options for this DisplayData
		virtual void setDefaultOptions();

		// apply options stored in val to the DisplayData; return value
		// True means a refresh is needed...
		virtual Bool setOptions(Record &rec, Record &recOut);

		// retrieve the current and default options and parameter types.
		virtual Record getOptions();

		// an explicit refresh: should be called if the DisplayData is
		// changed such that drawing is required.  If clean is True,
		// the DD is totally rebuilt, in practice.  This is provided
		// for higher level control, even explicit control of refresh
		// where necessary.
		// ..."refresh(True)"... does not seem to work <drs:Mar 21 2013>
		virtual void refresh(Bool clean = False);

		// an explicit request to draw the axes and/or labels.  Returns
		// True if axes were drawn, otherwise False;
		virtual Bool labelAxes(const WCRefreshEvent &ev);
		virtual Bool canLabelAxes() const{
			return false;
		}

		// Return the class name of this DisplayData; useful mostly for
		// debugging purposes, and perhaps future use in the glish widget
		// interface.
		virtual String className() {
			return String("DisplayData");
		}
		virtual String description( ) const {
			return "not available";
		}

		// Return the DisplayData type; used by the WorldCanvasHolder to
		// determine the order of drawing.
		virtual Display::DisplayDataType classType() = 0;

		virtual String dataType( ) const = 0;
		virtual const IPosition dataShape( ) const = 0;
		virtual const uInt dataDim( ) const = 0;
		virtual const Unit dataUnit( ) const = 0;
		virtual std::vector<int> displayAxes( ) const = 0;

		// Get image analyis about images... for non-image
		// "DisplayData" this function will return null...
		// Use dataType() to check...
		// Creates a new object (or a null pointer) which may need to be deleted...
		virtual ImageAnalysis *imageanalysis( ) const {
			return 0;
		}
		// Returns a pointer that should *not* be deleted...
		virtual std::tr1::shared_ptr<ImageInterface<Float> > imageinterface( ) {
			return std::tr1::shared_ptr<ImageInterface<Float> >();
		}

		virtual void setSubstituteTitleText( const String /*title*/ ){

		}

		// Identify the WorldCanvasHolder for the given WorldCanvas.  Return
		// <src>0</src> if the DisplayData does not know of a
		// WorldCanvasHolder for the WorldCanvas.
		const WorldCanvasHolder *findHolder(const WorldCanvas *wCanvas) const;
		WorldCanvasHolder *findHolder(const WorldCanvas *wCanvas);

		// Return a sorted Block of all animation frame numbers currently set
		// onto all WCHs where this DD is registered.
		// The frame numbers returned are guaranteed to be in the range
		// 0 <= zIndex < axZrng, where axZrng is the total number of frames
		// on the Z axis.  axZrng can be supplied; the default is nelements().
		virtual Block<Int> allZIndices(Int axZrng=-1) const;

		// Will be called just before registering the [GTk]DD on a [GTk]PanelDisplay
		// which has none registered on it yet.  The DD can set the initial
		// animator position in this case by overriding this method to set
		// preferredZIndex and return True.
		virtual Bool zIndexHint(Int& /*preferredZIndex*/) const {
			return False;
		}

		// Overide DisplayEH::handleEvent. This base class on forwards the
		// event on to listeners
		virtual void handleEvent(DisplayEvent &ev);

		// Is this DD the CS master of the passed WCH?
		// Defaulting wch to 0 asks whether this DD is CS master of _some_ WCH
		// on which it is registered.  (That option is mostly a kludge, since the
		// DD may be CS master of some WCHs and not others).
		virtual Bool isCSmaster(const WorldCanvasHolder* wch=0) const;

		// DD 'Absolute Pixel Coordinates', e.g. channel numbers, are internally
		// 0-based (they begin numbering at 0), but certain external user-interface
		// functions (e.g. showPosition(), used for position tracking) have
		// produced 1-based output traditionally for the glish-based viewer.
		// uiBase_, and related methods uiBase() and setUIBase(), allow newer
		// (python/Qt-based) code to cause external ui functions like showValue()
		// to report 0-based values instead.  Unless setUIBase(0) is called, the
		// traditional 1-based reporting behavior is retained by default.
		//
		// If you are using 0-basing in the user interface, you should call
		// setUIBase(0) right after constructing this DD, before other
		// user interface operations such as getOptions().
		// <group>
		virtual Int uiBase() {
			return uiBase_;
		}

		virtual void setUIBase(Int uibase) {
			if(uibase==0 || uibase==1) uiBase_ = uibase;
		}
		// </group>

		// Get and set method for the flag
		// <group>
		virtual Bool getDelTmpData( ) {
			return delTmpData_;
		}
		virtual void setDelTmpData(Bool delTmpData) {
			delTmpData_ = delTmpData;
		}
		// </group>

		virtual void setDisplayDataRed( DisplayData* /*dd*/ ){}
		virtual void setDisplayDataBlue( DisplayData* /*dd*/ ){}
		virtual void setDisplayDataGreen( DisplayData* /*dd*/ ){}

		const static String DATA_MIN;
		const static String DATA_MAX;

	protected:

		// DDs may override to adjust the internal stored current animation
		// index (activeZIndex_) if necessary, and to set return value False
		// iff the passed zindex won't work for the DD.  zIndexConformed_
		// should be set to the value returned; activeZIndex_ should also
		// be set appropriately.
		virtual Bool setActiveZIndex_(Int zindex) {
			activeZIndex_ = zindex;
			zIndexConformed_ = True;
			return zIndexConformed_;
		}


		// The Book says that we should not put data members in class that is
		// supposed to only define an interface, but by putting these here, we can
		// implement some of the restriction & eventhandling stuff so that people do
		// not have to repeat it.

		// buffer for storing restrictions
		AttributeBuffer  restrictions;

		// buffer for storing Attributes
		AttributeBuffer attributes;

		// list of DisplayDataElements, which are of type DisplayMethod or derived
		PtrBlock<void *> DDelement;

		// Protected interface for WorldCanvasHolder, can be called by the friends
		// of DisplayData, but are nobody else's business. I decided to do this in
		// this way to avoid that the programmer has to both register the
		// DisplayData with the WorldCanvasHolder AND the WorldCanvasHolder with the
		// DisplayData. Now one of the two is done automatically.
		// The WorldCanvasHolder is my friend. This is to hide the eventhandler
		// interfaces listed below from the outside world. The WorldCanvasHolder is
		// the only class that needs them
		friend class WorldCanvasHolder;

		// Indicates the 'current' animation frame, i.e. the zIndex on the canvas
		// for which confromsTo(wch) was last called.  Intended to be set only
		// by setActiveZIndex_() in this class or derived classes.
		Int activeZIndex_;

		// Flag indicating that temporary data should be removed when deleting
		// the object.
		Bool delTmpData_;

		// Somewhat limited-use state, saved here for 'efficiency'.  Indicates
		// that the last call to conformsToRstrs(), conformsToCS(), or
		// conformsToZIndex(), passed the respective compatibility tests.
		// (See conformed(), below).  Intended to be set only in the methods
		// conformsToRstrs(), conformsToCS() and setActiveZIndex_(), respectively.
		Bool rstrsConformed_, csConformed_, zIndexConformed_;

		// Returns result of last call to conformsTo(WCH&).  Methods like showValue()
		// which don't have access to the wch can use it instead, but that
		// shifts the burden elsewhere of being sure that conformsTo() was called
		// for the current WCH.  When possible, it is generally better and safer
		// to call conformsTo(wch) directly when needed, rather than querying this.
		Bool conformed() {
			return rstrsConformed_ && csConformed_ && zIndexConformed_;
		}
		//Added so that when two images are loaded with no velocity
		//alignment, they can still show their position coordinates without
		//having to be rstrsConformed.
		Bool isCsAndZIndexConformed() {
			return csConformed_ && zIndexConformed_;
		}
		// Set (coordinate) state of WCH's WC.  Called by WCH::executeSizeControl().
		// (See important notes on interface and implementation of this function
		// in the class synopsis above).
		virtual Bool sizeControl(WorldCanvasHolder& wcHolder,
		                         AttributeBuffer& holderBuf) = 0;

		// Delete temporary data. To be called by sub-classe
		// that now the filenames.
		virtual void delTmpData(String &tmpData);


		// Retrieve position, motion, refresh and display event handler lists.
		// <group>
		virtual const List<WCPositionEH*> *positionEventHandlerList() {
			return &itsPositionEHList;
		}
		virtual const List<WCMotionEH*> *motionEventHandlerList() {
			return &itsMotionEHList;
		}
		virtual const List<WCRefreshEH*> *refreshEventHandlerList() {
			return &itsRefreshEHList;
		}
		virtual const List<DisplayEH*> *displayEventHandlerList() {
			return &itsDisplayEHList;
		}
		// </group>

		// Position, motion and refresh event handlers that will generally
		// be called by a WorldCanvasHolder.
		// <group>
		virtual void positionEH(const WCPositionEvent &ev);
		virtual void motionEH(const WCMotionEvent &ev);
		virtual void refreshEH(const WCRefreshEvent& ev) = 0;
		// </group>

		// clean up the memory used by this DisplayData
		virtual void cleanup() = 0;

		// (Required) copy constructor.
		DisplayData(const DisplayData &other);

		// (Required) copy assignment.
		void operator=(const DisplayData &other);


	private:


		// is this data currently being displayed?
		DisplayState displaystate;

		// Colormap for this DisplayData, and its weight.
		Colormap *itsColormap;
		Float itsColormapWeight;

		// A list of WorldCanvasHolders for which this DisplayData works.
		// The list is maintained by the DisplayData itself based on the
		// notify routines used by the WorldCanvasHolder.  This can be used,
		// for example, to find which WorldCanvas belongs to which
		// WorldCanvasHolder.
		List<WorldCanvasHolder*> itsWCHList;

		// Lists of position, motion, refresh and display event handlers.
		List<WCPositionEH*> itsPositionEHList;
		List<WCMotionEH*> itsMotionEHList;
		List<WCRefreshEH*> itsRefreshEHList;
		List<DisplayEH*> itsDisplayEHList;

		// DD 'Absolute Pixel Coordinates', e.g. channel numbers, are internally
		// 0-based (begin numbering at 0), but certain external user-interface
		// functions (e.g. showPosition(), used for position tracking) have
		// produced 1-based output traditionally for the glish-based viewer.
		// uiBase_, and related methods uiBase() and setUIBase(), allow newer
		// (python/Qt-based) code to cause external ui functions like showValue()
		// report 0-based values instead.  Unless setUIBase(0) is called, the
		// traditional 1-based reporting behavior is retained by default.

		Int uiBase_;		// (initialized to 1).
	};


} //# NAMESPACE CASA - END

#endif
