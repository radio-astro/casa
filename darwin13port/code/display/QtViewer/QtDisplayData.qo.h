//# QtDisplayData.qo.h: Qt DisplayData wrapper.
//# Copyright (C) 2005
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

#ifndef QTDISPLAYDATA_H
#define QTDISPLAYDATA_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <display/Display/DParameterChoice.h>
#include <display/Display/DParameterRange.h>
#include <casa/BasicMath/Math.h>
#include <display/Display/DisplayEnums.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayDatas/DisplayDataOptions.h>
#include <display/Utilities/ImageProperties.h>
#include <vector>


#include <graphics/X11/X_enter.h>
#include <QtCore>
#include <QObject>
#include <graphics/X11/X_exit.h>

#include <tr1/memory>


namespace casa { //# NAMESPACE CASA - BEGIN

	class String;
	class Record;
	class DisplayData;
	class WCMotionEvent;
	template <class T> class ImageInterface;
	class QtDisplayPanel;
	class ImageRegion;
	class WorldCanvasHolder;
	class Colormap;
	class QtDisplayPanelGui;
	class WedgeDD;

	class QtDisplayData : public QObject {

		Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.

	public:

		static std::string path(const DisplayData *);

		QtDisplayData( QtDisplayPanelGui *panel, String path, String dataType, String displayType,
		               const viewer::DisplayDataOptions &ddo = viewer::DisplayDataOptions( ),
		               const viewer::ImageProperties &props = viewer::ImageProperties( ) );
		~QtDisplayData();
		String getPositionInformation( const Vector<double> world);
		virtual std::string name() {
			return name_;
		}
		virtual const char* nameChrs() {
			return name_.c_str();
		}
		virtual void setName(const std::string& name) {
			name_ = name;
		}
		const std::string DISPLAY_RASTER;
		const std::string DISPLAY_CONTOUR;
		const std::string DISPLAY_VECTOR;
		const std::string DISPLAY_MARKER;
		virtual std::string dataType() const {
			return dataType_;
		}
		virtual std::string displayType() {
			return displayType_;
		}

		//Display Type
		Bool isRaster() const;
		Bool isContour() const;
		Bool isVector() const;
		Bool isMarker() const;
		Bool isImage() const;

		//virtual Bool delTmpData() const;
		virtual void delTmpData() const;
		virtual void setDelTmpData(Bool delTmpData);

		std::string description( ) const;
		std::string path( ) const {
			return path_;
		}

		virtual String errMsg() {
			return errMsg_;
		}

		// retrieve the Record of options.  This is similar to a 'Parameter Set',
		// containing option types, default values, and meta-information,
		// suitable for building a user interface for controlling the DD.
		virtual Record getOptions();

		// retrieve wrapped DisplayData.
		//# (should probably be private, and 'friend'ed only to QtDP, which
		//# needs it for purposes like registration...).
		virtual DisplayData* dd() {
			return dd_;
		}

		// Did creation of wrapped DD fail?
		virtual Bool isEmpty() {
			return dd_==0;
		}



		// Possible valuse: Raster, Vector, Annotation, CanvasAnnotation
		virtual Display::DisplayDataType ddType();
		bool isSkyCatalog() const;
		bool isMS() const;


		// Can the QDD display tracking information?
		virtual Bool usesTracking() {
			return !isEmpty() &&
			       ddType()!=Display::Annotation &&
			       ddType()!=Display::CanvasAnnotation;
		}

		// Returns a String with value and position information,
		// suitable for a cursor tracking display.
		virtual pair<String,String> trackingInfo(const WCMotionEvent& ev);


		// Convert 2-D 'pseudoregion' (or 'mouse region' Record, from the region
		// mouse tools) to a full Image Region, with same number of axes as the
		// DD's Lattice (and relative to its DisplayCoordinateSystem).
		// Return value is 0 if the conversion can't be made or does not apply
		// to this DD for any reason (ignored by non-Lattice DDs, e.g.).
		//
		// If allChannels==True, the region is extended along spectral axis, if
		// it exists (but ONLY if the spectral axis is not also on display; the
		// visible mouse region always defines region shape on the display axes).
		//
		// If allAxes is True, the region is extended over all (non-display)
		// axes (including any spectral axis; allAxes=True makes the allChannels
		// setting irrelevant).
		//
		// If both allchannels and allAxes are False (the default), the region
		// will be confined to the currently-displayed plane.
		//
		// -->If the returned ImageRegion* is non-zero, the caller
		// -->is responsible for deleting it.
		virtual ImageRegion* mouseToImageRegion(Record mouseRegion,
		                                        WorldCanvasHolder* wch,
		                                        Bool allChannels=False,
		                                        Bool allPols=False,
		                                        Bool allRAs=False,
		                                        Bool allDECs=False,
		                                        Bool allAxes=False);
		virtual ImageRegion* mouseToImageRegion(
		    Record mouseRegion, WorldCanvasHolder* wch,
		    String& extChan, String& extPol);

		// Print statistics on image for given region.
		// Returns False if unable to do so.
		virtual Bool printRegionStats(ImageRegion& imgReg);
		virtual Bool printLayerStats(ImageRegion& imgReg);


		// Return the number of the spectral axis within the DD's original
		// image lattice and coordinate system (-1 if none).
		//virtual Int spectralAxis();
		virtual Int getAxisIndex(String axtype=String("Spectral"));

		//#  colorbar methods


		// Would this QDD want to display a color bar if registered and
		// conformant for drawing?
		virtual Bool wouldDisplayColorBar() {
			bool displayColorBar = false;
			bool wouldDisplayColorBar = false;
			if ( colorBarDisplayOpt_ != NULL ) {
				if (colorBarDisplayOpt_->value() == WEDGE_YES) {
					displayColorBar = true;
				}
				wouldDisplayColorBar = hasColorBar() && displayColorBar;
			}
			return wouldDisplayColorBar;
		}


		// Is a color bar WDD defined for this QDD?
		virtual Bool hasColorBar() {
			bool colorBarExists = false;
			if ( colorBar_!= 0 ) {
				colorBarExists = true;
			}
			return colorBarExists;
		}


		// User-requested adjustment to colorbar thickness (will probably
		// remain at the default value of 1).  Used by QtDisplayPanel to
		// aid in sizing colorbar panels.
		virtual Float colorBarSizeAdj() {
			if(!wouldDisplayColorBar()) return 0.;
			return max(colorBarThicknessOpt_->minimum(),
			           min(colorBarThicknessOpt_->maximum(),
			               colorBarThicknessOpt_->value()));
		}


		// Used (by QtDisplayPanel) to compute margin space for colorbar labels.
		// It is the (pgplot) character size for colorbar labels (default 1.2)
		// times a label space 'adjustment' user option (default 1.).
		virtual Float colorBarLabelSpaceAdj();


		// Retrieve color bar ('wedge') DD.  (0 if none.  It will exist if
		// hasColorBar() is True).  (Not for general use).
		//# (should probably be private, and 'friend'ed only to QtDP, which
		//# which manages its registration and other aspects).
		virtual WedgeDD* colorBar() {
			return colorBar_;
		}

		Colormap* getColorMap() const;


		// Does this DD currently own a colormap?
		virtual Bool hasColormap() const {
			return clrMap_!=0;
		}
		// Different DisplayDatas *could* have different colormap palettes
		// thus this is non-static and specific to a display data
		virtual bool isValidColormap( const QString &name ) const;
		void setColorMap( Colormap* colorMap );
		//Set the transparence of the color map for overlaying images.
		Bool setColormapAlpha( uInt alpha );
		void removeColorMap( const String& name );

		// Get/set colormap shift/slope ('fiddle') and brightness/contrast
		// settings.  (At present this is usually set for the PC's current
		// colormap via mouse tools.  These may want to to into get/setOptions
		// (thus into guis) eventually...).  Return value will be False if
		// DD has no colormap [at present].
		//<group>
		virtual Bool getCMShiftSlope(Vector<Float>& params) const;
		virtual Bool getCMBrtCont(Vector<Float>& params) const;
		virtual Bool setCMShiftSlope(const Vector<Float>& params);
		virtual Bool setCMBrtCont(const Vector<Float>& params);
		//</group>

		//Return the name of the z-axis.
		String getZAxisName();

		//# (dk note: If you use this, you must assure you do not change it in ways
		//# that will crash QDD.  Do not assume it is non-zero -- im_ may be zero if
		//# the QDD's image is complex.  However, if it _is_ non-zero, you should
		//# be able to assume it will exist for the life of the QDD).
		std::tr1::shared_ptr<ImageInterface<Float> > imageInterface() {
			return im_;
		}
		const viewer::ImageProperties &imageProperties( );

		// force unlocking of paged images
		void unlock( );
		static const String WEDGE_LABEL_CHAR_SIZE;
		static const String WEDGE_YES;


		void init();
		void initImage();
		void setImage(std::tr1::shared_ptr< ImageInterface<Float> > img);
		static void setGlobalColorOptions( bool global );
		void setInvertColorMap( bool invert );

	public slots:

		// (Should only be used by QtDisplayPanels to notify the QDD that it
		// has been registered, or is about to be unregistered, on the QDP).
		//<group>
		virtual void registerNotice(QtDisplayPanel*);
		virtual void unregisterNotice(QtDisplayPanel*);
		//</group>

		// Apply option values to the DisplayData.  Method will
		// emit optionsChanged() if other option values, limits, etc.
		// should also change as a result.
		// Set emitAll = True if the call was not initiated by the options gui
		// itself (e.g. via scripting or save-restore); that will assure that
		// the options gui does receive all option updates (via the optionsChanged
		// signal) and updates its user interface accordingly.

		virtual void setOptions(Record opts, Bool emitAll=False);
		void emitOptionsChanged( Record changedOpts );
		void setPlotTitle();

		virtual void done();


		virtual void checkAxis();


		const String &getColormap( ) {
			return clrMapName_;
		}
		void setColormap(const String& clrMapName) {
			setColormap_(clrMapName);
		}
		void setHistogramColorMapping( float minValue, float maxValue, float powerScale );
		//  This is used to get the display data to set a saturation range
		//  from another image.
		void setSaturationRange( double min, double max );

	signals:

		// Signals changes the DD has made internally to option values, limits,
		// etc., that ui (if any) will want to reflect.  Calling setOptions()
		// to change one option value may cause this to be emitted with any other
		// options which have changed as a result.
		void optionsChanged(Record changedOptions);

		// Emitted when problems encountered (in setOptions, e.g.)
		void qddError(String errmsg);

		// Emitted when options successfully set without error.
		//# (same purpose as above -- clean this up).
		void optionsSet();

		// Emitted when color bars may need rearrangement on panels where
		// this QDD is registered.  QDP connects this to its checkColorBars_()
		// slot.
		void colorBarChange();

		// Emitted when something in the DD (besides mouse movement) means
		// that tracking data for this QDD could be usefully recomputed
		// and redisplayed.  Underlying DDs can indicate this in setOptions
		// by defining the "trackingchange" field in chgdOpts (recOut).
		void trackingChange(QtDisplayData*);


		//# Emitted when an operation is successfully completed.  It is intended
		//# that, e.g., a gui that responds to qddError signals by setting
		//# an error message onto a status line vould clear the status line
		//# when this signal occurs.
		//# (let qdpg clear status line itself b4 QDP calls instead?...)
//#void qddOK();

		//# This object will be destroyed after this signal is processed.
		//# (Note: if this DD is managed in QtViewer's list, it is preferable
		//# to connect to QtViewerBase::ddRemoved() instead).
//# void dying(QtDisplayData*);

		// (mkApr2012) axisChanged and axisChangedProfile have the identical
		// functionality, to send out the names of the current x/y/z axes.
		// The profiler needs to be notified first such subsequent  plot
		// request from regions are accepted. For that reason there is the
		// signal axisChangedProfile which connects to the profiler and is
		// emitted prior to axisChanged (in QtDisplayData::checkAxis())
		void axisChanged(String, String, String, std::vector<int> );
		void axisChangedProfile(String, String, String, std::vector<int> );

		void spectrumChanged(String spcTypeUnit, String spcRval, String spcSys);

		void statsReady(const String&);
		void showColorHistogram( QtDisplayData* );

		void globalOptionsChanged( QtDisplayData*, Record);

	protected slots:

		// Set the color bar orientation option according to the master
		// value stored in the QtViewerBase (panel_->viewer()->colorBarsVertical_).
		// Connected to QtViewerBase's colorBarOrientationChange() signal;
		// also called during initialization.
		virtual void setColorBarOrientation_();


	protected:

		// Heuristic used internally to set initial axes to display on X, Y and Z,
		// for PADDs.  shape should be that of Image/Array, and have same nelements
		// as axs.  On return, axs[0], axs[1] and (if it exists) axs[2] will be axes
		// to display initially on X, Y, and animator, respectively.
		// If you pass a CS for the image, it will give special consideration to
		// Spectral [/ Direction] axes (users expect their spectral axes on Z, e.g.)
		//# (Lifted bodily from GTkDD).
		virtual void getInitialAxes_(Block<uInt>& axs, const IPosition& shape,
		                             const DisplayCoordinateSystem &cs);

		// Set named colormap onto underlying dd (called from public setOptions()).
		// Pass "" to remove/delete any existing colormap for the QDD.
		// In the case that no colormap is set on a dd that needs one (raster dds,
		// mostly), the drawing canvas will provide a default.
		// See ColormapDefinition.h, and the Table gui/colormaps/default.tbl (in the
		// data repository) for the list of valid default colormap names (and
		// information on creating/installing custom ones).  If an invalid name is
		// passed, an (ignorable) error message is signalled, and the dd's colormap
		// will remain unchanged.
		virtual void setColormap_(const String& clrMapName, bool invertChanged = false);
		virtual void removeColormap_() {
			setColormap_("");
		}



		//# (could be exposed publicly, if useful).
		//  Does this DD use/need a public colormap?
		virtual Bool usesClrMap_() {
			return (isRaster() || displayType_=="pksmultibeam");
		}
		//# These are the only DD types currently needing a colormap and
		//# supporting the selection option; add more if/when needed....

		// Can this QDD use a color bar?
		virtual Bool usesColorBar_() {
			return isRaster();
		}

		typedef std::map<const DisplayData*,QtDisplayData*> data_to_qtdata_map_type;
		static data_to_qtdata_map_type dd_source_map;

	private:
		// Not intended for use.
		QtDisplayData() : panel_(0), im_(), cim_(), dd_(0) {  }
		static bool globalColorSettings;
		bool setColorBarOptions( Record& opts, Record& chgdOpts );
		void checkGlobalChange( Record& chgdOpts );

		//If global color settings is checked and a new QtDisplayData is added,
		//it should pick up the global color settings already in place.
		void initGlobalColorSettings();
		void initColorSettings();
		Record getGlobalColorChangeRecord( Record& opts ) const;
		//# data

		QtDisplayPanelGui *panel_;
		std::string path_, dataType_, displayType_;

		const std::string TYPE_IMAGE;
		const std::string SKY_CATALOG;
		const std::string MS;
		std::tr1::shared_ptr<ImageInterface<Float> > im_;
		std::tr1::shared_ptr<ImageInterface<Complex> > cim_;
		DisplayData* dd_;

		std::string name_;

		bool invertColorMap;
		// Name of colormap used by dd_  ("" if none)
		String clrMapName_;
		// Color maps can be removed.  In such a case, the restoreColorMapName
		//holds the previous one so we can reset to that in case the one we
		//are using is removed.
		String restoreColorMapName;

		// Will be set onto underlying dd_ if needed (0 if none)
		Colormap* clrMap_;

		// Parses colormap choice out of a setOptions record.
		DParameterChoice* clrMapOpt_;

		// All the valid ('primary') colormap names.
		// (This interface doesn't support use of 'synonym' names).
		typedef ColormapDefinition::colormapnamemap colormapnamemap;
		colormapnamemap clrMapNames_;

		// Set of colormaps currently or previously used by this DD.  Once
		// a Colormap is created, it is retained for the life of the DD.
		//# (A main reason for this is to remember the colormap's
		//# 'transfer function' state (brightness/contrast/shift/slope) in case
		//# it is reused -- see PCITFiddler.h (colormap mouse tools)).
		typedef std::map<String, Colormap*> colormapmap;
		colormapmap clrMaps_;
		const static String COLOR_MAP;


		// Latest error message, retrievable via errMsg().  (Where possible, errors
		// are indicated via the qddError signal rather than a throw, and the code
		// attempts something sensible in case the caller chooses to ignore it).
		String errMsg_;


		// The DD that draws the color bar key for the main DD -- 0 if N/A.
		WedgeDD* colorBar_;

		//# User interface parsing objects for some color bar parameters that are
		//# controlled externally to the WedgeDD (colorBar_): whether to display
		//# the color bar, its thickness, label (margin) space, orientation,
		//# and character size.
		//# Changes to any of these DisplayParameters means that the arrangement
		//# of colorbars must be checked on all QtDisplayPanels where this QDD
		//# is registered (orientation change is sent to base viewer class and
		//# handled there).

		// Is color bar display turned on? ("Yes" / "No")
		DParameterChoice*       colorBarDisplayOpt_;

		// Manual user adjustment factor for color bar thickness.  Hopefully
		// the automatic choice (by QtDisplayPanel) will be adequate in most
		// cases, and this can remain at the default value of 1.
		DParameterRange<Float>* colorBarThicknessOpt_;

		// Manual user adjustment factor for color bar label space.  Hopefully
		// the automatic choice (by QtDisplayPanel) will be adequate in most
		// cases, and this can remain at the default value of 1.
		DParameterRange<Float>* colorBarLabelSpaceOpt_;

		// "horizontal" / "vertical"
		DParameterChoice*       colorBarOrientationOpt_;

		// Size of label characters on color bar (affects margins only).
		//# (WedgeDD reacts to this option, but it is also monitored on this level).
		DParameterRange<Float>* colorBarCharSizeOpt_;

		viewer::ImageProperties image_properties;

	};


} //# NAMESPACE CASA - END


//# Allows QtDisplayData* to be stored in a QVariant; e.g., to be
//# the data associated with a QAction....  See QMetaType and
//# QVariant class doc.  QVariants are rather well-designed
//# generic value holders.
//# Note: this declaration cannot be placed within the casa namespace.
Q_DECLARE_METATYPE(casa::QtDisplayData*)


#endif
