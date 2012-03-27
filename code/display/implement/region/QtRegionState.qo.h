#ifndef REGION_QTREGIONSTATE_H_
#define REGION_QTREGIONSTATE_H_

#include <casa/BasicSL/String.h>
#include <display/region/Region.h>
#include <display/region/QtRegionState.ui.h>
#include <display/region/QtRegionStats.qo.h>

namespace casa {
    namespace viewer {

	class QtRegion;

	class QtRegionState : public QFrame, protected Ui::QtRegionState {
	    Q_OBJECT
	    public:
		// initializing the display depends upon having signals & slots connected
		// which cannot happen until after the ctor of QtRegionState...
		void init( );

		QtRegionState( const QString &name, QtRegion *region, QWidget *parent=0 );
		~QtRegionState( );

		void updateCoord( ) { coordinates_reset_event(true); }
		void updateStatistics( std::list<RegionInfo> *stats );
		void clearStatistics( );

		std::string lineColor( ) const;
		int lineWidth( ) const { return line_width->value( ); }
		Region::LineStyle lineStyle( ) const;

		std::string textColor( ) const;
		std::string textFont( ) const;
		int textFontSize( ) const { return font_size->value( ); }
		int textFontStyle( ) const;
		std::string textValue( ) const;
		Region::TextPosition textPosition( ) const;
		void textPositionDelta( int &x, int &y ) const;

		void setTextValue( const std::string &l );
		void setTextFont( const std::string &f );
		void setTextFontSize( int s );
		void setTextFontStyle( int s );
		void setTextColor( const std::string &c );
		void setLineColor( const std::string &c );
		void setLineStyle( Region::LineStyle s );
		void setAnnotation( bool );
		void disableAnnotation( bool );

		int zMin( ) const;
		int zMax( ) const;
		int numFrames( ) const;

		bool isAnnotation( ) const;

		// reset the widget to its original state...
		void reset( const QString &name, QtRegion *r );

		/* QString getRegionCategory( ) const { return categories->tabText(categories->currentIndex( )); } */
		void justExposed( );

		void getCoordinatesAndUnits( Region::Coord &c, Region::Units &xu, Region::Units &yu, std::string &bounding_units ) const;
		void setCoordinatesAndUnits( Region::Coord c, Region::Units x_units, Region::Units y_units, const std::string &bounding_units );
		void updatePosition( const QString &x, const QString &y, const QString &angle,
				     const QString &bounding_width, const QString &bounding_height );

		// may be called after "outputRegions" signal to notify the
		// user that no regions were selected for output...
		void noOutputNotify( );

		// functions added with the introduction of RegionToolManager and the
		// unified selection and manipulation of the various region types...
		/* void mark( bool set=true ) { region_mark->setCheckState( set ? Qt::Checked : Qt::Unchecked ); } */
		/* bool marked( ) const { return region_mark->checkState( ) == Qt::Checked ? true : false; } */
		void mark( bool set=true ) { region_mark->setChecked( set ); }
		bool marked( ) const { return region_mark->isChecked( ); }
		void mark_toggle( ) { region_mark->setChecked(region_mark->isChecked( ) ? false : true); }

		void nowVisible( );

	    signals:
		void refreshCanvas( );
		void statisticsVisible( bool );
		void positionVisible( bool );
		void positionMove( const QString &x,const QString &y,const QString &coord,const QString &x_units,const QString &y_units,
				   const QString &width, const QString &height, const QString &bounding_units );
		void zRange( int z_min, int z_max );
		/* void regionCategoryChange( QString ); */
		void outputRegions( const QString &what, const QString &name, const QString &type, const QString &csys );
		void loadRegions( bool &handled, const QString &path, const QString &type );

	    protected slots:
		// updates canvas with any line changes
		void state_change( int );
		void state_change( bool );
		void state_change( const QString & );
		void states_change( int );
		void states_val_change( int );
		void coordsys_change( const QString &text );
		void coordinates_reset_event(bool);
		void coordinates_apply_event(bool);
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

	    protected:
		// keep track of which set of statistics
		// where selected when refreshing all...
		int selected_statistics;
		typedef std::list<QtRegionStats*> freestat_list;
		static freestat_list *freestats;
		QString last_line_color;
		QtRegion *region_;

		QString last_load_directory;
		QString last_save_directory;

		std::string bounding_index_to_string( int index ) const;

	};
    }
}

#endif
