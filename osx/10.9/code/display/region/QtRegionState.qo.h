//# QtRegionState.h: region properties, populates region dock
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


#ifndef REGION_QTREGIONSTATE_H_
#define REGION_QTREGIONSTATE_H_

#include <casa/BasicSL/String.h>
#include <display/region/RegionEnums.h>
#include <display/region/QtRegionState.ui.h>
#include <display/region/QtRegionStats.qo.h>
#include <display/Display/MouseToolState.h>

namespace casa {

	class HistogramTab;
	namespace viewer {

		class Region;


		class QtRegionState : public QFrame, protected Ui::QtRegionState {
			Q_OBJECT
		public:
			// initializing the display depends upon having signals & slots connected
			// which cannot happen until after the ctor of QtRegionState...
			void init( );


			QtRegionState( const QString &name,
			               QtMouseToolNames::PointRegionSymbols sym=QtMouseToolNames::SYM_UNKNOWN,
			               Region *region=0, QWidget *parent=0 );
			~QtRegionState( );

			bool statisticsIsVisible( ) {
				return categories->tabText(categories->currentIndex( )) == "stats";
			}

			Region *region( ) {
				return region_;
			}
			void setRegion( Region *r );

			void updateCoord( );
			void updateStatistics(  );
			void updateStatistics( std::list<shared_ptr<RegionInfo> > *stats );
			void updateFrameInformation( int count );
			void reloadStatistics( );
			void updateCenters( std::list<shared_ptr<RegionInfo> > *centers );
			void setCenterBackground(QString background);
			void clearStatistics( );
			void addHistogram(QWidget* histogram);
			void updateStackOrder( int firstImage );
			int getStackIndex() const;

			std::string lineColor( ) const;
			std::string centerColor( ) const;
			int lineWidth( ) const {
				return line_width->value( );
			}
			region::LineStyle lineStyle( ) const;

			int markerScale( ) const {
				return marker_scale->value( );
			}
			void setMarkerScale( int v );

			std::string textColor( ) const;
			std::string textFont( ) const;
			int textFontSize( ) const {
				return font_size->value( );
			}
			int textFontStyle( ) const;
			std::string textValue( ) const;
			region::TextPosition textPosition( ) const;
			void textPositionDelta( int &x, int &y ) const;

			void setTextValue( const std::string &l );
			void setTextPosition( region::TextPosition );
			void setTextDelta( const std::vector<int> & );
			void setTextFont( const std::string &f );
			void setTextFontSize( int s );
			void setTextFontStyle( int s );
			void setTextColor( const std::string & );
			void setLineColor( const std::string & );
			void setLineStyle( region::LineStyle );
			void setLineWidth( unsigned int );
			void setAnnotation( bool );
			void disableAnnotation( bool );

			int zMin( ) const;
			int zMax( ) const;
			int numFrames( ) const;

			bool isAnnotation( ) const;

			// reset the widget to its original state...
			void reset( const QString &name, Region *r );

			/* QString getRegionCategory( ) const { return categories->tabText(categories->currentIndex( )); } */
			/* void justExposed( ); */

			void getCoordinatesAndUnits( region::Coord &c, region::Units &xu, region::Units &yu, std::string &bounding_units ) const;
			void setCoordinatesAndUnits( region::Coord c, region::Units x_units, region::Units y_units, const std::string &bounding_units );
			void updatePosition( const QString &x, const QString &y, const QString &angle,
			                     const QString &bounding_width, const QString &bounding_height );

			// may be called after "outputRegions" signal to notify the
			// user that no regions were selected for output...
			void noOutputNotify( );

			// functions added with the introduction of RegionToolManager and the
			// unified selection and manipulation of the various region types...
			/* void mark( bool set=true ) { region_mark->setCheckState( set ? Qt::Checked : Qt::Unchecked ); } */
			/* bool marked( ) const { return region_mark->checkState( ) == Qt::Checked ? true : false; } */
			void mark( bool set=true );
			bool marked( ) const {
				return region_mark->isChecked( );
			}
			bool markCenter( ) const {
				return (markcenter_chk->checkState()==Qt::Checked);
			}
			bool skyComponent() const {
				return (skycomp_chk->checkState()==Qt::Checked);
			};
			bool mark_toggle( );

			void nowVisible( );
			QPushButton *getFitButton() {
				return imfit_fit;
			};

			void emitRefresh( ) {
				emit refreshCanvas( );
			}

			// return the current information mode of the region state, e.g. position, statistics, etc.
			std::string mode( ) const;
			const std::string LINE_COLOR_CHANGE;

			virtual QString HISTOGRAM_MODE( ) const {
				return "Histogram";
			}
			virtual QString STATISTICS_MODE( ) const {
				return "Statistics";
			}
			virtual QString FILE_MODE( ) const {
				return "File";
			}
			virtual QString FIT_MODE( ) const {
				return "Fit";
			}
			virtual QString PROPERTIES_MODE( ) const {
				return "Properties";
			}

		signals:
			void regionChange( viewer::Region *, std::string );
			void refreshCanvas( );
			void statisticsVisible( bool );
			void collectStatistics( );
			void updateHistogram();
			void positionVisible( bool );
			void translateX( const QString &/*x*/, const QString &/*x_units*/, const QString &/*coordsys*/ );
			void translateY( const QString &/*y*/, const QString &/*y_units*/, const QString &/*coordsys*/ );
			void resizeX( const QString &/*x*/, const QString &/*x_units*/, const QString &/*coordsys*/ );
			void resizeY( const QString &/*y*/, const QString &/*y_units*/, const QString &/*coordsys*/ );

			void zRange( int z_min, int z_max );
			/* void regionCategoryChange( QString ); */
			void outputRegions( const QString &what, const QString &name, const QString &type, const QString &csys );
			void loadRegions( const QString &path, const QString &type );

		public slots:
			void stackChange(QWidget*);

		protected slots:
			// updates canvas with any line changes
			void state_change( int );
			void state_change_region_mark( int );
			void color_state_change( int );
			void state_change( bool );
			void state_change( const QString & );
			void states_change( int );
			void states_val_change( int );
			void coordsys_change( const QString &text );
			void translate_x( );
			void translate_y( );
			void resize_x( );
			void resize_y( );
			void category_change( int );
			void filetab_change( int );
			// keeps text color in sync with line color (if they were the same before)
			void line_color_change(const QString & );
			QString default_extension( const QString & );

			void update_default_file_extension(const QString&);
			void update_save_type(const QString &);
			void save_region(bool);

			void update_load_type(const QString &);
			void load_regions( bool );

			void frame_min_change(int);
			void frame_max_change(int);

			void save_browser(bool);
			void load_browser(bool);

			void set_point_region_marker( int );

		protected:
			// keep track of which set of statistics
			// where selected when refreshing all...
			int selected_statistics;
			typedef std::list<QtRegionStats*> freestat_list;
			static freestat_list *freestats;
			static freestat_list *freecenters;
			QString last_line_color;
			Region *region_;

			std::string bounding_index_to_string( int index ) const;

			friend class Region;
			void clearRegion( ) {
				region_ = 0;
			}

		private:
			void initRegionState( );
			void statisticsUpdate( QtRegionStats *regionStats, shared_ptr<casa::viewer::RegionInfo> regionInfo );
			unsigned int setting_combo_box;
			int pre_dd_change_statistics_count;
			HistogramTab* histogramTab;

		};

		class QtPVLineState : public QtRegionState {
			Q_OBJECT
		public:
			QtPVLineState( const QString &name,
			               QtMouseToolNames::PointRegionSymbols sym=QtMouseToolNames::SYM_UNKNOWN,
			               Region *region=0, QWidget *parent=0 );
			QString STATISTICS_MODE( ) const {
				return "pV";
			}

		};

		class QtSliceCutState : public QtRegionState {
			Q_OBJECT
		public:
			QtSliceCutState( const QString &name,
			                 QtMouseToolNames::PointRegionSymbols sym=QtMouseToolNames::SYM_UNKNOWN,
			                 Region *region=0, QWidget *parent=0 );
			QString STATISTICS_MODE( ) const {
				return "Spatial Profile";
			}

		};

		class QtPointState : public QtRegionState {
			Q_OBJECT
		public:
			QtPointState( const QString &name,
			               QtMouseToolNames::PointRegionSymbols sym=QtMouseToolNames::SYM_UNKNOWN,
			               Region *region=0, QWidget *parent=0 );
		};

	}
}

#endif
