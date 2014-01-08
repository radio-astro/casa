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
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$


#include <display/region/Polyline.qo.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>

#include <display/DisplayDatas/PrincipalAxesDD.h>

#include <imageanalysis/Annotations/AnnPolyline.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <display/DisplayErrors.h>
#include <display/Slicer/SlicePlot.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/region/QtRegionState.qo.h>
#include <display/ds9/ds9writer.h>

#include <QtCore/qmath.h>

#include <tr1/memory>

namespace casa {
	namespace viewer {

		Polyline::Polyline( WorldCanvas *wc, QtRegionDock *d, const std::vector<std::pair<double,double> > &pts) :
			Region( "polyline", wc, d, new QtSliceCutState(QString("polyline") )),
			_ref_blc_x_(-1), _ref_blc_y_(-1), _ref_trc_x_(-1), _ref_trc_y_(-1),
			_drawing_blc_x_(-1), _drawing_blc_y_(-1), _drawing_trc_x_(-1), _drawing_trc_y_(-1) {
			for ( size_t i=0; i < pts.size(); ++i ) {
				_ref_points_.push_back(pt(pts[i].first,pts[i].second));
				_drawing_points_.push_back(pt(pts[i].first,pts[i].second));
			}
			initPlot();
			complete = true;
		}


// carry over from QtRegion... hopefully, removed soon...
		Polyline::Polyline( QtRegionSourceKernel *rs, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts, bool hold_signals ) :
			Region( "polyline", wc, rs->dock( ), hold_signals ), _ref_blc_x_(-1), _ref_blc_y_(-1), _ref_trc_x_(-1), _ref_trc_y_(-1),
			_drawing_blc_x_(-1), _drawing_blc_y_(-1), _drawing_trc_x_(-1), _drawing_trc_y_(-1) {
			for ( size_t i=0; i < pts.size(); ++i ) {
				_ref_points_.push_back(pt(pts[i].first,pts[i].second));
				_drawing_points_.push_back(pt(pts[i].first,pts[i].second));
			}
			initPlot();
			complete = true;
		}


		Polyline::Polyline( WorldCanvas *wc, QtRegionDock *d, double x1, double y1 ) :
			Region( "polyline", wc, d, new QtSliceCutState(QString("polyline"))),
			_ref_blc_x_(-1), _ref_blc_y_(-1), _ref_trc_x_(-1), _ref_trc_y_(-1),
			_drawing_blc_x_(-1), _drawing_blc_y_(-1), _drawing_trc_x_(-1), _drawing_trc_y_(-1) {
			_ref_points_.push_back(pt(x1,y1));
			_drawing_points_.push_back(pt(x1,y1));
			initPlot();
		}


		Polyline::Polyline( QtRegionSourceKernel *rs, WorldCanvas *wc, double x1, double y1, bool hold_signals) :
			Region( "polyline", wc, rs->dock( ), hold_signals, new QtSliceCutState(QString("polyline")) ),
			_ref_blc_x_(-1), _ref_blc_y_(-1), _ref_trc_x_(-1), _ref_trc_y_(-1),
			_drawing_blc_x_(-1), _drawing_blc_y_(-1), _drawing_trc_x_(-1), _drawing_trc_y_(-1) {
			mystate->setRegion(this);
			_ref_points_.push_back(pt(x1,y1));
			_drawing_points_.push_back(pt(x1,y1));
			initPlot();
		}

		RegionInfo * Polyline::newInfoObject( ImageInterface<Float>* image, PrincipalAxesDD * ) {
			SliceRegionInfo* sliceRegion = NULL;
			if ( image != NULL ) {
				RegionInfo::stats_t* dd_stats = new RegionInfo::stats_t();
				QString key( image->name(true).c_str());
				SlicePlot* slicePlot = NULL;
				if ( !slicePlots.contains(key)) {
					slicePlot = new SlicePlot();
					std::tr1::shared_ptr<ImageInterface<Float> > imagePtr( image );
					slicePlot->setImage( /*image*/imagePtr );
					setPlotLineColor( slicePlot );
					slicePlots.insert( key , slicePlot );
				}
				sliceRegion = new SliceRegionInfo( image->name(true), image->name(false), dd_stats );
			}
			return sliceRegion;
		}



		void Polyline::setPlotLineColor(SlicePlot* slicePlot ) {
			std::string sliceLineColor = lineColor();
			QString qtLineColor( sliceLineColor.c_str());
			slicePlot->setViewerCurveColor( this->id_, qtLineColor );
		}


		void Polyline::setPlotLineColor() {
			QList<QString> plotKeys = slicePlots.keys();
			for ( QList<QString>::iterator iter = plotKeys.begin(); iter != plotKeys.end(); iter++ ) {
				setPlotLineColor( slicePlots[*iter] );
			}
			emitUpdate();
		}

		void Polyline::initPlot() {
			SlicePlot* slicePlot = new SlicePlot();
			QString imageName;
			if ( wc_ != NULL  ) {
				DisplayData* dd = wc_->csMaster();
				if ( dd != NULL ) {
					std::tr1::shared_ptr<ImageInterface<float> > masterImage(dd->imageinterface());
					if ( masterImage != NULL ) {
						slicePlot->setImage( masterImage );
						imageName = masterImage->name(true).c_str();
					}
				}
			}
			slicePlots.insert( imageName, slicePlot );
			setPlotLineColor( slicePlot );
			connect( this, SIGNAL(regionUpdate( int, viewer::region::RegionChanges, const QList<double> &,
			                                    const QList<double>&,const QList<int> &, const QList<int> &)), this,
			         SLOT(polyLineRegionUpdate(  int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &,
			                                     const QList<int> &, const QList<int> & )));
			connect( this, SIGNAL(regionChange( viewer::Region *, std::string )), this,
			         SLOT(polyLineRegionChanged( viewer::Region*, std::string )));
			markerSegmentIndex = 0;
			markerPercentage = 0;
			showPositionMarker = false;
		}

		void Polyline::addPlot(QWidget* parent, string label) {
			QString imageLabel( label.c_str());
			SlicePlot* slicePlot = NULL;
			if ( slicePlots.contains( imageLabel )) {
				slicePlot = slicePlots[imageLabel];

				slicePlot->setParent( parent );
				QLayout* layout = new QHBoxLayout();
				layout->addWidget( slicePlot );
				parent->setLayout( layout );

				QList<int> pixelX;
				QList<int> pixelY;
				QList<double> worldX;
				QList<double> worldY;
				viewer::region::RegionTypes type;
				fetch_details( type, pixelX, pixelY, worldX, worldY );
				polyLineRegionUpdate( type, viewer::region::RegionChangeCreate,
				                      worldX, worldY, pixelX, pixelY);
			}

		}


		void Polyline::polyLineRegionUpdate(int regionId, viewer::region::RegionChanges change,
		                                    const QList<double> & worldX, const QList<double>& worldY,
		                                    const QList<int> & pixelX, const QList<int> & pixelY) {
			if ( change == viewer::region::RegionChangeNewChannel ) {
				int channelIndex = zIndex();
				QList<QString> keys = slicePlots.keys();
				for ( QList<QString>::iterator iter = keys.begin(); iter != keys.end();
				        iter++ ) {
					SlicePlot* slicePlot = slicePlots[*iter];
					slicePlot->updateChannel(channelIndex);
				}
			}
			updatePolyLine(regionId, change, worldX, worldY, pixelX, pixelY);
		}

		void Polyline::polyLineRegionChanged( viewer::Region* /*region*/, std::string changeType) {
			if ( changeType == this->state()->LINE_COLOR_CHANGE ) {
				setPlotLineColor();
			}
		}

		void Polyline::updatePolyLine(int regionId, viewer::region::RegionChanges changes,
		                              const QList<double> & worldX, const QList<double>& worldY,
		                              const QList<int> & pixelX, const QList<int> & pixelY) {
			QList<double> linearX = worldX;
			QList<QString> keys = slicePlots.keys();
			for ( QList<QString>::iterator iter = keys.begin(); iter != keys.end();
			        iter++ ) {
				slicePlots[*iter]->updatePolyLine( regionId, changes, worldX, worldY, pixelX, pixelY );

			}
		}

		void Polyline::addVertex( double x, double y, bool rewrite_last_point ) {
			if ( rewrite_last_point == false ) {
				_ref_points_.push_back(pt(x,y));
				_drawing_points_.push_back(pt(x,y));
			} else {
				if ( _ref_points_.size( ) < 1 || _drawing_points_.size( ) < 1 )
					throw internal_error("polyline inconsistency");
				_ref_points_[_ref_points_.size( ) - 1] = pt(x,y);
				_drawing_points_[_drawing_points_.size( ) - 1] = pt(x,y);
			}
			update_drawing_bounds_rectangle( );
		}

		void Polyline::setCenter(double &x, double &y, double &deltx, double &delty) {
			_center_x=x;
			_center_y=y;
			_center_delta_x=deltx;
			_center_delta_y=delty;
		}

		void Polyline::polylineComplete( ) {
			// for polylines signals remain blocked until polyline is complete...
			//Check the number of points for some reason, we are getting duplicate
			//last points.
			int drawingPointCount = _drawing_points_.size();
			if ( drawingPointCount >= 2 ) {
				if (_drawing_points_[drawingPointCount - 1] == _drawing_points_[drawingPointCount - 2 ] ) {
					_drawing_points_.pop_back();
					_ref_points_.pop_back();
				}
			}
			complete = true;
			releaseSignals( );
			updateStateInfo( true, region::RegionChangeModified );
		}

		void Polyline::move( double dx, double dy ) {
			if ( _drawing_points_.size( ) == 0 ) return;

			_drawing_points_[0].first += dx;
			_drawing_points_[0].second += dy;
			_drawing_blc_x_ = _drawing_trc_x_ = _drawing_points_[0].first;
			_drawing_blc_y_ = _drawing_trc_y_ = _drawing_points_[0].second;

			for ( unsigned int i=1; i < _drawing_points_.size( ); ++i ) {
				_drawing_points_[i].first += dx;
				_drawing_points_[i].second += dy;

				// moving the bounding box will be required for redrawing,
				// so update as we move the points...
				if ( _drawing_points_[i].first < _drawing_blc_x_ )
					_drawing_blc_x_ = _drawing_points_[i].first;
				if ( _drawing_points_[i].first > _drawing_trc_x_ )
					_drawing_trc_x_ = _drawing_points_[i].first;

				if ( _drawing_points_[i].second < _drawing_blc_y_ )
					_drawing_blc_y_ = _drawing_points_[i].second;
				if ( _drawing_points_[i].second > _drawing_trc_y_ )
					_drawing_trc_y_ = _drawing_points_[i].second;
			}

			updateStateInfo( true, region::RegionChangeModified );
			setDrawCenter(false);
			invalidateCenterInfo();
		}

		bool Polyline::within_vertex_handle( double x, double y ) const {
			double half_handle_delta_x = handle_delta_x / 2.0;
			double half_handle_delta_y = handle_delta_y / 2.0;
			bool inHandle = false;
			for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
				if ( x >= (_drawing_points_[i].first - half_handle_delta_x) &&
				        x <= (_drawing_points_[i].first + half_handle_delta_x) &&
				        y >= (_drawing_points_[i].second - half_handle_delta_y) &&
				        y <= (_drawing_points_[i].second + half_handle_delta_y) ) {
					inHandle = true;
					break;
				}
			}
			return inHandle;
		}

// returns point state (Region::PointLocation)
		region::PointInfo Polyline::checkPoint( double x, double y )  const {
			region::PointInfo pointInfo(x, y, region::PointOutside);
			if ( complete != false ) {
				unsigned int result = 0;
				double blc_x, blc_y, trc_x, trc_y;
				boundingRectangle( blc_x, blc_y, trc_x, trc_y );
				if ( x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y )
					result |= region::PointInside;
				unsigned int handle = check_handle( x, y );
				if ( handle )
					result |= region::PointHandle;
				if ( result != 0 ) {
					pointInfo = region::PointInfo( x,y, result, handle);
				} else {
					pointInfo = region::PointInfo( x,y, (unsigned int)region::PointOutside, handle);
				}
			}
			return pointInfo;

		}

// returns mouse movement state
		unsigned int Polyline::mouseMovement( double x, double y, bool other_selected ) {
			unsigned int result = 0;

			if ( visible_ == false ) return result;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );
			if ( x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y || within_vertex_handle( x, y ) ) {
				weaklySelect( mouse_in_region == false );
				mouse_in_region = true;
				result |= region::MouseSelected;
				result |= region::MouseRefresh;
				selected_ = true;
				draw( other_selected );		// this draw may not be necessary (?)...
				if ( other_selected == false ) {
					// mark flag as this is the region (how to mix in other shapes)
					// of interest for statistics updates...
					selectedInCanvas( );
				}
			} else if ( selected_ == true ) {
				weaklyUnselect( );
				mouse_in_region = false;
				if ( selected_ == true ) {
					selected_ = false;
					draw( other_selected );
					result |= region::MouseRefresh;
				}
			}
			return result;
		}

		bool Polyline::clickWithin( double x, double y ) const {
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );
			return x >= blc_x && x <= trc_x && y >= blc_y && y <= trc_y;
		}


		void Polyline::output( ds9writer &out ) const {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;
			std::string path = QtDisplayData::path(wc_->csMaster());
			out.setCsysSource(path.c_str( ));
			out.polyline(wc_,drawing_points( ));
		}

		unsigned int Polyline::check_handle( double x, double y ) const {
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );
			if ( visible_ == false ) return 0;
			bool blc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= blc_y && y <= (blc_y + handle_delta_y);
			bool tlc = x >= blc_x && x <= (blc_x + handle_delta_x) && y >= (trc_y - handle_delta_y) && y <= trc_y;
			bool brc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= blc_y && y <= (blc_y + handle_delta_y);
			bool trc = x >= (trc_x - handle_delta_x) && x <= trc_x && y >= (trc_y - handle_delta_y) && y <= trc_y;

			// state for resizing (upon click & moving within a handle)...
			// may not be needed with the new RegionToolManager... or might be...
			if ( blc ) {
				((Polyline*)this)->_x_origin_ = ScaleRight;
				((Polyline*)this)->_y_origin_ = ScaleTop;
			} else if ( tlc ) {
				((Polyline*)this)->_x_origin_ = ScaleRight;
				((Polyline*)this)->_y_origin_ = ScaleBottom;
			} else if ( brc ) {
				((Polyline*)this)->_x_origin_ = ScaleLeft;
				((Polyline*)this)->_y_origin_ = ScaleTop;
			} else if ( trc ) {
				((Polyline*)this)->_x_origin_ = ScaleLeft;
				((Polyline*)this)->_y_origin_ = ScaleBottom;
			}

			if ( blc || tlc || brc || trc )
				return trc ? 1 : brc ? 2 : blc ? 3 : tlc ? 4 : 0;

			double half_handle_delta_x = handle_delta_x / 2.0;
			double half_handle_delta_y = handle_delta_y / 2.0;
			for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
				if ( x >= (_drawing_points_[i].first - half_handle_delta_x) &&
				        x <= (_drawing_points_[i].first + half_handle_delta_x) &&
				        y >= (_drawing_points_[i].second - half_handle_delta_y) &&
				        y <= (_drawing_points_[i].second + half_handle_delta_y) )
					return i + 5;
			}

			return 0;
		}

		int Polyline::clickHandle( double x, double y ) const {
			return check_handle( x, y );
		}

		bool Polyline::valid_translation( double dx, double dy, double width_delta, double height_delta ) {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return false;

			const double lxmin = wc_->linXMin( );
			const double lxmax = wc_->linXMax( );
			const double lymin = wc_->linYMin( );
			const double lymax = wc_->linYMax( );

			const double x_delta = width_delta  / 2.0;
			const double y_delta = height_delta  / 2.0;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			double pt = blc_x + dx - x_delta;
			if ( pt < lxmin || pt > lxmax ) {
				return false;
			}
			pt = trc_x + dx + x_delta;
			if ( pt < lxmin || pt > lxmax ) {
				return false;
			}
			pt = blc_y + dy - y_delta;
			if ( pt < lymin || pt > lymax ) {
				return false;
			}
			pt = trc_y + dy + y_delta;
			if ( pt < lymin || pt > lymax ) {
				return false;
			}
			return true;
		}

		void Polyline::resize( double width_delta, double height_delta ) {
			double dx = width_delta / 2.0;
			double dy = height_delta / 2.0;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			blc_x -= dx;
			blc_y -= dy;
			trc_x += dx;
			trc_y += dy;

			update_reference_bounds_rectangle( );

			// set the drawing bounds to the new bounds...
			// update scaling
			_drawing_blc_x_ = blc_x;
			_drawing_blc_y_ = blc_y;
			_drawing_trc_x_ = trc_x;
			_drawing_trc_y_ = trc_y;

			update_drawing_state( );
			updateStateInfo( true, region::RegionChangeModified );
		}

		int Polyline::moveHandle( int handle, double x, double y ) {

			if ( handle >= 1 && handle <= 4 )
				handle = move_sizing_rectangle_handle( handle, x, y );
			else if ( handle >= 5 )
				handle = move_vertex( handle, x, y );
			else
				return 0;

			updateStateInfo( true, region::RegionChangeModified );
			setDrawCenter(false);
			invalidateCenterInfo();
			return handle;
		}

		int Polyline::move_vertex( int handle, double x, double y ) {
			int vertex = handle - 5;
			if ( vertex >= (int) _ref_points_.size( ) || vertex < 0 )
				return 0;

			// use current x & y scaling to translate drawing vertex
			// back to reference vertex...
			double x_scale = _ref_width_ / _drawing_width_;
			double y_scale = _ref_height_ / _drawing_height_;

			// moving a vertex can change drawing bounding rectangle...
			// calculate any adjustment...
			double new_blc_x = x, new_blc_y = y, new_trc_x = x, new_trc_y = y;
			for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
				if ( (int) i != vertex ) {
					if ( _drawing_points_[i].first < new_blc_x ) new_blc_x = _drawing_points_[i].first;
					if ( _drawing_points_[i].second < new_blc_y ) new_blc_y = _drawing_points_[i].second;
					if ( _drawing_points_[i].first > new_trc_x ) new_trc_x = _drawing_points_[i].first;
					if ( _drawing_points_[i].second > new_trc_y ) new_trc_y = _drawing_points_[i].second;
				}
			}

			// reflect any drawing bounding rectangle back to reference points...
			if ( new_blc_x != _drawing_blc_x_ ) {
				_ref_blc_x_ = _ref_blc_x_ + (new_blc_x - _drawing_blc_x_) * x_scale;
				_drawing_blc_x_ = new_blc_x;
			}
			if ( new_blc_y != _drawing_blc_y_ ) {
				_ref_blc_y_ = _ref_blc_y_ + (new_blc_y - _drawing_blc_y_) * y_scale;
				_drawing_blc_y_ = new_blc_y;
			}
			if ( new_trc_x != _drawing_trc_x_ ) {
				_ref_trc_x_ = _ref_trc_x_ + (new_trc_x - _drawing_trc_x_) * x_scale;
				_drawing_trc_x_ = new_trc_x;
			}
			if ( new_trc_y != _drawing_trc_y_ ) {
				_ref_trc_y_ = _ref_trc_y_ + (new_trc_y - _drawing_trc_y_) * y_scale;
				_drawing_trc_y_ = new_trc_y;
			}

			// update drawing rectangle for new scaling values...
			_drawing_blc_x_ = new_blc_x;
			_drawing_blc_y_ = new_blc_y;
			_drawing_trc_x_ = new_trc_x;
			_drawing_trc_y_ = new_trc_y;

			// use new scaling values to translate moved (drawing) vertex
			// back to reference vertex...
			x_scale = (_ref_trc_x_ - _ref_blc_x_) / (_drawing_trc_x_ - _drawing_blc_x_);
			y_scale = (_ref_trc_y_ - _ref_blc_y_) / (_drawing_trc_y_ - _drawing_blc_y_);

			// calculate offset between drawn image and reference image...
			double x_offset = _ref_blc_x_ - _drawing_blc_x_;
			double y_offset = _ref_blc_y_ - _drawing_blc_y_;

			// translate modified drawing vertex back to a reference vertex...
			_ref_points_[vertex] = pt( (x - _drawing_blc_x_) * x_scale + _drawing_blc_x_ + x_offset,
			                           (y - _drawing_blc_y_) * y_scale + _drawing_blc_y_ + y_offset );

			// update reference state...
			update_reference_bounds_rectangle( );

			// recalculate drawing vertices based upon updated reference vertices...
			update_drawing_state( );
			update_drawing_bounds_rectangle( );
			refresh_state_gui( ); /***updateStatistics***/

			return handle;
		}

		int Polyline::move_sizing_rectangle_handle( int handle, double x, double y ) {
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			int translation = 0;
			int new_handle = handle;

			switch ( handle ) {
			case 1:				// trc handle becomes:
				if ( x < blc_x ) {
					if ( y < blc_y ) {
						trc_x = blc_x;
						trc_y = blc_y;
						blc_x = x;
						blc_y = y;		// blc...
						translation = FLIP_X | FLIP_Y;
						new_handle = 3;		// flip-x, flip-y
					} else {
						trc_x = blc_x;
						trc_y = y;
						blc_x = x;		// tlc...
						translation = FLIP_X;
						new_handle = 4;		// flip-x (x values change)
					}
				} else if ( y < blc_y ) {
					trc_y = blc_y;
					blc_y = y;			// brc...
					translation = FLIP_Y;
					new_handle = 2;		// flip-y (y values change)
				} else {
					trc_x = x;
					trc_y = y;
				}
				break;
			case 2:				// brc handle becomes:
				if ( x < blc_x ) {
					if ( y > trc_y ) {
						blc_y = trc_y;
						trc_x = blc_x;
						trc_y = y;
						blc_x = x;		// tlc...
						translation = FLIP_X | FLIP_Y;
						new_handle = 4;		// flip-x, flip-y
					} else {
						trc_x = blc_x;
						blc_x = x;
						blc_y = y;		// blc...
						translation = FLIP_X;
						new_handle = 3;		// flip-x
					}
				} else if ( y > trc_y ) {
					blc_y = trc_y;
					trc_x = x;
					trc_y = y;			// trc...
					translation = FLIP_Y;
					new_handle = 1;		// flip-y
				} else {
					trc_x = x;
					blc_y = y;
				}
				break;
			case 3:				// blc handle becomes:
				if ( x > trc_x ) {
					if ( y > trc_y ) {
						blc_x = trc_x;
						blc_y = trc_y;
						trc_x = x;
						trc_y = y;		// trc...
						translation = FLIP_X | FLIP_Y;
						new_handle = 1;		// flip-x, flip-y
					} else {
						blc_x = trc_x;
						trc_x = x;
						blc_y = y;		// brc...
						translation = FLIP_X;
						new_handle = 2;		// flip-x
					}
				} else if ( y > trc_y ) {
					blc_y = trc_y;
					blc_x = x;
					trc_y = y;			// tlc...
					translation = FLIP_Y;
					new_handle = 4;		// flip-y
				} else {
					blc_x = x;
					blc_y = y;
				}
				break;
			case 4:				// tlc handle becomes:
				if ( x > trc_x ) {
					if ( y < blc_y ) {
						blc_x = trc_x;
						trc_y = blc_y;
						blc_y = y;
						trc_x = x;		// brc...
						translation = FLIP_X | FLIP_Y;
						new_handle = 2;		// flip-x, flip-y
					} else {
						blc_x = trc_x;
						trc_x = x;
						trc_y = y;		// trc...
						translation = FLIP_X;
						new_handle = 1;		// flip-x
					}
				} else if ( y < blc_y ) {
					trc_y = blc_y;
					blc_x = x;
					blc_y = y;			// blc...
					translation = FLIP_Y;
					new_handle = 3;		// flip-y
				} else {
					blc_x = x;
					trc_y = y;
				}
				break;
			}

			if ( blc_x > trc_x || blc_y > trc_y ) throw internal_error("rectangle inconsistency");

			update_reference_state(  translation, handle, new_handle );

			// set the drawing bounds to the new bounds...
			// update scaling
			_drawing_blc_x_ = blc_x;
			_drawing_blc_y_ = blc_y;
			_drawing_trc_x_ = trc_x;
			_drawing_trc_y_ = trc_y;

			update_drawing_state( );
			refresh_state_gui( ); /***updateStatistics***/

			return new_handle;
		}

		void Polyline::linearCenter( double &x, double &y ) const {
			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			x = linear_average(blc_x,trc_x);
			y = linear_average(blc_y,trc_y);
		}

		void Polyline::pixelCenter( double &x, double &y ) const {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			double blc_x, blc_y, trc_x, trc_y;
			boundingRectangle( blc_x, blc_y, trc_x, trc_y );

			double lx = linear_average(blc_x,trc_x);
			double ly = linear_average(blc_y,trc_y);

			try {
				linear_to_pixel( wc_, lx, ly, x, y );
			} catch(...) {
				return;
			}
		}

		void Polyline::drawRegion( bool selected ) {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			PixelCanvas *pc = wc_->pixelCanvas();
			if(pc==0) return;

			if ( _drawing_points_.size( ) == 0 ) return;

			int x1, y1, x2, y2;
			try {
				linear_to_screen( wc_, _drawing_points_[0].first, _drawing_points_[0].second, x1, y1 );
			} catch(...) {
				return;
			}

			// draw the center
			if (getDrawCenter())
				drawCenter( _center_x, _center_y, _center_delta_x, _center_delta_y);

			for ( unsigned int i=1; i < _drawing_points_.size( ); ++i ) {
				try {
					linear_to_screen( wc_, _drawing_points_[i].first, _drawing_points_[i].second, x2, y2 );
				} catch(...) {
					return;
				}
				pc->drawLine(x1,y1,x2,y2);
				x1 = x2;
				y1 = y2;
			}

			if ( selected && memory::nullptr.check( creating_region ) ) {

				// get bounding rectangle...
				double blc_x, blc_y, trc_x, trc_y;
				boundingRectangle( blc_x, blc_y, trc_x, trc_y );
				try {
					linear_to_screen( wc_, blc_x, blc_y, trc_x, trc_y, x1, y1, x2, y2 );
				} catch(...) {
					return;
				}

				// compute handle size...
				Int w = x2 - x1;
				Int h = y2 - y1;

				Int s = 0;
				if (w>=18 && h>=18) s = 6;
				else if (w>=15 && h>=15) s = 5;
				else if (w>=12 && h>=12) s = 4;
				else if (w>=9 && h>=9) s = 3;

				// get handle size in linear coordinates...
				double xdx, ydy;
				try {
					screen_to_linear( wc_, x1 + s, y1 + s, xdx, ydy );
				} catch(...) {
					return;
				}
				handle_delta_x = xdx - blc_x;
				handle_delta_y = ydy - blc_y;

				// draw outline rectangle for resizing whole polyline...
				pushDrawingEnv(region::DotLine);
				pc->drawRectangle( x1, y1, x2, y2 );
				popDrawingEnv( );

				// get resizing rectangle handle bounding values...
				int hx0 = x1;
				int hx1 = x1 + s;
				int hx2 = x2 - s;
				int hx3 = x2;
				int hy0 = y1;
				int hy1 = y1 + s;
				int hy2 = y2 - s;
				int hy3 = y2;	// set handle coordinates

				if (s) {
					// draw handles of outline rectangle for resizing whole polyline...
					pushDrawingEnv( region::SolidLine);
					if ( weaklySelected( ) ) {
						if ( marked_region_count( ) > 0 && mouse_in_region ) {
							pc->drawRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
							pc->drawRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
							pc->drawRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
							pc->drawRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
						} else {
							pc->drawFilledRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
							pc->drawFilledRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
							pc->drawFilledRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
							pc->drawFilledRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
						}

						for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
							int h_blc_x, h_blc_y, h_trc_x, h_trc_y;
							try {
								linear_to_screen( wc_, _drawing_points_[i].first - handle_delta_x / 2.0,
								                  _drawing_points_[i].second - handle_delta_y / 2.0,
								                  _drawing_points_[i].first + handle_delta_x / 2.0,
								                  _drawing_points_[i].second + handle_delta_y / 2.0,
								                  h_blc_x, h_blc_y, h_trc_x, h_trc_y );
							} catch(...) {
								return;
							}

							pc->drawFilledRectangle( h_blc_x, h_blc_y, h_trc_x, h_trc_y );
						}

					} else if ( marked( ) ) {
						pc->drawRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
						pc->drawRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
						pc->drawRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
						pc->drawRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);
					} else {
						pc->drawFilledRectangle(hx0, hy0 - 0, hx1 + 0, hy1 + 0);
						pc->drawFilledRectangle(hx2, hy0 - 0, hx3 + 0, hy1 + 0);
						pc->drawFilledRectangle(hx0, hy2 - 0, hx1 + 0, hy3 + 0);
						pc->drawFilledRectangle(hx2, hy2 - 0, hx3 + 0, hy3 + 0);

						for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
							int h_blc_x, h_blc_y, h_trc_x, h_trc_y;
							try {
								linear_to_screen( wc_, _drawing_points_[i].first - handle_delta_x / 2.0,
								                  _drawing_points_[i].second - handle_delta_y / 2.0,
								                  _drawing_points_[i].first + handle_delta_x / 2.0,
								                  _drawing_points_[i].second + handle_delta_y / 2.0,
								                  h_blc_x, h_blc_y, h_trc_x, h_trc_y );
							} catch(...) {
								return;
							}

							pc->drawFilledRectangle( h_blc_x, h_blc_y, h_trc_x, h_trc_y );
						}
					}

					popDrawingEnv( );
				}
			}
		}

		void Polyline::drawText() {
			Region::drawText();

			int drawCount = _drawing_points_.size();
			if ( drawCount >= 2 ) {
				if (!this->complete ) {

					Vector<Double> world1(2), world2(2);
					Vector<Double> pix1(2),   pix2(2);
					Vector<Double> diff(2);
					Double allDiff = 0;
					String unit("");

					// get the position of the start- and end-points
					pix1(0) = (Double)_drawing_points_[drawCount-2].first;
					pix1(1) = (Double)_drawing_points_[drawCount-2].second;
					pix2(0) = (Double)_drawing_points_[drawCount-1].first;
					pix2(1) = (Double)_drawing_points_[drawCount-1].second;

					// determine the positions in world coordinates,
					// we will skip drawing any text if we can't do this.

					if ( wc_->linToWorld( world1, pix1 ) && wc_ ->linToWorld( world2, pix2) ) {

						// pixToWorld sometimes seems to add a dimension
						Vector<Double> world3(world2.size(), 0.0);
						Vector<Double>   pix3(world2.size(), 0.0);

						// get the corner point in world-coordinates
						world3(0) = world1(0);
						world3(1) = world2(1);
						for (Int index=2; index<(Int)world2.size(); index++) {
							world3(index) = world2(index);
						}

						// get the corner point in pixel-coordinates
						if (wc_->worldToPix(pix3, world3)) {

							// extract the axis names and units
							Vector<String> aXisNames=wc_->worldAxisNames();
							Vector<String> unitNames=wc_->worldAxisUnits();

							// identify RA and DEC axis
							int itsRaIndex = -1;
							int itsDecIndex = -1;
							for (Int index=0; index < (Int)aXisNames.size(); index++) {
								if (aXisNames(index).contains("scension") && (index < 2))
									itsRaIndex=index;
								if (aXisNames(index).contains("eclination") && (index < 2))
									itsDecIndex=index;
							}

							diff(0) = fabs(world1(0)-world2(0));
							diff(1) = fabs(world1(1)-world2(1));
							if (itsRaIndex > -1 && itsDecIndex > -1) {
								diff(0) = diff(0)*3600.0*180.0/C::pi;
								diff(1) = diff(1)*3600.0*180.0/C::pi;
								diff(itsRaIndex) = diff(itsRaIndex) * cos(world3(itsDecIndex));
								unit = "\"";
							}
							allDiff = sqrt(diff(0)*diff(0) + diff(1)*diff(1));

							ostringstream ss;
							Vector<double> textPosition(2);
							try {
								double x1;
								double y1;
								viewer::linear_to_world( wc_, _drawing_points_[drawCount-1].first,
								                         _drawing_points_[drawCount-1].second, x1, y1 );
								textPosition[0] = x1;
								textPosition[1] = y1;
							} catch(...) {
								return;
							}

							double angle = 0.0;
							if ( allDiff > 0 ) {
								double xDistance = pix2(0) - pix1(0);
								double yDistance = pix2(1) - pix1(1);
								angle = qAsin( diff(0) / allDiff );
								if ( yDistance * xDistance > 0 ) {
									angle = C::pi-angle;
								}
								angle = angle * 180 / 3.14159;
							}
							ss <<  std::setiosflags(ios::scientific) <<
							   std::setiosflags(ios::fixed) << std::setprecision(4) << "(" <<
							   allDiff << unit <<", "<<angle<<" deg)";
							String dText(ss.str());
							wc_->drawText(textPosition, dText, Display::AlignCenter,  False);
						}
					}
				} else {
					drawPositionMarker();
				}
			}
		}


		void Polyline::drawPositionMarker() {
			if ( showPositionMarker ) {
				try {
					int segmentCount = _drawing_points_.size() - 1;
					if ( markerSegmentIndex < segmentCount ) {
						double x0 = _drawing_points_[markerSegmentIndex].first;
						double y0 = _drawing_points_[markerSegmentIndex].second;
						double pixX0;
						double pixY0;
						viewer::linear_to_pixel( wc_, x0, y0, pixX0, pixY0 );

						double x1 = _drawing_points_[markerSegmentIndex+1].first;
						double y1 = _drawing_points_[markerSegmentIndex+1].second;
						double pixX1;
						double pixY1;
						viewer::linear_to_pixel( wc_, x1, y1, pixX1, pixY1 );

						double xDistance = pixX1 - pixX0;
						double x = pixX0 + markerPercentage * xDistance;
						double yDistance = pixY1 - pixY0;
						double y = pixY0 + markerPercentage * yDistance;
						double markerX = 0;
						double markerY = 0;
						viewer::pixel_to_world( wc_, x, y, markerX, markerY );
						Vector<Float> px(1);
						px[0] = markerX;
						Vector<Float> py(1);
						py[0] = markerY;
						wc_->drawMarkers( px, py, Display::Cross, 10  );
					}
				} catch(...) {
					return;
				}
			}
		}

		void Polyline::setMarkerPosition( int /*regionId*/, int segmentIndex, float percentage ) {
			markerSegmentIndex = segmentIndex;
			markerPercentage = percentage;
			this->refresh();
		}

		void Polyline::setShowMarkerPosition( int /*regionId*/, bool show ) {
			if ( show != showPositionMarker ) {
				showPositionMarker = show;
				refresh();
			}
		}

		AnnotationBase *Polyline::annotation( ) const {
			if ( wc_ == 0 || wc_->csMaster() == 0 ) return 0;

			const DisplayCoordinateSystem &cs = wc_->coordinateSystem( );
			const Vector<String> &units = wc_->worldAxisUnits( );

			Vector<Quantity> xv(_drawing_points_.size( ));
			Vector<Quantity> yv(_drawing_points_.size( ));
			for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
				double x,y;
				try {
					linear_to_world( wc_, _drawing_points_[i].first, _drawing_points_[i].second, x, y );
				} catch(...) {
					return 0;
				}
				xv[i] = Quantity(x,units[0]);
				yv[i] = Quantity(y,units[1]);
			}

			Vector<Stokes::StokesTypes> stokes;
			/*Int polaxis =*/
			CoordinateUtil::findStokesAxis(stokes, cs);
			const DisplayData *dd = wc_->displaylist().front();

			AnnPolyline *poly = 0;
			try {
				std::vector<int> axes = dd->displayAxes( );
				IPosition shape(cs.nPixelAxes( ));
				for ( size_t i=0; i < shape.size( ); ++i )
					shape(i) = dd->dataShape( )[axes[i]];
				poly = new AnnPolyline( xv, yv, cs, shape, stokes );
			} catch ( AipsError &e ) {
				cerr << "Error encountered creating an AnnPolyline:" << endl;
				cerr << "\t\"" << e.getMesg( ) << "\"" << endl;
			} catch ( ... ) {
				cerr << "Error encountered creating an AnnPolyline..." << endl;
			}

			return poly;
		}

// return the *drawing* bounding rectangle...
		void Polyline::boundingRectangle( double &blcx, double &blcy, double &trcx, double &trcy ) const {
			blcx = _drawing_blc_x_;
			blcy = _drawing_blc_y_;
			trcx = _drawing_trc_x_;
			trcy = _drawing_trc_y_;
		}

		void Polyline::fetch_region_details( region::RegionTypes &type, std::vector<std::pair<int,int> > &pixel_pts,
		                                     std::vector<std::pair<double,double> > &world_pts ) const {

			if ( wc_ == 0 || wc_->csMaster() == 0 ) return;

			type = region::PolylineRegion;

			pixel_pts.resize(_drawing_points_.size( ));
			world_pts.resize(_drawing_points_.size( ));

			for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {

				double wx, wy;
				try {
					linear_to_world( wc_, _drawing_points_[i].first, _drawing_points_[i].second, wx, wy );
				} catch(...) {
					return;
				}
				world_pts[i].first = wx;
				world_pts[i].second = wy;

				double px, py;
				try {
					linear_to_pixel( wc_, _drawing_points_[i].first, _drawing_points_[i].second, px, py );
				} catch(...) {
					return;
				}
				pixel_pts[i].first = static_cast<int>(px);
				pixel_pts[i].second = static_cast<int>(py);
			}
		}

		void Polyline::update_drawing_bounds_rectangle( ) {

			_drawing_blc_x_ = _drawing_points_[0].first;
			_drawing_blc_y_ = _drawing_points_[0].second;
			_drawing_trc_x_ = _drawing_points_[0].first;
			_drawing_trc_y_ = _drawing_points_[0].second;

			for ( unsigned int i=1; i < _drawing_points_.size( ); ++i ) {
				if ( _drawing_points_[i].first < _drawing_blc_x_ ) _drawing_blc_x_ = _drawing_points_[i].first;
				if ( _drawing_points_[i].second < _drawing_blc_y_ ) _drawing_blc_y_ = _drawing_points_[i].second;
				if ( _drawing_points_[i].first > _drawing_trc_x_ ) _drawing_trc_x_ = _drawing_points_[i].first;
				if ( _drawing_points_[i].second > _drawing_trc_y_ ) _drawing_trc_y_ = _drawing_points_[i].second;
			}
			_drawing_width_ = _drawing_trc_x_ - _drawing_blc_x_;
			_drawing_height_ = _drawing_trc_y_ - _drawing_blc_y_;
		}

		void Polyline::update_reference_bounds_rectangle( ) {
			_ref_blc_x_ = _ref_points_[0].first;
			_ref_blc_y_ = _ref_points_[0].second;
			_ref_trc_x_ = _ref_points_[0].first;
			_ref_trc_y_ = _ref_points_[0].second;

			for ( unsigned int i=1; i < _ref_points_.size( ); ++i ) {
				if ( _ref_points_[i].first < _ref_blc_x_ ) _ref_blc_x_ = _ref_points_[i].first;
				if ( _ref_points_[i].second < _ref_blc_y_ ) _ref_blc_y_ = _ref_points_[i].second;
				if ( _ref_points_[i].first > _ref_trc_x_ ) _ref_trc_x_ = _ref_points_[i].first;
				if ( _ref_points_[i].second > _ref_trc_y_ ) _ref_trc_y_ = _ref_points_[i].second;
			}
			_ref_width_ = _ref_trc_x_ - _ref_blc_x_;
			_ref_height_ = _ref_trc_y_ - _ref_blc_y_;
		}

		void Polyline::update_drawing_state( ) {
			_drawing_width_ = _drawing_trc_x_ - _drawing_blc_x_;
			_drawing_height_ = _drawing_trc_y_ - _drawing_blc_y_;

			double x_scale = _drawing_width_ / _ref_width_;
			double y_scale = _drawing_height_ / _ref_height_;

			double x_offset = _drawing_blc_x_ - _ref_blc_x_;
			double y_offset = _drawing_blc_y_ - _ref_blc_y_;

			for ( unsigned int i=0; i < _drawing_points_.size( ); ++i ) {
				_drawing_points_[i] = pt( (_ref_points_[i].first - _ref_blc_x_) * x_scale + _ref_blc_x_ + x_offset,
				                          (_ref_points_[i].second - _ref_blc_y_) * y_scale + _ref_blc_y_ + y_offset );
			}
		}

		void Polyline::update_reference_state(  int transformations, int handle, int new_handle ) {
			// transform reference image to match the new drawing image orientation...
			if ( transformations & FLIP_X || transformations & FLIP_Y ) {
				for ( unsigned int i=0; i < _ref_points_.size( ); ++i ) {
					// (1) translate to the proper axis
					// (2) flip
					// (3) translate back
					if ( transformations & FLIP_X ) {
						if ( handle == 1 || handle == 2 ) {		// right outline handles
							_ref_points_[i].first = (_ref_points_[i].first - _ref_blc_x_) * -1.0 + _ref_blc_x_;
							// _x_origin_ = ScaleLeft;
						} else {					// left outline handles
							_ref_points_[i].first = (_ref_points_[i].first - _ref_trc_x_) * -1.0 + _ref_trc_x_;
							// _x_origin_ = ScaleRight;
						}
					}
					if ( transformations & FLIP_Y ) {
						if ( handle == 1 || handle == 4 ) {		// top outline handles
							_ref_points_[i].second = (_ref_points_[i].second - _ref_blc_y_) * -1.0 + _ref_blc_y_;
							// _y_origin_ = ScaleBottom;
						} else {					// bottom outline handles
							_ref_points_[i].second = (_ref_points_[i].second - _ref_trc_y_) * -1.0 + _ref_trc_y_;
							// _y_origin_ = ScaleTop;
						}
					}
				}
			}

			// update reference bounds, width & height... just in case...
			update_reference_bounds_rectangle( );

			// reflection (as a result of resizing) happens to the reference points...
			if ( new_handle == 1 ) {
				_x_origin_ = ScaleLeft;
				_y_origin_ = ScaleBottom;
			} else if ( new_handle == 2 ) {
				_x_origin_ = ScaleLeft;
				_y_origin_ = ScaleTop;
			} else if ( new_handle == 3 ) {
				_x_origin_ = ScaleRight;
				_y_origin_ = ScaleTop;
			} else if ( new_handle == 4 ) {
				_x_origin_ = ScaleRight;
				_y_origin_ = ScaleBottom;
			}

		}

		std::list<std::tr1::shared_ptr<RegionInfo> > *Polyline::generate_dds_centers() {
			std::list<std::tr1::shared_ptr<RegionInfo> > *region_centers = new std::list<std::tr1::shared_ptr<RegionInfo> >( );

			if( wc_==0 ) return region_centers;

			Int zindex = 0;
			if (wc_->restrictionBuffer()->exists("zIndex")) {
				wc_->restrictionBuffer()->getValue("zIndex", zindex);
			}

			DisplayData *dd = 0;
			const std::list<DisplayData*> &dds = wc_->displaylist( );
			Vector<Double> lin(2), wld(2);

			Vector<Double> x(_drawing_points_.size( ));
			Vector<Double> y(_drawing_points_.size( ));
			for ( unsigned int i = 0; i < _drawing_points_.size( ); ++i ) {
				lin(0) = _drawing_points_[i].first;
				lin(1) = _drawing_points_[i].second;
				if ( ! wc_->linToWorld(wld, lin)) return region_centers;
				x[i] = wld[0];
				y[i] = wld[1];
			}

			std::string errMsg_;
			std::map<String,bool> processed;
			for ( std::list<DisplayData*>::const_iterator ddi=dds.begin(); ddi != dds.end(); ++ddi ) {
				dd = *ddi;

				PrincipalAxesDD* padd = dynamic_cast<PrincipalAxesDD*>(dd);
				if (padd==0) continue;

				try {
					if ( ! padd->conformsTo(*wc_) ) continue;

					std::tr1::shared_ptr<ImageInterface<Float> > image( padd->imageinterface( ));

					if ( ! image ) continue;

					String description = image->name(false);
					String name = image->name(true);
					std::map<String,bool>::iterator repeat = processed.find(description);
					if (repeat != processed.end()) continue;
					processed.insert(std::map<String,bool>::value_type(description,true));

					RegionInfo::center_t *layercenter = new RegionInfo::center_t( );
					region_centers->push_back(std::tr1::shared_ptr<RegionInfo>(new SliceRegionInfo(name,description,layercenter)));
				} catch (const casa::AipsError& err) {
					errMsg_ = err.getMesg();
					fprintf( stderr, "Polyline::generate_dds_centers( ): %s\n", errMsg_.c_str() );
					continue;
				} catch (...) {
					errMsg_ = "Unknown error converting region";
					fprintf( stderr, "Polyline::generate_dds_centers( ): %s\n", errMsg_.c_str() );
					continue;
				}
			}
			return region_centers;
		}

		Polyline::~Polyline() {
			QList<QString> keys = slicePlots.keys();
			for ( QList<QString>::iterator iter = keys.begin(); iter != keys.end();
			        iter++ ) {
				delete slicePlots[*iter];
				slicePlots[*iter] = NULL;
			}
		}

		ImageRegion *Polyline::get_image_region( DisplayData */*dd*/ ) const {

			return NULL;
		}

	}
}
