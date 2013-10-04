//# CachingDisplayData.h: base class for auto-caching DisplayData objects
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

#ifndef TRIALDISPLAY_CACHINGDISPLAYDATA_H
#define TRIALDISPLAY_CACHINGDISPLAYDATA_H

#include <casa/aips.h>
#include <casa/Containers/List.h>
#include <display/DisplayDatas/DisplayData.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvas;
	class WorldCanvasHolder;
	class AttributeBuffer;
	class CachingDisplayMethod;

// <summary>
// Base class for auto-caching DisplayData objects.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" data="" tests="" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class=DisplayData> DisplayData </linkto>
// </prerequisite>

// <etymology>
// "CachingDisplayData" is an implementation of a <linkto
// class=DisplayData> DisplayData </linkto> which provides automatic
// caching of individual depictions of the data.
// </etymology>

// <synopsis>
// This class adds to the interface defined in <linkto
// class=DisplayData>DisplayData </linkto>.  It adds an "automagic"
// caching system, which takes care of managing a set of individual
// drawings made by the DisplayData, and using previously generated
// drawing commands where applicable.  For example, the user might be
// playing a movie of several frames in a DisplayData.  The automagic
// caching will ensure that after one complete loop, while parameters
// affecting the display of the data do not change, all subsequent
// drawings will be made from a cache of drawing commands.
// Furthermore, provided the cache is large enough, the user may alter
// parameters, view some more frames, then return the parameters to
// their original state, and there may still be older cached drawings
// which can be used.
//
// The automagic caching also handles the case where a single
// DisplayData is registered on more than one <linkto
// class=WorldCanvasHolder>WorldCanvasHolder </linkto>.  Thus a
// miniature view of the data could be shown in one window, and an
// expanded view in another, with full automagic caching available on
// both.
//
// This is a base class.  Derived classes must implement the
// <src>newDisplayMethod</src> method, which is called to construct a
// new <linkto class=CachingDisplayMethod> CachingDisplayMethod
// </linkto> when the cache cannot satisfy the current drawing
// request.  The method <src>optionsAsAttributes</src> should also be
// implemented: when a new CachingDisplayMethod is constructed, the
// return value of this method will be used to tag the parameters used
// in the drawing itself.  Finally, the method
// <src>cachingAttributes</src> can be over-ridden, and should
// return any additional <linkto class=Attribute> Attributes </linkto>
// which should be considered in searches of the cache.
// </synopsis>

// <motivation>
// Most DisplayData classes will offer more than one view of the data.
// Caching is therefore desirable to improve display speed for
// re-display of individual views of the data.  It is nice to keep the
// caching in one place, hence this class.
// </motivation>

	class CachingDisplayData : public DisplayData {

	public:

		// Constructor.
		CachingDisplayData();

		// Destructor.
		virtual ~CachingDisplayData();

		// Install the default options for this DisplayData.
		virtual void setDefaultOptions();

		// Apply options stored in <src>rec</src> to the DisplayData.  A
		// return value of <src>True</src> means a refresh is needed.
		// <src>recOut</src> contains any fields which were implicitly
		// changed as a result of the call to this function.  The options
		// handled by this class are:
		// <li> <src>cachesize</src>: an integer specifying the maximum
		// number of views which can be stored in the cache.  The default
		// value can be specified by the <src>display.cachesize</src>
		// variable in the user's <src>.aipsrc</src> file, and in lieu
		// of that, will be 256.  If <src>cachesize</src> is lowered by
		// a call to this method, the cache will be shrunk in size, with
		// the oldest drawings being removed first. </li>
		virtual Bool setOptions(Record &rec, Record &recOut);

		// Retrieve the current and default options and parameter types.
		virtual Record getOptions();

		// Refresh event handler which is called indirectly by the
		// WorldCanvas, via the WorldCanvasHolder.  This function will take
		// care of calling <src>newDisplayMethod</src> when necessary, and
		// otherwise using an existing (previously cached) draw list.
		virtual void refreshEH(const WCRefreshEvent &ev);

		// <group>
		virtual void notifyRegister(WorldCanvasHolder *wcHolder) ;
		virtual void notifyUnregister(WorldCanvasHolder& wcHolder,
		                              Bool ignoreRefresh = False) ;
		// </group>

		// Purges cache, to avoid reusing images with the
		// wrong colormap, then calls base class version.
		virtual void setColormap(Colormap *cmap, Float weight);

		// Empty cache completely.
		virtual void purgeCache();

		// Empty cache of all DMs for a given WCH.
		virtual void purgeCache(const WorldCanvasHolder& wch);



	protected:

		// Create a new (Caching)DisplayMethod for drawing on the given
		// WorldCanvas when the AttributeBuffers are suitably matched to the
		// current state of this DisplayData and of the WorldCanvas/Holder.
		virtual CachingDisplayMethod *newDisplayMethod(WorldCanvas *worldCanvas,
		        AttributeBuffer *wchAttributes,
		        AttributeBuffer *ddAttributes,
		        CachingDisplayData *dd) = 0;

		// Return the current options of this DisplayData as an
		// AttributeBuffer.  The caller must delete the returned buffer.
		virtual AttributeBuffer optionsAsAttributes();

		// Return any additional Attributes to consider when searching the
		// cache.  I guess I might put things like Colormap setup in here,
		// since we need a way to handle the Colormap size being altered,
		// and later returned to its original size/location in the
		// PCColorTable, etc.
		/*
		virtual AttributeBuffer cachingAttributes();
		*/

		// Turn caching on/off.
		virtual void setCaching(const Bool caching);

		// (Required) copy constructor.
		CachingDisplayData(const CachingDisplayData &other);

		// (Required) copy assignment.
		void operator=(const CachingDisplayData &other);


		// The default is False.  Derived DDs (such as WedgeDD) can set it True
		// so that the colormap on the PixelCanvas before the DD draws is
		// restored to it afterward.  The 'colormap fiddling' mouse tools can
		// (unfortunately) only operate on the PC's current colormap; this
		// kludge is an attempt to assure that the 'right' one is left there.
		Bool restorePCColormap_;

	private:

		// Caching state.
		Bool itsCachingState;

		// Default and actual maximum length of element list.
		Int itsDefaultMaximumCacheSize, itsOptionsMaximumCacheSize;

		// List containing the CachingDisplayMethods for this
		// CachingDisplayData.
		List<void *> itsElementList;

		// Iterator for itsElementList.
		ListIter<void *> *itsElementListIter;

		// Clear out cache entries beyond end of list.
		void trimCache();

		// Install the default options for this DisplayData.
		void installDefaultOptions();

	};


} //# NAMESPACE CASA - END

#endif

