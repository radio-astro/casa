//# PrincipalAxesDD.h: Base class for axis-bound DisplayData objects
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003,2004
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

#ifndef TRIALDISPLAY_PRINCIPALAXESDD_H
#define TRIALDISPLAY_PRINCIPALAXESDD_H

// aips includes:
#include <casa/aips.h>

// trial includes:
#include <display/Display/DisplayCoordinateSystem.h>

// display library includes:
#include <display/DisplayDatas/DisplayData.h>
#include <display/Utilities/StatusSink.h>

namespace casacore{

	template <class T> class Vector;
	class String;
	template <class T> class ImageInterface;
}

namespace casa { //# NAMESPACE CASA - BEGIN

// forwards:
	class WorldCanvas;
	class WorldCanvasHolder;
	class AttributeBuffer;
	class WCPositionEvent;
	class WCMotionEvent;
	class WCRefreshEvent;
	class DisplayCoordinateSystem;
	class WCCSNLAxisLabeller;
	class PrincipalAxesDM;

	class ImageAnalysis;

// <summary>
// Interface for DisplayDatas which have data arranged in axes.
// </summary>
//
// <synopsis>
// This class adds to the interface defined by DisplayData to
// provide the necessary infrastructure for managing data which
// is arranged in "axes," ie. lattice or column-based data.
// </synopsis>

	class PrincipalAxesDD : public DisplayData {

	public:

		// constructor
		PrincipalAxesDD( casacore::uInt xAxis, casacore::uInt yAxis, casacore::Int mAxis = -1, casacore::Bool axisLabels = true, viewer::StatusSink *sink=0 );

		// destructor
		virtual ~PrincipalAxesDD();

		// casacore::Coordinate transformation handlers, called by WorldCanvasHolder
		// <group>
		virtual casacore::Bool linToWorld(casacore::Vector<casacore::Double> &world,
		                        const casacore::Vector<casacore::Double> &lin);
		virtual casacore::Bool worldToLin(casacore::Vector<casacore::Double> &lin,
		                        const casacore::Vector<casacore::Double> &world);
		virtual casacore::Bool linToFullWorld(casacore::Vector<casacore::Double> &fullWorld,
		                            const casacore::Vector<casacore::Double> &lin);
		// </group>

		// casacore::Format a string containing coordinate and data information at the
		// given world coordinate
		virtual casacore::String showPosition(const casacore::Vector<casacore::Double> &world,
		                            const casacore::Bool &displayAxesOnly = false);

		// casacore::Format a string containing coordinate and data information at the
		// given world coordinate. This function let's you specify output
		// conditions (world/pixel and abs/rel)
		virtual casacore::String showPosition(const casacore::Vector<casacore::Double> &world,
		                            const casacore::Bool &showAbs, const casacore::Bool &displayAxesOnly);

		virtual void setSubstituteTitleText( const casacore::String text );
		// Is the DD is capable (in its current state) of drawing
		// in the current DisplayCoordinateSystem of the WCH's WorldCanvas?
		virtual casacore::Bool conformsToCS(const WorldCanvas &wc);

		// Miscellaneous information supply routines
		// <group>
		virtual casacore::Vector<casacore::String> worldAxisNames() const;
		virtual casacore::Vector<casacore::String> worldAxisUnits() const;
		virtual casacore::Vector<casacore::Double> worldAxisIncrements();
		virtual void worldAxisType(casacore::Coordinate::Type &type, casacore::Int &coordinate,
		                           casacore::Int &axisincoord, const casacore::uInt &worldaxisnum);
		virtual casacore::String worldAxisCode(const casacore::uInt &worldaxisnum);
		// </group>

		// Return the number of display elements (eg. drawable images) in this
		// DisplayData
		// <group>
		virtual casacore::uInt nelements(const WorldCanvasHolder &wcHolder) const;
		virtual casacore::uInt nelements() const;
		// </group>

		// Set and retrieve the minimum and maximum data values
		// <group>
		virtual void setDataMin(casacore::Double datmin);
		virtual void setDataMax(casacore::Double datmax);
		virtual casacore::Double getDataMin() const;
		virtual casacore::Double getDataMax() const;
		// </group>

		// sizeControlFunction, called by the WorldCanvasHolder to setup the
		// WorldCanvas linear coordinate system
		virtual casacore::Bool sizeControl(WorldCanvasHolder& wcHolder,
		                         AttributeBuffer& holderBuf);

		// Position event handler, called by the WorldCanvasHolder
		virtual void positionEH(const WCPositionEvent& ev);

		// Motion event handler, called by the WorldCanvasHolder
		virtual void motionEH(const WCMotionEvent& ev);


		// refresh handler, called by the WorldCanvasHolder
		virtual void refreshEH(const WCRefreshEvent& ev);

		// label/draw the axes
		virtual casacore::Bool labelAxes(const WCRefreshEvent &ev);
		virtual casacore::Bool canLabelAxes() const;

		// required function to tidy up our elements, primarily
		virtual void cleanup();

		// install the default options for this DisplayData
		virtual void setDefaultOptions();

		// apply options stored in val to the DisplayData; return value
		// true means a refresh is needed...
		virtual casacore::Bool setOptions(casacore::Record &rec, casacore::Record &recOut);


		// retrieve the current and default options and parameter types.
		virtual casacore::Record getOptions( bool scrub ) const;

		// distribute options to all the axis labellers
		virtual casacore::Bool setLabellerOptions(casacore::Record &rec, casacore::Record &recout);

		// retrieve options from the axis labellers
		virtual casacore::Record getLabellerOptions( bool scrub=false ) const;

		// Sets which axes are on display and animator, and positions for
		// animator and sliders.  Pass 'reset' as false if you don't need
		// an unzoom (e.g. when display axes are unchanged).
		virtual void setAxes(const casacore::uInt xAxis, const casacore::uInt yAxis, const casacore::uInt mAxis,
		                     const casacore::IPosition fixedPos, casacore::Bool reset=true);

		// query the number of dimensions in the data
		virtual casacore::uInt nDim() {
			return dataDim();
		}

		// retrieve the DisplayCoordinateSystem
		DisplayCoordinateSystem coordinateSystem() const {
			return itsCoordSys;
		}

		// retrieve the original DisplayCoordinateSystem
		DisplayCoordinateSystem originalCoordinateSystem() const {
			return itsOrigCoordSys;
		}

		// set the DisplayCoordinateSystem and the original DisplayCoordinateSystem
		void setCoordinateSystem(const DisplayCoordinateSystem &coordsys);

		// set the DisplayCoordinateSystem to be the same as the original
		// DisplayCoordinateSystem
		void restoreCoordinateSystem();

		// set a Linear casacore::Coordinate in case of pixToWorld undefined for the
		// blc/trc
		void installFakeCoordinateSystem();
		void removeFakeCoordinateSystem();

		// Return the class name of this DisplayData; useful mostly for
		// debugging purposes, and perhaps future use in the glish widget
		// interface.
		virtual casacore::String className() {
			return casacore::String("PrincipalAxesDD");
		}

		// Get the current fixed position
		virtual casacore::IPosition fixedPosition() const {
			return itsFixedPosition;
		}

		// Get the current display axis numbers
		virtual vector<int> displayAxes() const {
			return itsDisplayAxes;
		}

		// Get the current setting of pixel treatment mode
		virtual casacore::String pixelTreatment() const {
			return itsOptionsPixelTreatment;
		}


		//# (dk note 12/04: Axis Label state is maintained by eight different
		//# PADD methods; someone has begun using the bool below to bypass that
		//# maintenance.  Well, ok; it can work that way as a construction
		//# parameter that is unchanged thereafter.  But simply turning it back
		//# on later will not make that state available or consistent; it will
		//# just cause crashes.  Please see me for what's needed to properly
		//# support this method if you need to restore it).
		//#
		//# void usesAxisLabels(casacore::Bool state) {
		//#   itsUsesAxisLabels = state;
		//# }


		// Convert a 2D worldCoordinate of the WorldCanvas to a full worldcoordinate
		// and full pixel position
		virtual casacore::Bool getFullCoord(casacore::Vector<casacore::Double> &fullWorld,
		                          casacore::Vector<casacore::Double> &fullPixel,
		                          const casacore::Vector<casacore::Double> &world);

		virtual void notifyUnregister(WorldCanvasHolder& wcHolder,
		                              casacore::Bool ignoreRefresh = false);

		// allow external functions to translate axes...
		casacore::Int xlatePixelAxes( casacore::Int idx ) const {
			return itsTransPixelAxes[idx];
		}
		casacore::Int xlateFixedPixelAxes( casacore::Int idx ) const {
			return itsFixedPosition[idx];
		}
		const casacore::String &xaxisStr( ) const {
			return itsOptionsXAxis;
		}
		const casacore::String &yaxisStr( ) const {
			return itsOptionsYAxis;
		}
		const casacore::String &zaxisStr( ) const {
			return itsOptionsZAxis;
		}
		const casacore::String &spectStr( ) const;

		// Convert list of world axis names to pixel axis names
		casacore::Vector<casacore::String> worldToPixelAxisNames (const DisplayCoordinateSystem& cSys) const;

		const casacore::String &spectralunitStr( ) const;
		const static casacore::String HISTOGRAM_RANGE;

		bool hasMovieDimension( ) const {
			return has_nonsingleton_nondegenerate_nondisplayed_axis( *this );
		}

		std::string errorMessage( ) const { return error_string; }

	protected:

		// Set internal index (activeZIndex_) into the list of DMs (which cache
		// individual drawings), according to passed animator index.  This is
		// called by conformsTo() (via conformsToZIndex()), and serves to
		// 'focus' the DD on the current WCH's zIndex.
		// Returns true if there is only one frame, in which case activeZIndex_
		// is set to 0.  Thus a single frame is considered to apply to any
		// canvas zIndex setting (this allows a continuum image to be blinked
		// with a chosen channel of a spectral image, e.g.).
		// If there is more than one frame, the passed zindex is stored, and
		// the return value will indicate whether it lies within the DD's
		// current number of frames.
		virtual casacore::Bool setActiveZIndex_(casacore::Int zindex);

		// Derived classes will implement if applicable (at present,
		// LatticePADDs may draw if they have an image with beam data).
		virtual void drawBeamEllipse_(WorldCanvas* /*wc*/) {  }

		// allow PrincipalAxesDM objects to access the coord system in
		// parent PrincipalAxesDD object
		friend class PrincipalAxesDM;

		virtual void setNumImages(const casacore::uInt nimages) {
			itsNumImages = nimages;
		}

		static bool has_nonsingleton_nondegenerate_nondisplayed_axis( const DisplayData &other );

		// Internal state
		casacore::Bool iAmRubbish;

		// min and max to use for drawing
		casacore::Double datamin;
		casacore::Double datamax;

		// internal bookkeeping
		// number of axes
		casacore::uInt nArrayAxes;
		casacore::uInt nPixelAxes;
		casacore::uInt nWorldAxes;

		// format for diplaying numbers in the tracker
		casacore::Coordinate::formatType itsNotation;

		// do some setup work, part of construction and changing dataset
		void setup(casacore::IPosition fixedPos);

		// functions need by above
		virtual void setupElements();
		virtual void getMinAndMax() = 0;

		// Helper routine for setting up the transpose vectors for the coordinate
		// system.
		casacore::Bool indexInserted(casacore::Int index, casacore::Int length, casacore::Vector<casacore::Int>& testVec);

		// helper to transpose coordinates
		// <group>
		void normalToTransposed(casacore::Vector<casacore::Double>& coord, const casacore::Vector<casacore::Int>& transPos);
		void transposedToNormal(casacore::Vector<casacore::Double>& coord, const casacore::Vector<casacore::Int>& transPos);
		// </group>

		// Set velocity state
		void setVelocityState (DisplayCoordinateSystem& cSys,
		                       const casacore::String& velTypeString,
		                       const casacore::String& unitString);

		// Set Spectral formatting
		void setSpectralFormatting (DisplayCoordinateSystem& cSys,
		                            const casacore::String& velTypeString,
		                            const casacore::String& unitString,
		                            const casacore::String& frequency_system = "");

		// Determine the active image (if any).
		// (Deprecated (7/04).  Use  confromsToZIndex(wch)  instead).
		casacore::Bool findActiveImage(WorldCanvasHolder &wcHolder);

		// (Required) default constructor.
		PrincipalAxesDD();

		// (Required) copy constructor.
		PrincipalAxesDD(const PrincipalAxesDD &other);

		// (Required) copy assignment.
		void operator=(const PrincipalAxesDD &other);

		// ImageAnalysis class does not support complex images...
		// so these functions are used to allow only the
		// LatticePADisplayData<T> template to only return an
		// image analysis object for the non-complex version...
		ImageAnalysis *create_image_analysis( SHARED_PTR<casacore::ImageInterface<float> > ) const;
		ImageAnalysis *create_image_analysis( SHARED_PTR<casacore::ImageInterface<casacore::Complex> > ) const {
			return 0;
		}

		std::string error_string;

	private:

		// axis numbers IN ORIGINAL COORDINATE SYSTEM; zAxisNum = -1 means
		// no third axis in data set
		casacore::Int itsXAxisNum;
		casacore::Int itsYAxisNum;
		casacore::Int itsZAxisNum;

		// the x (0), y (1) and z/movie (2) axis numbers
		vector<int> itsDisplayAxes;

		// Store options:
		// Axis selection ---
		casacore::String itsOptionsXAxis, itsOptionsYAxis, itsOptionsZAxis;
		casacore::IPosition itsFixedPosition;

		// treatment of edge pixels
		casacore::String itsOptionsPixelTreatment;

		// number of elements (images) in this DisplayData
		casacore::uInt itsNumImages;

		// This CS is the one we were constructed with
		DisplayCoordinateSystem itsOrigCoordSys;
		DisplayCoordinateSystem itsCoordSysBackup;

		// This is the working CS which is transposed and has axes removed
		// all over the place

		// (dk note: The authors explain this poorly, and don't seem to make
		// the distinction even for themselves very well: there are _two_
		// relevant coordinate transformations here, not just one.  itsOrigCoordSys
		// defines the world space for the _data_ pixels.  itsCoordSys defines the
		// world space for the _canvas_.  Control and setting of the canvas CS is
		// poorly modularized, and the DDs remain involved in too much of the
		// _canvas's_ coordinate translation chores).

		DisplayCoordinateSystem itsCoordSys;

		// casacore::List of AxisLabellers.
		casacore::PtrBlock<void *> itsAxisLabellers;

		// Aspect selection ---
		casacore::String itsOptionsAspect;

		// Position tracking variables
		// This CS is the one we use to set the appropriate state for
		// PositionTracking.    It is pretty much the same as itsOriginalCoordSys
		// but the formtting and velocity state (if there is a SC) may differ

		DisplayCoordinateSystem itsPosTrackCoordSys;
		casacore::String itsSpectralUnit;
		casacore::String itsSpectralQuantity;
		casacore::Bool itsAbsolute;
		casacore::Bool itsFractionalPixels;

		// buffer for string sizecontrol done by this ImageDisplayData
		AttributeBuffer sizeControlBuf;

		// the coordinates of the axes that do not vary
		casacore::Vector<casacore::Double>    itsAddPixPos;

		// how the axes are permuted
		casacore::Vector<casacore::Int>       itsTransPixelAxes;
		casacore::Vector<casacore::Int>       itsTransWorldAxes;

		// world ranges for toMix conversions
		casacore::Vector<casacore::Double> itsWorldMin, itsWorldMax;

		// Temporaries  for coordinate conversions.
		// You should only use one temporary per function.
		casacore::Vector<casacore::Double> itsWorldInTmp1, itsPixelInTmp1;
		casacore::Vector<casacore::Bool> itsWorldAxesTmp1, itsPixelAxesTmp1;
		casacore::Vector<casacore::Double> itsFullWorldTmp1;
		//
		casacore::Vector<casacore::Double> itsPixelInTmp2;
		//
		casacore::Vector<casacore::Double> itsWorldInTmp3, itsWorldOutTmp3;
		casacore::Vector<casacore::Double> itsPixelInTmp3, itsPixelOutTmp3;
		casacore::Vector<casacore::Bool> itsWorldAxesTmp3, itsPixelAxesTmp3;
		//
		casacore::Vector<casacore::Double> itsFullWorldTmp4, itsFullPixelTmp4;

		casacore::Bool itsUsesAxisLabels;

		// Find out if casacore::SpectralCoordinate can have velocity units
		casacore::Bool canHaveVelocityUnit (const DisplayCoordinateSystem& cSys) const;

		// Remove list of pixel axes from CS
		void removePixelAxes (DisplayCoordinateSystem& cSys,
		                      casacore::uInt startAxis,
		                      const casacore::IPosition& fixedPosition);

		viewer::StatusSink *ssink;
		casacore::String titleText;

	};


} //# NAMESPACE CASA - END

#endif
