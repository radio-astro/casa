//# WorldCanvasHolder.cc: interface between DisplayDatas and WorldCanvas
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001,2002,2003
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

// Interface between the DisplayObjects(DisplayData) and the WorldCanvas.
// Registers functions on the WorldCanvas for eventhandling.  Keeps track of
// what is being displayed on the WorldCanvas so that WorldCanvas can ask the
// Holder to provide e.g.  coordinate transformation, pixelvalues or scale
// information.

#include <casa/iostream.h>

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/String.h>
#include <casa/BasicMath/Math.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/Attribute.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/Display/AttValBase.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayCanvas/WCAxisLabeller.h>

#include <cpgplot.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	const String WorldCanvasHolder::BLINK_MODE = "bIndex";

// Constructors and destructors:
	WorldCanvasHolder::WorldCanvasHolder(WorldCanvas *wCanvas) :
		itsWorldCanvas(wCanvas), controllingDD(NULL), itsLastCSmaster(0) {
		// Register this as an event handler for the WorldCanvas.
		itsWorldCanvas->addRefreshEventHandler(*this);
		itsWorldCanvas->addMotionEventHandler(*this);
		itsWorldCanvas->addPositionEventHandler(*this);
		itsWorldCanvas->setSizeControlHandler(this);
		itsWorldCanvas->setCoordinateHandler(this);
		Attribute unZoom("resetCoordinates", True);	// Will cause unzoom on new
		itsWorldCanvas->setAttribute(unZoom);		// canvas (unless someone adds
		// a zoom order to override).
		blinkMode = false;
	}

	WorldCanvasHolder::~WorldCanvasHolder() {
		// remove all displaydatas

		worldCanvas()->hold();
		for ( std::list<DisplayData*>::iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter ) {
			(*iter)->notifyUnregister(*this, True);
		}
		itsDisplayList.clear( );

		// Unregister this as various event handlers for the WorldCanvas.
		itsWorldCanvas->removeRefreshEventHandler(*this);
		itsWorldCanvas->removeMotionEventHandler(*this);
		itsWorldCanvas->removePositionEventHandler(*this);
		// Delete the iterator for the list of DisplayDatas.
		worldCanvas()->release();
		itsWorldCanvas = 0;
	}

// Adding, removing and querying DisplayDatas
	void WorldCanvasHolder::addDisplayData(DisplayData *dData, int position ) {
		if (dData == 0) {
			throw (AipsError("WorldCanvasHolder::addDisplayData - "
			                 "null pointer passed"));
		}
		worldCanvas()->hold();
		// Notify DisplayData
		dData->notifyRegister(this);

		if ( position == -1 ){
			// and add the new displayData
			itsDisplayList.push_back(dData);
		}
		else {
			int i = 0;
			std::list<DisplayData*>::iterator iter = itsDisplayList.begin();
			while ( i < position && iter != itsDisplayList.end()){
				iter++;
				i++;
			}
			itsDisplayList.insert(iter,dData);
		}

		//Block below was taken out, because we now have a mode where there
		//can be NO CSMaster.
		/*if(worldCanvas()->csMaster()==0) {
			executeSizeControl(worldCanvas());
		}*/
		// If the new DD can assume the CS master role, let it set up
		// WC state immediately, since there is no master at present.
		worldCanvas()->release();
	}
	void WorldCanvasHolder::removeDisplayData(DisplayData &dData,
	        Bool ignoreRefresh) {
		worldCanvas()->hold();
		std::list <DisplayData*>::iterator pos = find( itsDisplayList.begin(), itsDisplayList.end(), &dData );
		if ( pos != itsDisplayList.end() ) {
			//Line below was taken out because we can now have a CS master that is not
			//in the display list.
			//if(worldCanvas()->isCSmaster(&dData)) worldCanvas()->csMaster() = 0;

			itsDisplayList.erase(pos);
			// Notify DisplayData
			dData.notifyUnregister(*this, ignoreRefresh);
		}

		//If (there is nothing to display, and no master image has been
		//designated) OR (the one that is going away is the CSMaster),
		//tell the canvas there is no CS master.
		if ( ( itsDisplayList.size() == 0 && controllingDD == NULL ) ||
				controllingDD == &dData ){
			worldCanvas()->csMaster() = NULL;
		}

		if(csMaster()==0){
			executeSizeControl(worldCanvas());
		}
		// If any remaining DD can assume CS master role, let it set up
		// WC state immediately, since there is no master at present.
		worldCanvas()->release();
	}

	const uInt WorldCanvasHolder::nDisplayDatas() const {
		return itsDisplayList.size();
	}

// Refreshing the WorldCanvas
	void WorldCanvasHolder::refresh(const Display::RefreshReason &reason,
	                                const Bool &/*explicitrequest*/) {
		// NOTE - do not use low-level PixelCanvas routines here, eg.
		// PixelCanvas::copyBackBufferToFrontBuffer.  If you do, you
		// can break other WorldCanvases writing to the same PC.
		/*
		itsWorldCanvas->acquirePGPLOTdevice();
		if (explicitrequest && (reason != Display::BackCopiedToFront)) {
		itsWorldCanvas->setDrawBuffer(Display::BackBuffer);
		}
		 */
		itsWorldCanvas->refresh(reason);
		/*
		if (explicitrequest && (reason != Display::BackCopiedToFront) &&
		  itsWorldCanvas->refreshAllowed()) {
		itsWorldCanvas->copyBackBufferToFrontBuffer();
		itsWorldCanvas->setDrawBuffer(Display::FrontBuffer);
		WCRefreshEvent ev(itsWorldCanvas, Display::BackCopiedToFront);
		itsWorldCanvas->callRefreshEventHandlers(ev);
		}
		itsWorldCanvas->releasePGPLOTdevice();
		 */
	}

// Handle events and requests originating from the WorldCanvas.


	Bool WorldCanvasHolder::executeSizeControl(WorldCanvas *wCanvas) {
		// 'Size control' is concerned with setting and control of fundamental
		// World Canvas state, particularly its various coordinate transformations
		// (see comments in DisplayData.h).  This routine chooses a 'CS master' DD,
		// which is responsible for controlling this state.  It is typically called
		// during refresh, but also when the CS master might need to change (e.g.
		// when DDs are added/deleted), in order to keep WC state current.

		if (wCanvas == 0) {
			throw (AipsError("WorldCanvasHolder::sizeControlEH - "
			                 "null pointer passed"));
		}
		static AttributeBuffer sizeControlAtts;


		// Set initial draw area size and location, in screen pixels.  The user's
		// current canvas margin settings are stored on the WC as attributes,
		// in units of PGPLOT characters. Current character height and width are
		// retrieved here from PGPLOT.


		Float xlen=0., ylen=0.;
		// unit character dimensions, in screen pixels.

		if(wCanvas->canvasXSize()>0 && wCanvas->canvasYSize()>0) {
			// (...shouldn't have to test that...grr....
			// CYA for some half-assed PC init...)
			wCanvas->acquirePGPLOTdevice();
			cpgsch(1.0);
			cpgscf(1);
			cpglen(3, "X", &xlen, &ylen);
			drawUnit = ylen;
			wCanvas->releasePGPLOTdevice();
		}

		Int space_l = 0, space_b = 0, space_r = 0, space_t = 0;
		String attString;
		attString = "leftMarginSpacePG";
		if (wCanvas->existsAttribute(attString)) {
			wCanvas->getAttributeValue(attString, space_l);
		}
		attString = "rightMarginSpacePG";
		if (wCanvas->existsAttribute(attString)) {
			wCanvas->getAttributeValue(attString, space_r);
		}
		attString = "bottomMarginSpacePG";
		if (wCanvas->existsAttribute(attString)) {
			wCanvas->getAttributeValue(attString, space_b);
		}
		attString = "topMarginSpacePG";
		if (wCanvas->existsAttribute(attString)) {
			wCanvas->getAttributeValue(attString, space_t);
		}
		space_l = Int(ylen * space_l);
		space_b = Int(ylen * space_b);
		space_r = Int(ylen * space_r);
		space_t = Int(ylen * space_t);
		// (dk note: I'm not sure whether xlen was omitted in preference
		// to ylen accidentally or on purpose; some left-margin labels
		// _are_ printed vertically, e.g....)


		AttributeBuffer drawArea;

		Int dSize, dOffset, cSize;

		cSize = Int(wCanvas->canvasXSize());
		dOffset = max(0, min(cSize-1,  space_l ));
		dSize = max(3, cSize - dOffset - space_r);
		drawArea.set("canvasDrawXOffset", uInt(dOffset));
		drawArea.set("canvasDrawXSize", uInt(dSize));

		cSize = Int(wCanvas->canvasYSize());
		dOffset = max(0, min(cSize-1,  space_b ));
		dSize = max(3, cSize - dOffset - space_t);
		drawArea.set("canvasDrawYOffset", uInt(dOffset));
		drawArea.set("canvasDrawYSize", uInt(dSize));

		wCanvas->setAttributes(drawArea);



		sizeControlAtts.clear();

		Attribute cmAtt("colormodel",
		                Int(worldCanvas()->pixelCanvas()->pcctbl()->colorModel()));
		sizeControlAtts.add(cmAtt);


		// Try to find a suitable CS master.  (If the previous one is still
		// registered, it will likely remain CS master).

		// Only the CS master DD should set the WC CS and corresponding axis codes.
		// A DD's sizeControl method should determine whether it has permission
		// to become master by testing 'wch->isCSMaster(this)'.  It
		// should set all WC coordinate state and return True _only_ if it has
		// permission and accepts the CS master role.  In that case, it will also
		// be called upon to perform coordinate transform (WCCoordinateHandler)
		// chores for the WC[H].
		// You may notice that no 'conformsTo()' tests (such as for blink or zindex
		// restriction) are considered before offering the master role.  These may
		// vary with canvas; we want the same general CS control placed over an
		// entire [multicanvas] panel.  The DD should be able to control CS even if
		// it is not currently drawing because of zindex range or blink restrictions.
		// See comments in DisplayData.h for additional detail.

		// Give current CS master (if any) the chance to remain master
		// (it will probably do so).
		DisplayData* csMasterDD = worldCanvas()->csMaster();
		Bool masterFound = false;
		if ( csMasterDD != NULL ){
			masterFound = csMasterDD->sizeControl(*this, sizeControlAtts);
		}
		else if ( controllingDD != NULL ){
			//If the master image is unregistered, it will no longer be in the world canvas,
			//i.e., csMasterDD will be NULL.  However, it should still be the controllinDD for
			//this class so we put it back in the world canvas for size control.
			worldCanvas()->csMaster() = controllingDD;
			masterFound =controllingDD->sizeControl(*this, sizeControlAtts);
		}

		// Even if master role is already taken, all sizeControl routines are still
		// executed, to give give the DDs a chance to do minor adjustments (to
		// maximum zoom extents, for example).  (At present (6/04), no non-master
		// DD is making any such adjustments, however).
		for ( std::list<DisplayData*>::iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter ) {
			if ( ! (*iter)->isDisplayable( ) ) {
				continue;	// not displayable
			}
			if ( worldCanvas()->isCSmaster(*iter) ) {
				continue;	// (already given the chance).
			}
			if ( ! masterFound ) {
				worldCanvas()->csMaster() = *iter;	// (This assignment does not
				// yet confirm the CS master; it only indicates an offer
				// at this stage.  But setting itsCSmaster here also puts the
				// dd in charge, at least temporarily, of any WC coordinate
				// conversions it needs to do during sizeControl execution).
			}
			bool ddSizeControl=(*iter)->sizeControl(*this, sizeControlAtts);
			if ( ddSizeControl ) {
				masterFound=True;
			}
			// CS master confirmed.
		}

		if (masterFound){
			wCanvas->setAttributes(sizeControlAtts);
		}

		// Store the WC state attributes produced by sizeControl[s].
		else {
			clearCSMasterSettings( wCanvas );
		}

		itsLastCSmaster = worldCanvas()->csMaster( );
		// (Used during the next call to this routine, when the _next_
		// CS master does sizeControl and wants to determine whether it
		// (or anyone) wasCSmaster()).

		return masterFound;
	}

	void WorldCanvasHolder::clearCSMasterSettings( WorldCanvas* wCanvas, bool clearZoom ){
		// None assumed CS master role (canvas may be empty).  Remove
		// any old axis codes; next master will set them to suit itself.
		// Assure that the zoom window is reset when a DD _does_ accept CS master.

		worldCanvas()->csMaster() = 0;

		String xAxis = "xaxiscode (required match)";
		String yAxis = "yaxiscode (required match)";
		wCanvas->removeAttribute(xAxis);
		wCanvas->removeAttribute(yAxis);

		if ( clearZoom ){
			Attribute unZoom("resetCoordinates", True);
			itsWorldCanvas->setAttribute(unZoom);	// "zoom-to-extent" order.

			String zoomB = "manualZoomBlc", zoomT = "manualZoomTrc";
			itsWorldCanvas->removeAttribute(zoomB);	  // These will not be
			itsWorldCanvas->removeAttribute(zoomT);
		}
	}

	bool WorldCanvasHolder::setCSMaster( DisplayData* dd ) {
		bool acceptedPosition = false;
		if ( dd != NULL && dd->isDisplayable()) {
			DisplayData* currentCSMaster = worldCanvas()->csMaster();

			//Make it the cs master
			worldCanvas()->csMaster() = dd;

			//See if it accepted the position
			acceptedPosition = dd->isCSmaster( this );
			if ( acceptedPosition ) {
				//Make it the cs master
				worldCanvas()->csMaster() = dd;
				controllingDD = dd;

				//Store the old one
				itsLastCSmaster = currentCSMaster;

				//Tell everything to clean up cached drawings.  The
				//drawing box will change will this new CS Master.
				for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
						        iter != itsDisplayList.end(); ++iter ) {
					(*iter)->cleanup();
				}

				//Execute size control
				worldCanvas()->hold();
				acceptedPosition = executeSizeControl( worldCanvas());
				worldCanvas()->release();
			}
		}
		else if ( dd == NULL ){
			controllingDD = NULL;
			clearCSMasterSettings( worldCanvas());
		}
		return acceptedPosition;
	}

	Bool WorldCanvasHolder::syncCSmaster(const WorldCanvasHolder* wch) {
		// used by PanelDisplay on new WCHs to keep a consistent CS master on
		// all its main display WC[H]s.  Sets [default] CS master dd to that of
		// passed wch (if that dd is registered here).
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter ) {
			if ( ! (*iter)->isDisplayable( ) ) continue; // not displayable
			if ( *iter == wch->csMaster()) {
				worldCanvas()->csMaster() = *iter;
				executeSizeControl(worldCanvas());
				// Makes sure the new master sets up WC state immediately.
				return True;
			}
		}

		return False;
	}


	void WorldCanvasHolder::operator()(const WCRefreshEvent &ev) {
		// DisplayDatas are not expected to draw unbuffered data...
		if (ev.reason() == Display::BackCopiedToFront) {
			return;
		}

		// retrieve the world and pixel canvases
		WorldCanvas *wc = ev.worldCanvas();
		PixelCanvas *pc = wc->pixelCanvas();

		pc->disable(Display::ClipWindow);
		wc->setDrawBuffer(Display::BackBuffer);

		//executeSizeControl( wc );

		// set clip window to entire WorldCanvas, and clear.
		pc->setClipWindow(wc->canvasXOffset(), wc->canvasYOffset(),
		                  wc->canvasXOffset() + wc->canvasXSize() - 1,
		                  wc->canvasYOffset() + wc->canvasYSize() - 1);
		pc->enable(Display::ClipWindow);
		wc->clear();

		// If there are no DisplayDatas, or if noone has set WC coordinate state,
		// do not draw.
		if (nDisplayDatas() == 0 || csMaster()==0) {
			// disable the clip window
			pc->disable(Display::ClipWindow);
			return;
		}

		wc->acquirePGPLOTdevice();

		// set the clip window to draw area of the WorldCanvas
		pc->setClipWindow(wc->canvasXOffset() + wc->canvasDrawXOffset(),
		                  wc->canvasYOffset() + wc->canvasDrawYOffset(),
		                  wc->canvasXOffset() + wc->canvasDrawXOffset() +
		                  wc->canvasDrawXSize() - 1,
		                  wc->canvasYOffset() + wc->canvasDrawYOffset() +
		                  wc->canvasDrawYSize() - 1);
		pc->enable(Display::ClipWindow);


		// record dd conformity to WC[H] state.  Non-conforming DDs
		// will not be requested to draw.

		Int dd;
		Vector<Bool> conforms = getConformance();
		clearSubstituteTitles();
		setControllingTitle( conforms );
		//We have to redo the conforms vector here because we may have changed
		//the world canvas controlling dd.
		//conforms = getConformance();

		// iteration one - do  rasters:
		dd = 0;
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); iter++, ++dd ) {
			if ( conforms[dd] ) {
				if ( (*iter)->classType( ) == Display::Raster ) {
					(*iter)->refreshEH(ev);
				}
			}
		}

		wc->flushComponentImages();


		// iteration two - do vector graphics:
		dd = 0;
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter, ++dd ) {
			if ( conforms[dd] ) {
				if ( (*iter)->classType() == Display::Vector )
					(*iter)->refreshEH(ev);
			}
		}


		// iteration three - do annotation graphics in the draw area:
		dd = 0;
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter, ++dd ) {
			if ( conforms[dd] ) {
				if ( (*iter)->classType() == Display::Annotation )
					(*iter)->refreshEH(ev);
			}
		}


		// set the clip window to entire WorldCanvas
		pc->setClipWindow(wc->canvasXOffset(), wc->canvasYOffset(),
		                  wc->canvasXOffset() + wc->canvasXSize() - 1,
		                  wc->canvasYOffset() + wc->canvasYSize() - 1);
		pc->enable(Display::ClipWindow);


		// iteration four - do full canvas annotation graphics:
		dd = 0;
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter, ++dd ) {
			if ( conforms[dd] ) {
				if ( (*iter)->classType() == Display::CanvasAnnotation )
					(*iter)->refreshEH(ev);
			}
		}


		// "iteration" five - do axis labelling:
		// At present we only draw axis labels of first reg'd
		// DisplayData that responds.  A DD should return False unless
		// it is set up to label axes.

		// (12/04 dk note: There remain problems with labelling by (non-CS-master)
		// DDs, if their CS is not identical to the WC/master CS, as when a first
		// (master) DD has labels turned off, and a second DD is identical to the
		// first, but restricted to an inner region and with labelling turned on.
		// This is because the canvas CS (which the labeller _should_ use) was
		// never properly distinguished from the DD's own CS (which the old
		// AxisLabellers do use).  Newer WorldAxesDD uses the WC CS and doesn't
		// have this problem.  The old ones could be fixed as well, and all those
		// wasteful per-plane labellers in PADD::setupElements() eliminated).
		//
		// (10/07 dk: imperfect attempts have been made to correct the above.  Image
		// DDs now use WC CS for labelling, though still (wastefully) using a
		// private CS as well sometimes....
		labelAxes( conforms, ev );

		wc->releasePGPLOTdevice();
		// disable the clip window
		pc->disable(Display::ClipWindow);

	}


	Vector<Bool> WorldCanvasHolder::getConformance() const {
		Vector<Bool> conforms(itsDisplayList.size());
		WorldCanvas* wc = this->worldCanvas();
		Int dd = 0;
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
			iter != itsDisplayList.end(); ++iter,++dd ) {
			if ( ! (*iter)->isDisplayable( ) ) {
				conforms[dd] = False;
			}
			else {
				conforms[dd] = (*iter)->conformsTo( wc );
			}
		}
		return conforms;
	}

	void WorldCanvasHolder::labelAxesNormal( const Vector<Bool>& conforms,
				const WCRefreshEvent & ev ){
		int displayCount = 0;
		int dd = itsDisplayList.size() - 1;
		for ( std::list<DisplayData*>::const_reverse_iterator iter = itsDisplayList.rbegin();
								iter != itsDisplayList.rend(); ++iter, dd-- ) {
			if ( conforms[dd] ) {
				if ( (*iter)->classType() == Display::Raster ) {
					if ( (*iter)->labelAxes(ev)) {
						displayCount = 1;
						break;
					}
				}
			}
		}

		//We could not find a raster to label the axes so we just take the first one
		//that works.
		if ( displayCount == 0 ) {
			dd = itsDisplayList.size() - 1;
			for ( std::list<DisplayData*>::const_reverse_iterator iter = itsDisplayList.rbegin();
								iter != itsDisplayList.rend(); ++iter, --dd ) {
				if ( conforms[dd]  ) {
					if ( (*iter)->classType( ) != Display::Raster ) {
						if ( (*iter)->labelAxes(ev)) {
							break;
						}
					}
				}
			}
		}

	}

	void WorldCanvasHolder::labelAxesBlink( const Vector<Bool>& conforms,
			const WCRefreshEvent & ev ){
		int displayCount = 0;
		int dd = 0;
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
								iter != itsDisplayList.end(); ++iter, ++dd ) {
			if ( conforms[dd] ) {
				if ( (*iter)->classType() == Display::Raster ) {
					if ( (*iter)->labelAxes(ev)) {
						displayCount = 1;
						break;
					}
				}
			}
		}

		//We could not find a raster to label the axes so we just take the first one
		//that works.
		if ( displayCount == 0 ) {
			dd = 0;
			for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
								iter != itsDisplayList.end(); ++iter, ++dd ) {
				if ( conforms[dd]  ) {
					if ( (*iter)->classType( ) != Display::Raster ) {
						if ( (*iter)->labelAxes(ev)) {
							break;
						}
					}
				}
			}
		}
	}

	void WorldCanvasHolder::labelAxes( const Vector<Bool>& conforms,
			const WCRefreshEvent &ev ){
		int displayCount = 0;

		//First try to label the axes using the controllingDD.
		if ( controllingDD != NULL ) {
			if ( controllingDD->isDisplayable()) {
				if ( controllingDD->labelAxes(ev)) {
					displayCount = 1;
				}
			}
		}

		if ( displayCount == 0 ){
			if ( blinkMode ){
				labelAxesBlink( conforms, ev );
			}
			else {
				labelAxesNormal( conforms, ev );
			}
		}
	}


	void WorldCanvasHolder::setControllingTitle( const Vector<Bool>& conforms ){
		if ( controllingDD != NULL ) {
			if ( controllingDD->isDisplayable()) {
				String titleDDName = getTitleDDName( conforms );
				controllingDD->setSubstituteTitleText( titleDDName );
			}
		}
		else {
			//No controlling DD so we are going to use a fake one.
			DisplayData* titleDD = NULL;
			if ( blinkMode ){
				titleDD = getTitleDDBlink( conforms );
			}
			else {
				titleDD = getTitleDDNormal( conforms );
			}

			if ( titleDD != NULL ){
				clearCSMasterSettings(this->worldCanvas(), false);
				worldCanvas()->csMaster() = titleDD;
				//Preserve the zoom when there is no CS Master
				itsLastCSmaster=titleDD;
				executeSizeControl(worldCanvas() );
			}
		}
	}

	void WorldCanvasHolder::clearSubstituteTitles( ){
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
								iter != itsDisplayList.end(); ++iter) {
			(*iter)->setSubstituteTitleText("");
		}
	}

	DisplayData* WorldCanvasHolder::getTitleDDNormal( const Vector<Bool> & conforms ) const {
		DisplayData* titleDD = NULL;
		int dd = 0;
		for ( std::list<DisplayData*>::const_reverse_iterator iter = itsDisplayList.rbegin();
				iter != itsDisplayList.rend(); ++iter, ++dd ) {
			if ( conforms[dd] ) {
				if ( (*iter)->classType() == Display::Raster ) {
					if ( (*iter)->canLabelAxes()) {
						titleDD = (*iter);
						break;
					}
				}
			}
		}

		//We could not find a raster to label the axes so we just take the first one
		//that works.
		if ( titleDD == NULL ) {
			for ( std::list<DisplayData*>::const_reverse_iterator iter = itsDisplayList.rbegin();
											iter != itsDisplayList.rend(); ++iter, ++dd ) {
				if ( conforms[dd] ) {
					if ( (*iter)->classType() != Display::Raster ) {
					//In blink mode, we don't show contours, vectors, or markers
					//separately unless they are the only ones.
						if ( (*iter)->canLabelAxes()) {
							titleDD= (*iter );
							break;
						}
					}
				}
			}
		}
		return titleDD;
	}

	DisplayData* WorldCanvasHolder::getTitleDDBlink( const Vector<Bool> & conforms ) const {
		DisplayData* titleDD = NULL;
		//WorldCanvas* wc = this->worldCanvas();
		int dd = 0;
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
				iter != itsDisplayList.end(); ++iter, ++dd ) {
			if ( (*iter)->classType() == Display::Raster ) {
				if ( conforms[dd] ){
					if ( (*iter)->canLabelAxes()) {
						titleDD = (*iter);
						break;
					}
				}
			}
		}

		//We could not find a raster to label the axes so we just take the first one
		//that works.
		dd = 0;
		if ( titleDD == NULL ) {
			for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
											iter != itsDisplayList.end(); ++iter, ++dd ) {
				if ( conforms[dd] ) {
					if ( (*iter)->classType() != Display::Raster ) {
					//In blink mode, we don't show contours, vectors, or markers
					//separately unless they are the only ones.
						if ( (*iter)->canLabelAxes()) {
							titleDD= (*iter );
							break;
						}
					}
				}
			}
		}
		return titleDD;
	}

	String WorldCanvasHolder::getTitleDDName( const Vector<Bool>& conforms ) const {
		DisplayData* titleDD = NULL;
		if ( blinkMode ){
			titleDD = getTitleDDBlink( conforms );
		}
		else {
			titleDD = getTitleDDNormal( conforms );
		}
		String titleDDName;
		if ( titleDD != NULL ){
			titleDDName = getTitle( titleDD );
		}
		return titleDDName;
	}

	String WorldCanvasHolder::getTitle( DisplayData* dd ) const {
		String title;
		if ( dd != NULL ){
			Record record = dd->getOptions(  );

			if ( record.isDefined(  WCAxisLabeller::PLOT_TITLE)){
				Record rangeRecord = record.subRecord( WCAxisLabeller::PLOT_TITLE );
				title = rangeRecord.asString( "value");
			}
		}
		return title;
	}

// Distribute a WCPositionEvent over the DisplayDatas
	void WorldCanvasHolder::operator()(const WCPositionEvent &ev) {
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter ) {
			(*iter)->positionEH(ev);
		}
	}

// Distribute a WCMotionEvent over the DisplayDatas
	void WorldCanvasHolder::operator()(const WCMotionEvent &ev) {
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter ) {
			(*iter)->motionEH(ev);
		}
	}

// Distribute generic events sent via this new interface over the DDs
	void WorldCanvasHolder::handleEvent(DisplayEvent& ev) {
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter ) {
			(*iter)->handleEvent(ev);
		}
	}


// Coordinate conversions and ancillary information.  In future, naturally
// this should be handled by the WC's own CS.  Now done exclusively by
// the CS master DD, which should be equivalent in most cases).

	Bool WorldCanvasHolder::linToWorld(Vector<Double> &world, const Vector<Double> &lin) {
		if ( worldCanvas( )->csMaster( )  != 0 ) {
			Bool result = worldCanvas( )->csMaster( )->linToWorld(world, lin);
			if ( result == False ) {
				error_string = worldCanvas( )->csMaster( )->errorMessage( );
			}
			return result;
		} else
			error_string = "no coordinate system";

		return False;
	}


	Bool WorldCanvasHolder::worldToLin(Vector<Double> &lin, const Vector<Double> &world) {
		if ( worldCanvas( )->csMaster( ) != 0 ) {
			Bool result = worldCanvas( )->csMaster( )->worldToLin(lin, world);
			if ( result == False ) {
				error_string = worldCanvas( )->csMaster( )->errorMessage( );
			}
			return result;
		} else
			error_string = "no coordinate system";

		return False;
	}


	const uInt WorldCanvasHolder::nelements() {
		// Returns the maximum animation frames of all registered DDs
		// compatible with the current WC CS.  (This supports, e.g., blinking
		// between a continuum image and a selected slice of a spectral one;
		// the spectral DD's frames will be counted even if its blink
		// restriction invalidates it for drawing at the moment).

		//If we have no data, we have no frames.
		int firstGuessCount = itsDisplayList.size();
		if ( firstGuessCount == 0 ){
			return firstGuessCount;
		}

		executeSizeControl(worldCanvas());
		// makes sure WC state is up-to-date (e.g., with latest
		// axis change on CS master).

		uInt maxNelements = 0;
		for ( std::list<DisplayData*>::const_iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter ) {

			if ( ! (*iter)->isDisplayable( ) ) continue;	// not displayable

			if ( worldCanvas()->csMaster( ) == 0 || worldCanvas()->isCSmaster(*iter) || (*iter)->conformsToCS(*worldCanvas()) ) {
				maxNelements = max(maxNelements, (*iter)->nelements());
			}
		}

		return maxNelements;
	}

	Float WorldCanvasHolder::getDrawUnit() const {
		return drawUnit;
	}

	void WorldCanvasHolder::cleanup() {
		for ( std::list<DisplayData*>::iterator iter = itsDisplayList.begin();
		        iter != itsDisplayList.end(); ++iter ) {
			(*iter)->cleanup();
		}

		itsWorldCanvas->clear();
	}


} //# NAMESPACE CASA - END

