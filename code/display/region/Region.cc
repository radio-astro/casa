//# Region.cc: base class for non-GUI regions
//# Copyright (C) 2012
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
//#	   Internet email: aips2-request@nrao.edu.
//#	   Postal address: AIPS++ Project Office
//#			   National Radio Astronomy Observatory
//#			   520 Edgemont Road
//#			   Charlottesville, VA 22903-2475 USA
//#
//# $Id: $

#include <display/region/Region.qo.h>
#include <images/Regions/WCUnion.h>
#include <casa/Quanta/MVAngle.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/QtViewer/QtPixelCanvas.qo.h>
#include <images/Images/SubImage.h>
#include <measures/Measures/MCDirection.h>
#include <casa/Quanta/MVTime.h>
#include <display/DisplayErrors.h>

#include <imageanalysis/Annotations/AnnRegion.h>
#include <imageanalysis/Annotations/RegionTextList.h>
#include <display/region/HistogramTab.qo.h>
#include <display/region/Polyline.qo.h>
#include <images/Images/ImageStatistics.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/ComponentShape.h>
#include <components/ComponentModels/SkyCompRep.h>
#include <imageanalysis/ImageAnalysis/ImageFitter.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <casadbus/types/nullptr.h>
#include <math.h>
#include <algorithm>
#include <casa/BasicMath/Functors.h>
#include <cstdlib>
#include <QDir>
#include <QDebug>

#include <display/region/QtRegionDock.qo.h>

#include <tr1/memory>


extern "C" void casa_viewer_pure_virtual( const char *file, int line, const char *func ) {
	fprintf( stderr, "%s:%d pure virtual '%s( )' called...\n", file, line, func );
}

namespace casa {
	namespace viewer {

		const int Region::SEXAGPREC = 9;

		struct strip_white_space {
			strip_white_space(size_t s) : size(s+1), off(0), buf(new char[size]) { }
			strip_white_space( const strip_white_space &other ) : size(other.size), off(other.off),
				buf(new char[size]) {
				strcpy(buf,other.buf);
			}
			~strip_white_space( ) {
				delete [] buf;
			}
			void operator( )( char c ) {
				if ( ! isspace(c) ) buf[off++] = c;
			};
			operator std::string( ) {
				buf[off] = '\0';
				return std::string(buf);
			}
			operator String( ) {
				buf[off] = '\0';
				return String(buf);
			}
			size_t size;
			size_t off;
			char *buf;
		};

		static inline AnnotationBase::LineStyle viewer_to_annotation( region::LineStyle ls ) {
			return ls == region::SolidLine ? AnnotationBase::SOLID : ls == region::DotLine ? AnnotationBase::DOTTED : AnnotationBase::DASHED;
		}

		std::tr1::shared_ptr<Region> Region::creating_region;


		Region::Region( const std::string &name, WorldCanvas *wc,  QtRegionDock *d,
						bool hold_signals_, QtRegionState *supplied_state,
		                QtMouseToolNames::PointRegionSymbols sym ) :  dock_(d), histogram( NULL ),
			position_visible(true), /*** it is assumed that the initial ***
			                         *** state for region dock is with  ***
			                         *** position coordinates visible   ***/
			id_(QtId::get_id( )),
			hold_signals(hold_signals_ ? 1 : 0),
			wc_(wc), selected_(false), visible_(true),
			complete(false), z_index_within_range(true),
			draw_center_(false), name_(name) {

			mystate = supplied_state == 0 ? new QtRegionState( QString::fromStdString(name_), sym, this ) : supplied_state;
			init( );
		}

		void Region::init( ) {
			last_z_index = wc_ == 0 ? 0 : wc_->zIndex( );
			// if ( wc_->restrictionBuffer()->exists("zIndex")) {
			//	wc_->restrictionBuffer()->getValue("zIndex", last_z_index);
			// }
			connect( mystate, SIGNAL(regionChange(viewer::Region*,std::string)), SIGNAL(regionChange(viewer::Region*,std::string)) );

			connect( mystate, SIGNAL(refreshCanvas( )), SLOT(refresh_canvas_event( )) );
			connect( mystate, SIGNAL(statisticsVisible(bool)), SLOT(refresh_statistics_event(bool)) );
			connect( mystate, SIGNAL(collectStatistics( )), SLOT(reload_statistics_event( )) );
			connect( mystate, SIGNAL(positionVisible(bool)), SLOT(refresh_position_event(bool)) );

			connect( mystate, SIGNAL(translateX(const QString &, const QString &, const QString &)), SLOT(translate_x(const QString&,const QString&, const QString &)) );
			connect( mystate, SIGNAL(translateY(const QString &, const QString &, const QString &)), SLOT(translate_y(const QString&,const QString&, const QString &)) );
			connect( mystate, SIGNAL(resizeX(const QString &, const QString &, const QString &)), SLOT(resize_x(const QString&,const QString&, const QString &)) );
			connect( mystate, SIGNAL(resizeY(const QString &, const QString &, const QString &)), SLOT(resize_y(const QString&,const QString&, const QString &)) );

			connect (mystate->getFitButton(), SIGNAL(clicked()), this, SLOT(updateCenterInfo()));

			connect( mystate, SIGNAL(zRange(int,int)), SLOT(refresh_zrange_event(int,int)) );
			connect( dock_, SIGNAL(deleteRegion(QtRegionState*)), SLOT(revoke_region(QtRegionState*)) );
			connect( dock_, SIGNAL(deleteAllRegions( )), SLOT(revoke_region( )) );
			connect( dock_, SIGNAL(saveRegions(std::list<QtRegionState*>, RegionTextList &)), SLOT(output(std::list<QtRegionState*>, RegionTextList &)) );
			connect( dock_, SIGNAL(saveRegions(std::list<QtRegionState*>, ds9writer &)), SLOT(output(std::list<QtRegionState*>, ds9writer &)) );

			dock_->addRegion(this,mystate);
			signal_region_change( region::RegionChangeCreate );
		}

		Region::~Region( ) {
			dock_->removeRegion(mystate);
			signal_region_change( region::RegionChangeDelete );
			disconnect(mystate, 0, 0, 0);
			delete histogram;
			// QtRegionState is cleaned up by Qt...
			mystate->clearRegion( );
		}

		bool Region::degenerate( ) const {
			// incomplete regions can not yet be found to be degenerate...
			if ( complete == false ) return false;
			double blcx, blcy, trcx, trcy;
			boundingRectangle(blcx,blcy,trcx,trcy);
			double pblcx, pblcy, ptrcx, ptrcy;
			try {
				linear_to_pixel( wc_, blcx, blcy, trcx, trcy, pblcx, pblcy, ptrcx, ptrcy );
			} catch (...) {
				return true;
			}
			// non-degenerate if (un-zoomed) any pixel dimensions are less than zero...
			return (ptrcx - pblcx) < 1 && (ptrcy - pblcy) < 1;
		}

		int Region::numFrames( ) const {
			return dock_->numFrames( );
		}

		void Region::zRange( int &min, int &max ) const {
			min = mystate->zMin( );
			max = mystate->zMax( );
		}

		int Region::zIndex( ) const {
			return wc_ == 0 ? last_z_index : wc_->zIndex( );
		}

		bool Region::worldBoundingRectangle( double &width, double &height, const std::string &units ) const {
			width = 0.0;
			height = 0.0;

			if ( wc_ == 0 ) return false;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			// calculate the width
			Vector<Double> left_linearv(2);
			Vector<Double> right_linearv(2);
			Vector<Double> left_worldv(2);
			Vector<Double> right_worldv(2);

			left_linearv(0) = blc_x;
			left_linearv(1) = (trc_y + blc_y) / 2.0;
			right_linearv(0) = trc_x;
			right_linearv(1) = left_linearv(1);
			if ( ! wc_->linToWorld( left_worldv, left_linearv ) ||
			        ! wc_->linToWorld( right_worldv, right_linearv ) ) {
				return false;
			}

			MDirection::Types cccs = current_casa_coordsys( );
			MDirection left( Quantum<Vector<Double> > (left_worldv,"rad"), cccs );
			MDirection right( Quantum<Vector<Double> > (right_worldv,"rad"), cccs );

			Quantity widthq = left.getValue( ).separation(right.getValue( ),units.c_str( ));
			width = widthq.getValue( );

			// calculate the height
			Vector<Double> top_linearv(2);
			Vector<Double> bottom_linearv(2);
			Vector<Double> top_worldv(2);
			Vector<Double> bottom_worldv(2);

			top_linearv(0) = (trc_x + blc_x) / 2.0;
			top_linearv(1) = trc_y;
			bottom_linearv(0) = top_linearv(0);
			bottom_linearv(1) = blc_y;
			if ( ! wc_->linToWorld( top_worldv, top_linearv ) ||
			        ! wc_->linToWorld( bottom_worldv, bottom_linearv ) ) {
				return false;
			}

			MDirection top( Quantum<Vector<Double> > (top_worldv,"rad"), cccs );
			MDirection bottom( Quantum<Vector<Double> > (bottom_worldv,"rad"), cccs );

			Quantity heightq = top.getValue( ).separation(bottom.getValue( ),units.c_str( ));
			height = heightq.getValue( );

			return true;
		}

		void Region::setDrawingEnv( ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
			PixelCanvas *pc = wc_->pixelCanvas();
			if(pc==0) return;

			Int x0 = wc_->canvasXOffset();
			Int x1 = x0 + wc_->canvasXSize() - 1;
			Int y0 = wc_->canvasYOffset();
			Int y1 = y0 + wc_->canvasYSize() - 1;

			pc->setClipWindow(x0,y0, x1,y1);
			pc->enable(Display::ClipWindow);

			pc->setLineWidth(1);
			pc->setCapStyle(Display::CSRound);
			pc->setColor(lineColor());

			Display::LineStyle current_ls = pc->getLineStyle( );
			switch ( current_ls ) {
			case Display::LSSolid:
				ls_stack.push_back(ls_ele(region::SolidLine,lineWidth()));
				break;
			case Display::LSDashed:
				ls_stack.push_back(ls_ele(region::DashLine,lineWidth()));
				break;
			case Display::LSDoubleDashed:
				ls_stack.push_back(ls_ele(region::LSDoubleDashed,lineWidth()));
				break;
			}

			set_line_style( ls_ele(lineStyle( ),lineWidth()) );

			pc->setDrawFunction(Display::DFCopy);
		}

		void Region::resetDrawingEnv( ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
			PixelCanvas *pc = wc_->pixelCanvas();
			if ( pc == 0 ) return;

			pc->disable(Display::ClipWindow);

			set_line_style( ls_stack.back( ) );
			ls_stack.pop_back( );

			if ( ls_stack.size( ) != 0 )
				throw internal_error( "Region stack inconsistency" );
		}

		void Region::setTextEnv( ) {
			PixelCanvas *pc = wc_->pixelCanvas();
			if ( pc == 0 ) return;
			pc->setColor(textColor( ));
			QtPixelCanvas* qpc = dynamic_cast<QtPixelCanvas*>(pc);
			if ( qpc != 0 ) {
				int text_mod = textFontStyle( );
				qpc->setFont( textFont( ), textFontSize( ),
				              text_mod & region::BoldText ? true : false,
				              text_mod & region::ItalicText ? true : false );
			}
		}

		void Region::resetTextEnv( ) {
		}

		void Region::pushDrawingEnv( region::LineStyle ls, int thickness ) {
			ls_stack.push_back(ls_ele(current_ls,lineWidth()));
			set_line_style(ls_ele(ls, thickness));
		}

		void Region::popDrawingEnv( ) {
			set_line_style( ls_stack.back( ) );
			ls_stack.pop_back( );
		}

		void Region::refresh( ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
			PixelCanvas *pc = wc_->pixelCanvas();
			if ( pc == 0 ) return;
			pc->copyBackBufferToFrontBuffer();
			pc->setDrawBuffer(Display::FrontBuffer);
			pc->callRefreshEventHandlers(Display::BackCopiedToFront);
		}


		bool Region::within_drawing_area( ) {
			double blcx, blcy, trcx, trcy;
			boundingRectangle(blcx,blcy,trcx,trcy);
			int sblcx, sblcy, strcx, strcy;
			linear_to_screen( wc_, blcx, blcy, trcx, trcy, sblcx, sblcy, strcx, strcy );
			return wc_->inDrawArea(sblcx,sblcy) && wc_->inDrawArea(strcx,strcy);
		}

		void Region::draw( bool other_selected ) {
			visible_ = true;
			if ( wc_ == 0 || wc_->csMaster() == 0 ) {
				visible_ = false;
				return;
			}

			if ( ! within_drawing_area( ) ) {
				visible_ = false;
				return;
			}

			// When stepping through a cube, this detects that a different plane is being displayed...
			// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
			int new_z_index = wc_->zIndex( );
			int z_min, z_max;
			zRange(z_min,z_max);

			if ( new_z_index < z_min || new_z_index > z_max ) {
				visible_ = false;
				clearStatistics( );
				return;
			} else {
				visible_ = true;
			}

			if ( new_z_index != last_z_index ) {
				updateStateInfo( true, region::RegionChangeNewChannel );
				invalidateCenterInfo( );
			}
			last_z_index = new_z_index;
			// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

			setDrawingEnv( );
			drawRegion( (! other_selected && selected( )) || marked( ) || weaklySelected( ) );
			//if (draw_center_) cout << "center drawn" << endl; else cout << "center NOT drawn" << endl;
			resetDrawingEnv( );

			setTextEnv( );
			drawText( );
			resetTextEnv( );
		}

		void Region::invalidateCenterInfo( ) {
			// set the background to "darkgrey"
			mystate->setCenterBackground(QString("#a9a9a9"));
		}

		bool Region::weaklySelected( ) const {
			return dock_->isWeaklySelectedRegion(this);
		}
		void Region::weaklySelect( bool scroll_dock ) {
			dock_->addWeaklySelectedRegion(this);
			dock_->selectRegion(mystate, scroll_dock);
			dock_->selectedCountUpdateNeeded( );
			signal_region_change( region::RegionChangeSelected );
		}

		void Region::weaklyUnselectLimited( ) {
			dock_->removeWeaklySelectedRegion(this);
		}
		void Region::weaklyUnselect( ) {
			weaklyUnselectLimited( );
			const region::region_list_type &weak = dock_->weaklySelectedRegionSet( );
			if ( weak.size( ) > 0 ) {
				Region *region = dynamic_cast<Region*>(*weak.begin( ));
				if ( region ) dock_->selectRegion(region->state( ));
			} else {
				const region::region_list_type &marked = dock_->selectedRegionSet( );
				if ( marked.size( ) > 0 ) {
					Region *region = dynamic_cast<Region*>(*marked.begin( ));
					if ( region ) dock_->selectRegion(region->state( ));
				} else {
					updateStateInfo( false, region::RegionChangeFocus );
				}
			}
			dock_->selectedCountUpdateNeeded( );
		}

		void Region::selectedInCanvas( ) {
			dock_->selectRegion(mystate);
		}

		void Region::drawText( ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
			PixelCanvas *pc = wc_->pixelCanvas();

			double lin_blc_x, lin_blc_y, lin_trc_x, lin_trc_y;
			boundingRectangle( lin_blc_x, lin_blc_y, lin_trc_x, lin_trc_y );
			int blc_x, blc_y, trc_x, trc_y;
			try {
				linear_to_screen( wc_, lin_blc_x, lin_blc_y, lin_trc_x, lin_trc_y, blc_x, blc_y, trc_x, trc_y );
			} catch(...) {
				return;
			}

			int x=0, y=0;
			int dx, dy;
			textPositionDelta( dx, dy );

			int m_angle = 0;
			region::TextPosition tp = textPosition( );
			std::string text = textValue( );
			int text_height = pc->textHeight(text);
			int text_width = pc->textWidth(text);


			Display::TextAlign alignment = Display::AlignCenter;

			const int offset = 5;
			switch ( tp ) {
			case region::TopText:
				alignment = Display::AlignBottomLeft;
				y = trc_y + offset;
				x = (int) (((double) (blc_x + trc_x - text_width) + 0.5) / 2.0);
				break;
			case region::RightText:
				alignment = Display::AlignBottomLeft;
				x = trc_x + offset;
				y = (int) (((double) (blc_y + trc_y - text_height) + 0.5) / 2.0);
				break;
			case region::BottomText:
				alignment = Display::AlignTopLeft;
				y = blc_y - offset;
				x = (int) (((double) (blc_x + trc_x - text_width) + 0.5) / 2.0);
				break;
			case region::LeftText:
				alignment = Display::AlignBottomRight;
				x = blc_x - offset;
				y = (int) (((double) (blc_y + trc_y - text_height) + 0.5) / 2.0);
				break;
			}

			pc->drawText( x + dx, y + dy, text, m_angle, alignment );
		}

		void Region::drawCenter(double &x, double &y ) {
			PixelCanvas *pc = wc_->pixelCanvas();
			if(pc==0) return;
			int x_screen, y_screen;
			try {
				linear_to_screen( wc_, x, y, x_screen, y_screen);
			} catch(...) {
				return;
			}
			pc->drawFilledRectangle(x_screen-2, y_screen-2, x_screen+2, y_screen+2);
		}

		void Region::drawCenter(double &x, double &y, double &deltx, double &delty) {
			PixelCanvas *pc = wc_->pixelCanvas();
			if(pc==0) return;
			if (!markCenter()) return;
			// switch to the center color
			pc->setColor(centerColor());

			// draw the center
			drawCenter(x, y);

			// find the scale such that the arrow
			// is the size of the larger box size
			double blcx, blcy, trcx, trcy, scale;
			int x1, x2, y1, y2;
			boundingRectangle(blcx,blcy,trcx,trcy);
			scale = fabs(trcy-blcy) > fabs(trcx-blcx) ? 0.5*fabs(trcy-blcy) : 0.5*fabs(trcx-blcx);

			// compute the screen coos
			try {
				linear_to_screen( wc_, x + scale*deltx, y + scale*delty, x1, y1);
			} catch(...) {
				return;
			}
			try {
				linear_to_screen( wc_, x - scale*deltx, y - scale*delty, x2, y2);
			} catch(...) {
				return;
			}

			// draw the line
			pc->drawLine(x1, y1, x2, y2);

			// switch back to the general line color
			pc->setColor(lineColor());
		}

		void Region::mark(bool set) {
			if ( set != mystate->marked( ) ) {
				mystate->mark(set);
				emit selectionChanged(this,set);
			}
		}

		bool Region::mark_toggle( ) {
			bool result = mystate->mark_toggle( );
			emit selectionChanged(this,result);
			return result;
		}

		size_t Region::selected_region_count( ) {
			return dock_->selectedRegionCount( );
		}
		size_t Region::marked_region_count( ) {
			return dock_->markedRegionCount( );
		}

		void Region::status( const std::string &msg, const std::string &type ) {
			dock_->status(msg,type);
		}

		void Region::emitUpdate( ) {
			region::RegionTypes type;
			QList<int> pixelx, pixely;
			QList<double> worldx, worldy;

			fetch_details( type, pixelx, pixely, worldx, worldy );
			emit regionUpdateResponse( id_, QString( type == region::RectRegion ? "rectangle" : type == region::PointRegion ? "point" :
			                           type == region::EllipseRegion ? "ellipse" : type == region::PolyRegion ? "polygon" :
			                           type == region::PolylineRegion ? "polyline" : "error"),
			                           QString::fromStdString(name( )), worldx, worldy, pixelx, pixely, QString::fromStdString(lineColor( )),
			                           QString::fromStdString(textValue( )), QString::fromStdString(textFont( )), textFontSize( ),
			                           textFontStyle( ) );
		}

		void Region::refresh_state_gui( ) {
			std::string mode = mystate->mode( );
			if ( mode == "position" ) {
				region::Coord c;
				region::Units xu,yu;
				std::string whu;
				std::string x, y, angle;
				double width, height;
				mystate->getCoordinatesAndUnits( c, xu, yu, whu );
				getPositionString( x, y, angle, width, height, c, xu, yu, whu );

				QString qwidth;
				QString qheight;
				if ( width < 0.001 && height < 0.001 ) {
					qwidth = QString("%1").arg(width,0,'g',5);
					qheight = QString("%1").arg(height,0,'g',5);
				} else {
					qwidth = QString("%1").arg(width);
					qheight = QString("%1").arg(height);
				}

				mystate->updatePosition( QString::fromStdString(x),
				                         QString::fromStdString(y),
				                         QString::fromStdString(angle),
				                         qwidth, qheight );
			} else if ( mode == mystate->STATISTICS_MODE( ).toStdString() ) {
				mystate->updateStatistics( );
			} else if ( mode == mystate->HISTOGRAM_MODE( ).toStdString() ) {
				this->updateHistogramRegion();
			}


#if 0
			// update statistics, when needed...
			// if ( statistics_visible == false ) {
			// 	if ( region_modified ) statistics_update_needed = true;
			// } else if ( (statistics_update_needed || region_modified ) && regionVisible( ) ) {
			// 	reload_statistics_event( );
			// }

			// update position, when needed...
			if ( position_visible == false ) {
				if ( region_modified ) position_update_needed = true;
			} else if ( (position_update_needed || region_modified) && regionVisible( ) ) {
			}
#endif

		}

// indicates that region movement requires that the statistcs be updated...
		void Region::updateStateInfo( bool /*region_modified*/, region::RegionChanges change ) {
			signal_region_change( change );
			refresh_state_gui( );

		}

		void Region::adjustCorners( double /*blcx*/, double /*blcy*/, double /*trcx*/, double /*trcy*/ ) {
			fprintf( stderr, "!!!!!!!!!!!!!!!!!!!!>>> 	void Region::adjustCorners( double blcx, double blcy, double trcx, double trcy )\n" );

		}

		void Region::clearStatistics( ) {
			statistics_update_needed = true;
			mystate->clearStatistics( );
		}

		int &Region::colorIndex( ) {
			return dock_->colorIndex( );
		}
		std::pair<int,int> &Region::tabState( ) {
			return dock_->tabState( );
		}
		std::map<std::string,int> &Region::coordState( ) {
			return dock_->coordState( );
		}

		void Region::selectedCountUpdateNeeded( ) {
			dock_->selectedCountUpdateNeeded( );
		}

		QString Region::getSaveDir( ) {
			if ( dock_->saveDir( ).isNull( ) ) {
				if ( ! dock_->loadDir( ).isNull( ) )
					dock_->saveDir( ) = dock_->loadDir( );
				else
					dock_->saveDir( ) = QDir::currentPath();
			}
			return dock_->saveDir( );
		}
		void Region::putSaveDir( QString dir ) {
			dock_->saveDir( ) = dir;
		}
		QString Region::getLoadDir( ) {
			if ( dock_->loadDir( ).isNull( ) ) {
				if ( ! dock_->saveDir( ).isNull( ) )
					dock_->loadDir( ) = dock_->saveDir( );
				else
					dock_->loadDir( ) = QDir::currentPath();
			}
			return dock_->loadDir( );
		}
		void Region::putLoadDir( QString dir ) {
			dock_->loadDir( ) = dir;
		}

		void Region::refresh_canvas_event( ) {
			refresh( );
		}
		void Region::refresh_statistics_event( bool visible ) {
			statistics_visible = visible;
			if ( hold_signals ) {
				held_signals[region::RegionChangeStatsUpdate] = true;
				return;
			}
			updateStateInfo( false, region::RegionChangeFocus );
		}
		void Region::refresh_position_event( bool visible ) {
			position_visible = visible;
			updateStateInfo( false, region::RegionChangeUpdate );
		}

		void Region::translate_x( const QString &x, const QString &x_units, const QString &coordsys ) {
			if ( translateX( x.toStdString( ), x_units.toStdString( ), coordsys.toStdString( ) ) ) {
				refresh( );
			}
		}
		void Region::translate_y( const QString &y, const QString &y_units, const QString &coordsys ) {
			if ( translateY( y.toStdString( ), y_units.toStdString( ), coordsys.toStdString( ) ) ) {
				refresh( );
			}
		}
		void Region::resize_x( const QString &x, const QString &x_units, const QString &coordsys ) {
			resizeX( x.toStdString( ), x_units.toStdString( ), coordsys.toStdString( ) );
		}
		void Region::resize_y( const QString &y, const QString &y_units, const QString &coordsys ) {
			resizeY( y.toStdString( ), y_units.toStdString( ), coordsys.toStdString( ) );
		}



		void Region::updateCenterInfo() {
			std::list<std::tr1::shared_ptr<RegionInfo> > *rc = generate_dds_centers( );
			mystate->updateCenters(rc);

			// set the background to standard color which is some kind of grey
			mystate->setCenterBackground(QString("#e8e8e8"));
		}
		void Region::refresh_zrange_event( int min, int max ) {
			int index = zIndex( );
			if ( z_index_within_range == true && (index < min || index > max) ) {
				z_index_within_range = false;
				refresh( );
			} else if ( z_index_within_range == false && index >= min && index <= max ) {
				z_index_within_range = true;
				refresh( );
			}
		}

		void Region::revoke_region( ) {
			dock_->revokeRegion(this);
		}

		void Region::revoke_region( QtRegionState *redacted_state ) {
			if ( redacted_state == mystate ) {
				revoke_region( );
			}
		}

		void Region::reload_statistics_event( ) {
			statistics_update_needed = false;
			std::list<std::tr1::shared_ptr<RegionInfo> > *rl = generate_dds_statistics( );
			// send statistics to region state object...
			mystate->updateStatistics(rl);
			delete rl;
		}

		void Region::output( std::list<QtRegionState*> ol, RegionTextList &regionlist ) {
			std::list<QtRegionState*>::iterator iter = find( ol.begin(), ol.end( ), mystate );
			if ( iter != ol.end( ) ) {

				AnnotationBase *ann = annotation( );

				if ( ann == 0 ) {
					fprintf( stderr, "Failed to create region annotation...\n" );
					return;
				}

				AnnRegion *reg = dynamic_cast<AnnRegion*>(ann);
				if ( reg ) reg->setAnnotationOnly((*iter)->isAnnotation( ));

				// int number_frames = (*iter)->numFrames( );
				ann->setLabel( (*iter)->textValue( ) );

				ann->setColor( (*iter)->lineColor( ) );
				ann->setLineStyle( viewer_to_annotation((*iter)->lineStyle( )) );
				ann->setLineWidth( (*iter)->lineWidth( ) );

				ann->setFont( (*iter)->textFont( ) );
				ann->setFontSize( (*iter)->textFontSize( ) );
				int font_style = (*iter)->textFontStyle( );

				switch ( textPosition( ) ) {
				case region::BottomText:
					ann->setLabelPosition("bottom");
					break;
				case region::LeftText:
					ann->setLabelPosition("left");
					break;
				case region::RightText:
					ann->setLabelPosition("right");
					break;
				default:
					ann->setLabelPosition("top");
				}
				ann->setLabelColor(textColor( ));

				vector<int> delta(2);
				textPositionDelta( delta[0], delta[1] );
				if ( delta[0] != 0 || delta[1] != 0 ) {
					ann->setLabelOffset(delta);
				}

				ann->setFontStyle( font_style & region::ItalicText && font_style & region::BoldText ? AnnotationBase::ITALIC_BOLD :
				                   font_style & region::ItalicText ? AnnotationBase::ITALIC :
				                   font_style & region::BoldText ? AnnotationBase::BOLD : AnnotationBase::NORMAL );

				regionlist.addLine(AsciiAnnotationFileLine(ann));
			}
		}

		void Region::output( std::list<QtRegionState*> ol, ds9writer &out ) {
			std::list<QtRegionState*>::iterator iter = find( ol.begin(), ol.end( ), mystate );
			if ( iter != ol.end( ) ) {
				output(out);
			}
		}


		bool Region::doubleClick( double /*x*/, double /*y*/ ) {
			int output_count = 0;
			const char buf[ ] = "---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----\n";
			std::list<std::tr1::shared_ptr<RegionInfo> > *info = generate_dds_statistics( );
			for ( std::list<std::tr1::shared_ptr<RegionInfo> >::iterator iter = info->begin( ); iter != info->end( ); ++iter ) {
				std::tr1::shared_ptr<RegionInfo::stats_t> stats = (*iter)->list( );

				if (memory::nullptr.check(stats))
					continue;

				// output label....
				fputs( buf, stdout );
				fprintf( stdout, "(%s)\n", (*iter)->label().c_str( ) );
				output_count++;

				size_t width = 0;
				for ( RegionInfo::stats_t::iterator stats_iter = stats->begin( ); stats_iter != stats->end( ); ++stats_iter ) {
					size_t w = (*stats_iter).first.size( );
					if ( w > width ) width = w;
					w = (*stats_iter).second.size( );
					if ( w > width ) width = w;
				}
				char format[10];
				sprintf( format, "%%%lus ", (width > 0 && width < 30 ? width : 15) );
				for ( RegionInfo::stats_t::iterator stats_iter = stats->begin( ); stats_iter != stats->end( ); ) {
					RegionInfo::stats_t::iterator row = stats_iter;
					for ( int i=0; i < 5 && row != stats->end( ); ++i ) {
						fprintf( stdout, format, (*row).first.c_str( ) );
						++row;
					}
					fprintf( stdout, "\n");
					row = stats_iter;
					for ( int i=0; i < 5 && row != stats->end( ); ++i ) {
						fprintf( stdout, format, (*row).second.c_str( ) );
						++row;
					}
					fprintf( stdout, "\n" );
					stats_iter = row;
				}
			}

			if ( output_count > 0 ) fputs( buf, stdout );

			delete info;
			return false;
		}

		static std::string as_string( double v ) {
			char buf[256];
			sprintf( buf, "%g", v );
			return std::string(buf);
		}

		static std::string as_string( int v ) {
			char buf[256];
			sprintf( buf, "%d", v );
			return std::string(buf);
		}

// region units as string... note, for HMS, DMS radian conversion is first done...
		static inline const char *as_string( region::Units units ) {
			return units == region::Degrees ? "deg" : "rad";
		}

		void Region::setLabel( const std::string &l ) {
			mystate->setTextValue(l);
		}
		void Region::setLabelPosition( region::TextPosition pos ) {
			mystate->setTextPosition( pos );
		}
		void Region::setLabelDelta( const std::vector<int> &delta ) {
			mystate->setTextDelta( delta );
		}

		void Region::setFont( const std::string &font, int font_size, int font_style, const std::string &font_color ) {
			if ( font != "" ) mystate->setTextFont(font);
			if ( font_size >= 0 ) mystate->setTextFontSize(font_size);
			mystate->setTextFontStyle( font_style );
			if ( font_color != "" ) mystate->setTextColor( font_color );
		}

		void Region::setLine( const std::string &line_color, region::LineStyle line_style, unsigned int line_width ) {
			if ( line_color != "" ) mystate->setLineColor( line_color );
			mystate->setLineStyle( line_style );
			mystate->setLineWidth( line_width );
		}

		void Region::setAnnotation(bool ann) {
			mystate->setAnnotation(ann);
		}

		void Region::getCoordinatesAndUnits( region::Coord &c, region::Units &x_units, region::Units &y_units, std::string &width_height_units ) const {
			c = current_region_coordsys( );
			x_units = current_xunits( );
			y_units = current_yunits( );
			width_height_units = (x_units == region::Radians ? "rad" : "deg");
		}

		static inline double wrap_angle( double before, double after ) {
			const double UNIT_WRAPAROUND = 2.0 * M_PI;
			if ( after < 0 && before > 0 )
				return after + UNIT_WRAPAROUND;
			else if ( after > 0 && before < 0 )
				return after - UNIT_WRAPAROUND;
			return after;
		}

		static inline region::Coord casa_to_viewer( MDirection::Types type ) {
			return type == MDirection::J2000 ? region::J2000 :
			       type == MDirection::B1950 ? region::B1950 :
			       type == MDirection::GALACTIC ? region::Galactic :
			       type == MDirection::SUPERGAL ? region::SuperGalactic :
			       type == MDirection::ECLIPTIC ? region::Ecliptic : region::J2000;
		}

		static inline MDirection::Types viewer_to_casa( region::Coord type ) {
			return type == region::J2000 ?  MDirection::J2000 :
			       type == region::B1950 ?	 MDirection::B1950 :
			       type == region::Galactic ? MDirection::GALACTIC :
			       type == region::SuperGalactic ? MDirection::SUPERGAL :
			       type == region::Ecliptic ? MDirection::ECLIPTIC : MDirection::J2000;
		}

		static inline MDirection::Types string_to_casa_coordsys( const std::string &s ) {
			return s == "J2000" ? MDirection::J2000 :
			       s == "B1950" ? MDirection::B1950 :
			       s == "galactic" ? MDirection::GALACTIC :
			       s == "super galactic" ? MDirection::SUPERGAL :
			       s == "ecliptic" ? MDirection::ECLIPTIC : MDirection::J2000;
		}

		static inline String string_to_casa_units( const std::string &s ) {
			return s == "degrees" ? "deg" : "rad";
		}


// static inline void convert_units( Quantum<Vector<double> > &q, region::Units new_units ) {
//     q.convert( as_string(new_units) );
// }

		static inline void convert_units( double &x, const std::string &xunits, region::Units new_x_units, double &y, const std::string &yunits, region::Units new_y_units ) {
			Quantum<double> resultx(x, xunits.c_str( ));
			Quantum<double> resulty(y, yunits.c_str( ));
			x = resultx.getValue(as_string(new_x_units));
			y = resulty.getValue(as_string(new_y_units));
		}

		Quantum<Vector<double> > Region::convert_angle( double x, const std::string &xunits, double y, const std::string &yunits,
		        MDirection::Types original_coordsys, MDirection::Types new_coordsys,
		        const std::string &new_units ) {
			Quantum<double> xq(x,String(xunits));
			Quantum<double> yq(y,String(yunits));
			MDirection md = MDirection::Convert(MDirection(xq,yq,original_coordsys), new_coordsys)();
			casa::Quantum<casa::Vector<double> > result = md.getAngle("rad");
			xq.convert("rad");
			yq.convert("rad");
			result.getValue( )(0) = wrap_angle(xq.getValue( ), result.getValue( )(0));
			result.getValue( )(1) = wrap_angle(yq.getValue( ), result.getValue( )(1));
			result.convert(String(new_units));
			return result;
		}


		void Region::getPositionString( std::string &x, std::string &y, std::string &angle,
		                                double &bounding_width, double &bounding_height, region::Coord coord,
		                                region::Units new_x_units, region::Units new_y_units, const std::string &bounding_units ) const {
			if ( wc_ == 0 ) {
				x = y = angle = "internal error";
				return;
			}

			if ( wc_->csMaster() == 0 ) {
				x = y = angle = "no wcs";
				return;
			}

			if ( coord == region::DefaultCoord ) coord = current_region_coordsys( );
			if ( new_x_units == region::DefaultUnits ) new_x_units = current_xunits( );
			if ( new_y_units == region::DefaultUnits ) new_y_units = current_xunits( );
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			MDirection::Types cccs = current_casa_coordsys( );

			if ( cccs == MDirection::N_Types ) {
				// this impiles that the coordinate system does not have a direction coordinate...
				// so it is probably a measurement set... treat as a pixel coordinate for now...
				double center_x, center_y;
				try {
					linear_to_pixel( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), center_x, center_y );
				} catch(...) {
					return;
				}
				x = as_string(center_x);
				y = as_string(center_y);

				// set bounding width/height
				double p_blc_x, p_blc_y, p_trc_x, p_trc_y;
				try {
					linear_to_pixel( wc_, blc_x, blc_y, trc_x, trc_y, p_blc_x, p_blc_y, p_trc_x, p_trc_y );
				} catch(...) {
					return;
				}
				bounding_width = fabs(p_trc_x-p_blc_x);
				bounding_height = fabs(p_trc_y-p_blc_y);

				angle = as_string(0);
				return;
			}

			region::Coord cvcs = casa_to_viewer(cccs);
			double result_x, result_y;
			const Vector<String> &units = wc_->worldAxisUnits();

			// seting bounding units
			if ( bounding_units == "pixel" ) {
				double p_blc_x, p_blc_y, p_trc_x, p_trc_y;
				try {
					linear_to_pixel( wc_, blc_x, blc_y, trc_x, trc_y, p_blc_x, p_blc_y, p_trc_x, p_trc_y );
				} catch(...) {
					return;
				}
				bounding_width = fabs(p_trc_x-p_blc_x);
				bounding_height = fabs(p_trc_y-p_blc_y);
			} else {
				worldBoundingRectangle( bounding_width, bounding_height, bounding_units );
			}

			if ( coord == cvcs ) {
				try {
					linear_to_world( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), result_x, result_y );
				} catch(...) {
					return;
				}
				convert_units( result_x, units[0], new_x_units, result_y, units[1], new_y_units );

			} else {
				try {
					linear_to_world( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), result_x, result_y );
				} catch(...) {
					return;
				}
				Quantum<Vector<double> > result = convert_angle( result_x, units[0], result_y, units[1], cccs, viewer_to_casa(coord) );
				result_x = result.getValue(as_string(new_x_units))(0);
				result_y = result.getValue(as_string(new_y_units))(1);

			}

			const Vector<String> &axis_labels = wc_->worldAxisNames( );

			if ( new_x_units == region::Pixel ) {
				double center_x, center_y;
				try {
					linear_to_pixel( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), center_x, center_y );
				} catch(...) {
					return;
				}
				x = as_string(center_x);
			} else if ( new_x_units == region::Sexagesimal ) {
				if ( axis_labels(0) == "Declination" || (coord != region::J2000 && coord != region::B1950) ) {
					// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
					// D.M.S
					// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
					// MVAngle::operator(double norm) => 2*pi*norm to 2pi*norm+2pi
					//x = MVAngle(result_x)(0.0).string(MVAngle::ANGLE_CLEAN,SEXAGPREC);
					// MVAngle::operator( ) => -pi to +pi
					x = MVAngle(result_x)( ).string(MVAngle::ANGLE_CLEAN,SEXAGPREC);
				} else {
					// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
					// H:M:S
					// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
					x = MVAngle(result_x)(0.0).string(MVAngle::TIME,SEXAGPREC);
				}
			} else {
				x = as_string(result_x);
			}

			if ( new_y_units == region::Pixel ) {
				double center_x, center_y;
				try {
					linear_to_pixel( wc_, linear_average(blc_x,trc_x), linear_average(blc_y,trc_y), center_x, center_y );
				} catch(...) {
					return;
				}
				y = as_string(center_y);
			} else if ( new_y_units == region::Sexagesimal ) {
				if ( axis_labels(1) == "Declination"  || (coord != region::J2000 && coord != region::B1950) ) {
					// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
					// D.M.S
					// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
					// MVAngle::operator(double norm) => 2*pi*norm to 2pi*norm+2pi
					//y = MVAngle(result_y)(0.0).string(MVAngle::ANGLE_CLEAN,SEXAGPREC);
					// MVAngle::operator( ) => -pi to +pi
					y = MVAngle(result_y)( ).string(MVAngle::ANGLE_CLEAN,SEXAGPREC);
				} else {
					// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
					// H:M:S
					// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
					y = MVAngle(result_y)(0.0).string(MVAngle::TIME,SEXAGPREC);
				}
			} else {
				y = as_string(result_y);
			}
			angle = "";

		}

		bool Region::translateX( const std::string &x, const std::string &x_units, const std::string &coordsys ) {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return false;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			// current center of region...
			double cur_center_x = linear_average(blc_x,trc_x);
			double cur_center_y = linear_average(blc_y,trc_y);

			double new_center_x, new_center_y = cur_center_y;
			if ( x_units == "pixel" ) {
				try {
					double dummy;
					pixel_to_linear( wc_, atof(x.c_str( )), 0, new_center_x, dummy );
				} catch( const casa::AipsError &err) {
					status( "coordinate conversion failed: " + err.getMesg( ), "error" );
					return false;
				} catch( const std::exception &exc ) {
					status( std::string("coordinate conversion failed: ") + exc.what( ), "error" );
					return false;;
				} catch(...) {
					status( "coordinate conversion failed...", "error" );
					return false;
				}
			} else {
				Quantity xq;
				if ( x_units == "sexagesimal" ) {
					// read in to convert HMS/DMS...
					MVAngle::read(xq,std::for_each(x.begin(),x.end(),strip_white_space(x.size())));
				} else if ( x_units == "degrees" ) {
					xq = Quantity( atof(x.c_str( )), "deg" );
				} else if ( x_units == "radians" ) {
					xq = Quantity( atof(x.c_str( )), "rad" );
				} else {
					status( "unknown units: " + x_units, "error" );
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}

				MDirection::Types cccs = current_casa_coordsys( );
				MDirection::Types input_direction = string_to_casa_coordsys(coordsys);
				//		const DisplayCoordinateSystem &cs = wc_->coordinateSystem( );

				Vector<Double> worldv(2);
				Vector<Double> linearv(2);
				linearv(0) = cur_center_x;
				linearv(1) = cur_center_y;
				if ( ! wc_->linToWorld( worldv, linearv ) ) {
					status( "linear to world coordinate converison failed...", "error" );
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}


				if ( cccs != input_direction ) {
					// convert current center to input direction type...
					MDirection cur_center( Quantum<Vector<Double> > (worldv,"rad"), cccs );
					MDirection input_center = MDirection::Convert(cur_center,input_direction)( );
					Quantum<Vector<Double> > inputq = input_center.getAngle("rad");
					// get quantity in input direction as radians...
					inputq.getValue( )(0) = xq.getValue("rad");
					// convert new position (in input direction type) current direction type...
					MDirection new_center_input( inputq, input_direction );
					MDirection new_center = MDirection::Convert(new_center_input,cccs)( );
					// retrieve world position in the current direction type...
					Quantum<Vector<Double> > new_center_q = new_center.getAngle("rad");
					worldv = new_center_q.getValue("rad");
				} else {
					worldv(0) = xq.getValue("rad");
				}

				if ( ! wc_->worldToLin( linearv, worldv ) ) {
					status( "world to linear coordinate converison failed...", "error" );
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}
				new_center_x = linearv[0];
				new_center_y = linearv[1];
			}


			// trap attempts to move region out of visible area...
			if ( ! valid_translation( new_center_x - cur_center_x, new_center_y - cur_center_y, 0, 0 ) ) {
				status( "translation moves region outside of image...", "error" );
				updateStateInfo( true, region::RegionChangeReset );	// error: reset
				return false;
			}

			// move region...
			move( new_center_x - cur_center_x, new_center_y - cur_center_y );

			return true;
		}

		bool Region::translateY( const std::string &y, const std::string &y_units, const std::string &coordsys ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return false;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			// current center of region...
			double cur_center_x = linear_average(blc_x,trc_x);
			double cur_center_y = linear_average(blc_y,trc_y);

			double new_center_x=cur_center_x, new_center_y;
			if ( y_units == "pixel" ) {
				try {
					double dummy;
					pixel_to_linear( wc_, 0, atof(y.c_str( )), dummy, new_center_y );
				} catch( const casa::AipsError &err) {
					status( "coordinate conversion failed: " + err.getMesg( ), "error" );
					return false;
				} catch( const std::exception &exc ) {
					status( std::string("coordinate conversion failed: ") + exc.what( ), "error" );
					return false;;
				} catch(...) {
					status( "coordinate conversion failed...", "error" );
					return false;
				}
			} else {
				Quantity yq;
				if ( y_units == "sexagesimal" ) {
					// read in to convert HMS/DMS...
					MVAngle::read( yq, std::for_each(y.begin(),y.end(),strip_white_space(y.size())) );
				} else if ( y_units == "degrees" ) {
					yq = Quantity( atof(y.c_str( )), "deg" );
				} else if ( y_units == "radians" ) {
					yq = Quantity( atof(y.c_str( )), "rad" );
				} else {
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}

				MDirection::Types cccs = current_casa_coordsys( );
				MDirection::Types input_direction = string_to_casa_coordsys(coordsys);

				Vector<Double> worldv(2);
				Vector<Double> linearv(2);
				linearv(0) = cur_center_x;
				linearv(1) = cur_center_y;
				if ( ! wc_->linToWorld( worldv, linearv ) ) {
					status( "linear to world coordinate converison failed...", "error" );
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}

				if ( cccs != input_direction ) {
					// convert current center to input direction type...
					MDirection cur_center( Quantum<Vector<Double> > (worldv,"rad"), cccs );
					MDirection input_center = MDirection::Convert(cur_center,input_direction)( );
					Quantum<Vector<Double> > inputq = input_center.getAngle("rad");
					// get quantity in input direction as radians...
					inputq.getValue( )(1) = yq.getValue("rad");
					// convert new position (in input direction type) current direction type...
					MDirection new_center_input( inputq, input_direction );
					MDirection new_center = MDirection::Convert(new_center_input,cccs)( );
					// retrieve world position in the current direction type...
					Quantum<Vector<Double> > new_center_q = new_center.getAngle("rad");
					worldv = new_center_q.getValue("rad");
				} else {
					worldv(1) = yq.getValue("rad");
				}

				if ( ! wc_->worldToLin( linearv, worldv ) ) {
					status( "world to linear coordinate converison failed...", "error" );
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}
				new_center_x = linearv[0];
				new_center_y = linearv[1];
			}

			// trap attempts to move region out of visible area...
			if ( ! valid_translation( new_center_x - cur_center_x, new_center_y - cur_center_y, 0, 0 ) ) {
				status( "translation moves region outside of image...", "error" );
				updateStateInfo( true, region::RegionChangeReset );	// error: reset
				return false;
			}

			// move region...
			move( new_center_x - cur_center_x, new_center_y - cur_center_y );

			return true;
		}

		bool Region::resizeX( const std::string &x, const std::string &x_units, const std::string &coordsys ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return false;

			double cur_blc_x, cur_blc_y, cur_trc_x, cur_trc_y;
			boundingRectangle( cur_blc_x, cur_blc_y, cur_trc_x, cur_trc_y );

			if ( x_units == "pix" ) {
				double pix_blc_x, pix_blc_y, pix_trc_x, pix_trc_y;
				double new_blc_x, new_trc_x;
				double new_blc_y, new_trc_y;
				try {
					linear_to_pixel( wc_, cur_blc_x, cur_blc_y, cur_trc_x, cur_trc_y, pix_blc_x, pix_blc_y, pix_trc_x, pix_trc_y );
				} catch(...) {
					return false;
				}
				double cur_pix_distance = pix_trc_x - pix_blc_x;
				double new_pix_distance = atof(x.c_str( ));
				double shift = (new_pix_distance - cur_pix_distance) / 2.0;
				try {
					pixel_to_linear( wc_, pix_blc_x - shift, pix_blc_y, pix_trc_x + shift, pix_trc_y,
					                 new_blc_x, new_blc_y, new_trc_x, new_trc_y );
				} catch(...) {
					return false;
				}
				resize( (new_trc_x - new_blc_x) - (cur_trc_x - cur_blc_x), 0 );

			} else {

				Quantity new_distance;
				const char degstr[] = { 0x00B0, 0x0000 };
				if ( x_units == "\"" ) {
					new_distance = Quantity( atof(x.c_str( )), "arcsec" );
				} else if ( x_units == "\'" ) {
					new_distance = Quantity( atof(x.c_str( )), "arcmin" );
				} else if ( x_units == "rad" ) {
					new_distance = Quantity( atof(x.c_str( )), "rad" );
				} else if ( x_units == degstr ) {
					new_distance = Quantity( atof(x.c_str( )), "deg" );
				} else {
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}

				MDirection::Types cccs = current_casa_coordsys( );
				MDirection::Types input_direction = string_to_casa_coordsys(coordsys);

				Vector<Double> cur_blc_worldv(2);
				Vector<Double> cur_blc_linearv(2);
				Vector<Double> cur_trc_worldv(2);
				Vector<Double> cur_trc_linearv(2);

				/**** use the midpoint for other dimension ****/
				cur_blc_linearv(0) = cur_blc_x;
				cur_blc_linearv(1) = (cur_trc_y + cur_blc_y) / 2.0;
				cur_trc_linearv(0) = cur_trc_x;
				cur_trc_linearv(1) = cur_blc_linearv(1);
				if ( ! wc_->linToWorld( cur_blc_worldv, cur_blc_linearv ) ||
				        ! wc_->linToWorld( cur_trc_worldv, cur_trc_linearv ) ) {
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}

				MDirection cur_blc( Quantum<Vector<Double> > (cur_blc_worldv,"rad"), cccs );
				MDirection cur_trc( Quantum<Vector<Double> > (cur_trc_worldv,"rad"), cccs );

				// convert current blc/trc corners to input direction type...
				MDirection input_blc;
				MDirection input_trc;
				if ( cccs != input_direction ) {
					input_blc = MDirection::Convert(cur_blc,input_direction)( );
					input_trc = MDirection::Convert(cur_trc,input_direction)( );
				} else {
					input_blc = cur_blc;
					input_trc = cur_trc;
				}

				double original_distance = input_blc.getValue( ).separation(input_trc.getValue( ));
				double shift = (new_distance.getValue("rad") - original_distance) / 2.0;
				MDirection output_blc(input_blc);
				MDirection output_trc(input_trc);
				if ( input_blc.getAngle( ).getValue("rad")[0] < input_trc.getAngle( ).getValue("rad")[0] ) {
					output_blc.shiftLongitude(-shift);
					output_trc.shiftLongitude(shift);
				} else {
					output_blc.shiftLongitude(shift);
					output_trc.shiftLongitude(-shift);
				}

				Vector<Double> out_blc_worldv(2);
				Vector<Double> out_blc_linearv(2);
				Vector<Double> out_trc_worldv(2);
				Vector<Double> out_trc_linearv(2);
				out_blc_worldv(0) = output_blc.getAngle( ).getValue("rad")[0];
				out_blc_worldv(1) = output_blc.getAngle( ).getValue("rad")[1];
				out_trc_worldv(0) = output_trc.getAngle( ).getValue("rad")[0];
				out_trc_worldv(1) = output_trc.getAngle( ).getValue("rad")[1];
				if ( ! wc_->worldToLin( out_blc_linearv, out_blc_worldv ) ||
				        ! wc_->worldToLin( out_trc_linearv, out_trc_worldv ) ) {
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}

				resize( (out_trc_linearv[0] - out_blc_linearv[0]) - (cur_trc_x - cur_blc_x), 0 );
			}

			refresh( );
			return true;
		}

		bool Region::resizeY( const std::string &y, const std::string &y_units, const std::string &coordsys ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return false;

			double cur_blc_x, cur_blc_y, cur_trc_x, cur_trc_y;
			boundingRectangle( cur_blc_x, cur_blc_y, cur_trc_x, cur_trc_y );

			if ( y_units == "pix" ) {
				double pix_blc_x, pix_blc_y, pix_trc_x, pix_trc_y;
				double new_blc_x, new_trc_x;
				double new_blc_y, new_trc_y;
				try {
					linear_to_pixel( wc_, cur_blc_x, cur_blc_y, cur_trc_x, cur_trc_y, pix_blc_x, pix_blc_y, pix_trc_x, pix_trc_y );
				} catch(...) {
					return false;
				}
				double cur_pix_distance = pix_trc_y - pix_blc_y;
				double new_pix_distance = atof(y.c_str( ));
				double shift = (new_pix_distance - cur_pix_distance) / 2.0;
				try {
					pixel_to_linear( wc_, pix_blc_x, pix_blc_y - shift, pix_trc_x, pix_trc_y + shift,
					                 new_blc_x, new_blc_y, new_trc_x, new_trc_y );
				} catch(...) {
					return false;
				}
				resize( 0, (new_trc_y - new_blc_y) - (cur_trc_y - cur_blc_y) );

			} else {

				Quantity new_distance;
				const char degstr[] = { 0x00B0, 0x0000 };
				if ( y_units == "\"" ) {
					new_distance = Quantity( atof(y.c_str( )), "arcsec" );
				} else if ( y_units == "\'" ) {
					new_distance = Quantity( atof(y.c_str( )), "arcmin" );
				} else if ( y_units == "rad" ) {
					new_distance = Quantity( atof(y.c_str( )), "rad" );
				} else if ( y_units == degstr ) {
					new_distance = Quantity( atof(y.c_str( )), "deg" );
				} else {
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}

				MDirection::Types cccs = current_casa_coordsys( );
				MDirection::Types input_direction = string_to_casa_coordsys(coordsys);

				Vector<Double> cur_blc_worldv(2);
				Vector<Double> cur_blc_linearv(2);
				Vector<Double> cur_trc_worldv(2);
				Vector<Double> cur_trc_linearv(2);

				/**** use the midpoint for other dimension ****/
				cur_blc_linearv(0) = (cur_trc_x + cur_blc_x) / 2.0;
				cur_blc_linearv(1) = cur_blc_y;
				cur_trc_linearv(0) = cur_blc_linearv(0);
				cur_trc_linearv(1) = cur_trc_y;
				if ( ! wc_->linToWorld( cur_blc_worldv, cur_blc_linearv ) ||
				        ! wc_->linToWorld( cur_trc_worldv, cur_trc_linearv ) ) {
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}

				MDirection cur_blc( Quantum<Vector<Double> > (cur_blc_worldv,"rad"), cccs );
				MDirection cur_trc( Quantum<Vector<Double> > (cur_trc_worldv,"rad"), cccs );

				// convert current blc/trc corners to input direction type...
				MDirection input_blc;
				MDirection input_trc;
				if ( cccs != input_direction ) {
					input_blc = MDirection::Convert(cur_blc,input_direction)( );
					input_trc = MDirection::Convert(cur_trc,input_direction)( );
				} else {
					input_blc = cur_blc;
					input_trc = cur_trc;
				}

				double original_distance = input_blc.getValue( ).separation(input_trc.getValue( ));
				double shift = (new_distance.getValue("rad") - original_distance) / 2.0;
				MDirection output_blc(input_blc);
				MDirection output_trc(input_trc);
				if ( input_blc.getAngle( ).getValue("rad")[1] < input_trc.getAngle( ).getValue("rad")[1] ) {
					output_blc.shiftLatitude(-shift);
					output_trc.shiftLatitude(shift);
				} else {
					output_blc.shiftLatitude(shift);
					output_trc.shiftLatitude(-shift);
				}

				Vector<Double> out_blc_worldv(2);
				Vector<Double> out_blc_linearv(2);
				Vector<Double> out_trc_worldv(2);
				Vector<Double> out_trc_linearv(2);
				out_blc_worldv(0) = output_blc.getAngle( ).getValue("rad")[0];
				out_blc_worldv(1) = output_blc.getAngle( ).getValue("rad")[1];
				out_trc_worldv(0) = output_trc.getAngle( ).getValue("rad")[0];
				out_trc_worldv(1) = output_trc.getAngle( ).getValue("rad")[1];
				if ( ! wc_->worldToLin( out_blc_linearv, out_blc_worldv ) ||
				        ! wc_->worldToLin( out_trc_linearv, out_trc_worldv ) ) {
					updateStateInfo( true, region::RegionChangeReset );	// error: reset
					return false;
				}

				resize( 0, (out_trc_linearv[1] - out_blc_linearv[1]) - (cur_trc_y - cur_blc_y) );
			}

			refresh( );
			return true;
		}

		region::Coord Region::current_region_coordsys( ) const {
			return casa_to_viewer(current_casa_coordsys( ));
		}

		MDirection::Types Region::current_casa_coordsys( ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return MDirection::J2000;

			const DisplayCoordinateSystem &cs = wc_->coordinateSystem( );
			int index = cs.findCoordinate(Coordinate::DIRECTION);
			if ( index < 0 ) {
				// no direction coordinate...
				return MDirection::N_Types;
			}
			return cs.directionCoordinate(index).directionType(true);
		}

		region::Units Region::current_xunits( ) const {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return region::Degrees;
			const Vector<String> &units = wc_->worldAxisUnits();
			if ( units(0) == "rad" )
				return region::Radians;
			if ( units(0) == "deg" )
				return region::Degrees;
			return region::Degrees;
		}

		region::Units Region::current_yunits( ) const {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return region::Degrees;
			const Vector<String> &units = wc_->worldAxisUnits();
			if ( units(1) == "rad" )
				return region::Radians;
			if ( units(1) == "deg" )
				return region::Degrees;
			return region::Degrees;
		}


		void Region::set_line_style( const ls_ele &val ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
			PixelCanvas *pc = wc_->pixelCanvas();
			if ( pc == 0 ) return;
			if ( val.second >= 0 ) pc->setLineWidth(val.second);
			switch ( val.first ) {
			case region::DashLine:
				pc->setLineStyle( Display::LSDashed );
				current_ls = region::DashLine;
				break;
			case region::DotLine: {
				QtPixelCanvas* qpc = dynamic_cast<QtPixelCanvas*>(pc);
				if(qpc != NULL) {
					qpc->setQtLineStyle(Qt::DotLine);
					current_ls = region::DotLine;
				} else {
					pc->setLineStyle( Display::LSDashed );
					current_ls = region::DashLine;
				}
			}
			break;
			case region::LSDoubleDashed:
				pc->setLineStyle(Display::LSDoubleDashed );
				current_ls = region::DashLine;
				break;
			default:
				pc->setLineStyle(Display::LSSolid);
				current_ls = region::SolidLine;
				break;
			}
		}

		void linear_to_screen( WorldCanvas *wc, double lin_x1, double lin_y1, int &scr_x1, int &scr_y1 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
			static Vector<Double> linearv(2);

			// BEGIN - critical section
			linearv(0) = lin_x1;
			linearv(1) = lin_y1;
			if ( ! wc->linToPix( pixelv, linearv ) )
				throw internal_error( "linear to screen conversion failed: " + wc->errorMessage( ) );
			scr_x1 = static_cast<int>(pixelv(0));
			scr_y1 = static_cast<int>(pixelv(1));
			// END - critical section
		}

		void linear_to_screen( WorldCanvas *wc, double lin_x1, double lin_y1, double lin_x2, double lin_y2,
		                       int &scr_x1, int &scr_y1, int &scr_x2, int &scr_y2 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
			static Vector<Double> linearv(2);

			// BEGIN - critical section
			linearv(0) = lin_x1;
			linearv(1) = lin_y1;
			if ( ! wc->linToPix( pixelv, linearv ) )
				throw internal_error( "linear to screen conversion failed: " + wc->errorMessage( ) );
			scr_x1 = static_cast<int>(pixelv(0));
			scr_y1 = static_cast<int>(pixelv(1));

			linearv(0) = lin_x2;
			linearv(1) = lin_y2;
			if ( ! wc->linToPix( pixelv, linearv ) )
				throw internal_error( "linear to screen conversion failed: " + wc->errorMessage( ) );
			scr_x2 = static_cast<int>(pixelv(0));
			scr_y2 = static_cast<int>(pixelv(1));
			// END - critical section
		}

		void linear_to_pixel( WorldCanvas *wc, double lin_x1, double lin_y1, double &pix_x1, double &pix_y1 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			double world_x1, world_y1;
			linear_to_world( wc, lin_x1, lin_y1, world_x1, world_y1 );

			const DisplayCoordinateSystem &cs = wc->coordinateSystem( );

			static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
			static Vector<Double> worldv(2);

			// BEGIN - critical section
			if ( cs.nWorldAxes( ) != worldv.nelements( ) )
				worldv.resize(cs.nWorldAxes( ));

			worldv = cs.referenceValue( );

			if ( cs.nPixelAxes( ) != pixelv.nelements( ) )
				pixelv.resize(cs.nPixelAxes( ));

			pixelv = cs.referencePixel( );
			worldv(0) = world_x1;
			worldv(1) = world_y1;

			if ( ! cs.toPixel( pixelv, worldv ) )
					throw internal_error( "linear to pixel conversion failed: " + cs.errorMessage( ) );

			pix_x1 = pixelv(0);
			pix_y1 = pixelv(1);
			// END - critical section
		}

		void linear_to_pixel( WorldCanvas *wc, double lin_x1, double lin_y1, double lin_x2, double lin_y2,
		                      double &pix_x1, double &pix_y1, double &pix_x2, double &pix_y2 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			double world_x1, world_y1, world_x2, world_y2;
			linear_to_world( wc, lin_x1, lin_y1, lin_x2, lin_y2, world_x1, world_y1, world_x2, world_y2 );

			static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
			static Vector<Double> worldv(2);

			const DisplayCoordinateSystem &cs = wc->coordinateSystem( );

			// BEGIN - critical section

			if ( cs.nWorldAxes( ) != worldv.nelements( ) )
				worldv.resize(cs.nWorldAxes( ));

			worldv = cs.referenceValue( );


			if ( cs.nPixelAxes( ) != pixelv.nelements( ) )
				pixelv.resize(cs.nPixelAxes( ));

			pixelv = cs.referencePixel( );

			worldv(0) = world_x1;
			worldv(1) = world_y1;

			if ( ! cs.toPixel( pixelv, worldv ) )
				throw internal_error( "linear to pixel conversion failed: " + cs.errorMessage( ) );

			pix_x1 = pixelv(0);
			pix_y1 = pixelv(1);

			worldv(0) = world_x2;
			worldv(1) = world_y2;
			if ( ! cs.toPixel( pixelv, worldv ) )
				throw internal_error( "linear to pixel conversion failed: " + cs.errorMessage( ) );

			pix_x2 = pixelv(0);
			pix_y2 = pixelv(1);
			// END - critical section
		}

		void screen_to_linear( WorldCanvas *wc, int scr_x, int scr_y, double &lin_x, double &lin_y ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			// BEGIN - critical section
			static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
			static Vector<Double> linearv(2);
			pixelv(0) = scr_x;
			pixelv(1) = scr_y;
			if ( ! wc->pixToLin( linearv, pixelv ) )
				throw internal_error( "pixel to linear conversion failed: " + wc->errorMessage( ) );
			lin_x = linearv(0);
			lin_y = linearv(1);
			// END - critical section
		}

		void screen_to_linear( WorldCanvas *wc, int scr_x1, int scr_y1, int scr_x2, int scr_y2,
		                       double &lin_x1, double &lin_y1, double &lin_x2, double &lin_y2 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
			static Vector<Double> linearv(2);

			// BEGIN - critical section
			pixelv(0) = scr_x1;
			pixelv(1) = scr_y1;
			if ( ! wc->pixToLin( linearv, pixelv ) )
				throw internal_error( "pixel to linear conversion failed: " + wc->errorMessage( ) );

			lin_x1 = linearv(0);
			lin_y1 = linearv(1);

			pixelv(0) = scr_x2;
			pixelv(1) = scr_y2;
			if ( ! wc->pixToLin( linearv, pixelv ) )
				throw internal_error( "pixel to linear conversion failed: " + wc->errorMessage( ) );
			lin_x2 = linearv(0);
			lin_y2 = linearv(1);
			// END - critical section
		}

		void screen_offset_to_linear_offset( WorldCanvas *wc_, int sx, int sy, double &lx, double &ly ) {
			const int base = 200;
			double blcx, blcy, trcx, trcy;
			screen_to_linear( wc_, base, base, base+sx, base+sy, blcx, blcy, trcx, trcy );
			lx = trcx - blcx;
			ly = trcy - blcy;
		}

		void pixel_offset_to_linear_offset( WorldCanvas *wc, double sx, double sy, double &lx, double &ly ) {
			const int base = 10;
			double blcx, blcy, trcx, trcy;
			pixel_to_linear( wc, base, base, base+sx, base+sy, blcx, blcy, trcx, trcy );
			lx = trcx - blcx;
			ly = trcy - blcy;
			return;
		}

		void linear_offset_to_pixel_offset( WorldCanvas *wc_, double lx, double ly, double &px, double &py ) {
			const int base = 0;
			double blcx, blcy, trcx, trcy;

			linear_to_pixel( wc_, base, base, base+lx, base+ly, blcx, blcy, trcx, trcy );
			px = trcx - blcx;
			py = trcy - blcy;
		}

		void linear_offset_to_world_offset( WorldCanvas *wc_, double lx, double ly, MDirection::Types coordsys, const std::string &units, double &wx, double &wy ) {
			const int base = 0;
			double blcx, blcy, trcx, trcy;

			linear_to_world( wc_, base, base, base+lx, base+ly, blcx, blcy, trcx, trcy );
			MDirection::Types cccs = get_coordinate_type( wc_->coordinateSystem( ) );
			//	    region::Coord crcs = casa_to_viewer(cccs);
			if ( coordsys == cccs && units == "rad" ) {
				wx = fabs(trcx-blcx);
				wy = fabs(trcy-blcy);
				return;
			}

			Quantum<Vector<double> > blc, trc;
			if ( coordsys == cccs ) {
				const Vector<String> &units = wc_->worldAxisUnits();
				Vector<double> pts(2);
				pts[0] = blcx;
				pts[1] = blcy;
				blc = Quantum<Vector<double> >(pts,units[0]);
				pts[0] = trcx;
				pts[1] = trcy;
				trc = Quantum<Vector<double> >(pts,units[1]);
			} else {
				const Vector<String> &units = wc_->worldAxisUnits();
				blc = Region::convert_angle( blcx, units[0], blcy, units[1], cccs, coordsys );
				trc = Region::convert_angle( trcx, units[0], trcy, units[1], cccs, coordsys );
			}

			wx = fabs(trc.getValue(units.c_str( ))(0) - blc.getValue(units.c_str( ))(0));
			wy = fabs(trc.getValue(units.c_str( ))(1) - blc.getValue(units.c_str( ))(1));
		}


		void world_offset_to_linear_offset( WorldCanvas *wc_, MDirection::Types coordsys, const std::string &units, double wx, double wy, double &lx, double &ly ) {

			double basex = 0;
			double basey = 0;

			MDirection::Types cccs = get_coordinate_type( wc_->coordinateSystem( ) );
			if ( coordsys == cccs && units == "rad" ) {
				double blcx, blcy, trcx, trcy;
				world_to_linear( wc_, basex, basey, basex+wx, basey+wy, blcx, blcy, trcx, trcy );
				lx = fabs(trcx-blcx);
				ly = fabs(trcy-blcy);
				return;
			}

			// create blc, trc from offset...
			Quantum<Vector<double> > blc, trc;

			// start with offset in specified units...
			Quantity xoff(wx,units.c_str( ));
			Quantity yoff(wy,units.c_str( ));

			// create blc and trc in radians (with converted offsets)...
			Vector<double> pts(2);
			pts[0] = basex;
			pts[1] = basey;
			blc = Quantum<Vector<double> >(pts,"rad");
			pts[0] = basex + xoff.getValue("rad");
			pts[1] = basey + yoff.getValue("rad");
			trc = Quantum<Vector<double> >(pts,"rad");

			// convert from the specified coordinate system to the viewer's coordinate system...
			if ( coordsys != cccs ) {
				blc = Region::convert_angle( blc.getValue("rad")(0), "rad", blc.getValue("rad")(1), "rad", coordsys, cccs );
				trc = Region::convert_angle( trc.getValue("rad")(0), "rad", trc.getValue("rad")(1), "rad", coordsys, cccs );
			}

			double blcx, blcy, trcx, trcy;
			world_to_linear( wc_, blc.getValue("rad")(0), blc.getValue("rad")(1), trc.getValue("rad")(0), trc.getValue("rad")(1), blcx, blcy, trcx, trcy );
			lx = fabs(trcx-blcx);
			ly = fabs(trcy-blcy);

		}


		void linear_to_world( WorldCanvas *wc, double lin_x, double lin_y, double &world_x, double &world_y ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			// BEGIN - critical section
			static Vector<Double> linearv(2);	// avoid vector allocation for each conversion
			static Vector<Double> worldv(2);
			linearv(0) = lin_x;
			linearv(1) = lin_y;
			if ( ! wc->linToWorld( worldv, linearv ) )
				throw internal_error( "linear to world conversion failed: " + wc->errorMessage( ) );
			world_x = worldv(0);
			world_y = worldv(1);
			// END - critical section
		}

		void linear_to_world( WorldCanvas *wc, double lin_x1, double lin_y1, double lin_x2, double lin_y2,
		                      double &world_x1, double &world_y1, double &world_x2, double &world_y2 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			static Vector<Double> linearv(2);	// avoid vector allocation for each conversion
			static Vector<Double> worldv(2);

			// BEGIN - critical section
			linearv(0) = lin_x1;
			linearv(1) = lin_y1;
			if ( ! wc->linToWorld( worldv, linearv ) )
				throw internal_error( "linear to world conversion failed: " + wc->errorMessage( ) );

			world_x1 = worldv(0);
			world_y1 = worldv(1);

			linearv(0) = lin_x2;
			linearv(1) = lin_y2;
			if ( ! wc->linToWorld( worldv, linearv ) )
				throw internal_error( "linear to world conversion failed: " + wc->errorMessage( ) );
			world_x2 = worldv(0);
			world_y2 = worldv(1);
			// END - critical section
		}

		void world_to_linear( WorldCanvas *wc, double world_x, double world_y, double &lin_x, double &lin_y ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			static Vector<Double> worldv(2);	// avoid vector allocation for each conversion
			static Vector<Double> linearv(2);

			// BEGIN - critical section

			// if ( cs.nWorldAxes( ) != worldv.nelements( ) ) {
			//	worldv.resize(cs.nWorldAxes( ));
			//	worldv = cs.referenceValue( );
			// }

			worldv(0) = world_x;
			worldv(1) = world_y;

			if ( ! wc->worldToLin( linearv, worldv ) )
				throw internal_error( "world to linear conversion failed: " + wc->errorMessage( ) );
			lin_x = linearv(0);
			lin_y = linearv(1);

			// END - critical section
		}

		void world_to_linear( WorldCanvas *wc, double world_x1, double world_y1, double world_x2, double world_y2,
		                      double &lin_x1, double &lin_y1, double &lin_x2, double &lin_y2 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			// BEGIN - critical section
			static Vector<Double> worldv(2);	// avoid vector allocation for each conversion
			static Vector<Double> linearv(2);
			worldv(0) = world_x1;
			worldv(1) = world_y1;
			if ( ! wc->worldToLin( linearv, worldv ) )
				throw internal_error( std::string("world to linear conversion failed: ") + wc->errorMessage( ) );
			lin_x1 = linearv(0);
			lin_y1 = linearv(1);

			worldv(0) = world_x2;
			worldv(1) = world_y2;
			if ( ! wc->worldToLin( linearv, worldv ) )
				throw internal_error( "world to linear conversion failed: " + wc->errorMessage( ) );
			lin_x2 = linearv(0);
			lin_y2 = linearv(1);
			// END - critical section
		}

		void pixel_to_world( WorldCanvas *wc, double pix_x1, double pix_y1, double &world_x1, double &world_y1 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			const DisplayCoordinateSystem &cs = wc->coordinateSystem( );

			static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
			static Vector<Double> worldv(2);

			// BEGIN - critical section
			if ( cs.nWorldAxes( ) != worldv.nelements( ) )
				worldv.resize(cs.nWorldAxes( ));

			worldv = cs.referenceValue( );

			if ( cs.nPixelAxes( ) != pixelv.nelements( ) )
				pixelv.resize(cs.nPixelAxes( ));

			pixelv = cs.referencePixel( );

			pixelv(0) = pix_x1;
			pixelv(1) = pix_y1;

			if ( ! cs.toWorld( worldv, pixelv ) )
				throw internal_error( "pixel to world conversion failed: " + cs.errorMessage( ) );

			world_x1 = worldv(0);
			world_y1 = worldv(1);
			// END - critical section
		}

		void pixel_to_world( WorldCanvas *wc, double pix_x1, double pix_y1, double pix_x2, double pix_y2,
		                     double &world_x1, double &world_y1, double &world_x2, double &world_y2 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			const DisplayCoordinateSystem &cs = wc->coordinateSystem( );

			static Vector<Double> pixelv(2);	// avoid vector allocation for each conversion
			static Vector<Double> worldv(2);

			// BEGIN - critical section
			if ( cs.nWorldAxes( ) != worldv.nelements( ) )
				worldv.resize(cs.nWorldAxes( ));

			worldv = cs.referenceValue( );

			if ( cs.nPixelAxes( ) != pixelv.nelements( ) )
				pixelv.resize(cs.nPixelAxes( ));

			pixelv = cs.referencePixel( );

			pixelv(0) = pix_x1;
			pixelv(1) = pix_y1;

			if ( ! cs.toWorld( worldv, pixelv ) )
				throw internal_error( "pixel to world conversion failed: " + cs.errorMessage( ) );

			world_x1 = worldv(0);
			world_y1 = worldv(1);


			pixelv(0) = pix_x2;
			pixelv(1) = pix_y2;

			if ( ! cs.toWorld( worldv, pixelv ) )
				throw internal_error( "pixel to world conversion failed: " + cs.errorMessage( ) );

			world_x2 = worldv(0);
			world_y2 = worldv(1);
			// END - critical section
		}

		void pixel_to_linear( WorldCanvas *wc, double pix_x1, double pix_y1, double &lin_x1, double &lin_y1 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			double world_x1, world_y1;
			pixel_to_world( wc, pix_x1, pix_y1, world_x1, world_y1 );
			world_to_linear( wc, world_x1, world_y1, lin_x1, lin_y1 );
		}

		void pixel_to_linear( WorldCanvas *wc, double pix_x1, double pix_y1, double pix_x2, double pix_y2,
		                      double &lin_x1, double &lin_y1, double &lin_x2, double &lin_y2 ) {

			if ( wc == 0 )
				throw internal_error( "coordinate transformation without coordinate system" );

			double world_x1, world_y1;
			pixel_to_world( wc, pix_x1, pix_y1, world_x1, world_y1 );
			world_to_linear( wc, world_x1, world_y1, lin_x1, lin_y1 );

			pixel_to_world( wc, pix_x2, pix_y2, world_x1, world_y1 );
			world_to_linear( wc, world_x1, world_y1, lin_x2, lin_y2 );
		}


		void world_to_screen( WorldCanvas *wc, double world_x1, double world_y1, int &screen_x1, int &screen_y1  ) {
			double lin_x1, lin_y1;
			world_to_linear( wc, world_x1, world_y1, lin_x1, lin_y1 );
			linear_to_screen( wc, lin_x1, lin_y1, screen_x1, screen_y1 );
		}

		void world_to_screen( WorldCanvas *wc, double world_x1, double world_y1, double world_x2, double world_y2,
		                      int &screen_x1, int &screen_y1, int &screen_x2, int &screen_y2 ) {
			double lin_x1, lin_y1, lin_x2, lin_y2;
			world_to_linear( wc, world_x1, world_y1, world_x2, world_y2, lin_x1, lin_y1, lin_x2, lin_y2 );
			linear_to_screen( wc, lin_x1, lin_y1, lin_x2, lin_y2, screen_x1, screen_y1, screen_x2, screen_y2 );
		}

		void world_to_pixel( WorldCanvas *wc, double world_x1, double world_y1, double &pixel_x1, double &pixel_y1  ) {
			double lin_x1, lin_y1;
			world_to_linear( wc, world_x1, world_y1, lin_x1, lin_y1 );
			linear_to_pixel( wc, lin_x1, lin_y1, pixel_x1, pixel_y1 );
		}

		void world_to_pixel( WorldCanvas *wc, double world_x1, double world_y1, double world_x2, double world_y2,
		                      double &pixel_x1, double &pixel_y1, double &pixel_x2, double &pixel_y2 ) {
			double lin_x1, lin_y1, lin_x2, lin_y2;
			world_to_linear( wc, world_x1, world_y1, world_x2, world_y2, lin_x1, lin_y1, lin_x2, lin_y2 );
			linear_to_pixel( wc, lin_x1, lin_y1, lin_x2, lin_y2, pixel_x1, pixel_y1, pixel_x2, pixel_y2 );
		}

		MDirection::Types get_coordinate_type( const DisplayCoordinateSystem &cs ) {
			for ( uInt i=0; i < cs.nCoordinates(); ++i )
				if ( cs.type(i) == Coordinate::DIRECTION )
					return cs.directionCoordinate(i).directionType(true);
			return MDirection::EXTRA;
		}

		RegionInfo::center_t *Region::getLayerCenter( PrincipalAxesDD *padd, std::tr1::shared_ptr<ImageInterface<Float> > image, ImageRegion& imgReg) {
			if( image==0 || padd == 0 ) return 0;
			try {
				// store the coordinate system and the axis names
				const DisplayCoordinateSystem& cs = image->coordinates();
				Vector<String> nm	     = cs.worldAxisNames();
				Vector<String> axesNames   = padd->worldToPixelAxisNames( cs );


				// get all info on the hidden axis
				Int _axis_h_ = image->ndim() > 3 ? padd->xlatePixelAxes(3) : -1;		// get first "hidden axis
				String haxis = _axis_h_ >= 0 ? axesNames(_axis_h_) : "";
				// uiBase( ) sets zero/one based:
				Int hIndex = _axis_h_ >= 0 ? padd->xlateFixedPixelAxes(_axis_h_) + padd->uiBase( ) : -1;

				// get all info on the z-axis
				String zUnit, zspKey, zspVal;
				zUnit = padd->spectralunitStr( );
				String xaxis = padd->xaxisStr( );
				String yaxis = padd->yaxisStr( );
				String zaxis = padd->zaxisStr( );
				Int zIndex = padd->activeZIndex();

				Int xPos = -1;
				Int yPos = -1;
				Int zPos = -1;
				Int hPos = -1;
				// identify the display axes in the
				// coordinate systems of the image
				for (uInt k = 0; k < nm.nelements(); k++) {
					if (nm(k) == xaxis)
						xPos = k;
					if (nm(k) == yaxis)
						yPos = k;
					if (nm(k) == zaxis)
						zPos = k;
					if (nm(k) == haxis)
						hPos = k;
				}

				RegionInfo::center_t *layercenter = new RegionInfo::center_t( );

				String zLabel="";
				String hLabel="";
				Vector<Double> tPix,tWrld;
				tPix = cs.referencePixel();
				String tStr;
				if (zPos > -1) {
					tPix(zPos) = zIndex;//cout << " tPix: " << tPix<<endl;
					if (!cs.toWorld(tWrld,tPix)) {
					} else {//cout << " tWrld: " << tWrld<<endl;
						if (zUnit.length()>0) {
							zspKey = "Spectral_Vale";//cout << " zUnit: " << zUnit<<" tWrld(zPos): " << tWrld(zPos)<< " zPos: "<< zPos<<endl;
							// in case that the spectral axis is displayed on x or y,
							// the format() throws an exception and prevents the centering
							// copied from 'getLayerStats'.
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

				Record *rec = new Record(imgReg.toRecord(""));
				ImageFitter fitter(image, "", rec);
				Array<Float> medVals;

				// add a sky component to the fit
				if (skyComponent()) {
					// get a sky estimate via the median
					SubImage<Float> subImg(*image, imgReg);
					ImageStatistics<Float> stats(subImg, False);
					if (!stats.getConvertedStatistic(medVals, LatticeStatsBase::MEDIAN,True)) {
						//cout << "no idea" << endl;
						return 0;
					}
					if (medVals.size()>0)
						fitter.setZeroLevelEstimate(Double(medVals(IPosition(1,0))), False);
				}

				// do the fit
				ComponentList compList = fitter.fit();

				// fit that did not converge go back immediately
				if (!fitter.converged(0)) {
					layercenter->push_back(RegionInfo::center_t::value_type("Converged", "NO"));
					return layercenter;
				}

				Vector<Double> pVals;
				Vector<Quantity> pFlux;
				Vector<Double> pixCen;
				String errMsg;
				const uInt ncomponents=compList.nelements();
				for (uInt index=0; index<ncomponents; index++) {
					const ComponentShape *cShapeShape = compList.getShape(index);

					// the reference direction
					// contains as .getAngle() ra, dec in [rad]
					// and as .getRefString() the reference system,
					// which I think is as default equal to the image
					const MDirection mDir=compList.getRefDirection(index);
					Quantum< Vector< Double > >dirAngle=mDir.getAngle();

					// toRecord is the killer and contains
					// all information, including errors
					//skyComp.toRecord(errMsg, tabRecord);
					//cout << "TabRecord: "<< tabRecord << endl;

					Vector<Double> allpars;
					allpars=cShapeShape->parameters();
					uInt npars=cShapeShape->nParameters();
					if (npars > 2) {
						zspKey = "Ra_"+mDir.getRefString();
						//zspVal = MVTime(lat).string(MVTime::TIME, 9);
						zspVal = MVTime(Quantity((dirAngle.getValue())(0), dirAngle.getUnit())).string(MVTime::TIME, 9);
						layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

						//Quantity longitude = mDir.getValue().getLat("rad");
						zspKey = "Dec_"+mDir.getRefString();
						//zspVal = MVAngle(longitude).string(MVAngle::ANGLE_CLEAN, 8);
						zspVal = MVAngle(Quantity((dirAngle.getValue())(1), dirAngle.getUnit())).string(MVAngle::ANGLE_CLEAN, 8);
						layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

						// store major axis value in arcsecs
						zspKey = "W-Majorax";
						zspVal = String::toString(Quantity(allpars(0)/C::pi*180.0*3600.0, "arcsec"));
						layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

						// store minor axis value in arcsecs
						zspKey = "W-Minorax";
						zspVal = String::toString(Quantity(allpars(1)/C::pi*180.0*3600.0, "arcsec"));
						layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

						// store position angle in deg
						zspKey = "W-Posang";
						zspVal = String::toString(Quantity(allpars(2)*180.0/C::pi, "deg"));
						layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));
					}

					// get the pixel parameter values
					// for a Gauss, the vals are xcen, ycen, major_ax, minor_ax in [pix] and posang in [deg]
					pVals = cShapeShape->toPixel(image->coordinates().directionCoordinate(0));
					if (pVals.size()>4) {

						// make sure the x- and y-positions can be assigned correctly
						AlwaysAssert((xPos==0&&yPos==1)|| (xPos==1&&yPos==0), AipsError);

						// store the x-center
						zspKey = "Xcen";
						zspVal = String::toString(Quantity(pVals(xPos), "pix"));
						layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

						// store the y-center
						zspKey = "Ycen";
						zspVal = String::toString(Quantity(pVals(yPos), "pix"));
						layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

						// store major axis value in pix
						zspKey = "I-Majorax";
						zspVal = String::toString(Quantity(pVals(2), "pix"));
						layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

						// store minor axis value in pix
						zspKey = "I-Minorax";
						zspVal = String::toString(Quantity(pVals(3), "pix"));
						layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

						// store the position angle in [deg],
						// turn the angle if necessary
						Double posang;
						zspKey = "I-Posang";
						if (xPos==1 && yPos==0)
							posang=-1.0*pVals(4)*180.0/C::pi+90.0;
						else
							posang=pVals(4)*180.0/C::pi;
						while (posang <0.0)
							posang += 180.0;
						while (posang > 180.0)
							posang -= 180.0;
						zspVal = String::toString(Quantity(posang, "deg"));
						layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));


						// store the pixel values
						// for the center marking,
						// turn the angle if necessary
						pixCen.resize(3);
						pixCen(0) = pVals(xPos);
						pixCen(1) = pVals(yPos);
						if (xPos== 1 && yPos==0) {
							pixCen(2) = -1.0*pVals(4)+C::pi_2;
						} else {
							pixCen(2) = pVals(4);
						}
					}

					// the reference direction
					// contains as .getAngle() ra, dec in [rad]
					// and as .getRefString() the reference system,
					// which I think is as default equal to the image
					zspKey = "Radeg"; // would need some more digits??
					zspVal = String::toString(Quantity(((MVAngle(dirAngle.getValue()(0)))(0.5)).degree(),"deg"));
					layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

					zspKey = "Decdeg"; // would need some more digits??
					zspVal = String::toString(Quantity(MVAngle(dirAngle.getValue()(1)).degree(),"deg"));
					layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

					// get the integrated flux value
					compList.getFlux(pFlux, index);
					if (pFlux.size()>0) {

						// get the units of the image
						Unit imUnit=image->units();

						// get the peak flux from the integrated flux
						Quantity peakFlux=SkyCompRep::integralToPeakFlux(image->coordinates().directionCoordinate(0), ComponentType::GAUSSIAN, pFlux(0),
						                  imUnit, Quantity(allpars(0),"rad"), Quantity(allpars(1),"rad"), (image->imageInfo()).restoringBeam());

						//
						if ((imUnit.getName()).size()<1 || peakFlux.getUnit()!=imUnit.getName()) {
							// if the image has no units, or the units of the peakflux
							// is not identical to the original unit, store only the values;
							// "imfit" sometimes assumes unit...; not sure whether this is the
							// best solution (MK)
							zspKey = "IntegrFlux";
							zspVal = String::toString((pFlux(0)).getValue());
							layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

							zspKey = "PeakFlux";
							zspVal = String::toString(peakFlux.getValue());
							layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));
						} else {
							// if a unit was defined for the image, store
							// value and unit
							zspKey = "IntegrFlux";
							zspVal = String::toString(pFlux(0));
							layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));

							zspKey = "PeakFlux";
							zspVal = String::toString(peakFlux);
							layercenter->push_back(RegionInfo::center_t::value_type(zspKey, zspVal));
						}
					}
				}

				// store the sky results
				if (medVals.size()>0) {
					vector<Double> solution, error;
					fitter.getZeroLevelSolution(solution, error);
					layercenter->push_back(RegionInfo::center_t::value_type("Skylevel", String::toString(solution[0])));
				}

				// store fit result, which here can only be
				// positive (bad fits have merged off above
				if (fitter.converged(0)) {
					layercenter->push_back(RegionInfo::center_t::value_type("Converged", "YES"));
				} else {
					layercenter->push_back(RegionInfo::center_t::value_type("Converged", "NO"));
					return layercenter;
				}

				std::tr1::shared_ptr<LogIO> log(new LogIO());
				LogOrigin origin("Region", __FUNCTION__);;
				*log << origin << LogIO::NORMAL << "Centering results:" << LogIO::POST;
				for (RegionInfo::center_t::iterator it=layercenter->begin() ; it != layercenter->end(); it++ ) {
					*log << origin << LogIO::NORMAL << "	--- " << (*it).first << ": " << (*it).second << LogIO::POST;
				}

				if (pixCen.size()>1) {
					double pix_cent_x, pix_cent_y, lin_delta_x, lin_delta_y;
					double lin_x, lin_y, posang, tmpx, tmpy;

					// get the pixel center and the position angle
					pix_cent_x = (double)pixCen(0);
					pix_cent_y = (double)pixCen(1);
					posang     = (double)pixCen(2);

					// convert to linear center coos and the extend
					pixel_to_linear(wc_, pix_cent_x, pix_cent_y, lin_x, lin_y );
					pixel_to_linear(wc_, pix_cent_x+cos(posang), pix_cent_y+sin(posang), tmpx, tmpy);
					lin_delta_x = tmpx-lin_x;
					lin_delta_y = tmpy-lin_y;

					// set the center and let it drawn
					setCenter( lin_x, lin_y, lin_delta_x, lin_delta_y);
					setDrawCenter(true);
					refresh();
				}

				delete rec;
				return layercenter;
			} catch (const casa::AipsError& err) {
				std::string errMsg_ = err.getMesg();
				//fprintf( stderr, "Region::getLayerCenter( ): %s\n", errMsg_.c_str() );
				return 0;
			} catch (...) {
				std::string errMsg_ = "Unknown error computing region centers.";
				//fprintf( stderr, "Region::getLayerCenter( ): %s\n", errMsg_.c_str() );
				return 0;
			}
			// should not get to here
			return 0;
		}

		const std::set<Region*> &Region::get_selected_regions( ) {
			// std::list<Region*> regions = dock_->regions( );
			// for ( std::list<Region*>::iterator it=regions.begin( ); it != regions.end( ); ++it ) {
			// 	if ( (*it)->marked( ) ) {
			// 		Region *r = dynamic_cast<Region*>(*it);
			// 		if ( r ) result->push_back(r);
			// 	}
			// }
			// return result;
			return dock_->selectedRegionSet( );
		}

		ImageRegion_state Region::get_image_selected_region( DisplayData *dd ) {
			if ( dd == 0 ) return ImageRegion_state( );

			const std::set<Region*> &selected_regions = get_selected_regions( );
			ImageRegion *result = 0;
			size_t count = 0;

			if ( selected_regions.size( ) <= 0 ) {
				//*
				//* there were no selected regions...
				//*
				result = get_image_region( dd );
				count = 1;

			} else {
				//*
				//* create a compound region...
				//*

				// does the selected region list contain this region?
				bool contains_this = false;
				for ( std::set<Region*>::const_iterator it = selected_regions.begin( ); it != selected_regions.end( ); ++it ) {
					if ( *it == this ) {
						contains_this = true;
						break;
					}
				}

				// initialize compound region...
				//But we need to exclude polyline regions since they don't
				//have statistics.
				int imageRegionCount = contains_this ? 0 : 1;
				if ( type() == region::PolylineRegion ) {
					imageRegionCount = 0;
				}
				for ( std::set<Region*>::const_iterator it = selected_regions.begin( );
				        it != selected_regions.end( ); ++it ) {
					if ( ! *it ) continue; // OSX crash seemingly with a null pointer
					                       // ( was not able to determine how selected region
 					                       //   set ended up with a null pointer in it... )
					                       // <drs> Wed Apr 23 11:30:20 EDT 2014
					ImageRegion* imageRegion = (*it)->get_image_region( dd );
					if ( imageRegion != NULL ) {
						imageRegionCount++;
					}
				}
				PtrBlock<const ImageRegion*> rgns( imageRegionCount );
				if ( contains_this == false ) {
					ImageRegion* reg = get_image_region( dd );
					if ( reg != NULL ) {
						rgns[count++] = reg;
					}
				}
				for ( std::set<Region*>::const_iterator it = selected_regions.begin( );
				        it != selected_regions.end( ); ++it ) {
					ImageRegion* imageRegion = (*it)->get_image_region( dd );
					if ( imageRegion != NULL ) {
						rgns[count++] = imageRegion;
					}
				}
				try {
					WCUnion compound( rgns );
					result = new ImageRegion(compound);

				} catch(...) { }
			}
			return ImageRegion_state(result,count);
		}

		void Region::updateHistogramRegion() {
			if( wc_==0 ) {
				return;
			}

			//Put the updated regions into the histogram.
			DisplayData* masterDD = wc_->csMaster();
			if ( masterDD != NULL ) {
				ImageRegion* region = get_image_region( masterDD );
				if ( region == NULL ){
					return;
				}
				const std::list<DisplayData*> &dds = wc_->displaylist( );
				for ( std::list<DisplayData*>::const_iterator ddi=dds.begin(); ddi != dds.end(); ++ddi ) {
					DisplayData* dd = *ddi;
					if ( dd != NULL ) {
						PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
						if (padd==0) {
							continue;
						}
						std::tr1::shared_ptr<ImageInterface<float> > image(padd->imageinterface());
						if ( image.get() != NULL ) {
							histogram->addImage( image );
							histogram->setImageRegion( image->name(true).c_str(), region, id_);
						}
					}
				}
			}
		}


		std::list<std::tr1::shared_ptr<RegionInfo> > *Region::generate_dds_statistics(  ) {
			std::list<std::tr1::shared_ptr<RegionInfo> > *region_statistics = new std::list<std::tr1::shared_ptr<RegionInfo> >( );
			if( wc_==0 ) return region_statistics;

			Int zindex = 0;
			if (wc_->restrictionBuffer()->exists("zIndex")) {
				wc_->restrictionBuffer()->getValue("zIndex", zindex);
			}

			DisplayData *dd = 0;
			const std::list<DisplayData*> &dds = wc_->displaylist( );

			ImageRegion_state imageregion_state = get_image_selected_region( wc_->csMaster( ) );
			std::tr1::shared_ptr<ImageRegion> imageregion = imageregion_state;
			char region_component_count[128];
			sprintf( region_component_count, "%lu", imageregion_state.regionCount( ) );

			std::string errMsg_;
			std::map<String,bool> processed;
			for ( std::list<DisplayData*>::const_iterator ddi=dds.begin(); ddi != dds.end(); ++ddi ) {
				dd = *ddi;
				PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
				if (padd==0) {
					generate_nonimage_statistics( dd, region_statistics );
					continue;
				}

				try {

					std::tr1::shared_ptr<ImageInterface<Float> > image ( padd->imageinterface( ));

					if ( ! image ) continue;

					String full_image_name = image->name(false);
					std::map<String,bool>::iterator repeat = processed.find(full_image_name);
					if (repeat != processed.end()) continue;
					processed.insert(std::map<String,bool>::value_type(full_image_name,true));
					if ( name_ == "polyline" ) {
						RegionInfo *info = newInfoObject( image.get(), padd );
						if ( info ){
							region_statistics->push_back( std::tr1::shared_ptr<RegionInfo>(info));
						}
					} else if ( name_ == "p/v line" ) {

						get_image_region( dd );
						RegionInfo *info = newInfoObject( image.get(), padd );
						if ( info ) region_statistics->push_back( std::tr1::shared_ptr<RegionInfo>(info) );

					} else {

						if ( imageregion.get( ) == NULL  ) continue;

						RegionInfo::stats_t *dd_stats = getLayerStats(padd,image.get(),*imageregion);
						if ( dd_stats ) {
							dd_stats->push_back(std::pair<String,String>("region count",region_component_count));
							region_statistics->push_back(std::tr1::shared_ptr<RegionInfo>(new ImageRegionInfo( image->name(true), image->name(false), dd_stats)));
						}
					}
				} catch (const casa::AipsError& err) {
					errMsg_ = err.getMesg();
					continue;
				} catch (...) {
					errMsg_ = "Unknown error converting region";
					continue;
				}
			}

			return region_statistics;
		}

		RegionInfo::stats_t *Region::getLayerStats( PrincipalAxesDD *padd, ImageInterface<Float> *image, ImageRegion& imgReg ) {

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

				Int _axis_h_ = shp.size( ) > 3 ? padd->xlatePixelAxes(3) : -1;		// get first "hidden axis
				String zaxis = padd->zaxisStr( );

				const DisplayCoordinateSystem& cs = image->coordinates();

				Vector<String> axesNames = padd->worldToPixelAxisNames( cs );
				String haxis = _axis_h_ >= 0 ? axesNames(_axis_h_) : "";
				// uiBase( ) sets zero/one based:
				Int hIndex = _axis_h_ >= 0 ? padd->xlateFixedPixelAxes(_axis_h_) + padd->uiBase( ) : -1;
				Int zIndex = padd->activeZIndex();

				String zUnit, zspKey, zspVal;
				zUnit = padd->spectralunitStr( );

				String unit =  image->units().getName();

				IPosition pos = padd->fixedPosition();

				ImageStatistics<Float> stats(subImg, False);
				bool cursorAxesSet = stats.setAxes( cursorAxes );
				if ( ! cursorAxesSet ) return 0;
				stats.setList(True);
				Vector<String> nm = cs.worldAxisNames();

				Int zPos = -1;
				Int hPos = -1;
				for (size_t k = 0; k < nm.nelements(); k++) {
					if (nm(k) == zaxis)
						zPos = k;
					if (nm(k) == haxis)
						hPos = k;
				}

				// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
				//	 begin collecting statistics...
				// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
				RegionInfo::stats_t *layerstats = new RegionInfo::stats_t( );
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
						layerstats->push_back(RegionInfo::stats_t::value_type(zaxis,zLabel + tStr));

						if (zUnit.length()>0) {
							zspKey = "Spectral_Value";
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
							zspKey = "Spectral_Value";
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
						if (restFreq >0 && spCoord.pixelToVelocity(vel, zIndex)) {
							layerstats->push_back(RegionInfo::stats_t::value_type("Velocity",String::toString(vel)+"km/s"));
						}
						else {
							layerstats->push_back(RegionInfo::stats_t::value_type(zspKey,zspVal));
							// --- this line was executed, but was a NOP in the old code --- <drs>
							// layerstats->push_back(RegionInfo::image_stats_t::value_type("Doppler",MDoppler::showType(spCoord.velocityDoppler())));
						}
					}

					if (downcase(haxis).contains("freq")) {
						if (spCoord.pixelToVelocity(vel, hIndex)) {
							if (restFreq >0)
								layerstats->push_back(RegionInfo::stats_t::value_type("Velocity",String::toString(vel)+"km/s"));
							else
								layerstats->push_back(RegionInfo::stats_t::value_type(zspKey,zspVal));

							layerstats->push_back(RegionInfo::stats_t::value_type("Frame",MFrequency::showType(spCoord.frequencySystem())));
							layerstats->push_back(RegionInfo::stats_t::value_type("Doppler",MDoppler::showType(spCoord.velocityDoppler())));
						}
					}
				}


				if ( hLabel != "" ) layerstats->push_back(RegionInfo::stats_t::value_type(haxis,hLabel));

				// strip out extra quotes, e.g. '"ELECTRONS"'
				std::string unitval(unit.c_str( ));
				std::string::size_type p = 0;
				while( (p = unitval.find('"',p)) != unitval.npos ) {
					unitval.erase(p, 1);
				}
				layerstats->push_back(RegionInfo::stats_t::value_type("BrightnessUnit",unitval));

				Double beamArea = 0;
				ImageInfo ii = image->imageInfo();
				GaussianBeam beam = ii.restoringBeam(zIndex);
				DisplayCoordinateSystem cSys = image->coordinates();
				std::string imageUnits = image->units().getName();
				std::transform( imageUnits.begin(), imageUnits.end(), imageUnits.begin(), ::toupper );

				Int afterCoord = -1;
				Int dC = cSys.findCoordinate(Coordinate::DIRECTION, afterCoord);
				// use contains() not == so moment maps are dealt with nicely
				if ( ! beam.isNull() && dC!=-1 && imageUnits.find("JY/BEAM") != std::string::npos ) {
					DirectionCoordinate dCoord = cSys.directionCoordinate(dC);
					Vector<String> units(2);
					units(0) = units(1) = "rad";
					dCoord.setWorldAxisUnits(units);
					Vector<Double> deltas = dCoord.increment();
					beamArea = beam.getArea("rad2") / abs(deltas(0) * deltas(1));
				}

				if ( beamArea > 0 ) layerstats->push_back(RegionInfo::stats_t::value_type("BeamArea",String::toString(beamArea)));

				Bool statsOk = stats.getLayerStats(*layerstats, beamArea, zPos, zIndex, hPos, hIndex);
				if ( ! statsOk ) {
					delete layerstats;
					return 0;
				} else {
					return layerstats;
				}

			} catch (const casa::AipsError& err) {
				std::string errMsg_ = err.getMesg();
				//fprintf( stderr, "Region::getLayerStats( ): <AipsError> %s\n", errMsg_.c_str() );
				return 0;
			} catch (...) {
				std::string errMsg_ = "Unknown error computing region statistics.";
				//fprintf( stderr, "Region::getLayerStats( ): %s\n", errMsg_.c_str() );
				return 0;
			}
		}

		Int Region::getAxisIndex( ImageInterface<Float> *image, std::string axtype ) {

			if( image == 0 ) return -1;

			DisplayCoordinateSystem cs=image->coordinates( );

			try {
				Int nAxes = image->ndim();
				for(Int ax=0; ax<nAxes && ax<Int(cs.nWorldAxes()); ax++) {
					// coordno : type of coordinate
					// axisincoord : index within the coordinate list defined by coordno
					Int coordno, axisincoord;
					cs.findWorldAxis(coordno, axisincoord, ax);

					//cout << "coordno=" << coordno << "  axisincoord : " << axisincoord << "  type : " << cs.showType(coordno) << endl;

					if( cs.showType(coordno) == "Direction" ) {
						// Check for Right Ascension and Declination
						Vector<String> axnames = (cs.directionCoordinate(coordno)).axisNames(MDirection::DEFAULT);
						AlwaysAssert( axisincoord>=0 && axisincoord < (int) axnames.nelements(), AipsError);
						if( axnames[axisincoord] == axtype.c_str() ) {
							return ax;
						}
					} else {
						// Check for Stokes and Spectral
						if ( cs.showType(coordno) == axtype.c_str() ) {
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

		void linear_to_j2000( WorldCanvas *wc, double lx, double ly, double &wx, double &wy ) {
			MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
			linear_to_world( wc, lx, ly, wx, wy );
			if ( type == MDirection::J2000 ) return;
			Vector<double> pts(2);
			pts[0] = wx;
			pts[1] = wy;
			MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::J2000)( );
			casa::Quantum<casa::Vector<double> > newpts = direction.getAngle("rad");
			wx = newpts.getValue( )(0);
			wy = newpts.getValue( )(1);
		}

		void linear_to_j2000( WorldCanvas *wc, double lx1, double ly1, double lx2, double ly2,
		                      double &wx1, double &wy1, double &wx2, double &wy2 ) {

			MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
			linear_to_world( wc, lx1, ly1, lx2, ly2, wx1, wy1, wx2, wy2 );
			if ( type == MDirection::J2000 ) return;

			Vector<double> pts(2);
			pts[0] = wx1;
			pts[1] = wy1;
			MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::J2000)( );
			casa::Quantum<casa::Vector<double> > newpts = direction.getAngle("rad");
			wx1 = newpts.getValue( )(0);
			wy1 = newpts.getValue( )(1);

			pts[0] = wx2;
			pts[1] = wy2;
			direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::J2000)( );
			newpts = direction.getAngle("rad");
			wx2 = newpts.getValue( )(0);
			wy2 = newpts.getValue( )(1);

		}

		void to_linear( WorldCanvas *wc, MDirection::Types in_type, double x, double y, double &lx, double &ly ) {
			MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
			if ( type == in_type ) {
				world_to_linear( wc, x, y, lx, ly );
				return;
			} else {
				Vector<double> pts(2);
				pts[0] = x;
				pts[1] = y;
				MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"deg"), in_type),type)( );
				casa::Vector<double> newpts = direction.getAngle("rad").getValue( );
				world_to_linear( wc, newpts[0], newpts[1], lx, ly );
			}
		}

		void to_linear( WorldCanvas *wc, MDirection::Types in_type, double x1, double y1, double x2, double y2, double &lx1, double &ly1, double &lx2, double &ly2 ) {
			MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
			if ( type == in_type ) {
				world_to_linear( wc, x1, y1, lx1, ly1 );
				world_to_linear( wc, x2, y2, lx2, ly2 );
				return;
			} else {
				Vector<double> pts(2);
				pts[0] = x1;
				pts[1] = y1;
				MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"deg"), in_type),type)( );
				casa::Vector<double> newpts = direction.getAngle("rad").getValue( );
				world_to_linear( wc, newpts[0], newpts[1], lx1, ly1 );
				pts[0] = x2;
				pts[1] = y2;
				direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"deg"), in_type),type)( );
				newpts = direction.getAngle("rad").getValue( );
				world_to_linear( wc, newpts[0], newpts[1], lx1, ly2 );
			}
		}

		void to_linear_offset( WorldCanvas *wc, MDirection::Types in_type, double xoff, double yoff, double &lxoff, double &lyoff ) {
			const int base = 10;
			double blcx, blcy, trcx, trcy;
			to_linear( wc, in_type, base, base, base+xoff, base+yoff, blcx, blcy, trcx, trcy );
			lxoff = trcx - blcx;
			lyoff = trcy - blcy;
		}

		void linear_to_b1950( WorldCanvas *wc, double lx, double ly, double &wx, double &wy ) {
			MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
			linear_to_world( wc, lx, ly, wx, wy );
			if ( type == MDirection::B1950 ) return;

			Vector<double> pts;
			pts[0] = wx;
			pts[1] = wy;
			MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::B1950)( );
			casa::Quantum<casa::Vector<double> > newpts = direction.getAngle("rad");
			wx = newpts.getValue( )(0);
			wy = newpts.getValue( )(1);
		}

		void linear_to_b1950( WorldCanvas *wc, double lx1, double ly1, double lx2, double ly2,
		                      double &wx1, double &wy1, double &wx2, double &wy2 ) {

			MDirection::Types type = get_coordinate_type(wc->coordinateSystem( ));
			linear_to_world( wc, lx1, ly1, lx2, ly2, wx1, wy1, wx2, wy2 );
			if ( type == MDirection::B1950 ) return;

			Vector<double> pts(2);
			pts[0] = wx1;
			pts[1] = wy1;
			MDirection direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::B1950)( );
			casa::Quantum<casa::Vector<double> > newpts = direction.getAngle("rad");
			wx1 = newpts.getValue( )(0);
			wy1 = newpts.getValue( )(1);

			pts[0] = wx2;
			pts[1] = wy2;
			direction = MDirection::Convert(MDirection(Quantum<Vector<double> >(pts,"rad"),type),MDirection::B1950)( );
			newpts = direction.getAngle("rad");
			wx2 = newpts.getValue( )(0);
			wy2 = newpts.getValue( )(1);

		}

		void Region::signal_region_change( region::RegionChanges change ) {
			if ( hold_signals > 0 ) {
				held_signals[change] = true;
				return;
			}
			switch ( change ) {
			case region::RegionChangeUpdate:
			case region::RegionChangeCreate:
			case region::RegionChangeDelete:
			case region::RegionChangeReset:
			case region::RegionChangeFocus:
			case region::RegionChangeModified:
			case region::RegionChangeNewChannel:
			case region::RegionChangeSelected: {
				region::RegionTypes type;
				QList<int> pixelx, pixely;
				QList<double> worldx, worldy;

				if ( change != region::RegionChangeDelete ) {
					fetch_details( type, pixelx, pixely, worldx, worldy );
					if ( pixelx.size() == 0 || pixely.size() == 0 || worldx.size() == 0 || worldy.size() == 0 ) {
						return;
					}
				}

				if ( change == region::RegionChangeCreate ) {
					dock_->emitCreate( this );
					emit regionCreated( id_, QString( type == region::RectRegion ? "rectangle" : type == region::PointRegion ? "point" :
					                                  type == region::EllipseRegion ? "ellipse" : type == region::PolyRegion ? "polygon" :
					                                  type == region::PolylineRegion ? "polyline" : "error"),
					                    QString::fromStdString(name( )), worldx, worldy, pixelx, pixely, QString::fromStdString(lineColor( )), QString::fromStdString(textValue( )),
					                    QString::fromStdString(textFont( )), textFontSize( ), textFontStyle( ) );
				} else
					emit regionUpdate( id_, change, worldx, worldy, pixelx, pixely );
			}
			break;


			case region::RegionChangeStatsUpdate:
			case region::RegionChangeLabel:
				// fprintf( stderr, "====>> labelRegion( %d [id], %s [line color], %s [text], %s [font], %d [style], %d [size] )\n",
				// 	     id_, lineColor( ).c_str( ), textValue( ).c_str( ), textFont( ).c_str( ), textFontStyle( ), textFontSize( ) );
				break;
			}
		}

		void Region::fetch_details( region::RegionTypes &type, QList<int> &pixelx, QList<int> &pixely, QList<double> &worldx, QList<double> &worldy ) {

			std::vector<std::pair<int,int> > pixel_pts;
			std::vector<std::pair<double,double> > world_pts;

			fetch_region_details(type, pixel_pts, world_pts);

			for ( unsigned int i=0; i < pixel_pts.size(); ++i ) {
				pixelx.push_back(pixel_pts[i].first);
				pixely.push_back(pixel_pts[i].second);
			}

			for ( unsigned int i=0; i < world_pts.size(); ++i ) {
				worldx.push_back(world_pts[i].first);
				worldy.push_back(world_pts[i].second);
			}

		}

		void Region::process_held_signals( ) {
			if ( held_signals[region::RegionChangeCreate] ) {
				signal_region_change(region::RegionChangeCreate);
			} else {
				if ( held_signals[region::RegionChangeUpdate] ) {
					signal_region_change(region::RegionChangeUpdate);
				}
				if ( held_signals[region::RegionChangeLabel] ) {
					signal_region_change(region::RegionChangeLabel);
				}
			}

			if ( held_signals[region::RegionChangeStatsUpdate] )
				updateStateInfo( false, region::RegionChangeCreate );

			clear_signal_cache( );
		}

		void Region::clear_signal_cache( ) {
			held_signals[region::RegionChangeCreate] = false;
			held_signals[region::RegionChangeUpdate] = false;
			held_signals[region::RegionChangeLabel] = false;
		}

		void Region::update_histogram_event() {

			//Clear out the old histograms.
			clearHistograms();

			//Add in the new histograms
			updateHistogramRegion();
		}

		void Region::initHistogram() {
			if ( histogram == NULL ) {
				histogram = new HistogramTab(state());
                histogram->setContentsMargins(0, 0, 0, 0);
				state()->addHistogram( histogram );
			}
		}

		void Region::clearHistograms() {
			if ( histogram != NULL ) {
				histogram->clear();
			}
		}
	}
}
