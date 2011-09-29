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
		QtRegionState( const QString &name, QtRegion *region, QWidget *parent=0 );
		~QtRegionState( );
#if OLDSTUFF
		void clearstats( );
		void addstats( const std::string &name, std::list<std::pair<String,String> > *stats );
#endif
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

		int zMin( ) const;
		int zMax( ) const;
		int numFrames( ) const;

		bool marked( ) const { return region_mark->isChecked( ); }

		// reset the widget to its original state...
		void reset( const QString &name, QtRegion *r );

		/* QString getRegionCategory( ) const { return categories->tabText(categories->currentIndex( )); } */
		void justExposed( );

		void getCoordinatesAndUnits( Region::Coord &c, Region::Units &u ) const;
		void setCoordinatesAndUnits( Region::Coord c, Region::Units u );
		void updatePosition( const QString &x, const QString &y, const QString &angle );

		// may be called after "outputRegion" signal to notify the
		// user that no regions were selected for output...
		void noOutputNotify( );

	    signals:
		void refreshCanvas( );
		void statisticsVisible( bool );
		void positionVisible( bool );
		void positionMove(const QString &x,const QString &y,const QString &coord,const QString &units);
		void zRange( int z_min, int z_max );
		/* void regionCategoryChange( QString ); */
		void outputRegion( const QString &what, const QString &name );

	    protected slots:
		// updates canvas with any line changes
		void state_change( int );
		void state_change( bool );
		void state_change( const QString & );
		void states_change( int );
		void coordsys_change( const QString &text );
		void coordinates_reset_event(bool);
		void coordinates_apply_event(bool);
		void category_change( int );
		// keeps text color in sync with line color (if they were the same before)
		void line_color_change(const QString & );
		void save_region(bool);

		void frame_min_change(int);
		void frame_max_change(int);

	    protected:
		// keep track of which set of statistics
		// where selected when refreshing all...
		int selected_statistics;
		typedef std::list<QtRegionStats*> freestat_list;
		static freestat_list *freestats;
		QString last_line_color;
		QtRegion *region_;

	};
    }
}

#endif
