#ifndef REGION_QTREGIONSTATS_H_
#define REGION_QTREGIONSTATS_H_

#include <QtGui/QGroupBox>
#include <casa/BasicSL/String.h>
#include <display/region/QtRegionStats.ui.h>

class QStackedWidget;

namespace casa {
    namespace viewer {

	class QtRegionStats : public QGroupBox, protected Ui::QtRegionStats {
	    Q_OBJECT
	    public:
		QtRegionStats( QWidget *parent=0 );
		~QtRegionStats( );

		void reset( );
#if OLDSTUFF
		void addstats( std::list<std::pair<String,String> > *stats );
#endif

		void updateStatistics( const String &s, std::list<std::pair<String,String> > &list );

		void setNext( QStackedWidget *, QtRegionStats * );

	    protected slots:
		void go_next( );

	    protected:
		typedef std::pair<QGroupBox*,QLineEdit*> stat_field;
		typedef std::list<stat_field> statfield_list;
		statfield_list fields;

		QStackedWidget *container_;
		QtRegionStats *next_;
	};
    }
}

#endif
