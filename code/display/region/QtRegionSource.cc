//# qtregionsource.cc: Qt regionfactory for generating regions
//# with surrounding Gui functionality
//# Copyright (C) 2011
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
//# $Id: $

#include <display/region/QtRegionSource.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/region/Rectangle.h>
#include <display/region/Polygon.h>
#include <display/region/Polyline.qo.h>
#include <display/region/Ellipse.h>
#include <display/region/Point.h>
#include <display/region/PVLine.qo.h>
#include <display/ds9/ds9parser.h>
#include <display/DisplayDatas/DisplayData.h>
#include <imageanalysis/Annotations/AnnRectBox.h>
#include <imageanalysis/Annotations/AnnEllipse.h>
#include <imageanalysis/Annotations/AnnSymbol.h>
#include <imageanalysis/Annotations/AnnPolygon.h>
#include <imageanalysis/Annotations/AnnPolyline.h>
#include <measures/Measures/MCDirection.h>

namespace casa {
	namespace viewer {

		QtRegionSourceKernel::QtRegionSourceKernel( QtDisplayPanelGui *panel ) : panel_(panel) {
			connect( panel_, SIGNAL(axisToolUpdate(QtDisplayData*)), this, SLOT(updateRegionState(QtDisplayData*)) );

		}

		QtRegionSourceKernel::~QtRegionSourceKernel( ) { }

		void QtRegionSourceKernel::revokeRegion( Region *r ) {
			std::map<Region*,RegionCreator*>::iterator it = creator_of_region.find(r);
			if ( it != creator_of_region.end( ) ) {
				RegionCreator *creator = it->second;
				creator_of_region.erase(it);
				creator->revokeRegion(r);
			}
		}

		void QtRegionSourceKernel::dtorCalled( const dtorNotifier *r ) {
			RegionSourceKernel::dtorCalled( r );
			std::map<Region*,RegionCreator*>::iterator it = creator_of_region.find((Region*)r);
			if ( it != creator_of_region.end( ) ) {
				creator_of_region.erase(it);
			}
		}

// SHARED_PTR<Rectangle> QtRegionSourceKernel::rectangle( int blc_x, int blc_y, int trc_x, int trc_y ) {
//     return SHARED_PTR<Rectangle>(new QtRectangle( this, blc_x, blc_y, trc_x, trc_y ));
// }
		SHARED_PTR<Rectangle> QtRegionSourceKernel::rectangle( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
			Rectangle *result = new Rectangle( this, wc, blc_x, blc_y, trc_x, trc_y, true );

			// save Region to RegionSource mapping for later revocation...
			creator_of_region[result] = rc;

			// register for dtor callback...
			register_new_region( result );

			connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                        const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                      const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
			connect( result, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
			         this, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

			connect( result, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                         const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                       const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );

			connect( this, SIGNAL( newCorners( double, double, double, double)),
			         result, SLOT( adjustCorners( double, double, double, double)));
			result->releaseSignals( );
			return SHARED_PTR<Rectangle>(result);
		}

// SHARED_PTR<Rectangle> QtRegionSourceKernel::rectangle( int blc_x, int blc_y, int trc_x, int trc_y ) {
//     return SHARED_PTR<Rectangle>(new QtRectangle( this, blc_x, blc_y, trc_x, trc_y ));
// }
		SHARED_PTR<PVLine> QtRegionSourceKernel::pvline( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
			PVLine *result = new PVLine( this, wc, blc_x, blc_y, trc_x, trc_y, true );

			// save Region to RegionSource mapping for later revocation...
			creator_of_region[result] = rc;

			// register for dtor callback...
			register_new_region( result );

			connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                        const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                      const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
			connect( result, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
			         this, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

			connect( result, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                         const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                       const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );

			connect( this, SIGNAL( newCorners( double, double, double, double)),
			         result, SLOT( adjustCorners( double, double, double, double)));
			result->releaseSignals( );
			return SHARED_PTR<PVLine>(result);
		}

		void QtRegionSourceKernel::adjustPosition( double blcx, double blcy, double trcx, double trcy ) {
			emit newCorners( blcx, blcy, trcx, trcy );
		}

		SHARED_PTR<Polygon> QtRegionSourceKernel::polygon(
		    RegionCreator *rc, WorldCanvas *wc, double x1, double y1 ) {
			Polygon *result = new Polygon( this, wc, x1, y1, true );

			// save Region to RegionSource mapping for later revocation...
			creator_of_region[result] = rc;

			// register for dtor callback...
			register_new_region( result );

			connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                        const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                      const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
			connect( result, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
			         this, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

			connect( result, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                         const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                       const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );

			return SHARED_PTR<Polygon>(result);
			// return SHARED_PTR<Polygon>( );
		}

		SHARED_PTR<Polyline> QtRegionSourceKernel::polyline( RegionCreator *rc, WorldCanvas *wc, double x1, double y1 ) {
			Polyline *result = new Polyline( this, wc, x1, y1, true );

			// save Region to RegionSource mapping for later revocation...
			creator_of_region[result] = rc;

			// register for dtor callback...
			register_new_region( result );
			connect( result, SIGNAL( show1DSliceTool()), this, SIGNAL(show1DSliceTool()));
			connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                        const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                      const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
			connect( result, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
			         this, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );

			connect( result, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                         const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                       const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );

			return SHARED_PTR<Polyline>(result);
			// return SHARED_PTR<Polygon>( );
		}


		SHARED_PTR<Polygon> QtRegionSourceKernel::polygon(
		    RegionCreator *rc, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {
			Polygon *result = new Polygon( this, wc, pts, true );

			// save Region to RegionSource mapping for later revocation...
			creator_of_region[result] = rc;

			// register for dtor callback...
			register_new_region( result );

			connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                        const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                      const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
			connect( result, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
			         this, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );
			connect( result, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                         const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                       const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );

			result->releaseSignals( );
			return SHARED_PTR<Polygon>(result);
			// return SHARED_PTR<Polygon>( );
		}

		SHARED_PTR<Polyline> QtRegionSourceKernel::polyline(
		    RegionCreator *rc, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) {

			Polyline *result = new Polyline( this, wc, pts, false );

			// save Region to RegionSource mapping for later revocation...
			creator_of_region[result] = rc;

			// register for dtor callback...
			register_new_region( result );

			connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                        const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                      const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
			connect( result, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
			         this, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );
			connect( result, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                         const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                       const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );

			result->releaseSignals( );
			return SHARED_PTR<Polyline>(result);
		}

		SHARED_PTR<Rectangle> QtRegionSourceKernel::ellipse( RegionCreator *rc, WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
			Ellipse *result = new Ellipse( this, wc, blc_x, blc_y, trc_x, trc_y, true );

			// save Region to RegionSource mapping for later revocation...
			creator_of_region[result] = rc;

			// register for dtor callback...
			register_new_region( result );

			connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                        const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                      const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
			connect( result, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
			         this, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );
			connect( result, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                         const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                       const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );

			result->releaseSignals( );
			return SHARED_PTR<Rectangle>(result);
		}

		SHARED_PTR<Rectangle> QtRegionSourceKernel::point( RegionCreator *rc, WorldCanvas *wc, double x, double y,
		        QtMouseToolNames::PointRegionSymbols sym, int size ) {
			Point *result = new Point( this, wc, x, y, true, sym );
			result->setMarkerScale(size);

			// save Region to RegionSource mapping for later revocation...
			creator_of_region[result] = rc;

			// register for dtor callback...
			register_new_region( result );

			connect( result, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                        const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                                      const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
			connect( result, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ),
			         this, SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> & ) ) );
			connect( result, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                         const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ),
			         this, SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &, const QList<double> &,
			                       const QList<int> &, const QList<int> &, const QString &, const QString &, const QString &, int, int ) ) );
			connect( this, SIGNAL( newCorners( double, double, double, double)),
			         result, SLOT( adjustCorners( double, double, double, double)));
			result->releaseSignals( );
			return SHARED_PTR<Rectangle>(result);
		}

		QtMouseToolNames::PointRegionSymbols QtRegionSourceKernel::currentPointSymbolType( ) const {
			QtMouseToolNames::PointRegionSymbols result = QtMouseToolNames::SYM_DOT;
			int state = panel_->buttonToolState(QtMouseToolNames::POINT);
			if ( state >= 0 && state < QtMouseToolNames::SYM_POINT_REGION_COUNT )
				result = (QtMouseToolNames::PointRegionSymbols) state;
			return result;
		}

		QtRegionDock *QtRegionSourceKernel::dock( ) {
			return panel_->regionDock( );
		}
		int QtRegionSourceKernel::numFrames( ) const {
			return panel_->numFrames( );
		}

		void QtRegionSourceKernel::updateRegionState(QtDisplayData*) {
			generateExistingRegionUpdates( );
		}

		void QtRegionSourceKernel::loadRegions( const QString &path, const QString &type ) {
			bool handled = true;
			if ( ! handled ) {
				handled = true;
				ConstListIter<WorldCanvas*> wcl = panel_->displayPanel()->panelDisplay()->myWCLI;
				wcl.toStart( );
				// really need to change this to find the world canvas for the display data that is currently visible...
				// instead of just taking the first one in the list...  <<<see also QtDisplayPanel.cc>>>
				WorldCanvas *wc_ = 0;
				if(wcl.len() > 0) {
					wc_ = wcl.getRight( );
				}
				if ( wc_ ) {
					if ( type.compare("CASA region file") == 0 ) {
						load_crtf_regions( wc_, path );
					} else if ( type.compare("DS9 region file") == 0 ) {
						casa::viewer::ds9context context( wc_, panel_->displayPanel( )->toolMgr( ) );
						casa::viewer::ds9parser parser;
						parser.parse_file( context, path.toAscii( ).constData( ) );
					} else {
						fprintf( stderr, "QtRegionSourceKernel::loadRegions( bool &handled, const QString &path, const QString &type ):\n" );
						fprintf( stderr, "\tinternal error, invalid type: %s...\n", type.toAscii( ).constData( ) );
					}
				} else {
					fprintf( stderr, "QtRegionSourceKernel::loadRegions( bool &handled, const QString &path, const QString &type ):\n" );
					fprintf( stderr, "\tinternal error, no world canvas...\n" );
				}
			}
		}

		void QtRegionSourceKernel::load_crtf_regions( WorldCanvas *wc, const QString &path ) {
			// find viewers's coordinate system type...
			const DisplayCoordinateSystem &viewer_cs = wc->coordinateSystem( );
			MDirection::Types cstype = get_coordinate_type( viewer_cs );
			if ( cstype == MDirection::EXTRA ) return;

			// need shape(?!?!) to get the annotation region list...
			const DisplayData *dd = wc->displaylist().front();
			std::vector<int> axes = dd->displayAxes( );
			IPosition shape(viewer_cs.nPixelAxes( ));
			for ( unsigned int i=0; i < shape.size( ); ++i )
				shape(i) = dd->dataShape( )[axes[i]];

			try {
				RegionTextList region_list( path.toAscii( ).constData( ), viewer_cs, shape );
				const casa::Vector<AsciiAnnotationFileLine> &lines = region_list.getLines( );
				for ( uInt i=0; i < lines.size( ); ++i ) {
					if ( lines[i].getType() == AsciiAnnotationFileLine::ANNOTATION ) {
						CountedPtr<const AnnotationBase> annotation = lines[i].getAnnotationBase();
						const AnnRectBox *rectangle=0;
						const AnnEllipse *ellipse=0;
						const AnnSymbol *symbol=0;
						const AnnPolygon *polygon = 0;
						const AnnPolyline *polyline = 0;
						if ( (rectangle=dynamic_cast<const AnnRectBox*>(annotation.get())) ) {
							load_crtf_rectangle( wc, cstype, rectangle );
						} else if ( (ellipse = dynamic_cast<const AnnEllipse*>(annotation.get())) ) {
							load_crtf_ellipse( wc, cstype, ellipse );
						} else if ( (symbol = dynamic_cast<const AnnSymbol*>(annotation.get())) ) {
							load_crtf_point( wc, cstype, symbol );
						} else if ( (polygon = dynamic_cast<const AnnPolygon*>(annotation.get())) ) {
							load_crtf_polygon( wc, cstype, polygon );
						} else if ( (polyline = dynamic_cast<const AnnPolyline*>(annotation.get())) ) {
							load_crtf_polyline( wc, cstype, polyline );
						}
					}
				}
			} catch(...) {
				fprintf( stderr, "loading of %s failed...\n", path.toAscii( ).constData( ) );
				return;
			}
		}

		inline int get_font_style( AnnotationBase::FontStyle annfontstyle ) {
			return ( annfontstyle == AnnotationBase::NORMAL ? 0 :
			         annfontstyle == AnnotationBase::BOLD ? region::BoldText :
			         annfontstyle == AnnotationBase::ITALIC ? region::ItalicText :
			         annfontstyle == AnnotationBase::ITALIC_BOLD ? (region::BoldText | region::ItalicText) : 0 );
		}

		inline region::LineStyle get_line_style( AnnotationBase::LineStyle annlinestyle ) {
			return ( annlinestyle == AnnotationBase::SOLID ? region::SolidLine :
			         annlinestyle == AnnotationBase::DASHED ? region::DashLine :
			         annlinestyle == AnnotationBase::DOTTED ? region::DotLine : region::SolidLine );
		}

		void QtRegionSourceKernel::load_crtf_rectangle( WorldCanvas *wc, MDirection::Types cstype, const AnnRectBox *rectangle ) {

			MDirection::Types ann_cstype = get_coordinate_type( rectangle->getCsys( ) );
			if ( ann_cstype == MDirection::EXTRA ) return;

			// get BLC, TRC as quantities... <mdirection>
			casa::Vector<MDirection> corners = rectangle->getCorners();
			if ( corners.size() != 2 ) return;

			// convert to the viewer's world coordinates... <mdirection>
			MDirection blcmd = MDirection::Convert(MDirection(corners[0].getAngle("rad"),ann_cstype), cstype)();
			casa::Quantum<casa::Vector<double> > blcq = blcmd.getAngle("rad");
			MDirection trcmd = MDirection::Convert(MDirection(corners[1].getAngle("rad"),ann_cstype), cstype)();
			casa::Quantum<casa::Vector<double> > trcq = trcmd.getAngle("rad");
			std::vector<std::pair<double,double> > pts(2);
			try {
				world_to_linear( wc,blcq.getValue( )(0),blcq.getValue( )(1),trcq.getValue( )(0),trcq.getValue( )(1),
				                 pts[0].first, pts[0].second, pts[1].first, pts[1].second );
			} catch(...) {
				return;
			}

			// create the rectangle...
			const RegionCreator::creator_list_type &rect_creators = RegionCreator::findCreator( region::RectRegion );
			if ( rect_creators.size( ) <= 0 ) return;
			int font_style = get_font_style(rectangle->getFontStyle());
			region::LineStyle line_style = get_line_style(rectangle->getLineStyle( ));

			String label_position = rectangle->getLabelPosition( );
			rect_creators.front( )->create( region::RectRegion, wc, pts,
			                                rectangle->getLabel( ), ( label_position == "left" ? region::LeftText :
			                                        label_position == "right" ? region::RightText :
			                                        label_position == "bottom" ? region::BottomText : region::TopText ),
			                                rectangle->getLabelOffset( ),
			                                rectangle->getFont( ), rectangle->getFontSize( ), font_style, rectangle->getLabelColorString( ),
			                                rectangle->getColorString( ), line_style, rectangle->getLineWidth( ), rectangle->isAnnotationOnly( ), 0 );
		}


		void QtRegionSourceKernel::load_crtf_ellipse( WorldCanvas *wc, MDirection::Types cstype, const AnnEllipse *ellipse ) {
			MDirection::Types ann_cstype = get_coordinate_type( ellipse->getCsys( ) );
			if ( ann_cstype == MDirection::EXTRA ) return;

			// convert to the viewer's world coordinates... <mdirection>
			MDirection dir_center = MDirection::Convert(ellipse->getCenter( ), cstype)();
			casa::Vector<double> center = dir_center.getAngle("rad").getValue( );
			// 90 deg around 0 & 180 deg
			const double major_radius = ellipse->getSemiMajorAxis().getValue("rad");
			const double minor_radius = ellipse->getSemiMinorAxis().getValue("rad");
			const double pos_angle = ellipse->getPositionAngle( ).getValue("deg");
			const bool x_is_major = ((pos_angle > 45.0 && pos_angle < 135.0) ||
			                         (pos_angle > 225.0 && pos_angle < 315.0));
			const double xradius = (x_is_major ? major_radius : minor_radius);
			const double yradius = (x_is_major ? minor_radius : major_radius);

			std::vector<std::pair<double,double> > pts(2);
			try {
				world_to_linear(wc,center[0]-xradius,center[1]-yradius,center[0]+xradius,center[1]+yradius, pts[0].first, pts[0].second, pts[1].first, pts[1].second);
			} catch(...) {
				return;
			}

			// create the ellipse...
			const RegionCreator::creator_list_type &ellipse_creators = RegionCreator::findCreator( region::EllipseRegion );
			if ( ellipse_creators.size( ) <= 0 ) return;
			int font_style = get_font_style(ellipse->getFontStyle());
			region::LineStyle line_style = get_line_style(ellipse->getLineStyle( ));

			String label_position = ellipse->getLabelPosition( );
			ellipse_creators.front( )->create( region::EllipseRegion, wc, pts,
			                                   ellipse->getLabel( ), ( label_position == "left" ? region::LeftText :
			                                           label_position == "right" ? region::RightText :
			                                           label_position == "bottom" ? region::BottomText : region::TopText ),
			                                   ellipse->getLabelOffset( ),
			                                   ellipse->getFont( ), ellipse->getFontSize( ), font_style, ellipse->getLabelColorString( ),
			                                   ellipse->getColorString( ), line_style, ellipse->getLineWidth( ), ellipse->isAnnotationOnly( ), 0 );
		}


		void QtRegionSourceKernel::load_crtf_point( WorldCanvas *wc, MDirection::Types cstype, const AnnSymbol *symbol ) {

			MDirection::Types ann_cstype = get_coordinate_type( symbol->getCsys( ) );
			if ( ann_cstype == MDirection::EXTRA ) return;

			// get point
			MDirection dir_point = MDirection::Convert(symbol->getDirection( ), cstype)();
			casa::Vector<double> point = dir_point.getAngle("rad").getValue( );

			std::vector<std::pair<double,double> > pts(2);
			try {
				world_to_linear( wc, point[0], point[1], pts[0].first, pts[0].second );
			} catch(...) {
				return;
			}
			pts[1] = pts[0];	// points also have two corners...

			// create the point...
			const RegionCreator::creator_list_type &point_creators = RegionCreator::findCreator( region::PointRegion );
			if ( point_creators.size( ) <= 0 ) return;
			int font_style = get_font_style(symbol->getFontStyle());
			region::LineStyle line_style = get_line_style(symbol->getLineStyle( ));

			String label_position = symbol->getLabelPosition( );
			point_creators.front( )->create( region::PointRegion, wc, pts,
			                                 symbol->getLabel( ), ( label_position == "left" ? region::LeftText :
			                                         label_position == "right" ? region::RightText :
			                                         label_position == "bottom" ? region::BottomText : region::TopText ),
			                                 symbol->getLabelOffset( ),
			                                 symbol->getFont( ), symbol->getFontSize( ), font_style, symbol->getLabelColorString( ),
			                                 symbol->getColorString( ), line_style, symbol->getLineWidth( ), false, 0 );
		}

		void QtRegionSourceKernel::load_crtf_polygon( WorldCanvas *wc, MDirection::Types cstype, const AnnPolygon *polygon ) {

			MDirection::Types ann_cstype = get_coordinate_type( polygon->getCsys( ) );
			if ( ann_cstype == MDirection::EXTRA ) return;

			casa::Vector<MDirection> corners = polygon->getCorners();
			if ( corners.size() < 3 ) return;

			std::vector<std::pair<double,double> > pts(corners.size());
			for ( unsigned int i=0; i < corners.size( ); ++i ) {
				MDirection corner = MDirection::Convert(corners[i], cstype)();
				casa::Vector<double> point = corner.getAngle("rad").getValue( );
				try {
					world_to_linear( wc, point[0], point[1], pts[i].first, pts[i].second );
				} catch(...) {
					return;
				}
			}

			// create the polygon...
			const RegionCreator::creator_list_type &poly_creators = RegionCreator::findCreator( region::PolyRegion );
			if ( poly_creators.size( ) <= 0 ) return;
			int font_style = get_font_style(polygon->getFontStyle());
			region::LineStyle line_style = get_line_style(polygon->getLineStyle( ));

			String label_position = polygon->getLabelPosition( );
			poly_creators.front( )->create( region::PolyRegion, wc, pts,
			                                polygon->getLabel( ), ( label_position == "left" ? region::LeftText :
			                                        label_position == "right" ? region::RightText :
			                                        label_position == "bottom" ? region::BottomText : region::TopText ),
			                                polygon->getLabelOffset( ),
			                                polygon->getFont( ), polygon->getFontSize( ), font_style, polygon->getLabelColorString( ),
			                                polygon->getColorString( ), line_style, polygon->getLineWidth( ), polygon->isAnnotationOnly( ), 0 );
		}

		void QtRegionSourceKernel::load_crtf_polyline( WorldCanvas *wc, MDirection::Types cstype, const AnnPolyline *polyline ) {

			MDirection::Types ann_cstype = get_coordinate_type( polyline->getCsys( ) );
			if ( ann_cstype == MDirection::EXTRA ) return;

			casa::Vector<MDirection> corners = polyline->getCorners();
			if ( corners.size() < 2 ) return;

			std::vector<std::pair<double,double> > pts(corners.size());
			for ( unsigned int i=0; i < corners.size( ); ++i ) {
				MDirection corner = MDirection::Convert(corners[i], cstype)();
				casa::Vector<double> point = corner.getAngle("rad").getValue( );
				try {
					world_to_linear( wc, point[0], point[1], pts[i].first, pts[i].second );
				} catch(...) {
					return;
				}
			}

			// create the polyline...
			const RegionCreator::creator_list_type &poly_creators = RegionCreator::findCreator( region::PolylineRegion );
			if ( poly_creators.size( ) <= 0 ) return;
			int font_style = get_font_style(polyline->getFontStyle());
			region::LineStyle line_style = get_line_style(polyline->getLineStyle( ));

			String label_position = polyline->getLabelPosition( );
			poly_creators.front( )->create( region::PolylineRegion, wc, pts,
			                                polyline->getLabel( ), ( label_position == "left" ? region::LeftText :
			                                        label_position == "right" ? region::RightText :
			                                        label_position == "bottom" ? region::BottomText : region::TopText ),
			                                polyline->getLabelOffset( ),
			                                polyline->getFont( ), polyline->getFontSize( ), font_style, polyline->getLabelColorString( ),
			                                polyline->getColorString( ), line_style, polyline->getLineWidth( ), polyline->isAnnotationOnly( ), 0 );
		}


		QtRegionSource::QtRegionSource( RegionCreator *rc, QtDisplayPanelGui *panel ) :
			RegionSource( rc, shared_kernel_ptr_type(new QtRegionSourceKernel(panel)) ) { }

		QtRegionSource::QtRegionSource( RegionCreator *rc, QtDisplayPanelGui */*panel*/, const shared_kernel_ptr_type &kernel ) :
			RegionSource( rc, kernel ) { }

	}
}

