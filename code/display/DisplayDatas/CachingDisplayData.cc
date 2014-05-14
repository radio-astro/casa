//# CachingDisplayData.cc: base class for auto-caching DisplayData objects
//# Copyright (C) 1999,2000,2001,2002,2003
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

#include <casa/System/AipsrcValue.h>
#include <display/DisplayDatas/CachingDisplayData.h>
#include <display/DisplayDatas/CachingDisplayMethod.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/DisplayEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
	CachingDisplayData::CachingDisplayData() :
		DisplayData(), restorePCColormap_(False),
		itsCachingState(True) {
		AipsrcValue<Int>::find(itsDefaultMaximumCacheSize,
		                       String("display.cachesize"), 256);
		itsOptionsMaximumCacheSize = itsDefaultMaximumCacheSize;
		installDefaultOptions();
		itsElementListIter = new ListIter<void *>(&itsElementList);
	}

// Destructor.
	CachingDisplayData::~CachingDisplayData() {
		purgeCache();
		delete itsElementListIter;
	}

// Install the default options for this DisplayData.
	void CachingDisplayData::setDefaultOptions() {
		DisplayData::setDefaultOptions();
		installDefaultOptions();
	}

// Apply options stored in rec to the DisplayData.
	Bool CachingDisplayData::setOptions(Record &rec, Record &recOut) {
		Bool ret = DisplayData::setOptions(rec, recOut);
		Bool localchange = False, error;

		if (readOptionRecord(itsOptionsMaximumCacheSize, error, rec, "cachesize")) {
			// must trim cache in case max cache size has been made smaller
			trimCache();
		}

		ret = (ret || localchange);
		return ret;
	}

// Retrieve the current and default options and parameter types.
	Record CachingDisplayData::getOptions( bool scrub ) const {
		Record rec = DisplayData::getOptions(scrub);

		Record cachesize;
		cachesize.define("dlformat", "cachesize");
		cachesize.define("listname", "cache size");
		cachesize.define("help", "Controls the number of drawings this\n"
		                 "DisplayData may save/reuse.  For fast\n"
		                 "animation replay, set to the number of\n"
		                 "animation frames, at least.  But beware\n"
		                 "that the drawings can use a lot of memory.");
		cachesize.define("ptype", "intrange");
		cachesize.define("pmin", Int(1));
		cachesize.define("pmax", Int(2048));
		cachesize.define("default", itsDefaultMaximumCacheSize);
		cachesize.define("value", itsOptionsMaximumCacheSize);
		cachesize.define("allowunset", False);
		cachesize.define("context", "advanced");
		rec.defineRecord("cachesize", cachesize);

		return rec;
	}

// Refresh event handler which is called indirectly by the WorldCanvas,
// via the WorldCanvasHolder.
	void CachingDisplayData::refreshEH(const WCRefreshEvent &ev) {

		// find out which WorldCanvas and WorldCanvasHolder are being refreshed
		WorldCanvas *wc = ev.worldCanvas();
		if (!wc) {
			throw(AipsError("Refresh event has no WorldCanvas in "
			                "CachingDisplayData::refreshEH"));
		}
		WorldCanvasHolder *holder = findHolder(wc);
		if (!holder) {
			throw(AipsError("Unable to find WorldCanvasHolder in "
			                "CachingDisplayData::refreshEH"));
		}

		if(ev.reason() == Display::ColorTableChange ) purgeCache(*holder);
		// the old drawlists are valid for old colortable only--get rid of them.

		// Determine restrictions on the WorldCanvasHolder
		AttributeBuffer *wchRestrictions =
		    new AttributeBuffer(*(holder->restrictionBuffer()));
		if (!wchRestrictions) {
			throw(AipsError("Unable to obtain restrictions from WorldCanvasHolder in "
			                "CachingDisplayData::refreshEH"));
		}

		// Add WorldCanvas state to the wchRestrictions:
		wchRestrictions->set("canvasDrawXSize", wc->canvasDrawXSize());
		wchRestrictions->set("canvasDrawYSize", wc->canvasDrawYSize());
		wchRestrictions->set("canvasDrawXOffset", wc->canvasDrawXOffset());
		wchRestrictions->set("canvasDrawYOffset", wc->canvasDrawYOffset());
		wchRestrictions->set("linXMin", wc->linXMin());
		wchRestrictions->set("linXMax", wc->linXMax());
		wchRestrictions->set("linYMin", wc->linYMin());
		wchRestrictions->set("linYMax", wc->linYMax());
		wchRestrictions->set("canvasXOffset", wc->canvasXOffset());
		wchRestrictions->set("canvasYOffset", wc->canvasYOffset());
		wchRestrictions->set("canvasXSize", wc->canvasXSize());
		wchRestrictions->set("canvasYSize", wc->canvasYSize());
		wchRestrictions->set("pcForeground",
		                     wc->pixelCanvas()->deviceForegroundColor());
		wchRestrictions->set("pcBackground",
		                     wc->pixelCanvas()->deviceBackgroundColor());

		// Determine restrictions on this DisplayData
		AttributeBuffer *ddRestrictions =
		    new AttributeBuffer(*(restrictionBuffer()));
		if (!ddRestrictions) {
			throw(AipsError("Unable to obtain restrictions from CachingDisplayData in "
			                "CachingDisplayData::refreshEH"));
		}

		// Add DisplayData options state to the ddRestrictions:
		ddRestrictions->add(optionsAsAttributes());

		// need to add in cachingAttributes?

		CachingDisplayMethod *cdMethod = 0;

		if (itsCachingState) {
			// Try and find an existing suitable CachingDisplayMethod, and
			// store position of oldest cached CachingDisplayMethod in case
			// we have reached maximum allowed and need to delete one.
			Bool found = False;
			itsElementListIter->toStart();
			while (!itsElementListIter->atEnd() && !found) {
				cdMethod = (CachingDisplayMethod *)itsElementListIter->getRight();
				found = ((cdMethod->worldCanvas() == wc) &&
				         cdMethod->matches(*wchRestrictions) &&
				         cdMethod->matches(*ddRestrictions));
				(*itsElementListIter)++;
			}

			String newCoordsAttr("newCoordinates");

			if (wc->existsAttribute(newCoordsAttr)) {
				found = False;
			}


			if (!found) {
				// did not find suitable previously made DisplayMethod, so
				// make and store a new one:
				cdMethod = newDisplayMethod(wc, wchRestrictions, ddRestrictions, this);
				// preferentially add this CachingDisplayMethod at start of list
				// since it is quicker to find it there next time.  Thus the oldest
				// CachingDisplayMethod is always at the end of the list.
				itsElementListIter->toStart();
				itsElementListIter->addRight((void *)cdMethod);
				// and now trim the cache, ie. remove anything beyond allowable
				// size of cache.
				trimCache();
			} else {

			}
		} else {
			// just make a new DisplayMethod.
			cdMethod = newDisplayMethod(wc, wchRestrictions, ddRestrictions, this);
		}

		delete wchRestrictions;
		wchRestrictions=0;
		delete ddRestrictions;
		ddRestrictions=0;

		// install a colormap if needed

		Colormap* cmsave = wc->colormap();

		if (colormap() != 0) {
			wc->setColormap(colormap());
		} else if (!wc->pixelCanvas()->colormapRegistered()) { // (shouldn't happen)
			wc->setColormap((Colormap *)
			                (wc->pixelCanvas()->pcctbl()->colormapManager().getMap(0)));
		}

		// and finally draw the element
		cdMethod->draw(ev.reason(), *holder);


		// The default for restorePCColormap_ is False.  Derived DDs (such as
		// WedgeDD) can set it True so that the colormap on the PixelCanvas
		// before the DD draws is restored to it afterward.  The 'colormap
		// fiddling' mouse tools can (unfortunately) only operate on the PC's
		// current colormap; this kludge is an attempt to assure that the 'right'
		// one is left there.

		if(restorePCColormap_) wc->setColormap(cmsave);


		if (!itsCachingState) {
			// delete the used object
			delete cdMethod;
			cdMethod=0;
		}
	}

	AttributeBuffer CachingDisplayData::optionsAsAttributes() {
		AttributeBuffer buffer;
		// do not add "cachesize"! - it should not affect how a drawing
		// draws!
		return buffer;
	}

	void CachingDisplayData::setCaching(const Bool caching) {
		if (!caching && itsCachingState) {
			purgeCache();
			// caching turned off
		}
		itsCachingState = caching;
	}

	CachingDisplayData::CachingDisplayData(const CachingDisplayData &o) : DisplayData(o) {
	}

	void CachingDisplayData::operator=(const CachingDisplayData &) {
	}

	void CachingDisplayData::notifyRegister(WorldCanvasHolder *wcHolder) {
		DisplayData::notifyRegister(wcHolder);
	}

	void CachingDisplayData::notifyUnregister(WorldCanvasHolder& wcHolder,
	        Bool ignoreRefresh) {
		purgeCache(wcHolder);
		DisplayData::notifyUnregister(wcHolder, ignoreRefresh);
	}


// Empty cache completely
	void CachingDisplayData::purgeCache() {
		Int temp = itsOptionsMaximumCacheSize;
		itsOptionsMaximumCacheSize = 0;
		trimCache();
		itsOptionsMaximumCacheSize = temp;
	}


// Empty cache of all DMs for a given WCH.
	void CachingDisplayData::purgeCache(const WorldCanvasHolder& wch) {
		CachingDisplayMethod *cdm;
		itsElementListIter->toStart();
		while (!itsElementListIter->atEnd()) {
			cdm = (CachingDisplayMethod *)itsElementListIter->getRight();
			if (cdm->worldCanvas() == wch.worldCanvas()) {
				itsElementListIter->removeRight();
				delete cdm;
				cdm=0;
			} else {
				(*itsElementListIter)++;
			}
		}
	}


// Clear out cache entries beyond end of list
	void CachingDisplayData::trimCache() {
		CachingDisplayMethod *cdMethod = 0;
		if (Int(itsElementListIter->len()) > itsOptionsMaximumCacheSize) {
			itsElementListIter->pos(itsOptionsMaximumCacheSize);
			while (!itsElementListIter->atEnd()) {
				cdMethod = (CachingDisplayMethod *)itsElementListIter->getRight();
				itsElementListIter->removeRight();
				delete cdMethod;
				cdMethod=0;
			}
		}
	}

	void CachingDisplayData::installDefaultOptions() {
		itsOptionsMaximumCacheSize = itsDefaultMaximumCacheSize;
	}

	void CachingDisplayData::setColormap(Colormap *cmap, Float weight) {
		purgeCache();		// Avoid cache reuse with wrong colormap.
		DisplayData::setColormap(cmap, weight);
	}

} //# NAMESPACE CASA - END

