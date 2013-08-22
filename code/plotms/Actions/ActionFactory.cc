//# Copyright (C) 2009
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

#include "ActionFactory.h"
#include <plotms/Actions/ActionAnnotateRectangle.h>
#include <plotms/Actions/ActionAnnotateText.h>
#include <plotms/Actions/ActionCacheLoad.h>
#include <plotms/Actions/ActionCacheRelease.h>
#include <plotms/Actions/ActionClearPlotter.h>
#include <plotms/Actions/ActionClearRegions.h>
#include <plotms/Actions/ActionExport.h>
#include <plotms/Actions/ActionHoldReleaseDrawing.h>
#include <plotms/Actions/ActionInformation.h>
#include <plotms/Actions/ActionIterFirst.h>
#include <plotms/Actions/ActionIterLast.h>
#include <plotms/Actions/ActionIterNext.h>
#include <plotms/Actions/ActionIterPrev.h>
#include <plotms/Actions/ActionLocate.h>
#include <plotms/Actions/ActionMarkRegions.h>
#include <plotms/Actions/ActionPan.h>
#include <plotms/Actions/ActionPlot.h>
#include <plotms/Actions/ActionQuit.h>
#include <plotms/Actions/ActionSelectFlag.h>
#include <plotms/Actions/ActionSelectUnflag.h>
#include <plotms/Actions/ActionStackBack.h>
#include <plotms/Actions/ActionStackBase.h>
#include <plotms/Actions/ActionStackForward.h>
#include <plotms/Actions/ActionSubtractRegions.h>
#include <plotms/Actions/ActionSummary.h>
#include <plotms/Actions/ActionTrackerDisplay.h>
#include <plotms/Actions/ActionTrackerHover.h>
#include <plotms/Actions/ActionZoom.h>

namespace casa {

ActionFactory::ActionFactory() {
}

CountedPtr<PlotMSAction> ActionFactory::getAction( PlotMSAction::Type actionType, Client* client ){
	PlotMSAction* action = NULL;


	switch( actionType ){
	case PlotMSAction::SEL_FLAG:
		action = new ActionSelectFlag( client );
		break;
	case PlotMSAction::SEL_UNFLAG:
		action = new ActionSelectUnflag( client );
		break;
	case PlotMSAction::SEL_LOCATE:
		action = new ActionLocate( client );
		break;
	case PlotMSAction::SEL_INFO:
		action = new ActionInformation( client );
		break;
	case PlotMSAction::SEL_CLEAR_REGIONS:
		action = new ActionClearRegions( client );
		break;
	case PlotMSAction::ITER_FIRST:
		action = new ActionIterFirst( client );
		break;
	case PlotMSAction::ITER_PREV:
		action = new ActionIterPrev( client );
		break;
	case PlotMSAction::ITER_NEXT:
		action = new ActionIterNext( client );
		break;
	case PlotMSAction::ITER_LAST:
		action = new ActionIterLast( client );
		break;
	case PlotMSAction::TOOL_MARK_REGIONS:
		action = new ActionMarkRegions( client );
		break;
	case PlotMSAction::TOOL_SUBTRACT_REGIONS:
		action = new ActionSubtractRegions( client );
		break;
	case PlotMSAction::TOOL_ZOOM:
		action = new ActionZoom( client );
		break;
	case PlotMSAction::TOOL_PAN:
		action = new ActionPan( client );
		break;
	case PlotMSAction::TOOL_ANNOTATE_TEXT:
		action = new ActionAnnotateText( client );
		break;
	case PlotMSAction::TOOL_ANNOTATE_RECTANGLE:
		action = new ActionAnnotateRectangle( client );
		break;
	case PlotMSAction::TRACKER_ENABLE_HOVER:
		action = new ActionTrackerHover( client );
		break;
	case PlotMSAction::TRACKER_ENABLE_DISPLAY:
		action = new ActionTrackerDisplay( client );
		break;
	case PlotMSAction::STACK_BACK:
		action = new ActionStackBack( client );
		break;
	case PlotMSAction::STACK_BASE:
		action = new ActionStackBase( client );
		break;
	case PlotMSAction::STACK_FORWARD:
		action = new ActionStackForward( client );
		break;
	case PlotMSAction::CACHE_LOAD:
		action = new ActionCacheLoad( client );
		break;
	case PlotMSAction::CACHE_RELEASE:
		action = new ActionCacheRelease( client );
		break;
	case PlotMSAction::MS_SUMMARY:
		action = new ActionSummary( client );
		break;
	case PlotMSAction::PLOT:
		action = new ActionPlot( client );
		break;
	case PlotMSAction::PLOT_EXPORT:
		action = new ActionExport( client );
		break;
	case PlotMSAction::HOLD_RELEASE_DRAWING:
		action = new ActionHoldReleaseDrawing( client );
		break;
	case PlotMSAction::CLEAR_PLOTTER:
		action = new ActionClearPlotter( client );
		break;
	case PlotMSAction::QUIT:
		action = new ActionQuit( client );
		break;
	default:
		cout << "Action type is currently unimplemented: type="<<actionType<<endl;
	}
	CountedPtr<PlotMSAction> actionHolder(action );
	return actionHolder;
}

ActionFactory::~ActionFactory() {
}

} /* namespace casa */
