//# MultiPVTool.cc: Base class for MultiWorldCanvas event-based rectangle tools
//# Copyright (C) 2000,2001,2002
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

#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/MultiPVTool.h>
#include <display/region/PVLine.qo.h>
#include <casa/BasicMath/Math.h>
#include <display/Display/PanelDisplay.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <images/Images/ImageInterface.h>
#include <images/Regions/WCBox.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <images/Images/SubImage.h>
#include <images/Images/ImageStatistics.h>
#include <display/DisplayDatas/MSAsRaster.h>
#include <casadbus/types/nullptr.h>

// sometimes (?) gcc fails to instantiate this function, so this
// explicit instantiation request may be necessary... <drs>
// template bool casa::memory::operator==(casa::std::tr1::shared_ptr<casa::viewer::PVLine> const&, casa::viewer::PVLine*);


namespace casa { //# NAMESPACE CASA - BEGIN

	static inline int FLOOR( double v ) {
		return ifloor(v + .5);
	}

	MultiPVTool::MultiPVTool( viewer::RegionSourceFactory *rcs, PanelDisplay* pd,
			Display::KeySym keysym, const Bool /*persistent*/ ) :
		RegionTool(keysym),
		rfactory(rcs->newSource(this)),
		itsPVLineExists(False),
		itsActive(False),
		itsP1(2), itsP2(2),
		itsHX(4), itsHY(4),
		itsLastPressTime(-1.),		// 'long ago..'
		its2ndLastPressTime(-1.),
		pd_(pd) {  }

	MultiPVTool::~MultiPVTool() {
		reset();
	}

	void MultiPVTool::disable() {
		reset();
		MultiWCTool::disable();
	}

	void MultiPVTool::keyPressed(const WCPositionEvent &ev) {

		Int x = ev.pixX();
		Int y = ev.pixY();
		WorldCanvas *wc = ev.worldCanvas( );

		if ( ! wc->inDrawArea(x, y) ) return;

		its2ndLastPressTime = itsLastPressTime;
		itsLastPressTime = ev.timeOfEvent();

		double linx1, liny1;
		try {
			viewer::screen_to_linear( wc, x, y, linx1, liny1 );
		} catch(...) {
			return;
		}
#if INCLUDE_OLD_CODE
		// traverse in reverse order because the last region created is "on top"...
		// check for click within a handle...
		for ( pvlinelist::reverse_iterator iter = rectangles.rbegin(); iter != rectangles.rend(); ++iter ) {
			if ( (resizing_region_handle = (*iter)->clickHandle( linx1, liny1 )) != 0 ) {
				resizing_region = *iter;		// enter resizing state
				moving_regions.clear( );		// ensure that moving state is clear...
				return;
			}
		}

		// check for click within one (or more) regions...
		resizing_region = memory::anullptr;
		moving_regions.clear( );			// ensure that moving state is clear...
		for ( pvlinelist::iterator iter = rectangles.begin(); iter != rectangles.end(); ++iter ) {
			if ( (*iter)->clickWithin( linx1, liny1 ) )
				moving_regions.push_back(*iter);
		}
		moving_linx_ = linx1;
		moving_liny_ = liny1;
		if ( moving_regions.size( ) > 0 ) return;
#endif
		// start new rectangle
		start_new_rectangle( wc, x, y );		// enter resizing state
		refresh( );
		rectangleReady();
		return;
	}

	void MultiPVTool::moved(const WCMotionEvent &ev, const viewer::region::region_list_type &selected_regions ) {

		if (ev.worldCanvas() != itsCurrentWC) return;  // shouldn't happen

		Int x = ev.pixX();
		Int y = ev.pixY();
		if ( ! itsCurrentWC->inDrawArea(x, y) ) return;

		double linx, liny;
		try {
			viewer::screen_to_linear( itsCurrentWC, x, y, linx, liny );
		} catch(...) {
			return;
		}

		bool refresh_needed = false;
		bool region_selected = false;
		if ( memory::anullptr.check(resizing_region) == false ) {
			// resize the rectangle
			double linx1, liny1;
			try {
				viewer::screen_to_linear( itsCurrentWC, x, y, linx1, liny1 );
			} catch(...) {
				return;
			}
			resizing_region_handle = resizing_region->moveHandle( resizing_region_handle, linx1, liny1 );
			refresh_needed = true;
			// return;
		}

		std::tr1::shared_ptr<viewer::Region> creation(viewer::Region::creatingRegion( ));
		if ( memory::anullptr.check(creation) || checkType(creation->type( )) ) {
			int size = selected_regions.size( );
			pvlinelist processing = rectangles;
			for ( pvlinelist::reverse_iterator iter = processing.rbegin(); iter != processing.rend(); ++iter ) {
				unsigned int result = (*iter)->mouseMovement(linx,liny,size > 0);
				refresh_needed = refresh_needed | viewer::Region::refreshNeeded(result);
				region_selected = region_selected | viewer::Region::regionSelected(result);
			}

			if ( refresh_needed ) {
				refresh( );
			}
		}

#if USE_TRANSLATE_MOVING_REGIONS_INSTEAD
		// there seems to be skew between this code and RegionToolManager::translate_moving_regions
		if ( moving_regions.size( ) > 0 ) {
			// resize the rectangle
			double linx1, liny1;
			try {
				viewer::screen_to_linear( itsCurrentWC, x, y, linx1, liny1 );
			} catch(...) {
				return;
			}
			double dx = linx1 - moving_linx_;
			double dy = liny1 - moving_liny_;
			for( pvlinelist::iterator iter = moving_regions.begin( ); iter != moving_regions.end( ); ++iter ) {
				(*iter)->move( dx, dy );
			}
			moving_linx_ = linx1;
			moving_liny_ = liny1;
		}
#endif

		update_stats( ev );
		//  commented out: Fri Aug  5 14:09:07 EDT 2011
		// for ( pvlinelist::reverse_iterator iter = rectangles.rbegin(); iter != rectangles.rend(); ++iter ) {
		//     (*iter)->updateStatistics( true );
		// }

		// fprintf( stderr, "<3>" );
		// refresh();			// draw over in new state
		// updateRegion();
	}

	void MultiPVTool::keyReleased(const WCPositionEvent &ev) {

		// avoid degenerate rectangles...
		if ( memory::anullptr.check(creating_region) == false ) {
			viewer::Region::creatingRegionEnd( );
			if ( creating_region->degenerate( ) ) {
				viewer::Region *nix = creating_region.get( );
				rfactory->revokeRegion(nix);
				if ( creating_region == resizing_region ) {
					resizing_region = memory::anullptr;
					resizing_region_handle = 0;
				}
			}
			creating_region = memory::anullptr;
		}

		// 	Bool wasActive = itsActive;
		itsActive = False;
		if ( rectangles.size( ) == 0 ) {
			if (ev.timeOfEvent() - its2ndLastPressTime < doubleClickInterval()) {
				Int x = ev.pixX();
				Int y = ev.pixY();
				// Another link to Honglin's spectral profile hooks... but what's it do (it emits "echoClicked(clickPoint)") ? Double click without a rectangle...
				// #0  casa::QtRTRegion::doubleClicked (this=0x1080bf200, x=270, y=347) at /Users/drs/dev/viewer/code/display/implement/QtViewer/QtMouseTools.cc:150
				// #1  0x0000000100244448 in casa::MultiPVTool::keyReleased (this=0x1080bf210, ev=@0x7fff5fbfde80) at /Users/drs/dev/viewer/code/display/implement/DisplayEvents/MultiPVTool.cc:210
				// #2  0x0000000100228726 in casa::MultiWCTool::operator() (this=0x1080bf210, ev=@0x7fff5fbfde80) at /Users/drs/dev/viewer/code/display/implement/DisplayEvents/MultiWCTool.cc:169
				// #3  0x0000000100309759 in casa::WorldCanvas::callPositionEventHandlers (this=0x107cbb220, ev=@0x7fff5fbfde80) at /Users/drs/dev/viewer/code/display/implement/Display/WorldCanvas.cc:208
				// #4  0x00000001003174de in casa::WorldCanvas::operator() (this=0x107cbb220, pev=@0x7fff5fbfe3c0) at /Users/drs/dev/viewer/code/display/implement/Display/WorldCanvas.cc:357
				//
				// --- "echoClicked(Record)" winds its way through:
				// -------------------------- ./QtViewer/FileBox.cc --------------------------
				//   connect(qdp_, SIGNAL(activate(Record)),
				//                 SLOT(activate(Record)) );
				// void FileBox::activate(Record rcd) {
				// -------------------------- ./QtViewer/MakeMask.cc --------------------------
				//   connect(qdp_, SIGNAL(activate(Record)),
				//                 SLOT(activate(Record)) );
				// void MakeMask::activate(Record rcd) {
				// -------------------------- ./QtViewer/MakeRegion.cc --------------------------
				//   connect(qdp_, SIGNAL(activate(Record)),
				//                 SLOT(activate(Record)) );
				// void MakeRegion::activate(Record rcd) {
				// -------------------------- ./QtViewer/QtRegionManager.cc --------------------------
				//   connect(qdp_, SIGNAL(activate(Record)),
				//                 SLOT(activate(Record)) );
				// void QtRegionManager::activate(Record rec) {
				// -----------------------------------------------------------------------------------
				doubleClicked(x, y);
			} else return;
		}

		if ( ev.worldCanvas() != itsCurrentWC ) {
			reset();    // shouldn't happen.
			return;
		}

		if ( memory::anullptr.check(resizing_region) == false ) {
			// resize finished
			resizing_region = memory::anullptr;
		}

		if ( moving_regions.size( ) > 0 ) {
			// moving finished
			moving_regions.clear( );
		}

		if ( ev.modifiers( ) & Display::KM_Double_Click ) {

			// double click--invoke callbacks
			itsEmitted = True;
			itsLastPressTime = its2ndLastPressTime = -1.0;

			Int x = ev.pixX(), y = ev.pixY();
			double linx1, liny1;
			try {
				viewer::screen_to_linear( itsCurrentWC, x, y, linx1, liny1 );
			} catch(...) {
				return;
			}

			pvlinelist selected_regions;
			for ( pvlinelist::iterator iter = rectangles.begin(); iter != rectangles.end(); ++iter ) {
				if ( (*iter)->clickWithin( linx1, liny1 ) )
					selected_regions.push_back(*iter);
			}

			if ( selected_regions.size( ) > 0 ) {
				std::string errMsg_;
				std::map<String,bool> processed;
				DisplayData *dd = 0;
				List<DisplayData*> *dds = pd_->displayDatas( );
				for ( ListIter<DisplayData *> ddi(*dds); ! ddi.atEnd( ); ++ddi ) {
					dd = ddi.getRight( );
					MSAsRaster *msar = dynamic_cast<MSAsRaster*>(dd);
					if ( msar == 0 ) continue;
					for ( pvlinelist::iterator iter = selected_regions.begin(); iter != selected_regions.end(); ++iter ) {
						(*iter)->flag( msar );
					}
				}
			}

			refresh();	// current WC still valid, until new rect. started.
			// In particular, still valid during callbacks below.
		}

	}


	void MultiPVTool::otherKeyPressed(const WCPositionEvent &ev) {
		const int pixel_step = 1;
		WorldCanvas *wc = ev.worldCanvas( );
		if ( wc == itsCurrentWC &&
		        ( ev.key() == Display::K_Escape ||
		          ev.key() == Display::K_Left ||
		          ev.key() == Display::K_Right ||
		          ev.key() == Display::K_Up ||
		          ev.key() == Display::K_Down ) ) {
			uInt x = ev.pixX();
			uInt y = ev.pixY();

			resizing_region = memory::anullptr;
			moving_regions.clear( );		// ensure that moving state is clear...

			double linx, liny;
			try {
				viewer::screen_to_linear( wc, x, y, linx, liny );
			} catch(...) {
				return;
			}

			bool refresh_needed = false;
			for ( pvlinelist::iterator iter = rectangles.begin(); iter != rectangles.end(); ) {
				if ( (*iter)->regionVisible( ) ) {
					unsigned int result = (*iter)->mouseMovement(linx,liny,false);
					if ( viewer::Region::regionSelected(result) ) {
						if ( ev.key() == Display::K_Escape ) {
							pvlinelist::iterator xi = iter;
							++xi;
							rectangles.erase( iter );
							refresh_needed = true;
							iter = xi;
						} else {
							double dx=0, dy=0;
							try {
								switch ( ev.key( ) ) {
								case Display::K_Left:
									viewer::screen_offset_to_linear_offset( wc, -pixel_step, 0, dx, dy );
									break;
								case Display::K_Right:
									viewer::screen_offset_to_linear_offset( wc, pixel_step, 0, dx, dy );
									break;
								case Display::K_Down:
									viewer::screen_offset_to_linear_offset( wc, 0, -pixel_step, dx, dy );
									break;
								case Display::K_Up:
									viewer::screen_offset_to_linear_offset( wc, 0, pixel_step, dx, dy );
									break;
								default:
									break;
								}
							} catch(...) {
								return;
							}
							(*iter)->move( dx, dy );
							refresh_needed = true;
							++iter;
						}
					} else {
						++iter;
					}
				} else {
					++iter;
				}
			}
			if ( refresh_needed ) refresh( );
		}
	}


	void MultiPVTool::set( const Int &x1, const Int &y1,
	                       const Int &x2, const Int &y2) {
		if (!itsCurrentWC) return;
		Vector<Double> pix(2);
		pix(0)=x1;
		pix(1)=y1;
		itsCurrentWC->pixToLin(itsP1, pix);
		pix(0)=x2;
		pix(1)=y2;
		itsCurrentWC->pixToLin(itsP2, pix);
	}

	void MultiPVTool::get(Int &x1, Int &y1, Int &x2, Int &y2) const {
		if (!itsCurrentWC) return;
		get(x1,y1);
		Vector<Double> pix(2);
		itsCurrentWC->linToPix(pix, itsP2);
		x2 = ifloor(pix(0)+.5);
		y2 = ifloor(pix(1)+.5);
	}

	void MultiPVTool::get(Int &x1, Int &y1) const {
		if (!itsCurrentWC) return;
		Vector<Double> pix(2);
		itsCurrentWC->linToPix(pix, itsP1);
		x1 = ifloor(pix(0)+.5);
		y1 = ifloor(pix(1)+.5);
	}


	void MultiPVTool::draw(const WCRefreshEvent&/*ev*/, const viewer::region::region_list_type &selected_regions) {
		for ( pvlinelist::iterator iter = rectangles.begin(); iter != rectangles.end(); ++iter )
			(*iter)->draw( selected_regions.size( ) > 0 );
	}

	void MultiPVTool::reset(Bool skipRefresh) {
		itsActive = False;
		Bool wasShowing = rectangleDefined();
		itsPVLineExists= False;
		if(wasShowing && !skipRefresh) refresh();
		itsLastPressTime = its2ndLastPressTime = -1.;
	}

	void MultiPVTool::revokeRegion( viewer::Region *r ) {
		viewer::PVLine *rect = dynamic_cast<viewer::PVLine*>(r);
		if ( rect == 0 ) return;

		// pvlinelist::iterator iter = find( rectangles.begin(), rectangles.end(), (viewer::PVLine*) r );
		for ( pvlinelist::iterator iter = rectangles.begin(); iter != rectangles.end(); ++iter ) {
			if ( (*iter).get( ) == rect ) {
				rectangles.erase( iter );
				refresh( );
				break;
			}
		}
	}


	/**********************************************************************************************************
	***********************************************************************************************************
	ImageRegion* QtDisplayData::mouseToImageRegion( Record mouseRegion, WorldCanvasHolder* wch,
						    String& extChan, String& extPol) {

	if (dd_ == 0 || (im_ == 0 && cim_ == 0))
	    return 0;

	PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd_);

	if (padd==0) return 0;

	try {

	    if (!padd->conformsTo(*wch)) return 0;

	    String regionType = mouseRegion.asString("type");
	    if (regionType != "box" && regionType != "polygon")
		return 0;

	    Int nAxes = (im_!=0)? im_->ndim()  : cim_->ndim();
	    IPosition shp = (im_!=0)? im_->shape() : cim_->shape();
	    DisplayCoordinateSystem cs =  (im_ != 0) ? im_->coordinates() : cim_->coordinates();

	    Int zIndex = padd->activeZIndex();
	    IPosition pos = padd->fixedPosition();
	    Vector<Int> dispAxes = padd->displayAxes();

	    if (nAxes == 2)
		dispAxes.resize(2, True);

	    if ( nAxes < 2 || Int(shp.nelements()) != nAxes ||
		 Int(pos.nelements()) != nAxes ||
		 anyLT(dispAxes, 0) || anyGE(dispAxes, nAxes))
		return 0;

	    if (dispAxes.nelements() > 2u)
		pos[dispAxes[2]] = zIndex;

	    dispAxes.resize(2, True);

	    WCBox dummy;

	    Quantum<Double> px0(0.,"pix");
	    Vector<Quantum<Double> >  blcq(nAxes, px0), trcq(nAxes, px0);

	    Int spaxis = -1;
	    if (extChan.length() == 0)
		spaxis = getAxisIndex(String("Spectral"));

	    for (Int ax = 0; ax < nAxes; ax++) {
		if ( ax == dispAxes[0] || ax == dispAxes[1] ||
		     extChan.length() == 0 || ax == spaxis) {
		    trcq[ax].setValue(shp[ax]-1);
		} else  {
		    blcq[ax].setValue(pos[ax]);
		    trcq[ax].setValue(pos[ax]);
		}
	    }

	    Bool useWorldCoords = mouseRegion.isDefined("world");
	    Record coords = mouseRegion.subRecord( useWorldCoords ? "world" : "linear");

	    Vector<String> units(2, "pix");
	    if(useWorldCoords) units = coords.asArrayString("units");

	    if (regionType=="box") {
		Vector<Double> blc = coords.asArrayDouble("blc"), trc = coords.asArrayDouble("trc");

		for (Int i = 0; i < 2; i++) {
		    Int ax = dispAxes[i];

		    blcq[ax].setValue(blc[i]);
		    trcq[ax].setValue(trc[i]);

		    if (useWorldCoords) {
			blcq[ax].setUnit(units[i]);
			trcq[ax].setUnit(units[i]);
		    }
		}
	    }

	    WCBox box(blcq, trcq, cs, Vector<Int>());

	    if (regionType=="box")
		return new ImageRegion(box);

	    Vector<Double> x = coords.asArrayDouble("x"), y = coords.asArrayDouble("y");
	    Quantum<Vector<Double> > qx(x, units[0]), qy(y, units[1]);

	    WCPolygon poly(qx, qy, IPosition(dispAxes), cs);
	    return new ImageRegion(poly);

	} catch (const casa::AipsError& err) {
	    errMsg_ = err.getMesg();
	    return 0;
	} catch (...) {
	    errMsg_ = "Unknown error converting region";
	    return 0;
	}


	}
	**********************************************************************************************************
	*********************************************************************************************************/


	typedef ImageStatistics<Float>::stat_list getLayerStats_t;
	static getLayerStats_t *getLayerStats( PrincipalAxesDD *padd, ImageInterface<Float> *image, ImageRegion& imgReg ) {

		// Compute and print statistics on DD's image for
		// given region in all layers.

		//there are several possible path here
		//(1) modify ImageRegion record then create SubImage of 1 plane
		//(2) modify ImageRegion directly then creage SubImage of 1 plane
		//(3) make SubImage of SubImage to get one plane
		//(4) pass layer index to LatticeStatistcis
		//(5) do single plane statistic right here

		if( image==0 || padd == 0 ) return 0;

		try {

			SubImage<Float> subImg(*image, imgReg);
			IPosition shp = image->shape();
			IPosition sshp = subImg.shape();

			Vector<Int> dispAxes = padd->displayAxes();

			Vector<Int> cursorAxes(2);
			cursorAxes(0) = dispAxes[0];	//display axis 1
			cursorAxes(1) = dispAxes[1];	//display axis 2

			Int nAxes = image->ndim();
			Vector<int> otherAxes(0);
			otherAxes = IPosition::otherAxes(nAxes, cursorAxes).asVector();

			IPosition start(nAxes);
			IPosition stride(nAxes);
			IPosition end(sshp);
			start = 0;
			stride = 1;

			Int _axis_h_ = padd->xlatePixelAxes(3);		// get first "hidden axis
			String zaxis = padd->zaxisStr( );

			const DisplayCoordinateSystem& cs = image->coordinates();

			Vector<String> axesNames = padd->worldToPixelAxisNames( cs );
			String haxis = axesNames(_axis_h_);
			Int hIndex = _axis_h_ - 2 + padd->uiBase( );		// uiBase( ) sets zero/one based
			Int zIndex = padd->activeZIndex();

			String zUnit, zspKey, zspVal;
			zUnit = padd->spectralunitStr( );

			String unit =  image->units().getName();

			IPosition pos = padd->fixedPosition();

			ImageStatistics<Float> stats(subImg, False);
			if ( ! stats.setAxes(cursorAxes) ) return 0;
			stats.setList(True);
			String layerStats;
			Vector<String> nm = cs.worldAxisNames();

			Int zPos = -1;
			Int hPos = -1;
			for (unsigned int k = 0; k < nm.nelements(); k++) {
				if (nm(k) == zaxis)
					zPos = k;
				if (nm(k) == haxis)
					hPos = k;
			}

			// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
			//   begin collecting statistics...
			// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
			viewer::RegionInfo::stats_t *layerstats = new viewer::RegionInfo::stats_t( );

			String zLabel="";
			String hLabel="";
			Vector<Double> tPix,tWrld;
			tPix = cs.referencePixel();
			String tStr;
			if (zPos > -1) {
				tPix(zPos) = zIndex;
				if (!cs.toWorld(tWrld,tPix)) {
				} else {
					zLabel = ((DisplayCoordinateSystem)cs).format(tStr, Coordinate::DEFAULT, tWrld(zPos), zPos);
					layerstats->push_back(viewer::RegionInfo::stats_t::value_type(zaxis,zLabel + tStr));

					if (zUnit.length()>0) {
						zspKey = "Spectral_Vale";
						zspVal = ((DisplayCoordinateSystem)cs).format(zUnit,Coordinate::DEFAULT, tWrld(zPos), zPos)+zUnit;
					}
				}
			}

			if (hPos > -1) {
				tPix(hPos) = hIndex;

				if (!cs.toWorld(tWrld,tPix)) {
				} else {
					hLabel = ((DisplayCoordinateSystem)cs).format(tStr, Coordinate::DEFAULT, tWrld(hPos), hPos);
					if (zUnit.length()>0) {
						zspKey = "Spectral_Vale";
						zspVal = ((DisplayCoordinateSystem)cs).format(zUnit, Coordinate::DEFAULT, tWrld(zPos), zPos)+zUnit;
					}
				}
			}

			Int spInd = cs.findCoordinate(Coordinate::SPECTRAL);
			SpectralCoordinate spCoord;
			Int wSp=-1;
			if ( spInd>=0 ) {
				wSp= (cs.worldAxes(spInd))[0];
				spCoord=cs.spectralCoordinate(spInd);
				spCoord.setVelocity();
				Double vel;
				Double restFreq = spCoord.restFrequency();
				if (downcase(zaxis).contains("freq")) {
					if (spCoord.pixelToVelocity(vel, zIndex)) {
						if (restFreq >0)
							layerstats->push_back(viewer::RegionInfo::stats_t::value_type("Velocity",String::toString(vel)+"km/s"));
						else
							layerstats->push_back(viewer::RegionInfo::stats_t::value_type(zspKey,zspVal));

						// --- this line was executed, but was a NOP in the old code --- <drs>
						// layerstats->push_back(viewer::RegionInfo::stats_t::value_type("Doppler",MDoppler::showType(spCoord.velocityDoppler())));
					}
				}

				if (downcase(haxis).contains("freq")) {
					if (spCoord.pixelToVelocity(vel, hIndex)) {
						if (restFreq >0)
							layerstats->push_back(viewer::RegionInfo::stats_t::value_type("Velocity",String::toString(vel)+"km/s"));
						else
							layerstats->push_back(viewer::RegionInfo::stats_t::value_type(zspKey,zspVal));

						layerstats->push_back(viewer::RegionInfo::stats_t::value_type("Frame",MFrequency::showType(spCoord.frequencySystem())));
						layerstats->push_back(viewer::RegionInfo::stats_t::value_type("Doppler",MDoppler::showType(spCoord.velocityDoppler())));
					}
				}
			}


			layerstats->push_back(viewer::RegionInfo::stats_t::value_type(haxis,hLabel));
			layerstats->push_back(viewer::RegionInfo::stats_t::value_type("BrightnessUnit",unit));

			Double beamArea = 0;
			ImageInfo ii = image->imageInfo();
			GaussianBeam beam = ii.restoringBeam();
			DisplayCoordinateSystem cSys = image->coordinates();
			String imageUnits = image->units().getName();
			imageUnits.upcase();

			Int afterCoord = -1;
			Int dC = cSys.findCoordinate(Coordinate::DIRECTION, afterCoord);
			// use contains() not == so moment maps are dealt with nicely
			if (! beam.isNull() && dC!=-1 && imageUnits.contains("JY/BEAM")) {
				DirectionCoordinate dCoord = cSys.directionCoordinate(dC);
				Vector<String> units(2);
				units(0) = units(1) = "rad";
				dCoord.setWorldAxisUnits(units);
				Vector<Double> deltas = dCoord.increment();

// 		Double major = beam.getMajor("rad");
// 		Double minor = beam.getMinor("rad");
				beamArea = beam.getArea("rad2") / abs(deltas(0) * deltas(1));
			}

			layerstats->push_back(viewer::RegionInfo::stats_t::value_type("BeamArea",String::toString(beamArea)));

			Bool statsOk = stats.getLayerStats(*layerstats, beamArea, zPos, zIndex, hPos, hIndex);
			if ( ! statsOk ) {
				delete layerstats;
				return 0;
			} else {
				return layerstats;
			}

		} catch (const casa::AipsError& err) {
			std::string errMsg_ = err.getMesg();
			return 0;
		} catch (...) {
			std::string errMsg_ = "Unknown error computing region statistics.";
			return 0;
		}
	}

	static Int getAxisIndex( ImageInterface<Float> *image, String axtype ) {

		if( image == 0 ) return -1;

		const DisplayCoordinateSystem cs = image->coordinates();

		try {
			Int nAxes = image->ndim();
			for(Int ax=0; ax<nAxes && ax<Int(cs.nWorldAxes()); ax++) {
				// coordno : type of coordinate
				// axisincoord : index within the coordinate list defined by coordno
				Int coordno, axisincoord;
				cs.findWorldAxis(coordno, axisincoord, ax);

				//cout << "coordno=" << coordno << "  axisincoord : " << axisincoord << "  type : " << cs.showType(coordno) << endl;

				if( cs.showType(coordno) == String("Direction") ) {
					// Check for Right Ascension and Declination
					Vector<String> axnames = (cs.directionCoordinate(coordno)).axisNames(MDirection::DEFAULT);
					AlwaysAssert( axisincoord>=0 && axisincoord < (Int) axnames.nelements(), AipsError);
					if( axnames[axisincoord] == axtype ) {
						return ax;
					}
				} else {
					// Check for Stokes and Spectral
					if ( cs.showType(coordno)==axtype ) {
						return ax;
					}
				}
			}
		} catch (...) {
			std::string errMsg_ = "Unknown error converting region ***";
			// cerr<<"mse2ImgReg: "<<errMsg_<<endl;	//#dg
			// emit qddRegnError(errMsg_);
			return -1;
		}
		return -1;
	}

	void MultiPVTool::update_stats(const WCMotionEvent &/*ev*/) {
		if( ! rectangleDefined() || itsCurrentWC==0 ) return;

		WorldCanvasHolder *wch = pd_->wcHolder(itsCurrentWC);
		// Only reason pd_ is 'needed' by this tool (it shouldn't need it):
		// locating the important coordinate state 'zindex' on wch
		// (inaccessible from WC), instead of on WC, was a blunder....
		// .... see also QtMouseTools.cc
		if ( wch == 0 ) return;
		Int zindex = 0;
		if (wch->restrictionBuffer()->exists("zIndex")) {
			wch->restrictionBuffer()->getValue("zIndex", zindex);
		}

		Vector<Double> pixmax(2), pixmin(2), lin(2), blc(2), trc(2);
		Int x1, y1, x2, y2;

		get(x1, y1, x2, y2);
		if ( x1 == x2 || y1 == y2 ) return;

		pixmin(0) = min(x1, x2);
		pixmin(1) = min(y1, y2);
		if(!itsCurrentWC->pixToLin(lin, pixmin)) return;		// (unlikely)
		Bool wldOk = itsCurrentWC->linToWorld(blc, lin);

		pixmax(0) = max(x1, x2);
		pixmax(1) = max(y1, y2);
		if(!itsCurrentWC->pixToLin(lin, pixmax)) return;		// (unlikely)
		if(wldOk) wldOk = itsCurrentWC->linToWorld(trc, lin);

		DisplayData *dd = 0;
		List<DisplayData*> *dds = pd_->displayDatas( );

		std::string errMsg_;
		std::map<String,bool> processed;
		for ( ListIter<DisplayData *> ddi(*dds); ! ddi.atEnd( ); ++ddi ) {
			dd = ddi.getRight( );

			PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
			if (padd==0) continue;

			try {
				if ( ! padd->conformsTo(*itsCurrentWC) ) continue;

				std::tr1::shared_ptr<ImageInterface<Float> > image = padd->imageinterface( );

				if ( image == 0 ) continue;

				String full_image_name = image->name(false);
				std::map<String,bool>::iterator repeat = processed.find(full_image_name);
				if (repeat != processed.end()) continue;
				processed.insert(std::map<String,bool>::value_type(full_image_name,true));

				Int nAxes = image->ndim( );
				IPosition shp = image->shape( );
				const DisplayCoordinateSystem &cs = image->coordinates( );

				int zIndex = padd->activeZIndex( );
				IPosition pos = padd->fixedPosition( );
				Vector<Int> dispAxes = padd->displayAxes( );

				if ( nAxes == 2 ) dispAxes.resize(2,True);

				if ( nAxes < 2 || Int(shp.nelements()) != nAxes ||
				        Int(pos.nelements()) != nAxes ||
				        anyLT(dispAxes,0) || anyGE(dispAxes,nAxes) )
					continue;

				if ( dispAxes.nelements() > 2u )
					pos[dispAxes[2]] = zIndex;

				dispAxes.resize(2,True);

				WCBox dummy;
				Quantum<Double> px0(0.,"pix");
				Vector<Quantum<Double> > blcq(nAxes,px0), trcq(nAxes,px0);

				Int spaxis = getAxisIndex( image.get(), String("Spectral") );
				for (Int ax = 0; ax < nAxes; ax++) {
					if ( ax == dispAxes[0] || ax == dispAxes[1] || ax == spaxis) {
						trcq[ax].setValue(shp[ax]-1);
					} else  {
						blcq[ax].setValue(pos[ax]);
						trcq[ax].setValue(pos[ax]);
					}
				}

				Vector<String> units = itsCurrentWC->worldAxisUnits( );

				for (Int i = 0; i < 2; i++) {
					Int ax = dispAxes[i];

					blcq[ax].setValue(blc[i]);
					trcq[ax].setValue(trc[i]);

					blcq[ax].setUnit(units[i]);
					trcq[ax].setUnit(units[i]);
				}


				WCBox box(blcq, trcq, cs, Vector<Int>());
				ImageRegion *imageregion = new ImageRegion(box);


				getLayerStats_t *layerstats = getLayerStats( padd, image.get(), *imageregion );

#if OLDSTUFF
				// (1) create statistics matrix (gui)
				// (2) fill with layerstats
				if ( ! selected.isNull( ) )
					selected->addstats( image->name(true), layerstats );
#endif
				delete layerstats;

			} catch (const casa::AipsError& err) {
				errMsg_ = err.getMesg();
				continue;
			} catch (...) {
				errMsg_ = "Unknown error converting region";
				continue;
			}
		}
	}

	std::tr1::shared_ptr<viewer::PVLine> MultiPVTool::allocate_region( WorldCanvas *wc, double x1, double y1, double x2, double y2, VOID * ) const {
		////// this is the code we would like to use (removing the "region source"), but currently the profile tool
		////// queues off of events from the region source...
		// viewer::PVLine *result = new viewer::PVLine( wc, dock_, x1, y1, x2, y2, true );
		// result->releaseSignals( );
		// return std::tr1::shared_ptr<viewer::PVLine>(result);
		return rfactory->pvline( wc, x1, y1, x2, y2 );
	}

	void MultiPVTool::checkPoint( WorldCanvas */*wc*/, State &state ) {
		for ( pvlinelist::iterator iter = ((MultiPVTool*) this)->rectangles.begin(); iter != rectangles.end(); ++iter ) {
			viewer::region::PointInfo point_state = (*iter)->checkPoint( state.x(), state.y() );
			// should consider introducing a cptr_ref which somehow allows creating a
			// base class reference based on a counted pointer to a derived class...
			state.insert( this, &*(*iter), point_state );
		}
	}

	static std::set<viewer::region::RegionTypes> multi_rect_tool_region_set;
	const std::set<viewer::region::RegionTypes> &MultiPVTool::regionsCreated( ) const {
		if ( multi_rect_tool_region_set.size( ) == 0 ) {
			multi_rect_tool_region_set.insert( viewer::region::PVLineRegion );
		}
		return multi_rect_tool_region_set;
	}

	bool MultiPVTool::create( viewer::region::RegionTypes /*region_type*/, WorldCanvas *wc,
	                          const std::vector<std::pair<double,double> > &pts,
	                          const std::string &label, viewer::region::TextPosition label_pos, const std::vector<int> &label_off,
	                          const std::string &font, int font_size, int font_style, const std::string &font_color,
	                          const std::string &line_color, viewer::region::LineStyle line_style, unsigned int line_width,
	                          bool is_annotation, VOID *region_specific_state ) {

		if ( pts.size( ) != 2 ) return false;
		if ( itsCurrentWC == 0 ) itsCurrentWC = wc;
		std::tr1::shared_ptr<viewer::PVLine> result = allocate_region( wc, pts[0].first, pts[0].second, pts[1].first, pts[1].second, region_specific_state );
		result->setLabel( label );
		result->setLabelPosition( label_pos );
		result->setLabelDelta( label_off );
		// set line first...
		result->setLine( line_color, line_style, line_width );
		result->setFont( font, font_size, font_style, font_color );
		result->setAnnotation(is_annotation);
		rectangles.push_back( result );
		refresh( );
		return true;
	}

	void MultiPVTool::start_new_rectangle( WorldCanvas *wc, int x, int y ) {

		// As originally requested by Kumar, any non-modified regions would be erased when a
		// new one was started, but Juergen said that new regions should not automatically cause
		// other regions to disappear... instead, there <ESC> should cause the selected regions
		// to be be deleted...      <drs>
		// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
		// for ( pvlinelist::iterator iter = rectangles.begin(); iter != rectangles.end(); ++iter ) {
		//     // the only change that will count as "modifying" is a non-zero length name
		//     if ( (*iter)->modified( ) == false ) {
		// 	rectangles.erase( iter );
		// 	continue;
		//     }
		// }

		itsCurrentWC = wc;

		double linx, liny;
		try {
			viewer::screen_to_linear( itsCurrentWC, x, y, linx, liny );
		} catch(...) {
			return;
		}

		creating_region = resizing_region = allocate_region( wc, linx, liny, linx, liny, 0 );
		viewer::Region::creatingRegionBegin(std::tr1::dynamic_pointer_cast<viewer::Region>(creating_region));
		rectangles.push_back( resizing_region );

		if ( type( ) != POINTTOOL )
			resizing_region_handle = 1;
		else
			resizing_region = memory::anullptr;

		set(x,y, x,y);
		moving_regions.clear( );		// ensure that moving state is clear...
	}

} //# NAMESPACE CASA - END
